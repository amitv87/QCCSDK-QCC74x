#ifndef WIFI_H_VV2FWLIH
#define WIFI_H_VV2FWLIH

typedef enum {
    QCC74x_MODE_NONE,
    QCC74x_MODE_STA,      // card is STA
    QCC74x_MODE_AP,       // card is AP
    QCC74x_MODE_STA_AP,   // card is STA&AP
    QCC74x_MODE_SNIFFER,  // card is sniffer
    QCC74x_MODE_MAX,
} qcc74x_wifi_mode_t;

const char *qcc74x_mode_to_str(const qcc74x_wifi_mode_t mode);
int qcc74x_mode_xfer(qcc74x_wifi_mode_t *mode, const qcc74x_wifi_mode_t new_mode, qcc74x_wifi_mode_t *old_mode);
void qcc74x_change_eth_mac(void *dev);

#endif /* end of include guard: WIFI_H_VV2FWLIH */
