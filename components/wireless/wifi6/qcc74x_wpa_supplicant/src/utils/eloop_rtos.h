#ifndef __ELOOP_RTOS_H
#define __ELOOP_RTOS_H

#ifdef __cplusplus
extern "C" {
#endif

#define ELOOP_MSG_INBUF_LEN     512
#define ELOOP_MSG_OUTBUF_LEN    128

enum {
    ELOOP_EVT_WPA_IFACE_CTRL = 1,
    ELOOP_EVT_WPA_DRV_CTRL   = 2,
    ELOOP_EVT_WPA_L2_DATA    = 3,
};

#ifdef __cplusplus
}
#endif

/* __ELOOP_RTOS_H */
#endif
