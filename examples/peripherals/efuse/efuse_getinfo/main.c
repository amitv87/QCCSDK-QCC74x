#include "qcc74x_mtimer.h"
#include "board.h"
#include "qcc74x_efuse.h"

qcc74x_efuse_device_info_type device_info;
uint8_t chip_id[8];
uint8_t sign;
uint8_t aes;

const char *aes_mode[] = {"AES-NO", "AES-128", "AES-192", "AES-256"};

int main(void)
{
    struct qcc74x_device_s *efuse_dev;

    board_init();
    efuse_dev = qcc74x_device_get_by_name("ef_ctrl");
    if (NULL == efuse_dev) {
        printf("efuse device driver not found!\r\n");
        while (1);
    }

    qcc74x_efuse_get_device_info(&device_info);
    qcc74x_efuse_get_chipid(chip_id);
    qcc74x_efuse_read_secure_boot(&sign, &aes);

    /* printf device_info */
    printf("DEVICE_INFO:\r\n");
    printf("    PACKAGE: %s\r\n", device_info.package_name);
#if !defined(QCC74x_undef)
    printf("    PSRAM:   %s\r\n", device_info.psram_info_name);
#endif
    printf("    FLASH:   %s\r\n", device_info.flash_info_name);
#if !defined(QCC74x_undef) && !defined(QCC74x_undefL)
    printf("    VERSION: %d\r\n", device_info.version);
#endif
    printf("CHIP_ID:\r\n   ");
    for (int i = 0; i < sizeof(chip_id) / sizeof(chip_id[0]) - 2; i++) {
        printf(" %02X", chip_id[i]);
    }
    printf("\r\n");

    printf("SIGNATURE:\r\n    ");
    if (sign) {
        printf("ENABLE\r\n");
    } else {
        printf("DISABLE\r\n");
    }
    printf("AES_MODE:\r\n    %s\r\n", aes_mode[aes]);

    while (1) {
        qcc74x_mtimer_delay_ms(1000);
    }
}
