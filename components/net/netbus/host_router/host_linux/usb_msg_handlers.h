#ifndef USB_MSG_HANDLERS_H_XGVGJUFN
#define USB_MSG_HANDLERS_H_XGVGJUFN

#include "qcc74x_usb_eth.h"
#include "msg_handlers.h"

int append_usb_hdr(struct qcc74x_eth_device *dev, struct sk_buff *skb, uint16_t type);
int qcc74x_handle_rx_data(struct qcc74x_eth_device *dev, struct sk_buff *skb);

size_t build_simple_cmd_msg(void *buf, uint16_t cmd);
int parse_get_mac_rsp_msg(struct qcc74x_eth_device *dev, void *buf, int buf_len);

#endif /* end of include guard: USB_MSG_HANDLERS_H_XGVGJUFN */