#ifndef BTBLECONTROLLER_PORT_API_H_
#define BTBLECONTROLLER_PORT_API_H_

#include <stdint.h>

#define QCC743_A0              0
#define QCC743_A1              1

void btblecontroller_ble_irq_init(void *handler);
void btblecontroller_bt_irq_init(void *handler);
void btblecontroller_dm_irq_init(void *handler);
void btblecontroller_ble_irq_enable(uint8_t enable);
void btblecontroller_bt_irq_enable(uint8_t enable);
void btblecontroller_dm_irq_enable(uint8_t enable);
void btblecontroller_enable_ble_clk(uint8_t enable);
void btblecontroller_rf_restore();
int btblecontroller_efuse_read_mac(uint8_t mac[6]);
void btblecontroller_software_btdm_reset();
void btblecontroller_software_pds_reset();
void btblecontroller_pds_trim_rc32m();
uint8_t btblecontrolller_get_chip_version();
#endif