#ifndef _QCC74x_ISO11898_GLB_H
#define _QCC74x_ISO11898_GLB_H

#define GPIO_ISO11898_FUNC_TX 10
#define GPIO_ISO11898_FUNC_RX 11

struct qcc74x_device_s *qcc74x_device_get_iso11898(const char *name);
void qcc74x_gpio_iso11898_init(struct qcc74x_device_s *dev, uint8_t pin, uint8_t iso11898_func);
void qcc74x_iso11898_clock_enable(uint8_t enable);

#endif
