#include "qcc74x_mtimer.h"
#include "qcc74x_uart.h"
#include "board.h"

struct qcc74x_device_s *uartx;
static uint8_t uart_txbuf[128] = { 0 };

int main(void)
{
    board_init();
    board_uartx_gpio_init();

    for (uint8_t i = 0; i < 128; i++) {
        uart_txbuf[i] = i;
    }

    uartx = qcc74x_device_get_by_name(DEFAULT_TEST_UART);

    struct qcc74x_uart_config_s cfg;

    cfg.baudrate = 115200;
    cfg.data_bits = UART_DATA_BITS_8;
    cfg.stop_bits = UART_STOP_BITS_1;
    cfg.parity = UART_PARITY_NONE;
    cfg.flow_ctrl = 0;
    cfg.tx_fifo_threshold = 7;
    cfg.rx_fifo_threshold = 7;
    cfg.bit_order = UART_LSB_FIRST;
    qcc74x_uart_init(uartx, &cfg);

    qcc74x_uart_feature_control(uartx, UART_CMD_SET_CTS_EN, true);

    while (1) {
        for (uint8_t i = 0; i < 128; i++) {
            qcc74x_uart_putchar(uartx, uart_txbuf[i]);
        }
    }
}