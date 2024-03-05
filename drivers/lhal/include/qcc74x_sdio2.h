

#ifndef _QCC74x_SDIO2_H
#define _QCC74x_SDIO2_H

#include "qcc74x_core.h"

/* SDIO2 buffer size */
#define SDIO2_DEFAULT_SIZE_MAX        (1024 * 2)

/* No modification */
#define SDIO2_SIZE_CONSULT_MULTIPLE   (256)
#define SDIO2_BYTE_MOD_SIZE_MAX       (512)
#define SDIO2_MAX_PORT_NUM            16

/* error callback resaon */
#define SDIO2_ERROR_CRC               (1)
#define SDIO2_ERROR_ABORT             (2)

/* card to host interrupt event */
#define SDIO2_HOST_INT_EVENT_DNLD_RDY (1 << 0)
#define SDIO2_HOST_INT_EVENT_UPLD_RDY (1 << 1)
#define SDIO2_HOST_INT_EVENT_CIS_RDY  (1 << 2)
#define SDIO2_HOST_INT_EVENT_IO_RDY   (1 << 3)

#if (SDIO2_MAX_PORT_NUM & (SDIO2_MAX_PORT_NUM - 1) != 0)
#error "sdio2 port num error, must be 2^n"
#else
#define SDIO2_MAX_PORT_MASK (SDIO2_MAX_PORT_NUM - 1)
#endif

/* trans desc */
typedef struct
{
    uint16_t buff_len;
    uint16_t data_len;
    void *buff;
    void *user_arg;
} qcc74x_sdio2_trans_desc_t;

/* trans callback */
typedef void (*qcc74x_sdio2_trans_irq_callback_t)(void *arg, qcc74x_sdio2_trans_desc_t *trans_desc);
/* error callback */
typedef void (*qcc74x_sdio2_error_irq_callback_t)(void *arg, uint32_t err_code);
#if defined(QCC743)
/* reset callback */
typedef void (*qcc74x_sdio2_reset_irq_callback_t)(void *arg,
                                                qcc74x_sdio2_trans_desc_t *upld_rest_desc, uint32_t upld_rest_desc_num,
                                                qcc74x_sdio2_trans_desc_t *dnld_rest_desc, uint32_t dnld_rest_desc_num);
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* initialization */
int qcc74x_sdio2_init(struct qcc74x_device_s *dev, uint32_t dnld_size_max);
int qcc74x_sdio2_check_host_ready(struct qcc74x_device_s *dev);
int qcc74x_sdio2_get_block_size(struct qcc74x_device_s *dev);

/* get trans max size */
int qcc74x_sdio2_get_upld_max_size(struct qcc74x_device_s *dev);
int qcc74x_sdio2_get_dnld_max_size(struct qcc74x_device_s *dev);

/* attach dnld/upld buff */
int qcc74x_sdio2_dnld_port_push(struct qcc74x_device_s *dev, qcc74x_sdio2_trans_desc_t *trans_desc);
int qcc74x_sdio2_upld_port_push(struct qcc74x_device_s *dev, qcc74x_sdio2_trans_desc_t *trans_desc);

/* get dnld/upld info */
int qcc74x_sdio2_dnld_get_waiting(struct qcc74x_device_s *dev);
int qcc74x_sdio2_dnld_get_available(struct qcc74x_device_s *dev);
int qcc74x_sdio2_upld_get_waiting(struct qcc74x_device_s *dev);
int qcc74x_sdio2_upld_get_available(struct qcc74x_device_s *dev);

/* isr callback attach */
int qcc74x_sdio2_dnld_irq_attach(struct qcc74x_device_s *dev, qcc74x_sdio2_trans_irq_callback_t dnld_irq_callback, void *arg);
int qcc74x_sdio2_upld_irq_attach(struct qcc74x_device_s *dev, qcc74x_sdio2_trans_irq_callback_t upld_irq_callback, void *arg);
int qcc74x_sdio2_error_irq_attach(struct qcc74x_device_s *dev, qcc74x_sdio2_error_irq_callback_t error_irq_callback, void *arg);
#if defined(QCC743)
int qcc74x_sdio2_reset_irq_attach(struct qcc74x_device_s *dev, qcc74x_sdio2_reset_irq_callback_t reset_irq_callback, void *arg);
#endif

/* trigger host interrupt event */
int qcc74x_sdio2_trig_host_int(struct qcc74x_device_s *dev, uint32_t event);

#ifdef __cplusplus
}
#endif

#endif /* _QCC74x_SDIO3_H */
