#include "btblecontroller_port.h"
#if defined(CFG_IOT_SDK)
#include "qcc74x_irq.h"
#else
#include "qcc74x_irq.h"
#include "qcc74x_efuse.h"
#endif

#if defined(QCC743)
#include "qcc743_glb.h"
#include "wl_api.h"
#include "qcc743_pds.h"
#include "qcc743_mfg_media.h"
#if defined(CFG_IOT_SDK)
#include "qcc74x_efuse.h"
#include "qcc74x_efuse.h"
#endif
#define WL_API_RMEM_ADDR    0x20010600
#endif

#if defined(QCC74x_undef)
#include "qcc74x_undef_glb.h"
#endif 

#if defined(QCC74x_undefL)
#include "qcc74x_undefl_glb.h"
#endif

#if defined(CFG_IOT_SDK)
extern void qcc74x_irq_pending_clear(unsigned int source);
#endif

__attribute__((weak)) void btblecontroller_ble_irq_init(void *handler)
{
    #if defined(CFG_IOT_SDK)
    qcc74x_irq_pending_clear(BLE_IRQn);
    qcc74x_irq_register(BLE_IRQn, handler);
    qcc74x_irq_enable(BLE_IRQn);
    #else
    qcc74x_irq_clear_pending(BLE_IRQn);
    qcc74x_irq_attach(BLE_IRQn, (irq_callback)handler, NULL);
    qcc74x_irq_enable(BLE_IRQn);
    #endif
}

#if !defined(QCC74x_undefL)
__attribute__((weak)) void btblecontroller_bt_irq_init(void *handler)
{
    #if defined(CFG_IOT_SDK)
    qcc74x_irq_pending_clear(BT_IRQn);
    qcc74x_irq_register(BT_IRQn, handler);
    qcc74x_irq_enable(BT_IRQn);
    #else
    qcc74x_irq_clear_pending(BT_IRQn);
    qcc74x_irq_attach(BT_IRQn, (irq_callback)handler, NULL);
    qcc74x_irq_enable(BT_IRQn);
    #endif
}

__attribute__((weak)) void btblecontroller_dm_irq_init(void *handler)
{
    #if defined(CFG_IOT_SDK)
    qcc74x_irq_pending_clear(DM_IRQn);
    qcc74x_irq_register(DM_IRQn, handler);
    qcc74x_irq_enable(DM_IRQn);
    #else
    qcc74x_irq_clear_pending(DM_IRQn);
    qcc74x_irq_attach(DM_IRQn, (irq_callback)handler, NULL);
    qcc74x_irq_enable(DM_IRQn);
    #endif
}
#endif

__attribute__((weak)) void btblecontroller_ble_irq_enable(uint8_t enable)
{
    #if defined(CFG_IOT_SDK)
    if(enable)
    {
 
        qcc74x_irq_enable(BLE_IRQn); 
    }
    else
    {
        qcc74x_irq_disable(BLE_IRQn); 
    }
    #else
    if(enable)
    {
 
        qcc74x_irq_enable(BLE_IRQn); 
    }
    else
    {
        qcc74x_irq_disable(BLE_IRQn); 
    }
    #endif
}

#if !defined(QCC74x_undefL)
__attribute__((weak)) void btblecontroller_bt_irq_enable(uint8_t enable)
{
    #if defined(CFG_IOT_SDK)
    if(enable)
    {    
        qcc74x_irq_enable(BT_IRQn); 
    }
    else
    {
        qcc74x_irq_disable(BT_IRQn); 
    }
    #else

    if(enable)
    {
        qcc74x_irq_enable(BT_IRQn);
    }
    else
    {
        qcc74x_irq_disable(BT_IRQn);
    }
    #endif
}

__attribute__((weak)) void btblecontroller_dm_irq_enable(uint8_t enable)
{
    #if defined(CFG_IOT_SDK)
    if(enable)
    {    
        qcc74x_irq_enable(DM_IRQn); 
    }
    else
    {
        qcc74x_irq_disable(DM_IRQn); 
    }
    #else
    if(enable)
    {
        qcc74x_irq_enable(DM_IRQn); 
    }
    else
    {
        qcc74x_irq_disable(DM_IRQn); 
    }
    #endif
}
#endif

__attribute__((weak)) void btblecontroller_enable_ble_clk(uint8_t enable)
{
    #if defined(QCC74x_undefL)
    GLB_Set_BLE_CLK(enable);
    #endif
}

__attribute__((weak)) void btblecontroller_rf_restore()
{
  #if defined(QCC743)
  struct wl_cfg_t *wl_cfg;

  wl_cfg = wl_cfg_get((uint8_t *)WL_API_RMEM_ADDR);
  wl_cfg->mode = WL_API_MODE_BZ;
  wl_lp_init((uint8_t*)WL_API_RMEM_ADDR,2412);
  #endif
}

__attribute__((weak)) int btblecontroller_efuse_read_mac(uint8_t mac[6])
{
    int status = 0;
    uint8_t tmp[8] = {0};

    #if defined(CFG_IOT_SDK)
    #if defined(QCC74x_undefL)
    extern int qcc74x_wireless_mac_addr_get(uint8_t mac[8]);
    qcc74x_wireless_mac_addr_get(tmp);
    #else
    extern int qcc74x_efuse_read_mac(uint8_t mac[6]);
    status = qcc74x_efuse_read_mac(tmp);
    #endif
    #else
    status = mfg_media_read_macaddr_with_lock(tmp, 1);
    #endif
    mac[0] = tmp[0];
    mac[1] = tmp[1];
    mac[2] = tmp[2];
    mac[3] = tmp[3];
    mac[4] = tmp[4];
    mac[5] = tmp[5];
    return status;
}

#if defined(QCC743)
__attribute__((weak)) void btblecontroller_software_btdm_reset()
{
    GLB_AHB_MCU_Software_Reset(GLB_AHB_MCU_SW_BTDM);
}

__attribute__((weak)) void btblecontroller_software_pds_reset()
{
    GLB_AHB_MCU_Software_Reset(GLB_AHB_MCU_SW_PDS);
}

__attribute__((weak)) void btblecontroller_pds_trim_rc32m()
{
    PDS_Trim_RC32M();
}

__attribute__((weak)) int btblecontroller_printf(const char *fmt, ...)
{
    return 0;
}

__attribute__((weak)) uint8_t btblecontrolller_get_chip_version()
{
    extern void qcc74x_efuse_get_device_info(qcc74x_efuse_device_info_type *device_info);
    qcc74x_efuse_device_info_type device_info;
    qcc74x_efuse_get_device_info(&device_info);
    return device_info.version;
}
#endif

