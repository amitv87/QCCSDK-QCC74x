#if 0
#include <qcc74x_undefp.h>
#include <qcc74x_undefp_common.h>
#include <qcc74x_undefp_audio.h>
#include <qcc74x_undefp_glb.h>


uint32_t _audio_get_adc_analog_gain(Audio_ADC_ID_Type adc)
{
    uint32_t tmpVal = 0;

    /*ADC Power on */
    if (adc == AUDIO_ADC_0) {
        tmpVal = QCC74x_RD_REG(AUDIO_BASE, AUDIO_CODEC_ADC0);
        tmpVal = QCC74x_GET_REG_BITS_VAL(tmpVal, AUDIO_ADC0_PGA_GAIN);
    } else if (adc == AUDIO_ADC_1) {
        tmpVal = QCC74x_RD_REG(AUDIO_BASE, AUDIO_CODEC_ADC1);
        tmpVal = QCC74x_GET_REG_BITS_VAL(tmpVal, AUDIO_ADC1_PGA_GAIN);
    } else {
        tmpVal = QCC74x_RD_REG(AUDIO_BASE, AUDIO_CODEC_ADC2);
        tmpVal = QCC74x_GET_REG_BITS_VAL(tmpVal, AUDIO_ADC2_PGA_GAIN);
    }

    return tmpVal;
}

uint32_t _audio_get_dac_analog_gain(Audio_DAC_ID_Type adc)
{
    uint32_t tmpVal = 0;

    if (adc == AUDIO_DAC_0) {
        tmpVal = QCC74x_RD_REG(AUDIO_BASE, AUDIO_CODEC_DAC);
        tmpVal = QCC74x_GET_REG_BITS_VAL(tmpVal, AUDIO_DACL_LINEOUT_GAIN);
    } else {
        tmpVal = QCC74x_RD_REG(AUDIO_BASE, AUDIO_CODEC_DAC);
        tmpVal = QCC74x_GET_REG_BITS_VAL(tmpVal, AUDIO_DACR_LINEOUT_GAIN);
    }

    return tmpVal;
}


uint32_t blyoc_audio_get_volume()
{
    uint16_t val_analog[3];
    uint16_t val_digital[3];

    val_digital[0] = Audio_ADC_Get_Volume(AUDIO_ADC_0);
    val_digital[1] = Audio_ADC_Get_Volume(AUDIO_ADC_1);
    val_digital[2] = Audio_ADC_Get_Volume(AUDIO_ADC_2);
    val_analog[0]  = _audio_get_adc_analog_gain(AUDIO_ADC_0);
    val_analog[1]  = _audio_get_adc_analog_gain(AUDIO_ADC_0);
    val_analog[2]  = _audio_get_adc_analog_gain(AUDIO_ADC_0);
    printf("ad "
            "digital gain: %d %d %d "
            "analog gain: %d %d %d "
            "\r\n", val_digital[0], val_digital[1], val_digital[2],
                    (int16_t)val_analog[0], (int16_t)val_analog[1], (int16_t)val_analog[2]);

    val_digital[0] = Audio_DAC_Get_Volume(AUDIO_DAC_0);
    val_digital[1] = Audio_DAC_Get_Volume(AUDIO_DAC_1);
    val_analog[0]  = _audio_get_dac_analog_gain(AUDIO_DAC_0);
    val_analog[1]  = _audio_get_dac_analog_gain(AUDIO_DAC_1);

    printf("da "
         "digital gain: %d %d "
         "analog gain: %d %d "
         "\r\n", val_digital[0], val_digital[1],
                 (int16_t)(val_analog[0]), (int16_t)(val_analog[1]));
    return 0;
}
#endif
