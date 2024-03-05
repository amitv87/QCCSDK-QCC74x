

#ifndef _BSP_WM8978_H
#define _BSP_WM8978_H

#include <stdint.h>

int bsp_wm8978_init(struct qcc74x_device_s *i2c);
int bsp_wm8978_write_reg(uint8_t reg, uint16_t val);
uint16_t bsp_wm8978_read_reg(uint8_t reg);
void bsp_wm8978_config_adda(uint8_t dacen, uint8_t adcen);
void bsp_wm8978_config_input(uint8_t micen, uint8_t lineinen, uint8_t auxen);
void bsp_wm8978_config_output(uint8_t dacen, uint8_t bpsen);
void bsp_wm8978_setgain_mic(uint8_t gain);
void bsp_wm8978_setgain_linein(uint8_t gain);
void bsp_wm8978_setgain_aux(uint8_t gain);
void bsp_wm8978_config_i2s(uint8_t fmt, uint8_t bit);
void bsp_wm8978_setvol_headset(uint8_t voll, uint8_t volr);
void bsp_wm8978_setvol_speaker(uint8_t volx);
void bsp_wm8978_config_3d(uint8_t depth);
void bsp_wm8978_config_3deq(uint8_t dir);
void bsp_wm8978_config_eq1(uint8_t cfreq, uint8_t gain);
void bsp_wm8978_config_eq2(uint8_t cfreq, uint8_t gain);
void bsp_wm8978_config_eq3(uint8_t cfreq, uint8_t gain);
void bsp_wm8978_config_eq4(uint8_t cfreq, uint8_t gain);
void bsp_wm8978_config_eq5(uint8_t cfreq, uint8_t gain);

#endif
