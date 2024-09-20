#include "hci_uart_lp.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"
#include "softcrc.h"
#include "qcc74x_irq.h"
#include "qcc74x_gpio.h"
#include "qcc74x_timer.h"
#include "hosal_dma.h"
#include "qcc74x_undefl_glb.h"
#include "qcc74x_undefl_uart.h"
#include "qcc74x_undefl_dma.h"


typedef struct {
    uint8_t *data;
    uint32_t len;
}uart_msg_t;


#define LOG(fmt, ...)           printf(fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)      printf("[INFO: %s: %d] "fmt, __FILENAME__, __LINE__, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)     printf("[ERROR: %s: %d] "fmt, __FILENAME__, __LINE__, ##__VA_ARGS__)

#define SHOW_TX_DATA            1
#define SHOW_RX_DATA            1

#define UART_RING_BUF_ENABLE    1
#define UART_RING_BUF_SIZE      2048

#define UART_EVENT_DMA_TX_DONE  (1 << 0)
#define UART_EVENT_TX_DONE      (1 << 1)
#define UART_EVENT_ACK          (1 << 2)

#define UART_ID                 0
#define UART_BAUDRATE           115200  // ~11.5 bytes per millisecond
#define UART_TX_QUEUE_DEPTH     1
#define UART_RX_BUF_SIZE        512
#define UART_PREAMBLE_PATTERN   0x55
#define UART_TX_INTERVAL_MS     2       // ~23 bytes @115200bps
#define UART_TX_TIMEOUT_MS      500     // ~5750 bytes (ok if great than max dma transfer size 4095) @115200bps
#define UART_WAKEUP_TIMEOUT_MS  2000
#define UART_ACK_TIMEOUT_MS     1000

#if UART_ID == 0
#define UART_BASE               UART0_BASE
#define UART_TX_DMA_REQ         DMA_REQ_UART0_TX
#define UART_RX_DMA_REQ         DMA_REQ_UART0_RX
#endif


#if UART_RING_BUF_ENABLE != 0
#include "ring_buffer.h"
static uint8_t uartBuf[UART_RING_BUF_SIZE];
static Ring_Buffer_Type uartRB;
#endif

static QueueHandle_t uart_tx_queue = NULL;
static EventGroupHandle_t uart_event_group = NULL;
static TaskHandle_t uart_task_handle = NULL;
static uint8_t uart_tx_pin;
static uint8_t uart_rx_pin;
static uint8_t uart_preamble[23];  // ~2ms @115200bps
static uint8_t uart_ack[] = {0x4F, 0x4B};
static uint32_t uart_last_tx_end = 0;
static uint8_t uart_rx_buf[2][UART_RX_BUF_SIZE];
static uint32_t uart_rx_buf_idx = 0;
static hosal_dma_chan_t uart_tx_dma_ch = -1;
static hosal_dma_chan_t uart_rx_dma_ch = -1;


static int uart_send(uint8_t *data, uint32_t len)
{
    taskENTER_CRITICAL();
    
    xEventGroupClearBits(uart_event_group, UART_EVENT_DMA_TX_DONE);
    
    DMA_Channel_Disable(DMA0_ID, uart_tx_dma_ch);
    DMA_Channel_Update_SrcMemcfg(DMA0_ID, uart_tx_dma_ch, (uint32_t)data, len);
    DMA_Channel_Enable(DMA0_ID, uart_tx_dma_ch);
    
    taskEXIT_CRITICAL();
    
    EventBits_t bits = xEventGroupWaitBits(uart_event_group, UART_EVENT_DMA_TX_DONE, pdTRUE, pdTRUE, UART_TX_TIMEOUT_MS);
    if(!(bits & UART_EVENT_DMA_TX_DONE)){
        return -1;
    }
    
    while(UART_GetTxBusBusyStatus(UART_ID));
    
    return 0;
}

static int uart_send_from_isr(uint8_t *data, uint32_t len)
{
    uint32_t start = qcc74x_timer_now_us();
    
    DMA_Channel_Disable(DMA0_ID, uart_tx_dma_ch);
    DMA_Channel_Update_SrcMemcfg(DMA0_ID, uart_tx_dma_ch, (uint32_t)data, len);
    DMA_Channel_Enable(DMA0_ID, uart_tx_dma_ch);
    
    while(DMA_Channel_Is_Busy(DMA0_ID, uart_tx_dma_ch)){
        if(qcc74x_timer_now_us() - start >= UART_TX_TIMEOUT_MS * 1000){
            return -1;
        }
    }
    
    while(UART_GetTxBusBusyStatus(UART_ID));
    
    return 0;
}

static int uart_send_preamble(void)
{
    return uart_send(uart_preamble, sizeof(uart_preamble));
}

static int uart_send_data(uint8_t *data, uint32_t len)
{
    // wait for remote uart rx timeout
    while(qcc74x_timer_now_us() - uart_last_tx_end < UART_TX_INTERVAL_MS * 1000);
    
    if(uart_send(data, len) != 0){
        return -1;
    }
    
    uart_last_tx_end = qcc74x_timer_now_us();
    
    return 0;
}

static int uart_send_ack(void)
{
    // wait for remote uart rx timeout
    while(qcc74x_timer_now_us() - uart_last_tx_end < UART_TX_INTERVAL_MS * 1000);
    
    if(uart_send_from_isr(uart_ack, sizeof(uart_ack)) != 0){
        return -1;
    }
    
    uart_last_tx_end = qcc74x_timer_now_us();
    
    return 0;
}


static void UART_RTO_Callback(void)
{
    uint8_t *data = uart_rx_buf[uart_rx_buf_idx];
    uint32_t len = UART_RX_BUF_SIZE - DMA_Channel_TranferSize(DMA0_ID, uart_rx_dma_ch);
    uint32_t crc32;
    
    UART_IntClear(UART_ID, UART_INT_RTO);
    
    uart_rx_buf_idx = !uart_rx_buf_idx;
    DMA_Channel_Disable(DMA0_ID, uart_rx_dma_ch);
    DMA_Channel_Update_DstMemcfg(DMA0_ID, uart_rx_dma_ch, (uint32_t)uart_rx_buf[uart_rx_buf_idx], UART_RX_BUF_SIZE);
    DMA_Channel_Enable(DMA0_ID, uart_rx_dma_ch);
    
#if SHOW_RX_DATA != 0
    LOG("[%s] uart rx %lu:", __FILENAME__, len);
    for(uint32_t i=0; i<len; i++){
        LOG(" %02x", data[i]);
    }
    LOG("\r\n");
#endif
    
    // check ack
    if(len == sizeof(uart_ack)){
        if(memcmp(data, uart_ack, sizeof(uart_ack)) == 0){
            EventBits_t bits = xEventGroupGetBitsFromISR(uart_event_group);
            if(bits & UART_EVENT_TX_DONE){
                xEventGroupClearBitsFromISR(uart_event_group, UART_EVENT_TX_DONE);
                
                BaseType_t xHigherPriorityTaskWoken = pdFALSE;
                xEventGroupSetBitsFromISR(uart_event_group, UART_EVENT_ACK, &xHigherPriorityTaskWoken);
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
            return;
        }
    }
    
    // check crc32
    if(len > sizeof(crc32)){
        crc32 = QCC74x_Soft_CRC32(data, len - sizeof(crc32));
        if(memcmp(&crc32, data + len - sizeof(crc32), sizeof(crc32)) == 0){
            // send ack
            uart_send_ack();
            
#if UART_RING_BUF_ENABLE != 0
            // put data to ring buffer
            Ring_Buffer_Write(&uartRB, data, len - sizeof(crc32));
#endif
            
            // report data
            hci_uart_rx_done_callback(data, len - sizeof(crc32));
        }
    }
}

static void UART_RX_DMA_Callback(void *p_arg, uint32_t flag)
{
    if(flag == HOSAL_DMA_INT_TRANS_COMPLETE){
        // empty
    }
}

static void UART_TX_DMA_Callback(void *p_arg, uint32_t flag)
{
    if(flag == HOSAL_DMA_INT_TRANS_COMPLETE){
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xEventGroupSetBitsFromISR(uart_event_group, UART_EVENT_DMA_TX_DONE, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}


static void gpio_init(void)
{
    GLB_GPIO_Type pinList[] = {uart_tx_pin, uart_rx_pin};
    GLB_GPIO_Func_Init(GPIO_FUN_UART, pinList, sizeof(pinList)/sizeof(pinList[0]));
    
    GLB_UART_Fun_Sel(uart_tx_pin % 4, GLB_UART_SIG_FUN_UART0_TXD + 4 * UART_ID);
    GLB_UART_Fun_Sel(uart_rx_pin % 4, GLB_UART_SIG_FUN_UART0_RXD + 4 * UART_ID);
}

static void uart_init(void)
{
    UART_CFG_Type uartCfg = {
        0,                         /* UART clock */
        UART_BAUDRATE,             /* baudrate */
        UART_DATABITS_8,           /* data bits */
        UART_STOPBITS_1,           /* stop bits */
        UART_PARITY_NONE,          /* parity */
        DISABLE,                   /* Disable auto flow control */
        DISABLE,                   /* Disable rx input de-glitch function */
        DISABLE,                   /* Disable RTS output SW control mode */
        DISABLE,                   /* Disable tx output SW control mode */
        DISABLE,                   /* Disable tx lin mode */
        DISABLE,                   /* Disable rx lin mode */
        0,                         /* Tx break bit count for lin mode */
        UART_LSB_FIRST,            /* UART each data byte is send out LSB-first */
    };
    
    UART_FifoCfg_Type fifoCfg = {
        0,                         /* TX FIFO threshold */
        0,                         /* RX FIFO threshold */
        ENABLE,                    /* Enable tx dma req/ack interface */
        ENABLE,                    /* Enable rx dma req/ack interface */
    };
    
    GLB_Set_UART_CLK(1, HBN_UART_CLK_XCLK, 0);
    uartCfg.uartClk = 32000000;
    
    UART_DeInit(UART_ID);
    UART_Init(UART_ID, &uartCfg);
    UART_FifoConfig(UART_ID, &fifoCfg);
    UART_TxFreeRun(UART_ID, ENABLE);
    UART_SetRxTimeoutValue(UART_ID, 80);
    UART_IntMask(UART_ID, UART_INT_ALL, MASK);
    UART_IntMask(UART_ID, UART_INT_RTO, UNMASK);
    //UART_Enable(UART_ID, UART_TXRX);
    
    qcc74x_irq_register(UART0_IRQn + UART_ID, UART_RTO_Callback);
    qcc74x_irq_enable(UART0_IRQn + UART_ID);
}

static void dma_rx_init(void)
{
    DMA_Channel_Cfg_Type dmaRxCfg = {
        UART_BASE + 0x8C,          /* Source address of DMA transfer */
        0,                         /* Destination address of DMA transfer */
        0,                         /* Transfer length, 0~4095, this is burst count */
        DMA_TRNS_P2M,              /* Transfer dir control. 0: Memory to Memory, 1: Memory to peripheral, 2: Peripheral to memory */
        uart_rx_dma_ch,            /* Channel select 0-3 */
        DMA_TRNS_WIDTH_8BITS,      /* Transfer width. 0: 8  bits, 1: 16  bits, 2: 32  bits */
        DMA_TRNS_WIDTH_8BITS,      /* Transfer width. 0: 8  bits, 1: 16  bits, 2: 32  bits */
        DMA_BURST_SIZE_1,          /* Number of data items for burst transaction length. Each item width is as same as tansfer width. 0: 1 item, 1: 4 items, 2: 8 items, 3: 16 items */
        DMA_BURST_SIZE_1,          /* Number of data items for burst transaction length. Each item width is as same as tansfer width. 0: 1 item, 1: 4 items, 2: 8 items, 3: 16 items */
        DISABLE,
        DISABLE,
        0,
        DMA_PINC_DISABLE,          /* Source address increment. 0: No change, 1: Increment */
        DMA_MINC_ENABLE ,          /* Destination address increment. 0: No change, 1: Increment */
        UART_RX_DMA_REQ,           /* Source peripheral select */
        DMA_REQ_NONE,              /* Destination peripheral select */
    };
    
    DMA_Channel_Init(DMA0_ID, &dmaRxCfg);
}

static void dma_tx_init(void)
{
    DMA_Channel_Cfg_Type dmaTxCfg = {
        0,                         /* Source address of DMA transfer */
        UART_BASE + 0x88,          /* Destination address of DMA transfer */
        0,                         /* Transfer length, 0~4095, this is burst count */
        DMA_TRNS_M2P,              /* Transfer dir control. 0: Memory to Memory, 1: Memory to peripheral, 2: Peripheral to memory */
        uart_tx_dma_ch,            /* Channel select 0-3 */
        DMA_TRNS_WIDTH_8BITS,      /* Transfer width. 0: 8  bits, 1: 16  bits, 2: 32  bits */
        DMA_TRNS_WIDTH_8BITS,      /* Transfer width. 0: 8  bits, 1: 16  bits, 2: 32  bits */
        DMA_BURST_SIZE_1,          /* Number of data items for burst transaction length. Each item width is as same as tansfer width. 0: 1 item, 1: 4 items, 2: 8 items, 3: 16 items */
        DMA_BURST_SIZE_1,          /* Number of data items for burst transaction length. Each item width is as same as tansfer width. 0: 1 item, 1: 4 items, 2: 8 items, 3: 16 items */
        DISABLE,
        DISABLE,
        0,
        DMA_MINC_ENABLE,           /* Source address increment. 0: No change, 1: Increment */
        DMA_PINC_DISABLE,          /* Destination address increment. 0: No change, 1: Increment */
        DMA_REQ_NONE,              /* Source peripheral select */
        UART_TX_DMA_REQ,           /* Destination peripheral select */
    };
    
    DMA_Channel_Init(DMA0_ID, &dmaTxCfg);
}

static void dma_init(void)
{
    dma_rx_init();
    dma_tx_init();
}

static void uart_enable(void)
{
    uart_rx_buf_idx = 0;
    DMA_Channel_Disable(DMA0_ID, uart_rx_dma_ch);
    DMA_Channel_Update_DstMemcfg(DMA0_ID, uart_rx_dma_ch, (uint32_t)uart_rx_buf[uart_rx_buf_idx], UART_RX_BUF_SIZE);
    DMA_Channel_Enable(DMA0_ID, uart_rx_dma_ch);
    
    UART_Enable(UART_ID, UART_TXRX);
}


static void enable_uart_tx_pin_func(void)
{
    GLB_GPIO_Type pin = uart_tx_pin;
    GLB_GPIO_Func_Init(GPIO_FUN_UART, &pin, 1);
}

static void disable_uart_tx_pin_func(void)
{
    qcc74x_gpio_enable_input(uart_tx_pin, 0, 0);
    arch_delay_us(10);
}

static uint32_t get_uart_tx_pin_level(void)
{
    GLB_GPIO_Type pin = uart_tx_pin;
    return GLB_GPIO_Read(pin);
}

static bool is_remote_sleep(void)
{
    uint32_t level;
    
    disable_uart_tx_pin_func();
    
    level = get_uart_tx_pin_level();
    
    enable_uart_tx_pin_func();
    
    return level == 0;
}

static int uart_write_with_wakeup(uint8_t *data, uint32_t len)
{
    bool sleep;
    uint32_t start;
    
    sleep = is_remote_sleep();
    LOG_INFO("remote %s\r\n", sleep ? "sleep" : "active");
    
    if(sleep){
        start = qcc74x_timer_now_us();
        
        do{
            if(qcc74x_timer_now_us() - start >= UART_WAKEUP_TIMEOUT_MS * 1000){
                LOG_ERROR("remote wakeup timeout\r\n");
                return HCI_UART_WAKEUP_TIMEOUT;
            }
            
            if(uart_send_preamble() != 0){
                LOG_ERROR("remote wakeup fault\r\n");
                return HCI_UART_WAKEUP_FAULT;
            }
        }while(is_remote_sleep());
    }
    
#if SHOW_TX_DATA != 0
    LOG("[%s] uart tx %lu:", __FILENAME__, len);
    for(uint32_t i=0; i<len; i++){
        LOG(" %02x", data[i]);
    }
    LOG("\r\n");
#endif
    
    // wait for remote uart rx timeout
    if(sleep){
        arch_delay_ms(UART_TX_INTERVAL_MS);
    }
    
    if(uart_send_data(data, len) != 0){
        LOG_ERROR("uart tx fault\r\n");
        return HCI_UART_TX_FAULT;
    }
    
    return HCI_UART_OK;
}


static void hci_uart_task(void *pvParameters)
{
    uart_msg_t msg;
    
    memset(uart_preamble, UART_PREAMBLE_PATTERN, sizeof(uart_preamble));
    
    while(1){
        xQueuePeek(uart_tx_queue, &msg, portMAX_DELAY);
        
        while(1){
            if(uart_write_with_wakeup(msg.data, msg.len) == HCI_UART_OK){
                xEventGroupSetBits(uart_event_group, UART_EVENT_TX_DONE);
                EventBits_t bits = xEventGroupWaitBits(uart_event_group, UART_EVENT_ACK, pdTRUE, pdTRUE, UART_ACK_TIMEOUT_MS);
                if(bits & UART_EVENT_ACK){
                    // report ack
                    hci_uart_ack_received_callback();
                    
                    // pop
                    xQueueReceive(uart_tx_queue, &msg, 0);
                    free(msg.data);
                    break;
                }
                LOG_INFO("no ack\r\n");
            }
            LOG_INFO("retry\r\n");
        }
    }
}


int hci_uart_init(uint8_t tx_pin, uint8_t rx_pin)
{
    hosal_dma_init();
    
    uart_rx_dma_ch = hosal_dma_chan_request(0);
    if(uart_rx_dma_ch == -1){
        return HCI_UART_INIT_FAULT;
    }
    
    uart_tx_dma_ch = hosal_dma_chan_request(0);
    if(uart_tx_dma_ch == -1){
        hosal_dma_chan_release(uart_rx_dma_ch);
        uart_rx_dma_ch = -1;
        return HCI_UART_INIT_FAULT;
    }
    
    hosal_dma_irq_callback_set(uart_rx_dma_ch, UART_RX_DMA_Callback, NULL);
    hosal_dma_irq_callback_set(uart_tx_dma_ch, UART_TX_DMA_Callback, NULL);
    
    uart_tx_pin = tx_pin;
    uart_rx_pin = rx_pin;
    
    gpio_init();
    uart_init();
    dma_init();
    
#if UART_RING_BUF_ENABLE != 0
    Ring_Buffer_Init(&uartRB, uartBuf, sizeof(uartBuf), NULL, NULL);
#endif
    
    if(!uart_tx_queue){
        uart_tx_queue = xQueueCreate(UART_TX_QUEUE_DEPTH, sizeof(uart_msg_t));
    }
    
    if(!uart_event_group){
        uart_event_group = xEventGroupCreate();
    }
    
    if(!uart_task_handle){
        xTaskCreate(hci_uart_task, "hci_uart", 256, NULL, configMAX_PRIORITIES - 1, &uart_task_handle);
    }
    
    uart_enable();
    
    return HCI_UART_OK;
}

int hci_uart_write(uint8_t *data, uint32_t len)
{
    uint32_t crc32;
    uart_msg_t msg;
    
    if(uxQueueSpacesAvailable(uart_tx_queue) == 0){
        LOG_ERROR("uart tx queue full\r\n");
        return HCI_UART_TX_QUEUE_FULL;
    }
    
    crc32 = QCC74x_Soft_CRC32(data, len);
    
    msg.data = malloc(len + sizeof(crc32));
    memcpy(msg.data, data, len);
    memcpy(msg.data + len, &crc32, sizeof(crc32));
    msg.len = len + sizeof(crc32);
    
    if(!xPortIsInsideInterrupt()){
        xQueueSend(uart_tx_queue, &msg, 0);
    }else{
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xQueueSendFromISR(uart_tx_queue, &msg, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    
    return HCI_UART_OK;
}

#if UART_RING_BUF_ENABLE != 0
uint32_t hci_uart_get_rx_count(void)
{
    return Ring_Buffer_Get_Length(&uartRB);
}

uint32_t hci_uart_read(uint8_t *data, uint32_t len)
{
    uint32_t cnt;
    
    cnt = Ring_Buffer_Get_Length(&uartRB);
    if(cnt < len){
        len = cnt;
    }
    
    Ring_Buffer_Read(&uartRB, data, len);
    return len;
}
#endif

bool hci_uart_is_busy(void)
{
    // tx busy
    if(uxQueueSpacesAvailable(uart_tx_queue) != UART_TX_QUEUE_DEPTH){
        return true;
    }
    
    // rx busy
    if(DMA_LLI_Get_Dstaddr(DMA0_ID, uart_rx_dma_ch) != (uint32_t)uart_rx_buf[uart_rx_buf_idx]){
        return true;
    }
    
    // ring buffer not empty
#if UART_RING_BUF_ENABLE != 0
    if(Ring_Buffer_Get_Length(&uartRB) != 0){
        return true;
    }
#endif
    
    return false;
}

__attribute__((weak))
void hci_uart_ack_received_callback(void)
{
    LOG_INFO("ack received\r\n");
}

__attribute__((weak))
void hci_uart_rx_done_callback(uint8_t *data, uint32_t len)
{
    LOG_INFO("rx done\r\n");
}
