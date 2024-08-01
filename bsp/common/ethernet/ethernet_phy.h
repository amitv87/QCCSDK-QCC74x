#ifndef __ETHERNET_PHY_H__
#define __ETHERNET_PHY_H__

#if __has_include("ethernet_phy_user.h")

#include "ethernet_phy_user.h"

#else

#include "qcc74x_emac.h"
#include "qcc74x_mtimer.h"

/**
 * @brief EMAC phy configuration structure
 *
 * @param auto_negotiation EMAC phy speed and mode auto negotiation
 * @param full_duplex      EMAC phy duplex mode
 * @param phy_state        EMAC phy down,ready,up,running,nolink,halted, @ref PHY_STATE
 * @param use_irq          EMAC phy interrupt enable 0: no IRQ used
 * @param speed            EMAC phy speed mode
 * @param phy_address      EMAC phy address
 * @param phy_id           EMAC phy read phy id
 */
struct qcc74x_eth_phy_cfg_s {
    uint8_t auto_negotiation;
    uint8_t full_duplex;
    uint8_t phy_state;
    uint8_t use_irq;
    uint16_t speed;
    uint16_t phy_address;
    uint32_t phy_id;
};

/**
    ETH PHY chips
    PHY_8720
    PHY_8201F
*/

typedef enum eth_phy_status {
    ETH_PHY_STAT_EEROR,
    ETH_PHY_STAT_LINK_DOWN,
    ETH_PHY_STAT_LINK_INIT,
    ETH_PHY_STAT_LINK_UP,
    ETH_PHY_STAT_100MBITS_FULLDUPLEX,
    ETH_PHY_STAT_100MBITS_HALFDUPLEX,
    ETH_PHY_STAT_10MBITS_FULLDUPLEX,
    ETH_PHY_STAT_10MBITS_HALFDUPLEX,
    /* new added for GMAC */
    ETH_PHY_STAT_1000MBITS_FULLDUPLEX,
    ETH_PHY_STAT_1000MBITS_HALFDUPLEX,
} eth_phy_status_t;

#if defined PHY_8720
#include "phy_8720.h"
#define _PHY_FUNC_DEFINE(_func, ...) phy_8720_##_func(__VA_ARGS__)
#endif

#if defined PHY_BL3011
#include "phy_bl3011.h"
#define _PHY_FUNC_DEFINE(_func, ...) phy_bl3011_##_func(__VA_ARGS__)
#endif

#if defined PHY_AR8032
#include "phy_ar8032.h"
#define _PHY_FUNC_DEFINE(_func, ...) phy_ar8032_##_func(__VA_ARGS__)
#endif

eth_phy_status_t ethernet_phy_status_get(void);
int ethernet_phy_init(struct qcc74x_device_s *mac, struct qcc74x_eth_phy_cfg_s *eth_phy_cfg);

#endif

#endif
