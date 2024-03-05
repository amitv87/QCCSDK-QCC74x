#ifndef __QCC74x_HCI_WRAPPER_H__
#define __QCC74x_HCI_WRAPPER_H__

#include "net/buf.h"
#include "bluetooth.h"

struct rx_msg_struct{
    uint8_t pkt_type;
    uint16_t src_id;
    uint8_t *param;
    uint8_t param_len;
}__packed;

typedef enum {
  DATA_TYPE_COMMAND = 1,
  DATA_TYPE_ACL     = 2,
  DATA_TYPE_SCO     = 3,
  DATA_TYPE_EVENT   = 4
} serial_data_type_t;

uint8_t qcc74x_onchiphci_interface_init(void);
void qcc74x_onchiphci_interface_deinit(void);
void qcc74x_trigger_queued_msg(void);
int qcc74x_onchiphci_send_2_controller(struct net_buf *buf);

#endif //__QCC74x_CONTROLLER_H__