#include "qcc74x_mtimer.h"
#include "qcc74x_uart.h"
#include "board.h"

struct qcc74x_device_s *uartx;

static uint8_t uart_txbuf[128] = { 0 };
static uint8_t uart_rxbuf[128] = { 0 };

void uart_isr(int irq, void *arg)
{
    uint32_t intstatus = qcc74x_uart_get_intstatus(uartx);

    if (intstatus & UART_INTSTS_RX_LSE) {
        qcc74x_uart_int_clear(uartx, UART_INTCLR_RX_LSE);
        printf("enter rx lse interrupt");
    }
}

int main(void)
{
    board_init();
    board_uartx_gpio_init();

    uartx = qcc74x_device_get_by_name(DEFAULT_TEST_UART);

    for (uint8_t i = 0; i < 128; i++) {
        uart_txbuf[i] = i;
        uart_rxbuf[i] = 0;
    }

    struct qcc74x_uart_config_s cfg;

    cfg.baudrate = 2000000;
    cfg.data_bits = UART_DATA_BITS_8;
    cfg.stop_bits = UART_STOP_BITS_1;
    cfg.parity = UART_PARITY_NONE;
    cfg.flow_ctrl = 0;
    cfg.tx_fifo_threshold = 7;
    cfg.rx_fifo_threshold = 7;
    cfg.bit_order = UART_LSB_FIRST;

    /* lin config must enable before uart enable */
    qcc74x_uart_feature_control(uartx, UART_CMD_SET_BREAK_VALUE, 4);
    qcc74x_uart_feature_control(uartx, UART_CMD_SET_TX_LIN_VALUE, 1);
    qcc74x_uart_feature_control(uartx, UART_CMD_SET_RX_LIN_VALUE, 1);

    qcc74x_uart_init(uartx, &cfg);

    qcc74x_uart_errint_mask(uartx, false);
    qcc74x_irq_attach(uartx->irq_num, uart_isr, NULL);
    qcc74x_irq_enable(uartx->irq_num);

    for (uint8_t i = 0; i < 128; i++) {
        qcc74x_uart_putchar(uartx, uart_txbuf[i]);
        while (qcc74x_uart_feature_control(uartx, UART_CMD_GET_RX_FIFO_CNT, 1) == 0) {
        }
        uart_rxbuf[i] = qcc74x_uart_getchar(uartx);
    }
    printf("All data arrived\r\n");

    for (uint8_t j = 0; j < 128; j++) {
        if (uart_txbuf[j] != uart_rxbuf[j]) {
            printf("check fail, %d tx: %02x, rx: %02x\r\n", j, uart_txbuf[j], uart_rxbuf[j]);
        }
    }

    while (1) {
        qcc74x_mtimer_delay_ms(2000);
    }
}
