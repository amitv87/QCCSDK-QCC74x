#include "qcc74x_ir.h"
#include "board.h"

#if defined(QCC74x_undef) || defined(QCC74x_undef) || defined(QCC74x_undef) || defined(QCC74x_undefL)
#define TEST_IR_TX
#endif

#if defined(QCC74x_undef) || defined(QCC74x_undef) || defined(QCC74x_undef) || defined(QCC743)
#define TEST_IR_RX
#endif

#ifdef TEST_IR_TX
struct qcc74x_device_s *irtx;
#endif
#ifdef IR_RX_NEC
struct qcc74x_device_s *irrx;
#endif

/* main */
int main(void)
{
    board_init();

    printf("IR RC-5 case:\r\n");

    board_ir_gpio_init();

#ifdef TEST_IR_TX
    uint32_t tx_buffer[1] = { 0x123D };
    struct qcc74x_ir_tx_config_s tx_cfg;

    irtx = qcc74x_device_get_by_name("irtx");

    /* TX init */
    tx_cfg.tx_mode = IR_TX_RC5;
    qcc74x_ir_tx_init(irtx, &tx_cfg);
#endif

#ifdef TEST_IR_RX
    uint64_t rx_data;
    uint8_t rx_len;
    struct qcc74x_ir_rx_config_s rx_cfg;

    irrx = qcc74x_device_get_by_name("irrx");

    /* RX init */
    rx_cfg.rx_mode = IR_RX_RC5;
    rx_cfg.input_inverse = true;
    rx_cfg.deglitch_enable = false;
    qcc74x_ir_rx_init(irrx, &rx_cfg);

    /* Enable rx, wait for sending */
    qcc74x_ir_rx_enable(irrx, true);
#endif

#ifdef TEST_IR_TX
    /* Send */
    qcc74x_ir_send(irtx, tx_buffer, 1);
    printf("Send 0x%08lx\r\n", tx_buffer[0]);
#endif

#ifdef TEST_IR_RX
    /* Receive */
    rx_len = qcc74x_ir_receive(irrx, &rx_data);

#ifdef TEST_IR_TX
    /* Check data received */
    if (rx_data != tx_buffer[0]) {
        printf("Data error! receive bit: %d, value: 0x%016lx\r\n", rx_len, rx_data);
    } else {
        printf("Received correctly. receive bit: %d, value: 0x%016lx\r\n", rx_len, rx_data);
    }
#else
    printf("Receive bit: %d, value: 0x%016lx\r\n", rx_len, rx_data);
#endif
#endif

    printf("end\r\n");

    while (1) {
    }
}
