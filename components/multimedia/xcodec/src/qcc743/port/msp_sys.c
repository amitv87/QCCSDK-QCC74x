#if 1
#include <msp/kernel.h>
#include <msp_qcc743_platform.h>

extern QCC74x_Err_Type GLB_Config_AUDIO_PLL_To_491P52M(void);

// audio pll config
void msp_config_audiopll(void)
{
    GLB_Config_AUDIO_PLL_To_491P52M();
}
#endif
