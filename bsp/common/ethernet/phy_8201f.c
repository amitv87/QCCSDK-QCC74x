#include "phy_8201f.h"

#define CHECK_PARAM(expr) ((void)0)

static struct qcc74x_device_s *emac_dev = NULL;
static struct qcc74x_emac_phy_cfg_s *phy_8201f_cfg = NULL;

/**
 * @brief phy 8201f reset
 * @return int
 *
 */
int phy_8201f_reset(void)
{
    int timeout = 1000;
    uint16_t regval = PHY_8201F_RESET;

    /* pull the PHY from power down mode if it is in */
    if (0 != qcc74x_emac_phy_reg_read(emac_dev, PHY_8201F_BCR, &regval)) {
        printf("emac phy reg read fail 0x%04x\r\n", regval);
        return -1;
    }
    printf("emac phy reg read 0x%04x\r\n", regval);
    if (PHY_8201F_POWERDOWN == (regval & PHY_8201F_POWERDOWN)) {
        if (qcc74x_emac_phy_reg_write(emac_dev, PHY_8201F_BCR, regval & (~PHY_8201F_POWERDOWN)) != 0) {
            return -1;
        }
    }

    /* do sw reset */
    if (qcc74x_emac_phy_reg_write(emac_dev, PHY_8201F_BCR, PHY_8201F_RESET) != 0) {
        return -1;
    }

    for (; timeout; timeout--) {
        if (0 != qcc74x_emac_phy_reg_read(emac_dev, PHY_8201F_BCR, &regval)) {
            return -1;
        }

        if (!(regval & PHY_8201F_RESET)) {
            return 0;
        }

        qcc74x_mtimer_delay_ms(1);
    }
    printf("emac phy sw reset time out! 0x%04x\r\n", regval);
    return -1;
}

/**
 * @brief phy 8201f auto negotiate
 * @param cfg phy config
 * @return int
 *
 */
int phy_8201f_auto_negotiate(struct qcc74x_emac_phy_cfg_s *cfg)
{
    uint16_t regval = 0;
    uint16_t phyid1 = 0, phyid2 = 0;
    uint16_t advertise = 0;
    uint16_t lpa = 0;
    uint32_t timeout = 100; //10s,in 100ms

    if (0 != qcc74x_emac_phy_reg_read(emac_dev, PHY_8201F_PHYID1, &phyid1)) {
        printf("read emac phy id 1 error\r\n");
        return -1;
    }

    if (0 != qcc74x_emac_phy_reg_read(emac_dev, PHY_8201F_PHYID2, &phyid2)) {
        printf("read emac phy id 2 error\r\n");
        return -1;
    }
    printf("emac phy id 1 =%08x\r\n", (unsigned int)phyid1);
    printf("emac phy id 2 =%08x\r\n", (unsigned int)phyid2);
    if (cfg->phy_id != (((phyid1 << 16) | phyid2) & 0x000FFFF0)) {
        /* ID error */
        return -1;
    } else {
        cfg->phy_id = (phyid1 << 16) | phyid2;
    }

    if (0 != qcc74x_emac_phy_reg_read(emac_dev, PHY_8201F_BCR, &regval)) {
        return -1;
    }

    regval &= ~PHY_8201F_AUTONEGOTIATION;
    regval &= ~(PHY_8201F_LOOPBACK | PHY_8201F_POWERDOWN);
    regval |= PHY_8201F_ISOLATE;

    if (qcc74x_emac_phy_reg_write(emac_dev, PHY_8201F_BCR, regval) != 0) {
        return -1;
    }

    /* set advertisement mode */
    advertise = PHY_8201F_ADVERTISE_100BASETXFULL | PHY_8201F_ADVERTISE_100BASETXHALF |
                PHY_8201F_ADVERTISE_10BASETXFULL | PHY_8201F_ADVERTISE_10BASETXHALF |
                PHY_8201F_ADVERTISE_8023;

    if (qcc74x_emac_phy_reg_write(emac_dev, PHY_8201F_ADVERTISE, advertise) != 0) {
        return -1;
    }

    qcc74x_mtimer_delay_ms(16);

    if (0 != qcc74x_emac_phy_reg_read(emac_dev, PHY_8201F_BCR, &regval)) {
        return -1;
    }

    qcc74x_mtimer_delay_ms(16);
    regval |= (PHY_8201F_FULLDUPLEX_100M | PHY_8201F_AUTONEGOTIATION);

    if (qcc74x_emac_phy_reg_write(emac_dev, PHY_8201F_BCR, regval) != 0) {
        return -1;
    }

    qcc74x_mtimer_delay_ms(16);
    regval |= PHY_8201F_RESTART_AUTONEGOTIATION;
    regval &= ~PHY_8201F_ISOLATE;

    if (qcc74x_emac_phy_reg_write(emac_dev, PHY_8201F_BCR, regval) != 0) {
        return -1;
    }

    qcc74x_mtimer_delay_ms(100);

    while (1) {
        if (0 != qcc74x_emac_phy_reg_read(emac_dev, PHY_8201F_BSR, &regval)) {
            return -1;
        }

        if (regval & PHY_8201F_AUTONEGO_COMPLETE) {
            /* complete */
            break;
        }

        if (!(--timeout)) {
            return -1;
        }

        qcc74x_mtimer_delay_ms(100);
    }

    qcc74x_mtimer_delay_ms(100);

    if (0 != qcc74x_emac_phy_reg_read(emac_dev, PHY_8201F_LPA, &lpa)) {
        return -1;
    }

    printf("emac phy lpa@0x%x advertise@0x%x\r\n", lpa, advertise);

    if (((advertise & lpa) & PHY_8201F_ADVERTISE_100BASETXFULL) != 0) {
        /* 100BaseTX and Full Duplex */
        // printf("emac phy 100BaseTX and Full Duplex\r\n");
        cfg->full_duplex = 1;
        cfg->speed = 100;
        cfg->phy_state = PHY_STATE_READY;
    } else if (((advertise & lpa) & PHY_8201F_ADVERTISE_10BASETXFULL) != 0) {
        /* 10BaseT and Full Duplex */
        // printf("emac phy 10BaseTe and Full Duplex\r\n");
        cfg->full_duplex = 1;
        cfg->speed = 10;
        cfg->phy_state = PHY_STATE_READY;
    } else if (((advertise & lpa) & PHY_8201F_ADVERTISE_100BASETXHALF) != 0) {
        /* 100BaseTX and half Duplex */
        // printf("emac phy 100BaseTX and half Duplex\r\n");
        cfg->full_duplex = 0;
        cfg->speed = 100;
        cfg->phy_state = PHY_STATE_READY;
    } else if (((advertise & lpa) & PHY_8201F_ADVERTISE_10BASETXHALF) != 0) {
        /* 10BaseT and half Duplex */
        // printf("emac phy 10BaseTe and half Duplex\r\n");
        cfg->full_duplex = 0;
        cfg->speed = 10;
        cfg->phy_state = PHY_STATE_READY;
    }

    return 0;
}

/**
 * @brief phy 8201f link up
 * @param cfg phy config
 * @return int
 *
 */
int phy_8201f_link_up(struct qcc74x_emac_phy_cfg_s *cfg)
{
    uint16_t phy_bsr = 0;
    uint16_t phy_sr = 0;

    qcc74x_mtimer_delay_ms(16);

    if (0 != qcc74x_emac_phy_reg_read(emac_dev, PHY_8201F_BSR, &phy_bsr)) {
        return -1;
    }

    qcc74x_mtimer_delay_ms(16);

    if (!(PHY_8201F_LINKED_STATUS & phy_bsr)) {
        return 1; /* error */
    }

    qcc74x_mtimer_delay_ms(16);

    if (0 != qcc74x_emac_phy_reg_read(emac_dev, PHY_8201F_SR, &phy_sr)) {
        return -1;
    }

    if ((phy_bsr & PHY_8201F_BSR_100BASETXFULL) && PHY_8201F_SR_SPEED_MODE_COMPARE(phy_sr, PHY_8201F_SR_SPEED_100BASETXFULL)) {
        /* 100BaseTX and Full Duplex */
        cfg->full_duplex = 1;
        cfg->speed = 100;
        cfg->phy_state = PHY_STATE_UP;
    } else if ((phy_bsr & PHY_8201F_BSR_10BASETXFULL) && PHY_8201F_SR_SPEED_MODE_COMPARE(phy_sr, PHY_8201F_SR_SPEED_10BASETXFULL)) {
        /* 10BaseT and Full Duplex */
        cfg->full_duplex = 1;
        cfg->speed = 10;
        cfg->phy_state = PHY_STATE_UP;
    } else if ((phy_bsr & PHY_8201F_BSR_100BASETXHALF) && PHY_8201F_SR_SPEED_MODE_COMPARE(phy_sr, PHY_8201F_SR_SPEED_100BASETXHALF)) {
        /* 100BaseTX and half Duplex */
        cfg->full_duplex = 0;
        cfg->speed = 100;
        cfg->phy_state = PHY_STATE_UP;
    } else if ((phy_bsr & PHY_8201F_BSR_10BASETXHALF) && PHY_8201F_SR_SPEED_MODE_COMPARE(phy_sr, PHY_8201F_SR_SPEED_10BASETXHALF)) {
        /* 10BaseT and half Duplex */
        cfg->full_duplex = 0;
        cfg->speed = 10;
        cfg->phy_state = PHY_STATE_UP;
    } else {
        /* 10BaseT and half Duplex */
        cfg->full_duplex = -1;
        cfg->speed = -1;
        cfg->phy_state = PHY_STATE_DOWN;
        return -1;
    }

    return 0;
}

/**
 * @brief  Use energy detector for cable plug in/out detect.
 * @param  cfg: phy config
 * @return int
 *
 */
#if 0
int phy_8201f_poll_cable_status(struct qcc74x_emac_phy_cfg_s *cfg)
{
    uint16_t phy_regval = 0;

    CHECK_PARAM(NULL != phy_8201f_cfg);

    if (0 != qcc74x_emac_phy_reg_read(emac_dev, PHY_CTRL_STATUS, &phy_regval)) {
        return -1;
    }

    phy_8201f_cfg->phy_state = (PHY_CTRL_STATUS_ENERGYON & phy_regval) ? PHY_STATE_UP : PHY_STATE_DOWN;

    return !!(PHY_CTRL_STATUS_ENERGYON & phy_regval);
}
#endif

/**
 * @brief  Initialize EMAC PHY 8201f module
 * @param  cfg: phy config
 * @return int
 *
 */
int phy_8201f_init(struct qcc74x_device_s *emac, struct qcc74x_emac_phy_cfg_s *cfg)
{
    uint16_t phyReg;

    CHECK_PARAM(NULL != cfg);

    phy_8201f_cfg = cfg;
    emac_dev = emac;

    printf("emac phy addr:0x%04x\r\n", cfg->phy_address);

    qcc74x_emac_feature_control(emac, EMAC_CMD_SET_PHY_ADDRESS, cfg->phy_address);

    if (0 != phy_8201f_reset()) {
        printf("emac phy reset fail!\r\n");
        return -1;
    }

    if (cfg->auto_negotiation) {
        /*
        uint32_t cnt=0;
        do{
            if(qcc74x_emac_phy_reg_read(emac_dev, PHY_8201F_BSR, &phyReg) != SUCCESS){
                return -1;
            }
            cnt++;
            if(cnt>PHY_8201F_LINK_TO){
                return -1;
            }
        }while((phyReg & PHY_8201F_LINKED_STATUS) != PHY_8201F_LINKED_STATUS);
        */
        if (0 != phy_8201f_auto_negotiate(cfg)) {
            return -1;
        }
    } else {
        if (qcc74x_emac_phy_reg_read(emac_dev, PHY_8201F_BCR, &phyReg) != 0) {
            return -1;
        }

        phyReg &= (~PHY_8201F_FULLDUPLEX_100M);

        if (cfg->speed == 10) {
            if (cfg->full_duplex == 1) {
                phyReg |= PHY_8201F_FULLDUPLEX_10M;
            } else {
                phyReg |= PHY_8201F_HALFDUPLEX_10M;
            }
        } else {
            if (cfg->full_duplex == 1) {
                phyReg |= PHY_8201F_FULLDUPLEX_100M;
            } else {
                phyReg |= PHY_8201F_HALFDUPLEX_100M;
            }
        }

        if ((qcc74x_emac_phy_reg_write(emac_dev, PHY_8201F_BCR, phyReg)) != 0) {
            return -1;
        }
    }

    qcc74x_emac_feature_control(emac, EMAC_CMD_FULL_DUPLEX, cfg->full_duplex);

    return phy_8201f_link_up(cfg);
}

/**
 * @brief get phy 8201f module status
 * @return emac_phy_status_t @ref emac_phy_status_t enum
 */
emac_phy_status_t phy_8201f_status_get(void)
{
    CHECK_PARAM(NULL != phy_8201f_cfg);

    if ((100 == phy_8201f_cfg->speed) &&
        (phy_8201f_cfg->full_duplex) &&
        (PHY_STATE_UP == phy_8201f_cfg->phy_state)) {
        return EMAC_PHY_STAT_100MBITS_FULLDUPLEX;
    } else if (PHY_STATE_UP == phy_8201f_cfg->phy_state) {
        return EMAC_PHY_STAT_LINK_UP;
    } else {
        return EMAC_PHY_STAT_LINK_DOWN;
    }
}
