#include "msp_qcc743_platform.h"
#include "msp_qcc743_auadc.h"

/** @addtogroup  QCC743_Peripheral_Driver
 *  @{
 */

/** @addtogroup  AUADC
 *  @{
 */

/** @defgroup  AUADC_Private_Macros
 *  @{
 */

/*@} end of group AUADC_Private_Macros */

/** @defgroup  AUADC_Private_Types
 *  @{
 */

/*@} end of group AUADC_Private_Types */

/** @defgroup  AUADC_Private_Variables
 *  @{
 */

/*@} end of group AUADC_Private_Variables */

/** @defgroup  AUADC_Global_Variables
 *  @{
 */
#if 0
static intCallback_Type *auadcIntCbfArra[AUADC_INT_NUM_ALL] = { NULL };
#endif

/*@} end of group AUADC_Global_Variables */

/** @defgroup  AUADC_Private_Fun_Declaration
 *  @{
 */

/*@} end of group AUADC_Private_Fun_Declaration */

/** @defgroup  AUADC_Private_Functions
 *  @{
 */

/*@} end of group AUADC_Private_Functions */

/** @defgroup  AUADC_Public_Functions
 *  @{
 */

/****************************************************************************/ /**
 * @brief  Init AUADC adc module
 *
 * @param  cfg: cfg
 *
 * @return None
 *
*******************************************************************************/
void AUADC_Init(AUADC_Cfg_Type *cfg)
{
    uint32_t tmpVal = 0;

    /* set fir mode , select one order or two order fir filter */
    tmpVal = QCC74x_RD_REG(AUADC_BASE, AUADC_PDM_ADC_0);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_ADC_0_FIR_MODE, cfg->firMode);
    QCC74x_WR_REG(AUADC_BASE, AUADC_PDM_ADC_0, tmpVal);

    if (cfg->source == AUADC_SOURCE_ANALOG) {
        /*set adc source analog */
        tmpVal = QCC74x_RD_REG(AUADC_BASE, AUADC_PDM_DAC_0);
        tmpVal = QCC74x_CLR_REG_BIT(tmpVal, AUADC_ADC_0_SRC);
        QCC74x_WR_REG(AUADC_BASE, AUADC_PDM_DAC_0, tmpVal);
        /* disable pdm */
        tmpVal = QCC74x_RD_REG(AUADC_BASE, AUADC_PDM_PDM_0);
        tmpVal = QCC74x_CLR_REG_BIT(tmpVal, AUADC_PDM_0_EN);
        QCC74x_WR_REG(AUADC_BASE, AUADC_PDM_PDM_0, tmpVal);

    } else {
        /*set adc source pdm */
        tmpVal = QCC74x_RD_REG(AUADC_BASE, AUADC_PDM_DAC_0);
        tmpVal = QCC74x_SET_REG_BIT(tmpVal, AUADC_ADC_0_SRC);
        QCC74x_WR_REG(AUADC_BASE, AUADC_PDM_DAC_0, tmpVal);
        /* set pdm channel */
        tmpVal = QCC74x_RD_REG(AUADC_BASE, AUADC_PDM_PDM_0);
        tmpVal = QCC74x_SET_REG_BIT(tmpVal, AUADC_PDM_0_EN);
        if (cfg->source == AUADC_SOURCE_PDM_LEFT) {
            tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_ADC_0_PDM_SEL, 0);
        } else {
            tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_ADC_0_PDM_SEL, 1);
        }
        QCC74x_WR_REG(AUADC_BASE, AUADC_PDM_PDM_0, tmpVal);
    }

    /* Set Clock */
    tmpVal = QCC74x_RD_REG(AUADC_BASE, AUADC_AUDPDM_TOP);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_ADC_RATE, cfg->clk);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_PDM_ITF_INV_SEL, cfg->pdmItfInvEnable);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_ADC_ITF_INV_SEL, cfg->adcItfInvEnable);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_AUDIO_CKG_EN, cfg->auadcClkEnable);
    QCC74x_WR_REG(AUADC_BASE, AUADC_AUDPDM_TOP, tmpVal);

    /* To avoid excessive PDM frequency, the OSR was reduced from 128 to 64 */
    tmpVal = QCC74x_RD_REG(AUADC_BASE, AUADC_AUDADC_CMD);
    if (cfg->source != AUADC_SOURCE_ANALOG && cfg->clk >= AUADC_CLK_32K_HZ) {
        tmpVal = QCC74x_SET_REG_BIT(tmpVal, AUADC_AUDADC_AUDIO_OSR_SEL);
    } else {
        tmpVal = QCC74x_CLR_REG_BIT(tmpVal, AUADC_AUDADC_AUDIO_OSR_SEL);
    }
    QCC74x_WR_REG(AUADC_BASE, AUADC_AUDADC_CMD, tmpVal);

#if 0//ndef QCC74x_USE_HAL_DRIVER
    Interrupt_Handler_Register(AUPDM_IRQn, AUADC_IRQHandler);
#endif
}

/****************************************************************************/ /**
 * @brief  Init AUADC adc fifo
 *
 * @param  cfg: cfg
 *
 * @return None
 *
*******************************************************************************/
void AUADC_FifoInit(AUADC_FifoCfg_Type *cfg)
{
    uint32_t tmpVal = 0;

    CHECK_PARAM(IS_AUADC_RESOLUTION_TYPE(cfg->resolution));
    CHECK_PARAM(IS_AUADC_FIFO_AILGN_MODE(cfg->ailgnMode));
    CHECK_PARAM(IS_AUADC_FIFO_DQR_THRESHOLD_MODE(cfg->dmaThresholdMode));

    tmpVal = QCC74x_RD_REG(AUADC_BASE, AUADC_AUDADC_RX_FIFO_CTRL);

    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_RX_DATA_RES, cfg->resolution);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_RX_DATA_MODE, cfg->ailgnMode);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_RX_DRQ_CNT, cfg->dmaThresholdMode);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_RX_TRG_LEVEL, cfg->FifoIntThreshold);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_RX_DRQ_EN, cfg->dmaEn);

    QCC74x_WR_REG(AUADC_BASE, AUADC_AUDADC_RX_FIFO_CTRL, tmpVal);

    /* Set dma interface */
    tmpVal = QCC74x_RD_REG(AUADC_BASE, AUADC_AUDPDM_ITF);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_ADC_ITF_EN, cfg->dmaEn);
    QCC74x_WR_REG(AUADC_BASE, AUADC_AUDPDM_ITF, tmpVal);
}

/****************************************************************************/ /**
 * @brief  Init clear AUADC adc fifo
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void AUADC_FifoClear(void)
{
    uint32_t tmpVal = 0;

    tmpVal = QCC74x_RD_REG(AUADC_BASE, AUADC_AUDADC_RX_FIFO_CTRL);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_RX_FIFO_FLUSH, 1);
    QCC74x_WR_REG(AUADC_BASE, AUADC_AUDADC_RX_FIFO_CTRL, tmpVal);
}

/****************************************************************************/ /**
 * @brief  Enable Auadc
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void AUADC_Enable(void)
{
    uint32_t tmpVal = 0;

    tmpVal = QCC74x_RD_REG(AUADC_BASE, AUADC_AUDPDM_ITF);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_ADC_0_EN, 1);
    QCC74x_WR_REG(AUADC_BASE, AUADC_AUDPDM_ITF, tmpVal);
}

/****************************************************************************/ /**
 * @brief  Disable Auadc
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void AUADC_Disable(void)
{
    uint32_t tmpVal = 0;

    tmpVal = QCC74x_RD_REG(AUADC_BASE, AUADC_AUDPDM_ITF);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_ADC_0_EN, 0);
    QCC74x_WR_REG(AUADC_BASE, AUADC_AUDPDM_ITF, tmpVal);
}

/****************************************************************************/ /**
 * @brief  Disable FIFO Interface
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void AUADC_Start(void)
{
    uint32_t tmpVal = 0;

    tmpVal = QCC74x_RD_REG(AUADC_BASE, AUADC_AUDADC_RX_FIFO_CTRL);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_RX_CH_EN, 1);
    QCC74x_WR_REG(AUADC_BASE, AUADC_AUDADC_RX_FIFO_CTRL, tmpVal);
}

/****************************************************************************/ /**
 * @brief  Enable FIFO Interface
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
void AUADC_Stop(void)
{
    uint32_t tmpVal = 0;

    tmpVal = QCC74x_RD_REG(AUADC_BASE, AUADC_AUDADC_RX_FIFO_CTRL);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_RX_CH_EN, 0);
    QCC74x_WR_REG(AUADC_BASE, AUADC_AUDADC_RX_FIFO_CTRL, tmpVal);
}

/****************************************************************************/ /**
 * @brief  Config HPF
 *
 * @param  k1_enable: k1 ENABLE
 * @param  k1: k1 paramater
 * @param  k2_enable: k2 Enable
 * @param  k2: k2 paramater
 *
 * @return None
 *
*******************************************************************************/
void AUADC_HPFConfig(uint8_t k1_enable, uint8_t k1, uint8_t k2_enable, uint8_t k2)
{
    uint32_t tmpVal = 0;

    tmpVal = QCC74x_RD_REG(AUADC_BASE, AUADC_PDM_ADC_1);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_ADC_0_K1, k1);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_ADC_0_K1_EN, k1_enable);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_ADC_0_K2, k2_enable);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_ADC_0_K2_EN, k2);

    QCC74x_WR_REG(AUADC_BASE, AUADC_PDM_ADC_1, tmpVal);
}

/****************************************************************************/ /**
 * @brief  set adc volume
 *
 * @param  volume: volume
 *
 * @return None
 *
*******************************************************************************/
void AUADC_SetVolume(uint32_t volume)
{
    uint32_t tmpVal = 0;

    tmpVal = QCC74x_RD_REG(AUADC_BASE, AUADC_PDM_ADC_S0);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_ADC_S0_VOLUME, volume);
    QCC74x_WR_REG(AUADC_BASE, AUADC_PDM_ADC_S0, tmpVal);
}

/****************************************************************************/ /**
 * @brief  set auadc int mask
 *
 * @param  intType: intType
 * @param  intMask: intMask
 *
 * @return None
 *
*******************************************************************************/
void AUADC_IntMask(AUADC_INT_Type intType, QCC74x_Mask_Type intMask)
{
    uint32_t tmpVal = 0;

    CHECK_PARAM(IS_AUADC_INT_TYPE(intType));
    CHECK_PARAM(IS_QCC74x_MASK_TYPE(intMask));

    switch (intType) {
        case AUADC_INT_RX_FIFO_THR:
            tmpVal = QCC74x_RD_REG(AUADC_BASE, AUADC_AUDADC_RX_FIFO_CTRL);
            if (intMask) {
                tmpVal = QCC74x_CLR_REG_BIT(tmpVal, AUADC_RXA_INT_EN);
            } else {
                tmpVal = QCC74x_SET_REG_BIT(tmpVal, AUADC_RXA_INT_EN);
            }
            QCC74x_WR_REG(AUADC_BASE, AUADC_AUDADC_RX_FIFO_CTRL, tmpVal);
            break;
        case AUADC_INT_RX_FIFO_OVERRUN:
            tmpVal = QCC74x_RD_REG(AUADC_BASE, AUADC_AUDADC_RX_FIFO_CTRL);
            if (intMask) {
                tmpVal = QCC74x_CLR_REG_BIT(tmpVal, AUADC_RXO_INT_EN);
            } else {
                tmpVal = QCC74x_SET_REG_BIT(tmpVal, AUADC_RXO_INT_EN);
            }
            QCC74x_WR_REG(AUADC_BASE, AUADC_AUDADC_RX_FIFO_CTRL, tmpVal);
            break;
        case AUADC_INT_RX_FIFO_UNDERRUN:
            tmpVal = QCC74x_RD_REG(AUADC_BASE, AUADC_AUDADC_RX_FIFO_CTRL);
            if (intMask) {
                tmpVal = QCC74x_CLR_REG_BIT(tmpVal, AUADC_RXU_INT_EN);
            } else {
                tmpVal = QCC74x_SET_REG_BIT(tmpVal, AUADC_RXU_INT_EN);
            }
            QCC74x_WR_REG(AUADC_BASE, AUADC_AUDADC_RX_FIFO_CTRL, tmpVal);
            break;

        case AUADC_INT_NUM_ALL:
            if (intMask) {
                tmpVal = QCC74x_RD_REG(AUADC_BASE, AUADC_AUDADC_RX_FIFO_CTRL);
                tmpVal = QCC74x_CLR_REG_BIT(tmpVal, AUADC_RXA_INT_EN);
                tmpVal = QCC74x_CLR_REG_BIT(tmpVal, AUADC_RXU_INT_EN);
                tmpVal = QCC74x_CLR_REG_BIT(tmpVal, AUADC_RXO_INT_EN);
                QCC74x_WR_REG(AUADC_BASE, AUADC_AUDADC_RX_FIFO_CTRL, tmpVal);

            } else {
                tmpVal = QCC74x_RD_REG(AUADC_BASE, AUADC_AUDADC_RX_FIFO_CTRL);
                tmpVal = QCC74x_SET_REG_BIT(tmpVal, AUADC_RXA_INT_EN);
                tmpVal = QCC74x_SET_REG_BIT(tmpVal, AUADC_RXU_INT_EN);
                tmpVal = QCC74x_SET_REG_BIT(tmpVal, AUADC_RXO_INT_EN);
                QCC74x_WR_REG(AUADC_BASE, AUADC_AUDADC_RX_FIFO_CTRL, tmpVal);
            }

        default:
            break;
    }
}

/****************************************************************************/ /**
 * @brief  clear auadc int flag
 *
 * @param  intType: intType
 *
 * @return None
 *
*******************************************************************************/
void AUADC_IntClear(AUADC_INT_Type intType)
{
    uint32_t tmpVal = 0;

    CHECK_PARAM(IS_AUADC_INT_TYPE(intType));

    switch (intType) {
        case AUADC_INT_RX_FIFO_THR:
            tmpVal = QCC74x_RD_REG(AUADC_BASE, AUADC_AUDADC_RX_FIFO_STATUS);
            tmpVal = QCC74x_SET_REG_BIT(tmpVal, AUADC_RXA_INT);
            QCC74x_WR_REG(AUADC_BASE, AUADC_AUDADC_RX_FIFO_STATUS, tmpVal);
            break;

        case AUADC_INT_RX_FIFO_OVERRUN:
            tmpVal = QCC74x_RD_REG(AUADC_BASE, AUADC_AUDADC_RX_FIFO_STATUS);
            tmpVal = QCC74x_SET_REG_BIT(tmpVal, AUADC_RXO_INT);
            QCC74x_WR_REG(AUADC_BASE, AUADC_AUDADC_RX_FIFO_STATUS, tmpVal);
            break;

        case AUADC_INT_RX_FIFO_UNDERRUN:
            tmpVal = QCC74x_RD_REG(AUADC_BASE, AUADC_AUDADC_RX_FIFO_STATUS);
            tmpVal = QCC74x_SET_REG_BIT(tmpVal, AUADC_RXU_INT);
            QCC74x_WR_REG(AUADC_BASE, AUADC_AUDADC_RX_FIFO_STATUS, tmpVal);
            break;

        case AUADC_INT_NUM_ALL:

            tmpVal = QCC74x_RD_REG(AUADC_BASE, AUADC_AUDADC_RX_FIFO_STATUS);
            tmpVal = QCC74x_SET_REG_BIT(tmpVal, AUADC_RXO_INT);
            tmpVal = QCC74x_SET_REG_BIT(tmpVal, AUADC_RXU_INT);
            tmpVal = QCC74x_SET_REG_BIT(tmpVal, AUADC_RXA_INT);
            QCC74x_WR_REG(AUADC_BASE, AUADC_AUDADC_RX_FIFO_STATUS, tmpVal);
            break;

        default:
            break;
    }
}

#if 0
/****************************************************************************/ /**
 * @brief  register callback function
 *
 * @param  intType: intType
 * @param  cbFun: cbFun
 *
 * @return success or not
 *
*******************************************************************************/
QCC74x_Err_Type AUADC_Int_Callback_Install(AUADC_INT_Type intType, intCallback_Type *cbFun)
{
    CHECK_PARAM(IS_AUADC_INT_TYPE(intType));

    auadcIntCbfArra[intType] = cbFun;

    return SUCCESS;
}
#endif

/****************************************************************************/ /**
 * @brief  get int status
 *
 * @param  intType: intType
 *
 * @return interrupt flag status
 *
*******************************************************************************/
QCC74x_Sts_Type AUADC_GetIntStatus(AUADC_INT_Type intType)
{
    uint32_t tmpVal = 0;
    QCC74x_Sts_Type rlt = RESET;

    CHECK_PARAM(IS_AUADC_INT_TYPE(intType));

    switch (intType) {
        case AUADC_INT_RX_FIFO_THR:
            tmpVal = QCC74x_RD_REG(AUADC_BASE, AUADC_AUDADC_RX_FIFO_STATUS);
            rlt = QCC74x_IS_REG_BIT_SET(tmpVal, AUADC_RXA_INT);
            break;
        case AUADC_INT_RX_FIFO_OVERRUN:
            tmpVal = QCC74x_RD_REG(AUADC_BASE, AUADC_AUDADC_RX_FIFO_STATUS);
            rlt = QCC74x_IS_REG_BIT_SET(tmpVal, AUADC_RXO_INT);
            break;
        case AUADC_INT_RX_FIFO_UNDERRUN:
            tmpVal = QCC74x_RD_REG(AUADC_BASE, AUADC_AUDADC_RX_FIFO_STATUS);
            rlt = QCC74x_IS_REG_BIT_SET(tmpVal, AUADC_RXU_INT);
            break;

        default:
            break;
    }

    return rlt;
}

/****************************************************************************/ /**
 * @brief  Get Rx FIFO Count
 *
 * @param  None
 *
 * @return fifi count
 *
*******************************************************************************/
uint32_t AUADC_GetFifoCount(void)
{
    uint32_t tmpVal = 0;

    tmpVal = QCC74x_RD_REG(AUADC_BASE, AUADC_AUDADC_RX_FIFO_STATUS);
    tmpVal = QCC74x_GET_REG_BITS_VAL(tmpVal, AUADC_RXA_CNT);

    return tmpVal;
}

/****************************************************************************/ /**
 * @brief  Get Rx data
 *
 * @param  None
 *
 * @return raw data
 *
*******************************************************************************/
uint32_t AUADC_GetRawData(void)
{
    uint32_t tmpVal = 0;

    tmpVal = QCC74x_RD_REG(AUADC_BASE, AUADC_PDM_DAC_0);
    tmpVal = QCC74x_GET_REG_BITS_VAL(tmpVal, AUADC_ADC_0_SRC);

    if (tmpVal) {
        /* pdm interface */
        tmpVal = QCC74x_RD_REG(AUADC_BASE, AUADC_AUDADC_RX_FIFO_DATA);
    } else {
        /*analog interface */
        tmpVal = QCC74x_RD_REG(AUADC_BASE, AUADC_AUDADC_DATA);
        tmpVal = QCC74x_GET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_RAW_DATA);
    }

    return tmpVal;
}

/****************************************************************************/ /**
 * @brief  Get Rx data is ready or not
 *
 * @param  None
 *
 * @return reay or not
 *
*******************************************************************************/
QCC74x_Sts_Type AUADC_FifoDataReady(void)
{
    uint32_t tmpVal = 0;

    tmpVal = QCC74x_RD_REG(AUADC_BASE, AUADC_AUDADC_DATA);
    tmpVal = QCC74x_GET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_DATA_RDY);

    return tmpVal;
}

/****************************************************************************/ /**
 * @brief
 *
 * @param  None
 *
 * @return reay or not
 *
*******************************************************************************/
void AUADC_ADC_Config(AUADC_ADC_AnalogCfg_Type *adc_cfg)
{
    uint32_t tmpVal = 0;
    uint8_t ch_en = 0;

    if (adc_cfg == NULL) {
        return;
    }

    /* audadc_ana_cfg1 */
    tmpVal = QCC74x_RD_REG(AUADC_BASE, AUADC_AUDADC_ANA_CFG1);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_SEL_EDGE, adc_cfg->adc_edge_mode);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_CKB_EN, adc_cfg->adc_clock_phase_invert_en);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_PGA_LP_EN, adc_cfg->adc_clock_phase_invert_en);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_ICTRL_PGA_MIC, adc_cfg->pga_opmic_bias_cur);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_ICTRL_PGA_AAF, adc_cfg->pga_opaaf_bias_cur);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_PGA_RHPAS_SEL, adc_cfg->pga_hf_res);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_PGA_CHOP_CFG, adc_cfg->pga_chopper);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_PGA_CHOP_EN, adc_cfg->pga_chopper_en);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_PGA_CHOP_FREQ, adc_cfg->pga_chopper_freq);
    // tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_PGA_CHOP_CKSEL, adc_cfg->pga_chopper_clk_source);
    QCC74x_WR_REG(AUADC_BASE, AUADC_AUDADC_ANA_CFG1, tmpVal);

    /* audadc_ana_cfg2 */
    tmpVal = QCC74x_RD_REG(AUADC_BASE, AUADC_AUDADC_ANA_CFG2);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_SDM_LP_EN, adc_cfg->sdm_lowpower_en);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_ICTRL_ADC, adc_cfg->sdm_bias_cur);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_NCTRL_ADC1, adc_cfg->sdm_i_first_num);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_NCTRL_ADC2, adc_cfg->sdm_i_sec_num);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_DEM_EN, adc_cfg->dem_en);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_QUAN_GAIN, adc_cfg->sdm_qg);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_DITHER_ENA, adc_cfg->sdm_dither_en);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_DITHER_SEL, adc_cfg->sdm_dither_level);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_DITHER_ORDER, adc_cfg->sdm_dither_order);
    QCC74x_WR_REG(AUADC_BASE, AUADC_AUDADC_ANA_CFG2, tmpVal);

    /* audadc_cmd */
    tmpVal = QCC74x_RD_REG(AUADC_BASE, AUADC_AUDADC_CMD);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_PGA_PU, adc_cfg->pga_circuit_en);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_SDM_PU, adc_cfg->sdm_circuit_en);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_CONV, 0);
#if 1// fixme add new function for it
    if (adc_cfg->pga_positive_en) {
        ch_en |= (0x01 << 1);
    }
    if (adc_cfg->pga_negative_en) {
        ch_en |= (0x01 << 0);
    }
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_CHANNEL_EN, ch_en);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_CHANNEL_SELP, adc_cfg->pga_posi_ch);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_CHANNEL_SELN, adc_cfg->pga_nega_ch);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_PGA_MODE, adc_cfg->pga_coupled_mode);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_PGA_GAIN, adc_cfg->pga_gain);
#endif
    if (adc_cfg->adc_mode == AUADC_ADC_FILT_MODE_AUDIO) {
        // tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_AUDIO_FILTER_EN, 1);
        tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_MEAS_FILTER_EN, 0);
    } else if (adc_cfg->adc_mode == AUADC_ADC_FILT_MODE_MEASURE) {
        // tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_AUDIO_FILTER_EN, 0);
        tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_MEAS_FILTER_EN, 1);
    }
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_MEAS_FILTER_TYPE, adc_cfg->measure_filter_mode);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_MEAS_ODR_SEL, adc_cfg->measure_rate);
    QCC74x_WR_REG(AUADC_BASE, AUADC_AUDADC_CMD, tmpVal);
    tmpVal = QCC74x_SET_REG_BITS_VAL(tmpVal, AUADC_AUDADC_CONV, 1);
    QCC74x_WR_REG(AUADC_BASE, AUADC_AUDADC_CMD, tmpVal);
}

/****************************************************************************/ /**
 * @brief  auadc interrupt function
 *
 * @param  None
 *
 * @return None
 *
*******************************************************************************/
#if 0//ndef QCC74x_USE_HAL_DRIVER
void AUADC_IRQHandler(void)
{
    uint8_t intIndex = 0;

    for (intIndex = 0; intIndex < AUADC_INT_NUM_ALL; intIndex++) {
        if (AUADC_GetIntStatus(intIndex) == SET) {
            if (auadcIntCbfArra[intIndex] != NULL) {
                auadcIntCbfArra[intIndex]();
            }
            AUADC_IntClear(intIndex);
        }
    }
}
#endif

/*@} end of group AUADC_Public_Functions */

/*@} end of group AUADC */

/*@} end of group QCC743_Peripheral_Driver */
