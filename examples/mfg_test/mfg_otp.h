#ifndef MFG_OTP_H
#define MFG_OTP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct {
    const char *otp_name;
    uint32_t otp_addr;
    int (*otp_write_func)(uint32_t addr, uint32_t value);
    int (*otp_read_func)(uint32_t addr, uint32_t *value);
} qcc74x_manufacture_otp_struct;

void otp_init();
int otp_write(const qcc74x_manufacture_otp_struct *otp, uint32_t value);
int otp_read(const qcc74x_manufacture_otp_struct *otp, uint32_t *value);
int otp_set_part_number(uint32_t *value);
int otp_get_part_number(uint32_t *value);
int otp_get_mac_addr(uint8_t mac_0[6], uint8_t mac_1[6], uint8_t mac_2[6]);
const qcc74x_manufacture_otp_struct* otp_find_by_name(const char *name);
int otp_get_soc_signature(uint32_t *value);

#ifdef __cplusplus
}
#endif

#endif/* AT_CORE_H */
