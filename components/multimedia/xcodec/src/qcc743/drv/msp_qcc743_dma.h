#ifndef __MSP_QCC743_DMA_H__
#define __MSP_QCC743_DMA_H__

#include "msp_qcc743_platform.h"
#include "msp_dma_reg.h"
//#include "qcc743_common.h"

/** @addtogroup  QCC743_Peripheral_Driver
 *  @{
 */

/** @addtogroup  DMA
 *  @{
 */

/** @defgroup  DMA_Public_Types
 *  @{
 */

/**
 *  @brief DMA port type definition
 */
typedef enum {
    DMA0_ID,    /*!< DMA0 port define,WLSYS,8 channels */
    DMA_ID_MAX, /*!< DMA MAX ID define */
} DMA_ID_Type;

/**
 *  @brief DMA endian type definition
 */
typedef enum {
    DMA_LITTLE_ENDIAN = 0, /*!< DMA use little endian */
    DMA_BIG_ENDIAN,        /*!< DMA use big endian */
} DMA_Endian_Type;

/**
 *  @brief DMA synchronization logic  type definition
 */
typedef enum {
    DMA_SYNC_LOGIC_ENABLE = 0, /*!< DMA synchronization logic enable */
    DMA_SYNC_LOGIC_DISABLE,    /*!< DMA synchronization logic disable */
} DMA_Sync_Logic_Type;

/**
 *  @brief DMA transfer width type definition
 */
typedef enum {
    DMA_TRNS_WIDTH_8BITS = 0, /*!< DMA transfer width:8 bits */
    DMA_TRNS_WIDTH_16BITS,    /*!< DMA transfer width:16 bits */
    DMA_TRNS_WIDTH_32BITS,    /*!< DMA transfer width:32 bits */
    DMA_TRNS_WIDTH_64BITS,    /*!< DMA transfer width:64 bits, only for DMA2 channel 0 and channel 1, others should not use this */
} DMA_Trans_Width_Type;

/**
 *  @brief DMA transfer direction type definition
 */
typedef enum {
    DMA_TRNS_M2M = 0, /*!< DMA transfer tyep:memory to memory */
    DMA_TRNS_M2P,     /*!< DMA transfer tyep:memory to peripheral */
    DMA_TRNS_P2M,     /*!< DMA transfer tyep:peripheral to memory */
    DMA_TRNS_P2P,     /*!< DMA transfer tyep:peripheral to peripheral */
} DMA_Trans_Dir_Type;

/**
 *  @brief DMA burst size type definition
 */
typedef enum {
    DMA_BURST_SIZE_1 = 0, /*!< DMA burst size:1 * transfer width */
    DMA_BURST_SIZE_4,     /*!< DMA burst size:4 * transfer width */
    DMA_BURST_SIZE_8,     /*!< DMA burst size:8 * transfer width */
    DMA_BURST_SIZE_16,    /*!< DMA burst size:16 * transfer width */
} DMA_Burst_Size_Type;

/**
 *  @brief DMA destination peripheral type definition
 */
typedef enum {
    DMA_REQ_UART0_RX = 0,  /*!< DMA request peripheral:UART0 RX */
    DMA_REQ_UART0_TX = 1,  /*!< DMA request peripheral:UART0 TX */
    DMA_REQ_UART1_RX = 2,  /*!< DMA request peripheral:UART1 RX */
    DMA_REQ_UART1_TX = 3,  /*!< DMA request peripheral:UART1 TX */
    DMA_REQ_I2C0_RX = 6,   /*!< DMA request peripheral:I2C0 RX */
    DMA_REQ_I2C0_TX = 7,   /*!< DMA request peripheral:I2C0 TX */
    DMA_REQ_GPIO_TX = 9,   /*!< DMA request peripheral:GPIO TX */
    DMA_REQ_SPI0_RX = 10,  /*!< DMA request peripheral:SPI0 RX */
    DMA_REQ_SPI0_TX = 11,  /*!< DMA request peripheral:SPI0 TX */
    DMA_REQ_AUDIO_TX = 13, /*!< DMA request peripheral:AUDIO TX */
    DMA_REQ_I2C1_RX = 14,  /*!< DMA request peripheral:I2C1 RX */
    DMA_REQ_I2C1_TX = 15,  /*!< DMA request peripheral:I2C1 TX */
    DMA_REQ_I2S_RX = 16,   /*!< DMA request peripheral:I2S RX */
    DMA_REQ_I2S_TX = 17,   /*!< DMA request peripheral:I2S TX */
    DMA_REQ_DBI_TX = 20,   /*!< DMA request peripheral:DBI TX */
    DMA_REQ_SOLO_RX = 21,  /*!< DMA request peripheral:SOLO RX */
    DMA_REQ_GPADC_RX = 22, /*!< DMA request peripheral:GPADC RX */
    DMA_REQ_GPDAC_TX = 23, /*!< DMA request peripheral:GPDAC TX */
    DMA_REQ_PEC0_RX = 24,  /*!< DMA request peripheral:PEC0 RX */
    DMA_REQ_PEC1_RX = 25,  /*!< DMA request peripheral:PEC1 RX */
    DMA_REQ_PEC2_RX = 26,  /*!< DMA request peripheral:PEC2 RX */
    DMA_REQ_PEC3_RX = 27,  /*!< DMA request peripheral:PEC3 RX */
    DMA_REQ_PEC0_TX = 28,  /*!< DMA request peripheral:PEC0 TX */
    DMA_REQ_PEC1_TX = 29,  /*!< DMA request peripheral:PEC1 TX */
    DMA_REQ_PEC2_TX = 30,  /*!< DMA request peripheral:PEC2 TX */
    DMA_REQ_PEC3_TX = 31,  /*!< DMA request peripheral:PEC3 TX */
    DMA_REQ_NONE = 0,      /*!< DMA request peripheral:None */
} DMA_Periph_Req_Type;

/**
 *  @brief DMA channel type definition
 */
typedef enum {
    DMA_CH0 = 0, /*!< DMA channel 0 */
    DMA_CH1,     /*!< DMA channel 1 */
    DMA_CH2,     /*!< DMA channel 2 */
    DMA_CH3,     /*!< DMA channel 3 */
    DMA_CH_MAX,  /*!<  */
} DMA_Chan_Type;

/**
 *  @brief DMA LLI Structure PING-PONG
 */
typedef enum {
    PING_INDEX = 0, /*!< PING INDEX */
    PONG_INDEX,     /*!< PONG INDEX */
} DMA_LLI_PP_Index_Type;

/**
 *  @brief DMA interrupt type definition
 */
typedef enum {
    DMA_INT_TCOMPLETED = 0, /*!< DMA completed interrupt */
    DMA_INT_ERR,            /*!< DMA error interrupt */
    DMA_INT_ALL,            /*!< All the interrupt */
} DMA_INT_Type;

/**
 *  @brief DMA Configuration Structure type definition
 */
typedef struct
{
    DMA_Endian_Type endian;        /*!< DMA endian type */
    DMA_Sync_Logic_Type syncLogic; /*!< DMA synchronization logic */
} DMA_Cfg_Type;

/**
 *  @brief DMA channel Configuration Structure type definition
 */
typedef struct
{
    uint32_t srcDmaAddr;                 /*!< Source address of DMA transfer */
    uint32_t destDmaAddr;                /*!< Destination address of DMA transfer */
    uint32_t transfLength;               /*!< Transfer length, 0~4095, this is burst count */
    DMA_Trans_Dir_Type dir;              /*!< Transfer dir control. 0: Memory to Memory, 1: Memory to peripheral, 2: Peripheral to memory */
    DMA_Chan_Type ch;                    /*!< Channel select 0-7 */
    DMA_Trans_Width_Type srcTransfWidth; /*!< Transfer width. 0: 8  bits, 1: 16  bits, 2: 32  bits, 3: 64  bits(only for DMA2 channel 0 and
                                                 channel 1) */
    DMA_Trans_Width_Type dstTransfWidth; /*!< Transfer width. 0: 8  bits, 1: 16  bits, 2: 32  bits, 3: 64  bits(only for DMA2 channel 0 and
                                                 channel 1) */
    DMA_Burst_Size_Type srcBurstSize;    /*!< Number of data items for burst transaction length. Each item width is as same as tansfer width.
                                                 0: 1 item, 1: 4 items, 2: 8 items, 3: 16 items */
    DMA_Burst_Size_Type dstBurstSize;    /*!< Number of data items for burst transaction length. Each item width is as same as tansfer width.
                                                 0: 1 item, 1: 4 items, 2: 8 items, 3: 16 items */
    QCC74x_Fun_Type dstAddMode;              /*!<  */
    QCC74x_Fun_Type dstMinMode;              /*!<  */
    uint8_t fixCnt;                      /*!<  */
    uint8_t srcAddrInc;                  /*!< Source address increment. 0: No change, 1: Increment */
    uint8_t destAddrInc;                 /*!< Destination address increment. 0: No change, 1: Increment */
    DMA_Periph_Req_Type srcPeriph;       /*!< Source peripheral select */
    DMA_Periph_Req_Type dstPeriph;       /*!< Destination peripheral select */
} DMA_Channel_Cfg_Type;

/**
 *  @brief DMA LLI control structure type definition
 */
typedef struct
{
    uint32_t srcDmaAddr;            /*!< Source address of DMA transfer */
    uint32_t destDmaAddr;           /*!< Destination address of DMA transfer */
    uint32_t nextLLI;               /*!< Next LLI address */
    struct DMA_Control_Reg dmaCtrl; /*!< DMA transaction control */
} DMA_LLI_Ctrl_Type;

/**
 *  @brief DMA LLI configuration structure type definition
 */
typedef struct
{
    DMA_Trans_Dir_Type dir;        /*!< Transfer dir control. 0: Memory to Memory, 1: Memory to peripheral, 2: Peripheral to memory */
    DMA_Periph_Req_Type srcPeriph; /*!< Source peripheral select */
    DMA_Periph_Req_Type dstPeriph; /*!< Destination peripheral select */
} DMA_LLI_Cfg_Type;

/**
 *  @brief DMA LLI Ping-Pong Buf definition
 */
typedef struct
{
    uint8_t idleIndex;                             /*!< Index Idle lliListHeader */
    uint8_t dmaId;                                 /*!< DMA ID used */
    uint8_t dmaChan;                               /*!< DMA LLI Channel used */
    DMA_LLI_Ctrl_Type *lliListHeader[2];           /*!< Ping-Pong BUf List Header */
    void (*onTransCompleted)(DMA_LLI_Ctrl_Type *); /*!< Completed Transmit One List Callback Function */
} DMA_LLI_PP_Buf;

/**
 *  @brief DMA LLI Ping-Pong Structure definition
 */
typedef struct
{
    uint8_t pingpongIndex;                /*!< Ping or Pong Trigger TC */
    uint8_t dmaId;                        /*!< DMA ID used */
    uint8_t dmaChan;                      /*!< DMA LLI Channel used */
    struct DMA_Control_Reg dmaCtrlRegVal; /*!< DMA Basic Pararmeter */
    DMA_LLI_Cfg_Type *lliCfg;             /*!< LLI Config parameter */
    uint32_t operatePeriphAddr;           /*!< Operate Peripheral register address */
    uint32_t pingpongBufAddr[2];          /*!< Ping-Pong addr */
} DMA_LLI_PP_Struct;

/*@} end of group DMA_Public_Types */

/** @defgroup  DMA_Public_Constants
 *  @{
 */

/** @defgroup  DMA_ID_TYPE
 *  @{
 */
#define IS_DMA_ID_TYPE(type) (((type) == DMA0_ID) || \
                              ((type) == DMA_ID_MAX))

/** @defgroup  DMA_ENDIAN_TYPE
 *  @{
 */
#define IS_DMA_ENDIAN_TYPE(type) (((type) == DMA_LITTLE_ENDIAN) || \
                                  ((type) == DMA_BIG_ENDIAN))

/** @defgroup  DMA_SYNC_LOGIC_TYPE
 *  @{
 */
#define IS_DMA_SYNC_LOGIC_TYPE(type) (((type) == DMA_SYNC_LOGIC_ENABLE) || \
                                      ((type) == DMA_SYNC_LOGIC_DISABLE))

/** @defgroup  DMA_TRANS_WIDTH_TYPE
 *  @{
 */
#define IS_DMA_TRANS_WIDTH_TYPE(type) (((type) == DMA_TRNS_WIDTH_8BITS) ||  \
                                       ((type) == DMA_TRNS_WIDTH_16BITS) || \
                                       ((type) == DMA_TRNS_WIDTH_32BITS) || \
                                       ((type) == DMA_TRNS_WIDTH_64BITS))

/** @defgroup  DMA_TRANS_DIR_TYPE
 *  @{
 */
#define IS_DMA_TRANS_DIR_TYPE(type) (((type) == DMA_TRNS_M2M) || \
                                     ((type) == DMA_TRNS_M2P) || \
                                     ((type) == DMA_TRNS_P2M) || \
                                     ((type) == DMA_TRNS_P2P))

/** @defgroup  DMA_BURST_SIZE_TYPE
 *  @{
 */
#define IS_DMA_BURST_SIZE_TYPE(type) (((type) == DMA_BURST_SIZE_1) || \
                                      ((type) == DMA_BURST_SIZE_4) || \
                                      ((type) == DMA_BURST_SIZE_8) || \
                                      ((type) == DMA_BURST_SIZE_16))

/** @defgroup  DMA_PERIPH_REQ_TYPE
 *  @{
 */
#define IS_DMA_PERIPH_REQ_TYPE(type) (((type) == DMA_REQ_UART0_RX) || \
                                      ((type) == DMA_REQ_UART0_TX) || \
                                      ((type) == DMA_REQ_UART1_RX) || \
                                      ((type) == DMA_REQ_UART1_TX) || \
                                      ((type) == DMA_REQ_UART2_RX) || \
                                      ((type) == DMA_REQ_UART2_TX) || \
                                      ((type) == DMA_REQ_I2C0_RX) ||  \
                                      ((type) == DMA_REQ_I2C0_TX) ||  \
                                      ((type) == DMA_REQ_IR_TX) ||    \
                                      ((type) == DMA_REQ_GPIO_TX) ||  \
                                      ((type) == DMA_REQ_SPI0_RX) ||  \
                                      ((type) == DMA_REQ_SPI0_TX) ||  \
                                      ((type) == DMA_REQ_AUDIO_RX) || \
                                      ((type) == DMA_REQ_AUDIO_TX) || \
                                      ((type) == DMA_REQ_I2C1_RX) ||  \
                                      ((type) == DMA_REQ_I2C1_TX) ||  \
                                      ((type) == DMA_REQ_I2S_RX) ||   \
                                      ((type) == DMA_REQ_I2S_TX) ||   \
                                      ((type) == DMA_REQ_PDM_RX) ||   \
                                      ((type) == DMA_REQ_GPADC_RX) || \
                                      ((type) == DMA_REQ_GPADC_TX) || \
                                      ((type) == DMA_REQ_UART3_RX) || \
                                      ((type) == DMA_REQ_UART3_TX) || \
                                      ((type) == DMA_REQ_SPI1_RX) ||  \
                                      ((type) == DMA_REQ_SPI1_TX) ||  \
                                      ((type) == DMA_REQ_UART4_RX) || \
                                      ((type) == DMA_REQ_UART4_TX) || \
                                      ((type) == DMA_REQ_I2C2_RX) ||  \
                                      ((type) == DMA_REQ_I2C2_TX) ||  \
                                      ((type) == DMA_REQ_I2C3_RX) ||  \
                                      ((type) == DMA_REQ_I2C3_TX) ||  \
                                      ((type) == DMA_REQ_DSI_RX) ||   \
                                      ((type) == DMA_REQ_DBI_TX) ||   \
                                      ((type) == DMA_REQ_DBI_TX) ||   \
                                      ((type) == DMA_REQ_NONE))

/** @defgroup  DMA_CHAN_TYPE
 *  @{
 */
#define IS_DMA_CHAN_TYPE(type) (((type) == DMA_CH0) || \
                                ((type) == DMA_CH1) || \
                                ((type) == DMA_CH2) || \
                                ((type) == DMA_CH3) || \
                                ((type) == DMA_CH4) || \
                                ((type) == DMA_CH5) || \
                                ((type) == DMA_CH6) || \
                                ((type) == DMA_CH7) || \
                                ((type) == DMA_CH_MAX))

/** @defgroup  DMA_LLI_PP_INDEX_TYPE
 *  @{
 */
#define IS_DMA_LLI_PP_INDEX_TYPE(type) (((type) == PING_INDEX) || \
                                        ((type) == PONG_INDEX))

/** @defgroup  DMA_INT_TYPE
 *  @{
 */
#define IS_DMA_INT_TYPE(type) (((type) == DMA_INT_TCOMPLETED) || \
                               ((type) == DMA_INT_ERR) ||        \
                               ((type) == DMA_INT_ALL))

/*@} end of group DMA_Public_Constants */

/** @defgroup  DMA_Public_Macros
 *  @{
 */
#define DMA_PINC_ENABLE  1
#define DMA_PINC_DISABLE 0
#define DMA_MINC_ENABLE  1
#define DMA_MINC_DISABLE 0

/*@} end of group DMA_Public_Macros */

/** @defgroup  DMA_Public_Functions
 *  @{
 */

/**
 *  @brief DMA Functions
 */
#ifndef QCC74x_USE_HAL_DRIVER
void DMA0_ALL_IRQHandler(void);
void DMA1_ALL_IRQHandler(void);
void DMA2_INT0_IRQHandler(void);
void DMA2_INT1_IRQHandler(void);
void DMA2_INT2_IRQHandler(void);
void DMA2_INT3_IRQHandler(void);
void DMA2_INT4_IRQHandler(void);
void DMA2_INT5_IRQHandler(void);
void DMA2_INT6_IRQHandler(void);
void DMA2_INT7_IRQHandler(void);
#endif
void msp_DMA_Enable(DMA_ID_Type dmaId);
void msp_DMA_Disable(DMA_ID_Type dmaId);
void msp_DMA_Channel_Init(DMA_ID_Type dmaId, DMA_Channel_Cfg_Type *chCfg);
void msp_DMA_DeInit(DMA_ID_Type dmaId);
void msp_DMA_Channel_Update_SrcMemcfg(DMA_ID_Type dmaId, uint8_t ch, uint32_t memAddr, uint32_t len);
void msp_DMA_Channel_Update_DstMemcfg(DMA_ID_Type dmaId, uint8_t ch, uint32_t memAddr, uint32_t len);
uint32_t msp_DMA_Channel_TranferSize(DMA_ID_Type dmaId, uint8_t ch);
QCC74x_Sts_Type msp_DMA_Channel_Is_Busy(DMA_ID_Type dmaId, uint8_t ch);
void msp_DMA_Channel_Enable(DMA_ID_Type dmaId, uint8_t ch);
void msp_DMA_Channel_Disable(DMA_ID_Type dmaId, uint8_t ch);
void msp_DMA_Request_Enable(DMA_ID_Type dmaId, uint8_t ch);
void msp_DMA_Request_Disable(DMA_ID_Type dmaId, uint8_t ch);
uint32_t msp_DMA_SrcAddr_Get(DMA_ID_Type dmaId, uint8_t ch);
void msp_DMA_LLI_Init(DMA_ID_Type dmaId, uint8_t ch, DMA_LLI_Cfg_Type *lliCfg);
void msp_DMA_LLI_Update(DMA_ID_Type dmaId, uint8_t ch, uint32_t LLI);
uint32_t msp_DMA_LLI_Get_Counter(DMA_ID_Type dmaId, uint8_t ch);
void msp_DMA_IntMask(DMA_ID_Type dmaId, uint8_t ch, DMA_INT_Type intType, QCC74x_Mask_Type intMask);
void msp_DMA_LLI_PpBuf_Start_New_Transmit(DMA_LLI_PP_Buf *dmaPpBuf);
DMA_LLI_Ctrl_Type *msp_DMA_LLI_PpBuf_Remove_Completed_List(DMA_LLI_PP_Buf *dmaPpBuf);
void msp_DMA_LLI_PpBuf_Append(DMA_LLI_PP_Buf *dmaPpBuf, DMA_LLI_Ctrl_Type *dmaLliList);
void msp_DMA_LLI_PpBuf_Destroy(DMA_LLI_PP_Buf *dmaPpBuf);
//void DMA_Int_Callback_Install(DMA_ID_Type dmaId, DMA_Chan_Type dmaChan, DMA_INT_Type intType, intCallback_Type *cbFun);
void msp_DMA_LLI_PpStruct_Start(DMA_LLI_PP_Struct *dmaPpStruct);
void msp_DMA_LLI_PpStruct_Stop(DMA_LLI_PP_Struct *dmaPpStruct);
QCC74x_Err_Type msp_DMA_LLI_PpStruct_Init(DMA_LLI_PP_Struct *dmaPpStruct);
QCC74x_Err_Type msp_DMA_LLI_PpStruct_Set_Transfer_Len(DMA_LLI_PP_Struct *dmaPpStruct,
                                              uint16_t Ping_Transfer_len, uint16_t Pong_Transfer_len);

/*@} end of group DMA_Public_Functions */

/*@} end of group DMA */

/*@} end of group QCC743_Peripheral_Driver */

#endif /* __QCC743_DMA_H__ */