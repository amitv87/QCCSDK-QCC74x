#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include <qcc743.h>
#include <qcc743_glb.h>
#include <qcc74x_mtimer.h>
#include <qcc74x_efuse.h>
#include <lmac154.h>

#include <openthread_port.h>
#include <ot_radio_trx.h>
#include <ot_utils_ext.h>

void otPlatRadioGetIeeeEui64(otInstance *aInstance, uint8_t *aIeeeEui64) 
{
    uint8_t chipid[8];
    uint8_t mac_addr[6];
    int i;

    for (i = 2; i >= 0; i --) {
        if (!qcc74x_efuse_is_mac_address_slot_empty(2, 0)) {
            qcc74x_efuse_read_mac_address_opt(i, mac_addr, 0);
            break;
        }
    }

    qcc74x_efuse_get_chipid(chipid);
    if (i >= 0) {
        memcpy(aIeeeEui64, mac_addr, 6);
        memcpy(aIeeeEui64 + 6, chipid + 4, 2);
    }
    else {
        memcpy(aIeeeEui64 + 2, chipid, 6);
        aIeeeEui64[0] = 0x8C;
        aIeeeEui64[1] = 0xFD;
        aIeeeEui64[2] = 0xF0;
    }
}

uint64_t otPlatRadioGetNow(otInstance *aInstance)
{
    return qcc74x_mtimer_get_time_us();
}

otError otPlatRadioEnable(otInstance *aInstance) 
{
    ot_radioEnable();

    qcc74x_irq_enable(M154_INT_IRQn);

    return OT_ERROR_NONE;
}

otError otPlatRadioDisable(otInstance *aInstance) 
{
    qcc74x_irq_disable(M154_INT_IRQn);
    lmac154_disableRx();

    return OT_ERROR_NONE;
}

otError otPlatRadioReceive(otInstance *aInstance, uint8_t aChannel) 
{
    uint8_t ch = aChannel - OT_RADIO_2P4GHZ_OQPSK_CHANNEL_MIN;

    lmac154_setChannel((lmac154_channel_t)ch);
#if (OPENTHREAD_FTD) || (OPENTHREAD_MTD)
    lmac154_setRxStateWhenIdle(otThreadGetLinkMode(aInstance).mRxOnWhenIdle);
#endif
    lmac154_enableRx();

    return OT_ERROR_NONE;
}