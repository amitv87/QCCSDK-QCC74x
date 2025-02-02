#include "qcc74x_mtimer.h"
#include "qcc74x_spi.h"
#include "board.h"

#define SPI_MASTER_CASE 0
#define SPI_SLAVE_CASE  1

#define SPI_CASE_SELECT SPI_MASTER_CASE

#define BUFF_LEN (8 * 1024)

uint32_t tx_buff[BUFF_LEN / 4];
uint32_t rx_buff[BUFF_LEN / 4];

struct qcc74x_device_s *spi0;

/* poll test func */
int qcc74x_spi_poll_test(uint32_t data_width)
{
    uint32_t data_mask;
    uint32_t *p_tx = (uint32_t *)tx_buff;
    uint32_t *p_rx = (uint32_t *)rx_buff;

    switch (data_width) {
        case SPI_DATA_WIDTH_8BIT:
            data_mask = 0x000000FF;
            break;
        case SPI_DATA_WIDTH_16BIT:
            data_mask = 0x0000FFFF;
            break;
        case SPI_DATA_WIDTH_24BIT:
            data_mask = 0x00FFFFFF;
            break;
        case SPI_DATA_WIDTH_32BIT:
            data_mask = 0xFFFFFFFF;
            break;
        default:
            printf("data_width err\r\n");
            return -1;
            break;
    }

    /* data init */
    for (uint16_t i = 0; i < BUFF_LEN / 4; i++) {
        p_tx[i] = i;
        p_rx[i] = 0;
    }

    /* set data width */
    qcc74x_spi_feature_control(spi0, SPI_CMD_SET_DATA_WIDTH, data_width);

    /* send data */
    for (uint16_t i = 0; i < BUFF_LEN / 4; i++) {
        p_rx[i] = qcc74x_spi_poll_send(spi0, p_tx[i]);
#if (SPI_CASE_SELECT == SPI_MASTER_CASE)
        qcc74x_mtimer_delay_us(10); /* delay for slave device prepare ok */
#endif
    }

    /* check data */
    for (uint16_t i = 0; i < BUFF_LEN / 4; i++) {
        if (p_rx[i] != (p_tx[i] & data_mask)) {
            printf("data error, data[%d]:tx 0x%08lX, rx 0x%08lX\r\n", i, p_tx[i], p_rx[i]);
            return -1;
        }
    }
    printf("data check success\r\n");

    return 0;
}

/* poll_exchange test func */
int qcc74x_spi_poll_exchange_test(uint32_t data_width)
{
    void *p_tx = (uint32_t *)tx_buff;
    void *p_rx = (uint32_t *)rx_buff;

    /* data init */
    switch (data_width) {
        case SPI_DATA_WIDTH_8BIT:
            for (uint16_t i = 0; i < BUFF_LEN; i++) {
                ((uint8_t *)p_tx)[i] = i;
                ((uint8_t *)p_rx)[i] = 0;
            }
            break;
        case SPI_DATA_WIDTH_16BIT:
            for (uint16_t i = 0; i < BUFF_LEN / 2; i++) {
                ((uint16_t *)p_tx)[i] = i << 0;
                ((uint16_t *)p_rx)[i] = 0;
            }
            break;
        case SPI_DATA_WIDTH_24BIT:
            for (uint16_t i = 0; i < BUFF_LEN / 4; i++) {
                ((uint32_t *)p_tx)[i] = ((i << 0) | i) & 0x00FFFFFF;
                ((uint32_t *)p_rx)[i] = 0;
            }
            break;
        case SPI_DATA_WIDTH_32BIT:
            for (uint16_t i = 0; i < BUFF_LEN / 4; i++) {
                ((uint32_t *)p_tx)[i] = (i << 0) | i;
                ((uint32_t *)p_rx)[i] = 0;
            }
            break;
        default:
            return -1;
            break;
    }

    /* set data width */
    qcc74x_spi_feature_control(spi0, SPI_CMD_SET_DATA_WIDTH, data_width);

    /* send data */
    printf("spi poll exchange width %ld, len %d\r\n", data_width, BUFF_LEN);
    qcc74x_spi_poll_exchange(spi0, p_tx, p_rx, BUFF_LEN);

    /* check data */
    for (uint16_t i = 0; i < BUFF_LEN / 4; i++) {
        if (((uint32_t *)p_rx)[i] != ((uint32_t *)p_tx)[i]) {
            printf("data error, data[%d]:tx 0x%08lX, rx 0x%08lX\r\n", i, ((uint32_t *)p_tx)[i], ((uint32_t *)p_rx)[i]);
            return -1;
        }
    }
    printf("data check success\r\n");

    return 0;
}

volatile uint32_t spi_tc_done_count = 0;

void spi_isr(int irq, void *arg)
{
    uint32_t intstatus = qcc74x_spi_get_intstatus(spi0);
    if (intstatus & SPI_INTSTS_TC) {
        qcc74x_spi_int_clear(spi0, SPI_INTCLR_TC);
        //printf("tc done\r\n");
        spi_tc_done_count++;
    }
    if (intstatus & SPI_INTSTS_TX_FIFO) {
        //printf("tx fifo\r\n");
    }
    if (intstatus & SPI_INTSTS_RX_FIFO) {
        //printf("rx fifo\r\n");
    }
}

int main(void)
{
    board_init();
    board_spi0_gpio_init();

    struct qcc74x_spi_config_s spi_cfg = {
#if (SPI_CASE_SELECT == SPI_MASTER_CASE)
        .freq = 1 * 1000 * 1000,
        .role = SPI_ROLE_MASTER,
#else
        .freq = 32 * 1000 * 1000,
        .role = SPI_ROLE_SLAVE,
#endif
        .mode = SPI_MODE3,
        .data_width = SPI_DATA_WIDTH_8BIT,
        .bit_order = SPI_BIT_MSB,
        .byte_order = SPI_BYTE_LSB,
        .tx_fifo_threshold = 0,
        .rx_fifo_threshold = 0,
    };

    spi0 = qcc74x_device_get_by_name("spi0");
    qcc74x_spi_init(spi0, &spi_cfg);

    // qcc74x_spi_txint_mask(spi0, false);
    // qcc74x_spi_rxint_mask(spi0, false);
    qcc74x_spi_tcint_mask(spi0, false);
    qcc74x_irq_attach(spi0->irq_num, spi_isr, NULL);
    qcc74x_irq_enable(spi0->irq_num);

    qcc74x_spi_feature_control(spi0, SPI_CMD_SET_CS_INTERVAL, 0);

    printf("\r\n************** spi poll send 8-bit test **************\r\n");
    if (qcc74x_spi_poll_test(SPI_DATA_WIDTH_8BIT) < 0) {
        printf("poll send 8-bit test error!!!\r\n");
    } else {
        printf("poll send 8-bit test success!\r\n");
    }
#if (SPI_CASE_SELECT == SPI_MASTER_CASE)
    qcc74x_mtimer_delay_ms(1000); /* delay for slave device prepare ok */
#endif
    printf("\r\n************** spi poll send 16-bit test **************\r\n");
    if (qcc74x_spi_poll_test(SPI_DATA_WIDTH_16BIT) < 0) {
        printf("poll send 16-bit test error!!!\r\n");
    } else {
        printf("poll send 16-bit test success!\r\n");
    }
#if (SPI_CASE_SELECT == SPI_MASTER_CASE)
    qcc74x_mtimer_delay_ms(1000); /* delay for slave device prepare ok */
#endif
    printf("\r\n************** spi poll send 24-bit test **************\r\n");
    if (qcc74x_spi_poll_test(SPI_DATA_WIDTH_24BIT) < 0) {
        printf("poll send 24-bit test error!!!\r\n");
    } else {
        printf("poll send 24-bit test success!\r\n");
    }
#if (SPI_CASE_SELECT == SPI_MASTER_CASE)
    qcc74x_mtimer_delay_ms(1000); /* delay for slave device prepare ok */
#endif
    printf("\r\n************** spi poll send 32-bit test **************\r\n");
    if (qcc74x_spi_poll_test(SPI_DATA_WIDTH_32BIT) < 0) {
        printf("poll send 32-bit test error!!!\r\n");
    } else {
        printf("poll send 32-bit test success!\r\n");
    }

#if (SPI_CASE_SELECT == SPI_MASTER_CASE)
    qcc74x_mtimer_delay_ms(1000); /* delay for slave device prepare ok */
#endif

    printf("\r\n************** spi poll exchange 8-bit test **************\r\n");

    if (qcc74x_spi_poll_exchange_test(SPI_DATA_WIDTH_8BIT) < 0) {
        printf("poll exchange 8-bit test error!!!\r\n");
    } else {
        printf("poll exchange 8-bit test success!\r\n");
    }
#if (SPI_CASE_SELECT == SPI_MASTER_CASE)
    qcc74x_mtimer_delay_ms(1000); /* delay for slave device prepare ok */
#endif
    printf("\r\n************** spi poll exchange 16-bit test **************\r\n");
    if (qcc74x_spi_poll_exchange_test(SPI_DATA_WIDTH_16BIT) < 0) {
        printf("poll exchange 16-bit test error!!!\r\n");
    } else {
        printf("poll exchange 16-bit test success!\r\n");
    }
#if (SPI_CASE_SELECT == SPI_MASTER_CASE)
    qcc74x_mtimer_delay_ms(1000); /* delay for slave device prepare ok */
#endif
    printf("\r\n************** spi poll exchange 24-bit test **************\r\n");
    if (qcc74x_spi_poll_exchange_test(SPI_DATA_WIDTH_24BIT) < 0) {
        printf("poll exchange 24-bit test error!!!\r\n");
    } else {
        printf("poll exchange 24-bit test success!\r\n");
    }
#if (SPI_CASE_SELECT == SPI_MASTER_CASE)
    qcc74x_mtimer_delay_ms(1000); /* delay for slave device prepare ok */
#endif
    printf("\r\n************** spi poll exchange 32-bit test **************\r\n");
    if (qcc74x_spi_poll_exchange_test(SPI_DATA_WIDTH_32BIT) < 0) {
        printf("poll exchange 32-bit test error!!!\r\n");
    } else {
        printf("poll exchange 32-bit test success!\r\n");
    }

#if (SPI_CASE_SELECT == SPI_MASTER_CASE)
    qcc74x_mtimer_delay_ms(1000); /* delay for slave device prepare ok */
#endif

    printf("\r\n************** spi poll exchange only send 32-bit test **************\r\n");
    qcc74x_spi_poll_exchange(spi0, tx_buff, NULL, BUFF_LEN);
    printf("poll exchange 32-bit only send test end!\r\n");
#if (SPI_CASE_SELECT == SPI_MASTER_CASE)
    qcc74x_mtimer_delay_ms(1000); /* delay for slave device prepare ok */
#endif
    printf("\r\n************** spi poll exchange only receive 32-bit test **************\r\n");
    qcc74x_spi_poll_exchange(spi0, NULL, rx_buff, BUFF_LEN);
    printf("poll exchange 32-bit only receive test end!\r\n");
#if (SPI_CASE_SELECT == SPI_MASTER_CASE)
    qcc74x_mtimer_delay_ms(1000); /* delay for slave device prepare ok */
#endif
    printf("\r\n************** spi poll exchange spare time clock 32-bit test **************\r\n");
    qcc74x_spi_poll_exchange(spi0, NULL, NULL, BUFF_LEN);
    printf("poll exchange 32-bit spare time clock test end!\r\n");

    printf("\r\nspi test end\r\n");

    printf("spi tc done count:%d\r\n", spi_tc_done_count);
    while (1) {
    }
}
