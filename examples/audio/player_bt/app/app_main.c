#include <FreeRTOS.h>
#include "task.h"

#include <app_bt.h>
#include <app_player.h>

#include "rfparam_adapter.h"
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

static int btblecontroller_em_config(void)
{
    extern uint8_t __LD_CONFIG_EM_SEL;
    volatile uint32_t em_size;

    em_size = (uint32_t)&__LD_CONFIG_EM_SEL;

    if (em_size == 0) {
        GLB_Set_EM_Sel(GLB_WRAM160KB_EM0KB);
    } else if (em_size == 32*1024) {
        GLB_Set_EM_Sel(GLB_WRAM128KB_EM32KB);
    } else if (em_size == 64*1024) {
        GLB_Set_EM_Sel(GLB_WRAM96KB_EM64KB);
    } else {
        GLB_Set_EM_Sel(GLB_WRAM96KB_EM64KB);
    }

    return 0;
}

void app_main_entry(void *arg)
{
    /* Set ble controller EM Size */
    btblecontroller_em_config();

    /* Init rf */
    if (0 != rfparam_init(0, NULL, 0)) {
        printf("PHY RF init failed!\r\n");
        return 0;
    }

    /* For bt status save */
    qcc74x_mtd_init();
    easyflash_init();

#if CONFIG_CODEC_USE_I2S_RX || CONFIG_CODEC_USE_I2S_TX
    extern msp_i2s_port_init(void);
    msp_i2s_port_init();
#endif
    /* Init player */
    app_player_init();

    /* Init bt */
    app_bt_init();

    vTaskDelete(NULL);
}
