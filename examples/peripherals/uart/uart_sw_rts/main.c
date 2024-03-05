#include "qcc74x_mtimer.h"
#include "qcc74x_uart.h"
#include "board.h"

struct qcc74x_device_s *uartx;

void uart_isr(int irq, void *arg)
{
    uint32_t intstatus = qcc74x_uart_get_intstatus(uartx);

    if (intstatus & UART_INTSTS_RX_FIFO) {
        printf("rx fifo\r\n");
        while (qcc74x_uart_rxavailable(uartx)) {
            printf("0x%02x\r\n", qcc74x_uart_getchar(uartx));
        }
        qcc74x_uart_feature_control(uartx, UART_CMD_SET_RTS_VALUE, 1);
    }
    if (intstatus & UART_INTSTS_RTO) {
        qcc74x_uart_int_clear(uartx, UART_INTCLR_RTO);
        printf("rto\r\n");
        while (qcc74x_uart_rxavailable(uartx)) {
            printf("0x%02x\r\n", qcc74x_uart_getchar(uartx));
        }
    }
}

int main(void)
{
    board_init();
    board_uartx_gpio_init();

    uartx = qcc74x_device_get_by_name(DEFAULT_TEST_UART);

    struct qcc74x_uart_config_s cfg;

    cfg.baudrate = 256000;
    cfg.data_bits = UART_DATA_BITS_8;
    cfg.stop_bits = UART_STOP_BITS_1;
    cfg.parity = UART_PARITY_NONE;
    cfg.flow_ctrl = 0;
    cfg.tx_fifo_threshold = 7;
    cfg.rx_fifo_threshold = 7;
    cfg.bit_order = UART_LSB_FIRST;
    qcc74x_uart_init(uartx, &cfg);

    qcc74x_uart_rxint_mask(uartx, false);
    qcc74x_irq_attach(uartx->irq_num, uart_isr, NULL);
    qcc74x_irq_enable(uartx->irq_num);

    qcc74x_uart_feature_control(uartx, UART_CMD_SET_SW_RTS_CONTROL, true);
    qcc74x_uart_feature_control(uartx, UART_CMD_SET_RTS_VALUE, 0);
    while (1) {
        qcc74x_mtimer_delay_ms(2000);
    }
}
