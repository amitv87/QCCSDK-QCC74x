#ifndef _YOC_QCC74x_AUDIO_H_
#define _YOC_QCC74x_AUDIO_H_

#if 1
#include <msp_qcc743_platform.h>
#include <msp_qcc743_dma.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  @brief Audio Channel Sel
 */
typedef enum {
    QCC74x_AUDIO_LEFT_CHANNEL_  = 1, /*!< select mono mode left only */
    QCC74x_AUDIO_STEREO_CHANNEL_ = 3,   /*!< select stereo */
    QCC74x_AUDIO_THREE_CHANNEL_  = 7,
} QCC74x_Audio_Channel_Type;

/**
 *  @brief Audio Play/recording bit width
 */
typedef enum {
    QCC74x_AUDIO_BIT_WIDTH_16, /*!< 16 bit */
    QCC74x_AUDIO_BIT_WIDTH_24, /*!< 24 bit */
    QCC74x_AUDIO_BIT_WIDTH_32, /*!< 32 bit */
} QCC74x_Audio_BitWidth_Type;

#define AUDIO_DEFAULT_INITIAL_VOLUME         (0x0)
#define AUDIO_TX_FIFO_ADDR                      SOC_AUDIO_TX_BUF
#define AUDIO_RX_FIFO_ADDR                      SOC_AUDIO_RX_BUF
#define AUDIO_ALIGNMENT_MASK (0x001f)
#define AUDIO_ALIGNMENT_BYTES (32)

/* audio sample  rate */
#define QCC74x_AUDIO_RATE_8K                 8000
#define QCC74x_AUDIO_RATE_16K                16000
#define QCC74x_AUDIO_RATE_24K                24000
#define QCC74x_AUDIO_RATE_32K                32000
#define QCC74x_AUDIO_RATE_48K                48000
#define QCC74x_AUDIO_RATE_44_1K              44100
#define QCC74x_AUDIO_RATE_96K                96000
#define QCC74x_AUDIO_RATE_192K               192000

/* audio bit width */
#define QCC74x_AUDIO_BIT_WIDTH_16            16
#define QCC74x_AUDIO_BIT_WIDTH_24            24
#define QCC74x_AUDIO_BIT_WIDTH_32            32
/* audio track */
#define QCC74x_AUDIO_LEFT_CHANNEL            1
#define QCC74x_AUDIO_STEREO_CHANNEL          2
#define QCC74x_AUDIO_THREE_CHANNEL           3

typedef enum {
    QCC74x_EVENT_NODE_READ_COMPLETE        = 0U,  ///< A peroid data read completed
    QCC74x_EVENT_NODE_WRITE_COMPLETE       = 1U,  ///< A peroid data write completed
    QCC74x_EVENT_WRITE_BUFFER_EMPTY          = 2U,  ///< Fifo is empty
    QCC74x_EVENT_READ_BUFFER_FULL            = 3U,  ///< Fifo is full
    QCC74x_EVENT_ERROR_OVERFLOW              = 4U,  ///< Fifo overflow error
    QCC74x_EVENT_ERROR_UNDERFLOW             = 5U,  ///< Fifo underflow error
    QCC74x_EVENT_ERROR                       = 6U,  ///< The device has a hardware error
} audio_codec_event_t;

typedef struct
{
    uint32_t srcDmaAddr;            /*!< Source address of DMA transfer */
    uint32_t destDmaAddr;           /*!< Destination address of DMA transfer */
    uint32_t nextLLI;               /*!< Next LLI address */
    struct DMA_Control_Reg dmaCtrl; /*!< DMA transaction control */
} _dmahw_cfg_t;

 #ifdef __cplusplus
 }
 #endif

#endif
 #endif /* _YOC_QCC74x_AUDIO_H_ */

