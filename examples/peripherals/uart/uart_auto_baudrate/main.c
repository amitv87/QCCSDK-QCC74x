#include "qcc74x_mtimer.h"
#include "qcc74x_uart.h"
#include "qcc74x_clock.h"
#include "board.h"

struct qcc74x_device_s *uartx;

void uart_isr(int irq, void *arg)
{
    uint32_t intstatus = qcc74x_uart_get_intstatus(uartx);
    int ret;
    uint32_t baudrate;

    if (intstatus & UART_INTSTS_RX_AD5) {
        qcc74x_uart_int_clear(uartx, UART_INTCLR_RX_AD5);
        ret = qcc74x_uart_feature_control(uartx, UART_CMD_GET_AUTO_BAUD, UART_AUTO_BAUD_0X55);
        baudrate = qcc74x_clk_get_peripheral_clock(QCC74x_DEVICE_TYPE_UART, uartx->idx) / (ret + 1);
        printf("Detected baudrate by 0x55 is %d\r\n", baudrate);
    }
  
    if (intstatus & UART_INTSTS_RX_ADS) {
        qcc74x_uart_int_clear(uartx, UART_INTCLR_RX_ADS);
        ret = qcc74x_uart_feature_control(uartx, UART_CMD_GET_AUTO_BAUD, UART_AUTO_BAUD_START);
        baudrate = qcc74x_clk_get_peripheral_clock(QCC74x_DEVICE_TYPE_UART, uartx->idx) / (ret + 1);
        printf("Detected baudrate by startbit is %d\r\n", baudrate);
    }
}

int main(void)
{
    board_init();
    board_uartx_gpio_init();

    uartx = qcc74x_device_get_by_name(DEFAULT_TEST_UART);

    struct qcc74x_uart_config_s cfg;

    cfg.baudrate = 2000000;
    cfg.data_bits = UART_DATA_BITS_8;
    cfg.stop_bits = UART_STOP_BITS_1;
    cfg.parity = UART_PARITY_NONE;
    cfg.flow_ctrl = 0;
    cfg.tx_fifo_threshold = 7;
    cfg.rx_fifo_threshold = 7;
    cfg.bit_order = UART_LSB_FIRST;
    qcc74x_uart_init(uartx, &cfg);

    qcc74x_uart_feature_control(uartx, UART_CMD_SET_AUTO_BAUD, 1);
    qcc74x_uart_feature_control(uartx, UART_CMD_SET_ABR_ALLOWABLE_ERROR, 7);

    qcc74x_irq_attach(uartx->irq_num, uart_isr, NULL);
    qcc74x_irq_enable(uartx->irq_num);

    while (1) {
    }
}
