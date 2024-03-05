#include <FreeRTOS.h>
#include "task.h"

#include <app_player.h>
#include "qcc743_glb.h"

void vAssertCalled(void)
{
    printf("vAssertCalled\r\n");
    __disable_irq();

    while (1);
}

void wifi_event_handler(uint32_t code)
{
}

void app_main_entry(void *arg)
{
    /* romsfs init mount use media factory*/
    romfs_mount(0x378000);

    /* Init player */
    app_player_init();

#if CONFIG_CODEC_USE_I2S_RX || CONFIG_CODEC_USE_I2S_TX
    extern msp_i2s_port_init(void);
    msp_i2s_port_init();
#endif
    extern int app_play_local_music();
    app_play_local_music();

    vTaskDelete(NULL);
}

