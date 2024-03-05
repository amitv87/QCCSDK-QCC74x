#ifndef __QCC74x_EFLASH_LOADER_USB_H__
#define __QCC74x_EFLASH_LOADER_USB_H__

#include "stdint.h"

int32_t qcc74x_eflash_loader_usb_init();

int32_t qcc74x_eflash_loader_usb_handshake_poll();

uint32_t *qcc74x_eflash_loader_usb_recv(uint32_t *recv_len, uint32_t maxlen, uint32_t timeout);

int32_t qcc74x_eflash_loader_usb_send(uint32_t *data, uint32_t len);

int32_t qcc74x_eflash_loader_usb_wait_tx_idle(uint32_t timeout);

int32_t qcc74x_eflash_loader_usb_change_rate(uint32_t oldval, uint32_t newval);

int32_t qcc74x_eflash_loader_usb_deinit(void);

#define QCC74x_EFLASH_LOADER_IF_USB_RX_TIMEOUT 8000 /*ms*/

#endif
