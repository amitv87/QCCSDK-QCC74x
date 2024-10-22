#ifndef MAIN_H_UPSXCZ5H
#define MAIN_H_UPSXCZ5H

#include "config.h"
#include "ctl_port.h"
#include <linux/mutex.h>

#define QCC74x_DRV_NAME "qcwnet"

#define QCC74x_DEVICE_STATUS_DRV_REMOVING  (1 << 0)
#define QCC74x_DEVICE_STATUS_CARD_REMOVING (1 << 1)
#define QCC74x_DEVICE_STATUS_CARD_PRESENT  (1 << 2)
#define QCC74x_DEVICE_STATUS_CARD_DEAD     (1 << 3)

struct qcc74x_device {
    struct mutex mutex;
    u8 status;
    struct qcc74x_eth_device *eth_dev;
};

extern struct qcc74x_device gl_dev;

int qcc74x_register_eth_drv(void);
void qcc74x_unregister_eth_drv(void);

bool qcc74x_card_ok(void);

#endif /* end of include guard: MAIN_H_UPSXCZ5H */
