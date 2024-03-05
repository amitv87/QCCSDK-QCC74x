#include <FreeRTOS.h>
#include "task.h"

#include <app_wifi.h>
#include <app_player.h>

#include "rfparam_adapter.h"
#include "qcc743_glb.h"

void vAssertCalled(void)
{
    printf("vAssertCalled\r\n");
    __disable_irq();

    while (1);
}

void app_main_entry(void *arg)
{
    /* Init rf */
    if (0 != rfparam_init(0, NULL, 0)) {
        printf("PHY RF init failed!\r\n");
        return 0;
    }

#if CONFIG_CODEC_USE_I2S_RX || CONFIG_CODEC_USE_I2S_TX
    extern msp_i2s_port_init(void);
    msp_i2s_port_init();
#endif
    /* Init wifi */
    app_wifi_init();

    /* Init player */
    app_player_init();

    vTaskDelete(NULL);
}

