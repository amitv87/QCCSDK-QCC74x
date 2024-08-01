#include "phy_ar8032.h"

#define CHECK_PARAM(expr) ((void)0)

static struct qcc74x_device_s *emac_dev = NULL;
static struct qcc74x_eth_phy_cfg_s *phy_ar8032_cfg = NULL;

/**
 * @brief phy ar8032 reset
 * @return int
 *
 */
int phy_ar8032_reset(void)
{
    int timeout = 1000;
    uint16_t regval = PHY_AR8032_RESET;

    /* pull the PHY from power down mode if it is in */
    if (0 != qcc74x_emac_phy_reg_read(emac_dev, PHY_AR8032_BCR, &regval)) {
        printf("emac phy reg read fail 0x%04x\r\n", regval);
        return -1;
    }
    printf("emac phy reg read 0x%04x\r\n", regval);
    if (PHY_AR8032_POWERDOWN == (regval & PHY_AR8032_POWERDOWN)) {
        if (qcc74x_emac_phy_reg_write(emac_dev, PHY_AR8032_BCR, regval & (~PHY_AR8032_POWERDOWN)) != 0) {
            return -1;
        }
    }

    /* do sw reset */
    if (qcc74x_emac_phy_reg_write(emac_dev, PHY_AR8032_BCR, (PHY_AR8032_RESET | PHY_AR8032_AUTONEGOTIATION)) != 0) {
        return -1;
    }

    for (; timeout; timeout--) {
        if (0 != qcc74x_emac_phy_reg_read(emac_dev, PHY_AR8032_BCR, &regval)) {
            return -1;
        }

        if (!(regval & PHY_AR8032_RESET)) {
            return 0;
        }

        qcc74x_mtimer_delay_ms(1);
    }
    printf("emac phy sw reset time out! 0x%04x\r\n", regval);
    return -1;
}

/**
 * @brief phy ar8032 auto negotiate
 * @param cfg phy config
 * @return int
 *
 */
int phy_ar8032_auto_negotiate(struct qcc74x_eth_phy_cfg_s *cfg)
{
    uint16_t regval = 0;
    uint16_t phyid1 = 0, phyid2 = 0;
    uint16_t advertise = 0;
    uint16_t lpa = 0;
    uint32_t timeout = 100; //10s,in 100ms

    if (0 != qcc74x_emac_phy_reg_read(emac_dev, PHY_AR8032_PHYID1, &phyid1)) {
        printf("read emac phy id 1 error\r\n");
        return -1;
    }

    if (0 != qcc74x_emac_phy_reg_read(emac_dev, PHY_AR8032_PHYID2, &phyid2)) {
        printf("read emac phy id 2 error\r\n");
        return -1;
    }
    printf("emac phy id 1 =%08x\r\n", (unsigned int)phyid1);
    printf("emac phy id 2 =%08x\r\n", (unsigned int)phyid2);
    if (cfg->phy_id != (((phyid1 << 16) | phyid2) & 0xFFFFFFF0)) {
        /* ID error */
        return -1;
    } else {
        cfg->phy_id = (phyid1 << 16) | phyid2;
    }

    if (0 != qcc74x_emac_phy_reg_read(emac_dev, PHY_AR8032_BCR, &regval)) {
        return -1;
    }


    regval &= ~PHY_AR8032_AUTONEGOTIATION;
    regval &= ~(PHY_AR8032_LOOPBACK | PHY_AR8032_POWERDOWN);
    regval |= PHY_AR8032_ISOLATE;

    if (qcc74x_emac_phy_reg_write(emac_dev, PHY_AR8032_BCR, regval) != 0) {
        return -1;
    }

    /* set advertisement mode */
    advertise = PHY_AR8032_ADVERTISE_100BASETXFULL | PHY_AR8032_ADVERTISE_100BASETXHALF |
                PHY_AR8032_ADVERTISE_10BASETXFULL | PHY_AR8032_ADVERTISE_10BASETXHALF |
                PHY_AR8032_ADVERTISE_8023;

    if (qcc74x_emac_phy_reg_write(emac_dev, PHY_AR8032_ADVERTISE, advertise) != 0) {
        return -1;
    }

    qcc74x_mtimer_delay_ms(16);

    if (0 != qcc74x_emac_phy_reg_read(emac_dev, PHY_AR8032_BCR, &regval)) {
        return -1;
    }

    printf("emac phy reg read BCR 0x%04x\r\n", regval);
    qcc74x_mtimer_delay_ms(16);
    regval |= (PHY_AR8032_FULLDUPLEX_100M | PHY_AR8032_AUTONEGOTIATION);


    if (qcc74x_emac_phy_reg_write(emac_dev, PHY_AR8032_BCR, regval) != 0) {
        return -1;
    }

    qcc74x_mtimer_delay_ms(16);
    regval |= PHY_AR8032_RESTART_AUTONEGOTIATION;
    regval &= ~PHY_AR8032_ISOLATE;

    if (qcc74x_emac_phy_reg_write(emac_dev, PHY_AR8032_BCR, regval) != 0) {
        return -1;
    }

    qcc74x_mtimer_delay_ms(100);
    if (0 != qcc74x_emac_phy_reg_read(emac_dev, PHY_AR8032_BSR, &regval)) {
        return -1;
    }
    printf("emac phy reg read BSR 0x%04x\r\n", regval);
    while (1) {
        if (0 != qcc74x_emac_phy_reg_read(emac_dev, PHY_AR8032_BSR, &regval)) {
            return -1;
        }

        if (regval & PHY_AR8032_AUTONEGO_COMPLETE) {
            /* complete */
            break;
        }

        if (!(--timeout)) {
            printf("emac phy auto negotiation timeout!\r\n");
            // return -1;
            break;
        }

        qcc74x_mtimer_delay_ms(100);
    }

    qcc74x_mtimer_delay_ms(100);

    if (0 != qcc74x_emac_phy_reg_read(emac_dev, PHY_AR8032_LPA, &lpa)) {
        return -1;
    }

    printf("emac phy lpa@0x%x advertise@0x%x\r\n", lpa, advertise);

    if (((advertise & lpa) & PHY_AR8032_ADVERTISE_100BASETXFULL) != 0) {
        /* 100BaseTX and Full Duplex */
        // printf("emac phy 100BaseTX and Full Duplex\r\n");
        cfg->full_duplex = 1;
        cfg->speed = 100;
        cfg->phy_state = PHY_STATE_READY;
    } else if (((advertise & lpa) & PHY_AR8032_ADVERTISE_10BASETXFULL) != 0) {
        /* 10BaseT and Full Duplex */
        // printf("emac phy 10BaseTe and Full Duplex\r\n");
        cfg->full_duplex = 1;
        cfg->speed = 10;
        cfg->phy_state = PHY_STATE_READY;
    } else if (((advertise & lpa) & PHY_AR8032_ADVERTISE_100BASETXHALF) != 0) {
        /* 100BaseTX and half Duplex */
        // printf("emac phy 100BaseTX and half Duplex\r\n");
        cfg->full_duplex = 0;
        cfg->speed = 100;
        cfg->phy_state = PHY_STATE_READY;
    } else if (((advertise & lpa) & PHY_AR8032_ADVERTISE_10BASETXHALF) != 0) {
        /* 10BaseT and half Duplex */
        // printf("emac phy 10BaseTe and half Duplex\r\n");
        cfg->full_duplex = 0;
        cfg->speed = 10;
        cfg->phy_state = PHY_STATE_READY;
    }

    return 0;
}

/**
 * @brief phy ar8032 link up
 * @param cfg phy config
 * @return int
 *
 */
int phy_ar8032_link_up(struct qcc74x_eth_phy_cfg_s *cfg)
{
    uint16_t phy_bsr = 0;

    qcc74x_mtimer_delay_ms(16);

    if (0 != qcc74x_emac_phy_reg_read(emac_dev, PHY_AR8032_BSR, &phy_bsr)) {
        return -1;
    }

    if (!(PHY_AR8032_LINKED_STATUS & phy_bsr)) {
        return -1; /* error */
    }

    cfg->phy_state = PHY_STATE_UP;

    return 0;
}

/**
 * @brief  Initialize EMAC PHY ar8032 module
 * @param  cfg: phy config
 * @return int
 *
 */
int phy_ar8032_init(struct qcc74x_device_s *emac, struct qcc74x_eth_phy_cfg_s *cfg)
{
    uint16_t phyReg;

    CHECK_PARAM(NULL != cfg);

    phy_ar8032_cfg = cfg;
    emac_dev = emac;

    printf("emac phy addr:0x%04x\r\n", cfg->phy_address);

    qcc74x_emac_feature_control(emac, EMAC_CMD_SET_PHY_ADDRESS, cfg->phy_address);

    if (0 != phy_ar8032_reset()) {
        printf("emac phy reset fail!\r\n");
        return -1;
    }

    if (cfg->auto_negotiation) {
        if (0 != phy_ar8032_auto_negotiate(cfg)) {
            return -1;
        }
    } else {
        if (qcc74x_emac_phy_reg_read(emac_dev, PHY_AR8032_BCR, &phyReg) != 0) {
            return -1;
        }

        phyReg &= (~PHY_AR8032_FULLDUPLEX_100M);

        if (cfg->speed == 10) {
            if (cfg->full_duplex == 1) {
                phyReg |= PHY_AR8032_FULLDUPLEX_10M;
            } else {
                phyReg |= PHY_AR8032_HALFDUPLEX_10M;
            }
        } else {
            if (cfg->full_duplex == 1) {
                phyReg |= PHY_AR8032_FULLDUPLEX_100M;
            } else {
                phyReg |= PHY_AR8032_HALFDUPLEX_100M;
            }
        }

        if ((qcc74x_emac_phy_reg_write(emac_dev, PHY_AR8032_BCR, phyReg)) != 0) {
            return -1;
        }
    }

    qcc74x_emac_feature_control(emac, EMAC_CMD_FULL_DUPLEX, cfg->full_duplex);

    return phy_ar8032_link_up(cfg);
}

/**
 * @brief get phy ar8032 module status
 * @return eth_phy_status_t @ref eth_phy_status_t enum
 */
eth_phy_status_t phy_ar8032_status_get(void)
{
    CHECK_PARAM(NULL != phy_ar8032_cfg);

    if ((100 == phy_ar8032_cfg->speed) &&
        (phy_ar8032_cfg->full_duplex) &&
        (PHY_STATE_UP == phy_ar8032_cfg->phy_state)) {
        return ETH_PHY_STAT_100MBITS_FULLDUPLEX;
    } else if (PHY_STATE_UP == phy_ar8032_cfg->phy_state) {
        return ETH_PHY_STAT_LINK_UP;
    } else {
        return ETH_PHY_STAT_LINK_DOWN;
    }
}