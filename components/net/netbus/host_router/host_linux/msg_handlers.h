#ifndef MSG_HANDLERS_H_H6RXKLZN
#define MSG_HANDLERS_H_H6RXKLZN

#ifdef QCC74x_INTF_SDIO
#include "qcc74x_sdio_eth.h"
#else
#include "qcc74x_usb_eth.h"
#endif

int push_host2device_msg(const uint8_t *payload, size_t len);;
void qcc74x_handle_cmd(struct qcc74x_eth_device *dev, const void *data_ptr, const uint16_t data_len);

#endif /* end of include guard: MSG_HANDLERS_H_H6RXKLZN */
