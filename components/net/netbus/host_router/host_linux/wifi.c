#include "wifi.h"

#include <linux/etherdevice.h>
#include <linux/version.h>

#ifdef QCC74x_INTF_SDIO
#include "qcc74x_sdio_eth.h"
#else
#include "qcc74x_usb_eth.h"
#endif

const char *qcc74x_mode_to_str(const qcc74x_wifi_mode_t mode)
{
    const char *str[] = {
        [QCC74x_MODE_NONE]    = "NONE",
        [QCC74x_MODE_STA]     = "STA",
        [QCC74x_MODE_AP]      = "AP",
        [QCC74x_MODE_STA_AP]  = "STA_AP",
        [QCC74x_MODE_SNIFFER] = "SNIFFER",
        [QCC74x_MODE_MAX]     = "UNKNOWN",
    };

    if (QCC74x_MODE_NONE <= mode && mode < QCC74x_MODE_MAX) {
        return str[mode];
    } else {
        return str[QCC74x_MODE_MAX];
    }
}

int qcc74x_mode_xfer(qcc74x_wifi_mode_t *mode, const qcc74x_wifi_mode_t new_mode, qcc74x_wifi_mode_t *old_mode)
{
    int ret = 0;

    if (old_mode) {
        *old_mode = *mode;
    }
    if (new_mode == *mode) {
        goto exit;
    }
    if (*mode == QCC74x_MODE_NONE) {
        *mode = new_mode;
    } else if (*mode == QCC74x_MODE_STA) {
        *mode = new_mode;
    } else if (*mode == QCC74x_MODE_AP) {
        *mode = new_mode;
    } else {
        ret = -1;
    }
exit:
    return ret;
}

void qcc74x_change_eth_mac(void *dev)
{
    u8 *mac;
    struct qcc74x_eth_device *d = dev;

    mac = d->sta_mac;
    if (d->mode == QCC74x_MODE_AP)
        mac = d->ap_mac;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 16, 0)
    eth_hw_addr_set(d->net, mac);
#else
    memcpy(d->net->dev_addr, mac, ETH_ALEN);
#endif
}
