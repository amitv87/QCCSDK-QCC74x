#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include <qcc743.h>
#include <qcc743_glb.h>
#include <qcc74x_mtimer.h>
#include <qcc74x_efuse.h>
#include <qcc74x_flash.h>
#include <lmac154.h>
#include <zb_timer.h>

#include <openthread_port.h>
#include <ot_radio_trx.h>
#include <ot_utils_ext.h>

void otPlatRadioGetIeeeEui64(otInstance *aInstance, uint8_t *aIeeeEui64) 
{
    int i;
    uint32_t flash_id = 0;

    memset(aIeeeEui64, 0, 8);
    for (i = 2; i >= 0; i --) {
        if (!qcc74x_efuse_is_mac_address_slot_empty(i, 0)) {
            qcc74x_efuse_read_mac_address_opt(i, aIeeeEui64, 0);
            return;
        }
    }

    aIeeeEui64[0] = 0x8C;
    aIeeeEui64[1] = 0xFD;
    aIeeeEui64[2] = 0xF0;
    flash_id = qcc74x_flash_get_jedec_id();
    aIeeeEui64[3] = (flash_id >> 24) & 0xff;
    aIeeeEui64[4] = (flash_id >> 16) & 0xff;
    aIeeeEui64[5] = (flash_id >> 8) & 0xff;
    aIeeeEui64[6] = flash_id & 0xff;
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
    uint8_t ch = lmac154_getChannel();

#if (OPENTHREAD_FTD) || (OPENTHREAD_MTD)
    if (lmac154_isRxStateWhenIdle() != otThreadGetLinkMode(aInstance).mRxOnWhenIdle) {
        lmac154_setRxStateWhenIdle(otThreadGetLinkMode(aInstance).mRxOnWhenIdle);
    }
#endif

    if (ch != (aChannel - OT_RADIO_2P4GHZ_OQPSK_CHANNEL_MIN)) {

        lmac154_disableRx();

        lmac154_setChannel(aChannel - OT_RADIO_2P4GHZ_OQPSK_CHANNEL_MIN);
    }

    lmac154_enableRx();

    return OT_ERROR_NONE;
}