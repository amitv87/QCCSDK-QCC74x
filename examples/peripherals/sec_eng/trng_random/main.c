#include "qcc74x_mtimer.h"
#include "qcc74x_sec_trng.h"
#include "board.h"

static void trng_dump_data(uint8_t *data, uint8_t len)
{
    int i;
    printf("TRNG data \r\n");
    for (i = 0; i < len; i++) {
        printf("%02x ", data[i]);
    }
    printf("\r\n");
}

int main(void)
{
    uint8_t data[32];

    board_init();

    struct qcc74x_device_s *trng;

    trng = qcc74x_device_get_by_name("trng");

    while (1) {
        qcc74x_trng_read(trng, data);
        trng_dump_data(data, 32);
        qcc74x_mtimer_delay_ms(2000);
    }
}