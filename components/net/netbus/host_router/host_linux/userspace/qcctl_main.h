#ifndef QCCTL_MAIN_H_LTNVH5QR
#define QCCTL_MAIN_H_LTNVH5QR

#include <stdint.h>

typedef void (*rx_data_cb_t)(void *cb_arg, const uint8_t *data, const size_t data_len);

int tx_transparent_host2device_msg(int fd, const void *cmd, const int cmd_len);
int rx_data_cb_register(rx_data_cb_t cb, void *cb_arg);

#endif /* end of include guard: QCCTL_MAIN_H_LTNVH5QR */
