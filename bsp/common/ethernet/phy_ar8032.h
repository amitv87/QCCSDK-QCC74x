#ifndef __PHY_AR8032_H__
#define __PHY_AR8032_H__

#include <string.h>
#include "ethernet_phy.h"


#define PHY_AR8032_LINK_TO                             ((uint32_t)0x00000FFFU)
#define PHY_AR8032_AUTONEGO_COMPLETED_TO               ((uint32_t)0x00000FFFU)
/* PHY Registers */
#define PHY_AR8032_BCR                                 ((uint16_t)0x00U)   /*!< Transceiver Basic Control Register; default: 0x1140 */
#define PHY_AR8032_RESET                               ((uint16_t)0x8000U) /*!< PHY Reset */
#define PHY_AR8032_LOOPBACK                            ((uint16_t)0x4000U) /*!< Select loop-back mode */
#define PHY_AR8032_FULLDUPLEX_100M                     ((uint16_t)0x2100U) /*!< Set the full-duplex mode at 100 Mb/s  */
#define PHY_AR8032_HALFDUPLEX_100M                     ((uint16_t)0x2000U) /*!< Set the half-duplex mode at 100 Mb/s  */
#define PHY_AR8032_FULLDUPLEX_10M                      ((uint16_t)0x0100U) /*!< Set the full-duplex mode at 10 Mb/s   */
#define PHY_AR8032_HALFDUPLEX_10M                      ((uint16_t)0x0000U) /*!< Set the half-duplex mode at 10 Mb/s   */
#define PHY_AR8032_AUTONEGOTIATION                     ((uint16_t)0x1000U) /*!< Enable auto-negotiation function      */
#define PHY_AR8032_RESTART_AUTONEGOTIATION             ((uint16_t)0x0200U) /*!< Restart auto-negotiation function     */
#define PHY_AR8032_POWERDOWN                           ((uint16_t)0x0800U) /*!< Select the power down mode            */
#define PHY_AR8032_ISOLATE                             ((uint16_t)0x0400U) /*!< Isolate PHY from MII                  */

#define PHY_AR8032_BSR                                 ((uint16_t)0x01U)   /*!< Transceiver Basic Status Register     */
#define PHY_AR8032_BSR_100BASETXFULL                   (1 << 14)           /*!< 100Base full-duplex                   */
#define PHY_AR8032_BSR_100BASETXHALF                   (1 << 13)           /*!< 100Base half-duplex                   */
#define PHY_AR8032_BSR_10BASETXFULL                    (1 << 12)           /*!< 10Base full-duplex                    */
#define PHY_AR8032_BSR_10BASETXHALF                    (1 << 11)           /*!< 10Base half-duplex                    */
#define PHY_AR8032_BSR_REMOTE_FAULT                    (1 << 4)            /*!< remote fault condition detected       */
#define PHY_AR8032_AUTONEGO_COMPLETE                   ((uint16_t)0x0020U) /*!< Auto-Negotiation process completed    */
#define PHY_AR8032_LINKED_STATUS                       ((uint16_t)0x0004U) /*!< Valid link established   link is up   */
#define PHY_AR8032_JABBER_DETECTION                    ((uint16_t)0x0002U) /*!< Jabber condition detected             */

#define PHY_AR8032_PHYID1                              ((uint16_t)0x02U) /*!< PHY ID 1  default 0x0000                */
#define PHY_AR8032_PHYID2                              ((uint16_t)0x03U) /*!< PHY ID 2  default 0x0128                */
#define PHY_AR8032_ADVERTISE                           ((uint16_t)0x04U) /*!< Auto-negotiation advertisement          */
#define PHY_AR8032_ADVERTISE_100BASETXFULL             (1 << 8)          /*!< 100Base-TX full-duplex                  */
#define PHY_AR8032_ADVERTISE_100BASETXHALF             (1 << 7)          /*!< 100Base-TX half-duplex                  */
#define PHY_AR8032_ADVERTISE_10BASETXFULL              (1 << 6)          /*!< 10Base-Te full-duplex                   */
#define PHY_AR8032_ADVERTISE_10BASETXHALF              (1 << 5)          /*!< 10Base-Te half-duplex                   */
#define PHY_AR8032_ADVERTISE_8023                      (1 << 0)          /*!< IEEE 802.3 mode                         */

#define PHY_AR8032_LPA                                 ((uint16_t)0x05U) /*!< Auto-negotiation link partner ability   */
#define PHY_AR8032_EXPANSION                           ((uint16_t)0x06U) /*!< Auto-negotiation expansion           */

int phy_ar8032_init(struct qcc74x_device_s *emac, struct qcc74x_eth_phy_cfg_s *cfg);
eth_phy_status_t phy_ar8032_status_get(void);

int phy_ar8032_auto_negotiate(struct qcc74x_eth_phy_cfg_s *cfg);
int phy_ar8032_link_up(struct qcc74x_eth_phy_cfg_s *cfg);

#endif  // __PHY_AR8032_H__
