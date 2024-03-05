#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__

#include "lwip/err.h"
#include "lwip/netif.h"
#include "ethernet_phy.h"

#define DHCP_OFF              (uint8_t)0
#define DHCP_START            (uint8_t)1
#define DHCP_WAIT_ADDRESS     (uint8_t)2
#define DHCP_ADDRESS_ASSIGNED (uint8_t)3
#define DHCP_TIMEOUT          (uint8_t)4
#define DHCP_LINK_DOWN        (uint8_t)5

/* Exported types ------------------------------------------------------------*/
err_t ethernetif_init(struct netif *netif);
void ethernet_link_check_state(struct netif *netif);
void ethernet_link_status_updated(struct netif *netif);

#endif
