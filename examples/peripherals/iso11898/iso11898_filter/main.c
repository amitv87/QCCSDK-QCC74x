#include "qcc74x_mtimer.h"
#include "qcc74x_iso11898.h"
#include "qcc74x_iso11898_glb.h"
#include "qcc74x_gpio.h"
#include "board.h"

struct qcc74x_device_s *iso11898;

struct qcc74x_iso11898_config_s cfg = {
    .prescaler = 1,                   /*!< specifies the length of a time quantum, ranges from 1 to 63, tq=tclk*2*(prescaler+1) */
    .mode = ISO11898_MODE_NORMAL,     /*!< specifies the CAN operating mode. @ref ISO11898_MODE_XXX */
    .sample = ISO11898_SAMPLE_TRIPLE, /*!< specifies the number of sample times. @ref ISO11898_SAMPLE_XXX */
    .sjw = ISO11898_SJW_2TQ,          /*!< specifies the maximum number of time quanta the ISO11898 hardware is allowed to lengthen or shorten a bit to perform resynchronization. @ref ISO11898_SJW_XXX */
    .bs1 = ISO11898_BS1_5TQ,          /*!< specifies the number of time quanta in Bit Segment 1. @ref ISO11898_BS1_XXX */
    .bs2 = ISO11898_BS2_4TQ,          /*!< specifies the number of time quanta in Bit Segment 2. @ref ISO11898_BS2_XXX */
};

struct qcc74x_iso11898_filter_s filter = {
    .filter_type = ISO11898_FILTER_TYPE_SINGLE,
    .code0 = 0,
    .code1 = 0,
    .code2 = 0,
    .code3 = 0x30,
    .mask0 = 0xFF,
    .mask1 = 0xFF,
    .mask2 = 0xFF,
    .mask3 = 0xFF,
};

struct qcc74x_iso11898_msg_s msg_tx = {
    .std_id = 0x59,                                             /*!< specifies the standard identifier, this parameter can be a value between 0 to 0x7FF */
    .ext_id = 0x154863,                                         /*!< specifies the extended identifier, this parameter can be a value between 0 to 0x1FFFFFFF */
    .id_type = ISO11898_ID_EXTENDED,                            /*!< specifies the type of identifier for the message that will be send or received. @ref ISO11898_ID_XXX */
    .rtr = ISO11898_RTR_DATA,                                   /*!< specifies the type of frame for the message that will be send or received, @ref ISO11898_RTR_XXX */
    .dlc = 8,                                                   /*!< specifies the length of the frame that will be send or received, this parameter can be a value between 0 to 8 */
    .data = { 0x2E, 0x53, 0xF6, 0x81, 0xD4, 0xC0, 0x7A, 0xB9 }, /*!< contains the data to be send or received, it ranges from 0 to 0xFF */
};

void iso11898_isr(int irq, void *arg)
{
    uint32_t int_status = qcc74x_iso11898_get_int_status(iso11898);

    if (int_status & ISO11898_INTSTS_RX) {
        struct qcc74x_iso11898_msg_s msg;
        qcc74x_iso11898_recv(iso11898, &msg, 100);
        printf("Receive \r\n");
        if (msg.id_type == ISO11898_ID_STANDARD) {
            printf("standard frame, ID = 0x%03x \r\n", msg.std_id);
        } else {
            printf("extend frame, ID = 0x%08x \r\n", msg.ext_id);
        }
        if (msg.rtr == ISO11898_RTR_DATA) {
            printf("data frame[%d]\r\n", msg.dlc);
            for (uint32_t i = 0; i < msg.dlc; i++) {
                printf("%02X ", msg.data[i]);
            }
            printf("\r\n");
        } else {
            printf("remote frame\r\n");
        }
    }
    if (int_status & ISO11898_INTSTS_TX) {
        printf("tx succeed!\r\n");
        // qcc74x_iso11898_send(iso11898, &msg_tx, 1000);
    }
    if (int_status & ISO11898_INTSTS_ARBITRATION_LOST) {
        printf("arbitration failed!\n");
    }
    if (int_status & ISO11898_INTSTS_BUS_ERROR) {
        printf("bus error!\n");
    }
    if (int_status & ISO11898_INTSTS_ERROR_PASSIVE) {
        printf("passive error!\n");
    }
    if (int_status & ISO11898_INTSTS_DATA_OVERRUN) {
        printf("data overrun!\n");
    }
    if (int_status & ISO11898_INTSTS_ERROR_WARNING) {
        printf("warning error!\n");
    }
}

void board_iso11898_gpio_init()
{
    struct qcc74x_device_s *gpio;

    gpio = qcc74x_device_get_by_name("gpio");
    qcc74x_gpio_iso11898_init(gpio, GPIO_PIN_16, GPIO_ISO11898_FUNC_TX);
    qcc74x_gpio_iso11898_init(gpio, GPIO_PIN_17, GPIO_ISO11898_FUNC_RX);
}

int main(void)
{
    board_init();
    qcc74x_iso11898_clock_enable(1);
    board_iso11898_gpio_init();

    iso11898 = qcc74x_device_get_iso11898("iso11898");

    qcc74x_iso11898_init(iso11898, &cfg);
    qcc74x_iso11898_set_filter(iso11898, &filter);
    qcc74x_irq_attach(iso11898->irq_num, iso11898_isr, NULL);
    qcc74x_iso11898_txint_mask(iso11898, false);
    qcc74x_iso11898_rxint_mask(iso11898, false);
    qcc74x_iso11898_errint_mask(iso11898, false);
    qcc74x_irq_enable(iso11898->irq_num);
    qcc74x_iso11898_send(iso11898, &msg_tx, 1000);

    return 0;
}
