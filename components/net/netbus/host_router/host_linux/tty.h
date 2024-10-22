#ifndef TTY_H_8OXTIMDC
#define TTY_H_8OXTIMDC

#ifdef QCC74x_INTF_SDIO
#include "qcc74x_sdio_eth.h"
#else
#include "qcc74x_usb_eth.h"
#endif

int qcc74x_handle_ext_frame_tty(struct qcc74x_eth_device *dev, const uint8_t *frame, const uint16_t frame_len);

int qcc74x_tty_init(void);
void qcc74x_tty_exit(void);

struct qcc74x_tty_queued_data {
    u16 tty_id;
};

#endif /* end of include guard: TTY_H_8OXTIMDC */
