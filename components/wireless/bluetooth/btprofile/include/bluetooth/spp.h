#ifndef _INCLUDE_BLUETOOTH_SPP_H_
#define _INCLUDE_BLUETOOTH_SPP_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*bt_spp_callbck_func_t)(u8_t *data, u16_t length);

int bt_spp_init(void);
int bt_spp_connect(struct bt_conn *conn);
int bt_spp_disconnect(struct bt_conn *conn);
int bt_spp_send(uint8_t *buf_data,uint8_t length);
void spp_cb_register(bt_spp_callbck_func_t cb);

#ifdef __cplusplus
}
#endif

#endif /* _INCLUDE_BLUETOOTH_AVCTP_H_ */
