#ifndef __PHY_BL3011_H__
#define __PHY_BL3011_H__

#include <string.h>
#include "ethernet_phy.h"


#define PHY_BL3011_LINK_TO                             ((uint32_t)0x00000FFFU)
#define PHY_BL3011_AUTONEGO_COMPLETED_TO               ((uint32_t)0x00000FFFU)
/* PHY Registers */
#define PHY_BL3011_BCR                                 ((uint16_t)0x00U)   /*!< Transceiver Basic Control Register; default: 0x1140 */
#define PHY_BL3011_RESET                               ((uint16_t)0x8000U) /*!< PHY Reset */
#define PHY_BL3011_LOOPBACK                            ((uint16_t)0x4000U) /*!< Select loop-back mode */
#define PHY_BL3011_FULLDUPLEX_100M                     ((uint16_t)0x2000U) /*!< Set the full-duplex mode at 100 Mb/s  */
#define PHY_BL3011_HALFDUPLEX_100M                     ((uint16_t)0x2000U) /*!< Set the half-duplex mode at 100 Mb/s  */
#define PHY_BL3011_FULLDUPLEX_10M                      ((uint16_t)0x0000U) /*!< Set the full-duplex mode at 10 Mb/s   */
#define PHY_BL3011_HALFDUPLEX_10M                      ((uint16_t)0x0000U) /*!< Set the half-duplex mode at 10 Mb/s   */
#define PHY_BL3011_AUTONEGOTIATION                     ((uint16_t)0x1000U) /*!< Enable auto-negotiation function      */
#define PHY_BL3011_RESTART_AUTONEGOTIATION             ((uint16_t)0x0200U) /*!< Restart auto-negotiation function     */
#define PHY_BL3011_POWERDOWN                           ((uint16_t)0x0800U) /*!< Select the power down mode            */
#define PHY_BL3011_ISOLATE                             ((uint16_t)0x0400U) /*!< Isolate PHY from MII                  */

#define PHY_BL3011_BSR                                 ((uint16_t)0x01U)   /*!< Transceiver Basic Status Register     */
#define PHY_BL3011_BSR_100BASETXFULL                   (1 << 14)           /*!< 100Base full-duplex                   */
#define PHY_BL3011_BSR_100BASETXHALF                   (1 << 13)           /*!< 100Base half-duplex                   */
#define PHY_BL3011_BSR_10BASETXFULL                    (1 << 12)           /*!< 10Base full-duplex                    */
#define PHY_BL3011_BSR_10BASETXHALF                    (1 << 11)           /*!< 10Base half-duplex                    */
#define PHY_BL3011_BSR_REMOTE_FAULT                    (1 << 4)            /*!< remote fault condition detected       */
#define PHY_BL3011_AUTONEGO_COMPLETE                   ((uint16_t)0x0020U) /*!< Auto-Negotiation process completed    */
#define PHY_BL3011_LINKED_STATUS                       ((uint16_t)0x0004U) /*!< Valid link established   link is up   */
#define PHY_BL3011_JABBER_DETECTION                    ((uint16_t)0x0002U) /*!< Jabber condition detected             */

#define PHY_BL3011_PHYID1                              ((uint16_t)0x02U) /*!< PHY ID 1  default 0x0000                */
#define PHY_BL3011_PHYID2                              ((uint16_t)0x03U) /*!< PHY ID 2  default 0x0128                */
#define PHY_BL3011_ADVERTISE                           ((uint16_t)0x04U) /*!< Auto-negotiation advertisement          */
#define PHY_BL3011_ADVERTISE_100BASETXFULL             (1 << 8)          /*!< 100Base-TX full-duplex                  */
#define PHY_BL3011_ADVERTISE_100BASETXHALF             (1 << 7)          /*!< 100Base-TX half-duplex                  */
#define PHY_BL3011_ADVERTISE_10BASETXFULL              (1 << 6)          /*!< 10Base-Te full-duplex                   */
#define PHY_BL3011_ADVERTISE_10BASETXHALF              (1 << 5)          /*!< 10Base-Te half-duplex                   */
#define PHY_BL3011_ADVERTISE_8023                      (1 << 0)          /*!< IEEE 802.3 mode                         */

#define PHY_BL3011_LPA                                 ((uint16_t)0x05U) /*!< Auto-negotiation link partner ability   */
#define PHY_BL3011_EXPANSION                           ((uint16_t)0x06U) /*!< Auto-negotiation expansion           */

int phy_bl3011_init(struct qcc74x_device_s *emac, struct qcc74x_eth_phy_cfg_s *cfg);
eth_phy_status_t phy_bl3011_status_get(void);

int phy_bl3011_auto_negotiate(struct qcc74x_eth_phy_cfg_s *cfg);
int phy_bl3011_link_up(struct qcc74x_eth_phy_cfg_s *cfg);

#endif /* __PHY_BL3011_H__ */