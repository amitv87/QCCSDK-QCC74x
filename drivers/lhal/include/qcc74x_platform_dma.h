#ifndef _QCC74x_PLATFORM_DMA_H
#define _QCC74x_PLATFORM_DMA_H

#include "qcc74x_core.h"

/** @addtogroup LHAL
  * @{
  */

#define PLFM_DMA_CHANNEL_MAX 5

/** @addtogroup PLATFORM_DMA
  * @{
  */

struct qcc74x_platform_dma_desc {
    /** Application subsystem address which is used as source address for DMA payload
      * transfer*/
    uint32_t src;
    /** Points to the start of the embedded data buffer associated with this descriptor.
     *  This address acts as the destination address for the DMA payload transfer*/
    uint32_t dest;
    /// Complete length of the buffer in memory
    uint16_t length;
    /// Control word for the DMA engine (e.g. for interrupt generation)
    uint16_t ctrl;
    /// Pointer to the next element of the chained list
    uint32_t next;
};

/// Structure describing the DMA driver environment
struct qcc74x_platform_dma_env_tag {
    /** last DMA descriptor pushed for each channel, can point to descriptor already
     * deallocated, but then will not be use`d because root register will be NULL
     */
    volatile struct qcc74x_platform_dma_desc *last_dma[PLFM_DMA_CHANNEL_MAX];
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 ****************************************************************************************
 * @brief Initialize the bridge DMA registers
 *
 * @param [in]   dev device handle
 ****************************************************************************************
 */
void qcc74x_platform_dma_init(struct qcc74x_device_s *dev);

/**
 ****************************************************************************************
 * @brief Chains a chained list of descriptors in the DMA
 *
 * @param [in]   dev device handle
 * @param [in]   first First DMA descriptor of the list (filled by the caller)
 * @param [in]   last last DMA descriptor of the list (filled by the caller)
 *
 ****************************************************************************************
 */
void qcc74x_platform_dma_push(struct qcc74x_device_s *dev, struct qcc74x_platform_dma_desc *first, struct qcc74x_platform_dma_desc *last);

/**
 ****************************************************************************************
 * @brief Interrupt service routine when a bus error is detected while in a DMA transfer.
 * This error is considered as fatal and triggers a non-recoverable assertion.
 *
 * @param [in]   dev device handle
 ****************************************************************************************
 */
void qcc74x_platform_dma_buserr_isr(struct qcc74x_device_s *dev);

/**
 ****************************************************************************************
 * @brief Active wait until DMA channel become inactive
 *
 * @param [in]   dev device handle
 ****************************************************************************************
 */
void qcc74x_platform_dma_wait_eot(struct qcc74x_device_s *dev);

/**
 ****************************************************************************************
 * @brief Active wait until DMA channel become inactive
 *
 * @param [in]   dev device handle
 ****************************************************************************************
 */
void qcc74x_platform_dma_clear_eot(struct qcc74x_device_s *dev);

#ifdef __cplusplus
}
#endif

/**
  * @}
  */

/**
  * @}
  */

#endif