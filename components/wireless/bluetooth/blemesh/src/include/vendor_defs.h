/** @file
 *  @brief Bluetooth Mesh Profile APIs.
 */


#ifndef __QCC74x_VENDOR_DEFS__
#define __QCC74x_VENDOR_DEFS__

#include "include/access.h"

#define BT_MESH_TX_VND_SDU_MAX_SHORT               (BT_MESH_TX_SDU_MAX - 3 - BT_MESH_MIC_SHORT)
#define BT_MESH_TX_VND_SDU_MAX_LONG                (BT_MESH_TX_SDU_MAX - 3 - BT_MESH_MIC_LONG)


/* Vendor Models ID Added by qcc74x lab*/
#define BT_MESH_VND_MODEL_ID_SYNC_SRV              0x0000
#define BT_MESH_VND_MODEL_ID_SYNC_CLI              0x0001
#define BT_MESH_VND_MODEL_ID_DATA_SRV              0x0002
#define BT_MESH_VND_MODEL_ID_DATA_CLI              0x0003


/* vendor models opcode Added by qcc74x lab*/
/* ble mesh vendor sync Message Opcode */
#define BLE_MESH_MODEL_VND_OP_SYC_GET                            BT_MESH_MODEL_OP_3(0x01, QCC74x_COMP_ID)
#define BLE_MESH_MODEL_VND_OP_SYC_SET                            BT_MESH_MODEL_OP_3(0x02, QCC74x_COMP_ID)
#define BLE_MESH_MODEL_VND_OP_SYC_SET_UNACK                      BT_MESH_MODEL_OP_3(0x03, QCC74x_COMP_ID)
#define BLE_MESH_MODEL_VND_OP_SYC_STATUS                         BT_MESH_MODEL_OP_3(0x04, QCC74x_COMP_ID)
/* reserve 0x05 & 0x06 */

/* ble mesh vendor sync Message Opcode */
#define BLE_MESH_MODEL_VND_OP_DATA_GET                            BT_MESH_MODEL_OP_3(0x07, QCC74x_COMP_ID)
#define BLE_MESH_MODEL_VND_OP_DATA_SET                            BT_MESH_MODEL_OP_3(0x08, QCC74x_COMP_ID)
#define BLE_MESH_MODEL_VND_OP_DATA_SET_UNACK                      BT_MESH_MODEL_OP_3(0x09, QCC74x_COMP_ID)
#define BLE_MESH_MODEL_VND_OP_DATA_STATUS                         BT_MESH_MODEL_OP_3(0x0A, QCC74x_COMP_ID)
/* reserve 0x0B & 0x0C */



#endif /* __QCC74x_VENDOR_DEFS__ */
