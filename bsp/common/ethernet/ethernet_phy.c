#include "ethernet_phy.h"

/**
 * @brief Initialize ethernet phy module
 * @param mac emac or gmac device
 * @param eth_phy_cfg phy config
 * @return int
 *
 */
int ethernet_phy_init(struct qcc74x_device_s *mac, struct qcc74x_eth_phy_cfg_s *eth_phy_cfg)
{
    return _PHY_FUNC_DEFINE(init, mac, eth_phy_cfg);
}

/**
 * @brief get ethernet phy module status
 * @return eth_phy_status_t
 *
 */
eth_phy_status_t ethernet_phy_status_get(void)
{
    return _PHY_FUNC_DEFINE(status_get);
}