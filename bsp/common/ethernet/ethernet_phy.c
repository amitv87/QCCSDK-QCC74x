#include "ethernet_phy.h"

/**
 * @brief Initialize ethernet phy module
 * @param emac emac device
 * @param emac_phy_cfg phy config
 * @return int
 *
 */
int ethernet_phy_init(struct qcc74x_device_s *emac, struct qcc74x_emac_phy_cfg_s *emac_phy_cfg)
{
    return _PHY_FUNC_DEFINE(init, emac, emac_phy_cfg);
}

/**
 * @brief get ethernet phy module status
 * @return emac_phy_status_t
 *
 */
emac_phy_status_t ethernet_phy_status_get(void)
{
    return _PHY_FUNC_DEFINE(status_get);
}