#include "qcc74xsp_port.h"
#if QCC74xSP_BOOT2_SUPPORT_USB_IAP
#include "qcc74x_eflash_loader.h"
#include "hal_usb.h"
#include "usbd_core.h"
#include "usbd_cdc.h"
#include "qcc74xsp_common.h"
#include "xz_config.h"

#define CDC_IN_EP  0x82
#define CDC_OUT_EP 0x01
#define CDC_INT_EP 0x83

#define USBD_VID           0xFFFF
#define USBD_PID           0xFFFF
#define USBD_MAX_POWER     100
#define USBD_LANGID_STRING 1033

#define USB_CONFIG_SIZE (9 + CDC_ACM_DESCRIPTOR_LEN)

USB_DESC_SECTION const uint8_t cdc_descriptor[] = {
    USB_DEVICE_DESCRIPTOR_INIT(USB_2_0, 0x02, 0x02, 0x01, USBD_VID, USBD_PID, 0x0100, 0x01),
    USB_CONFIG_DESCRIPTOR_INIT(USB_CONFIG_SIZE, 0x02, 0x01, USB_CONFIG_BUS_POWERED, USBD_MAX_POWER),
    CDC_ACM_DESCRIPTOR_INIT(0x00, CDC_INT_EP, CDC_OUT_EP, CDC_IN_EP, 0x02),
    ///////////////////////////////////////
    /// string0 descriptor
    ///////////////////////////////////////
    USB_LANGID_INIT(USBD_LANGID_STRING),
    ///////////////////////////////////////
    /// string1 descriptor
    ///////////////////////////////////////
    0x12,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'B', 0x00,                  /* wcChar0 */
    'o', 0x00,                  /* wcChar1 */
    'u', 0x00,                  /* wcChar2 */
    'f', 0x00,                  /* wcChar3 */
    'f', 0x00,                  /* wcChar4 */
    'a', 0x00,                  /* wcChar5 */
    'l', 0x00,                  /* wcChar6 */
    'o', 0x00,                  /* wcChar7 */
    ///////////////////////////////////////
    /// string2 descriptor
    ///////////////////////////////////////
    0x24,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    'B', 0x00,                  /* wcChar0 */
    'o', 0x00,                  /* wcChar1 */
    'u', 0x00,                  /* wcChar2 */
    'f', 0x00,                  /* wcChar3 */
    'f', 0x00,                  /* wcChar4 */
    'a', 0x00,                  /* wcChar5 */
    'l', 0x00,                  /* wcChar6 */
    'o', 0x00,                  /* wcChar7 */
    ' ', 0x00,                  /* wcChar8 */
    'C', 0x00,                  /* wcChar9 */
    'D', 0x00,                  /* wcChar10 */
    'C', 0x00,                  /* wcChar11 */
    ' ', 0x00,                  /* wcChar13 */
    'D', 0x00,                  /* wcChar14 */
    'E', 0x00,                  /* wcChar15 */
    'M', 0x00,                  /* wcChar16 */
    'O', 0x00,                  /* wcChar17 */
    ///////////////////////////////////////
    /// string3 descriptor
    ///////////////////////////////////////
    0x16,                       /* bLength */
    USB_DESCRIPTOR_TYPE_STRING, /* bDescriptorType */
    '2', 0x00,                  /* wcChar0 */
    '0', 0x00,                  /* wcChar1 */
    '2', 0x00,                  /* wcChar2 */
    '1', 0x00,                  /* wcChar3 */
    '0', 0x00,                  /* wcChar4 */
    '3', 0x00,                  /* wcChar5 */
    '1', 0x00,                  /* wcChar6 */
    '0', 0x00,                  /* wcChar7 */
    '0', 0x00,                  /* wcChar8 */
    '0', 0x00,                  /* wcChar9 */
    ///////////////////////////////////////
    /// device qualifier descriptor
    ///////////////////////////////////////
    0x0a,
    USB_DESCRIPTOR_TYPE_DEVICE_QUALIFIER,
    0x00,
    0x02,
    0x02,
    0x02,
    0x01,
    0x40,
    0x01,
    0x00,

    0x00
};

void usb_send(uint8_t *buf, uint32_t len)
{
    //    while (len >= 64) {
    //        usbd_ep_write(0x82, buf, 64, NULL);
    //        buf += 64;
    //        len -= 64;
    //    }
    usbd_ep_write(0x82, buf, len, NULL);
    if (len == 64) {
        usbd_ep_write(0x82, NULL, 0, NULL);
    }
    //    if (len > 0) {
    //        usbd_ep_write(0x82, buf, len, NULL);
    //    } else {
    //        usbd_ep_write(0x82, NULL, 0, NULL);
    //    }
}

void usbd_cdc_acm_bulk_out(uint8_t ep)
{
    uint32_t actual_read_length = 0;
    uint8_t out_buffer[64];
    uint8_t *buf = g_eflash_loader_readbuf[g_rx_buf_index];

    if (usbd_ep_read(ep, out_buffer, 64, &actual_read_length) < 0) {
        //USBD_LOG_DBG("Read DATA Packet failed\r\n");
        usbd_ep_set_stall(ep);
        return;
    }

    //    qcc74x_dump_data(out_buffer,actual_read_length);
    //    usb_send(out_buffer,actual_read_length);

    if (g_rx_buf_len + actual_read_length < QCC74x_EFLASH_LOADER_READBUF_SIZE) {
        arch_memcpy(buf + g_rx_buf_len, out_buffer, actual_read_length);
        g_rx_buf_len += actual_read_length;
    } else {
        g_rx_buf_len = 0;
    }
    //LOG_D("%d\r\n",g_rx_buf_len);
    usbd_ep_read(ep, NULL, 0, NULL);
}

usbd_class_t cdc_class;
usbd_interface_t cdc_cmd_intf;
usbd_interface_t cdc_data_intf;

usbd_endpoint_t cdc_out_ep = {
    .ep_addr = CDC_OUT_EP,
    .ep_cb = usbd_cdc_acm_bulk_out
};

usbd_endpoint_t cdc_in_ep = {
    .ep_addr = CDC_IN_EP,
    .ep_cb = NULL
};

void qcc74x_eflash_loader_usb_if_init(void)
{
    hal_boot2_debug_usb_port_init();

    simple_malloc_init(g_malloc_buf, sizeof(g_malloc_buf));
    g_eflash_loader_readbuf[0] = vmalloc(QCC74x_EFLASH_LOADER_READBUF_SIZE);
    g_eflash_loader_readbuf[1] = vmalloc(QCC74x_EFLASH_LOADER_READBUF_SIZE);
    arch_memset(g_eflash_loader_readbuf[0], 0, QCC74x_EFLASH_LOADER_READBUF_SIZE);
    arch_memset(g_eflash_loader_readbuf[1], 0, QCC74x_EFLASH_LOADER_READBUF_SIZE);

    usbd_desc_register(cdc_descriptor);

    usbd_cdc_add_acm_interface(&cdc_class, &cdc_cmd_intf);
    usbd_cdc_add_acm_interface(&cdc_class, &cdc_data_intf);
    usbd_interface_add_endpoint(&cdc_data_intf, &cdc_out_ep);
    usbd_interface_add_endpoint(&cdc_data_intf, &cdc_in_ep);

    usbd_initialize();

    while (!usb_device_is_configured()) {
    }
}

int32_t qcc74x_eflash_loader_usb_send(uint32_t *data, uint32_t len)
{
    usb_send((uint8_t *)data, len);
    return QCC74x_EFLASH_LOADER_SUCCESS;
}

int32_t qcc74x_eflash_loader_usb_wait_tx_idle(uint32_t timeout)
{
    /*UART now can't judge tx idle now*/
    qcc74x_mtimer_delay_ms(timeout);

    return QCC74x_EFLASH_LOADER_SUCCESS;
}

int32_t qcc74x_eflash_loader_usb_init()
{
    qcc74x_eflash_loader_usb_deinit();
    qcc74x_eflash_loader_usb_if_init();

    return QCC74x_EFLASH_LOADER_SUCCESS;
}

int32_t qcc74x_eflash_loader_check_handshake_buf(uint8_t *buf, uint32_t len)
{
    if (len == 0)
        return -1;

    for (int i = 0; i < len; i++) {
        if (buf[i] != QCC74x_EFLASH_LOADER_HAND_SHAKE_BYTE)
            return -1;
    }
    return 0;
}

int32_t qcc74x_eflash_loader_usb_handshake_poll(uint32_t timeout)
{
    uint64_t time_now;
    uint32_t handshake_count = 0;
    uint8_t *buf = g_eflash_loader_readbuf[g_rx_buf_index];

    time_now = qcc74x_mtimer_get_time_ms();

    LOG_D("usb iap handshake poll\r\n");
    do {
        if (g_rx_buf_len >= 16) {
            for (int i = 0; i < 16; i++) {
                if (buf[i] == QCC74x_EFLASH_LOADER_HAND_SHAKE_BYTE) {
                    handshake_count++;

                    if (handshake_count > QCC74x_EFLASH_LAODER_HAND_SHAKE_SUSS_COUNT) {
                        break;
                    }
                }
            }
            if (handshake_count > QCC74x_EFLASH_LAODER_HAND_SHAKE_SUSS_COUNT) {
                break;
            }
        }
    } while ((timeout == 0) ? 1 : (qcc74x_mtimer_get_time_ms() - time_now < timeout * 1000));

    if (handshake_count >= QCC74x_EFLASH_LAODER_HAND_SHAKE_SUSS_COUNT) {
        LOG_D("iap handshake %d 0x55 rcv\r\n", handshake_count);

    } else {
        LOG_D("iap handshake err\r\n");
        return -1;
    }

    /*receive shake hanad signal*/
    usb_send((uint8_t *)"OK", 2);
    //arch_delay_ms(400);
    qcc74x_mtimer_delay_ms(400);
    /*init rx info */
    g_rx_buf_index = 0;
    g_rx_buf_len = 0;

    time_now = qcc74x_mtimer_get_time_ms();
    do {
        if (g_rx_buf_len > 0) {
            if (0 == qcc74x_eflash_loader_check_handshake_buf(buf, g_rx_buf_len)) {
                g_rx_buf_len = 0;
                time_now = qcc74x_mtimer_get_time_ms();
            } else {
                break;
            }
        }
    } while (qcc74x_mtimer_get_time_ms() - time_now < 50);

    return QCC74x_EFLASH_LOADER_SUCCESS;
}

uint32_t *qcc74x_eflash_loader_usb_recv(uint32_t *recv_len, uint32_t maxlen, uint32_t timeout)
{
    return NULL;
}

int32_t qcc74x_eflash_loader_usb_change_rate(uint32_t oldval, uint32_t newval)
{
    return QCC74x_EFLASH_LOADER_SUCCESS;
}

int32_t qcc74x_eflash_loader_usb_deinit()
{
    return QCC74x_EFLASH_LOADER_SUCCESS;
}
#endif
