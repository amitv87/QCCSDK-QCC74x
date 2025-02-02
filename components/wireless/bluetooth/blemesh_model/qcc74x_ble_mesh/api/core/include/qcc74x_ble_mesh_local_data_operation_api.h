// Copyright 2017-2019 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _QCC74x_BLE_MESH_LOCAL_DATA_OPERATION_API_H_
#define _QCC74x_BLE_MESH_LOCAL_DATA_OPERATION_API_H_

#include "qcc74x_ble_mesh_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief         Get the model publish period, the unit is ms.
 *
 * @param[in]     model: Model instance pointer.
 *
 * @return        Publish period value on success, 0 or (negative) error code from errno.h on failure.
 *
 */
int32_t qcc74x_ble_mesh_get_model_publish_period(qcc74x_ble_mesh_model_t *model);

/**
 * @brief         Get the address of the primary element.
 *
 * @return        Address of the primary element on success, or
 *                QCC74x_BLE_MESH_ADDR_UNASSIGNED on failure which means the device has not been provisioned.
 *
 */
uint16_t qcc74x_ble_mesh_get_primary_element_address(void);

/**
 * @brief         Check if the model has subscribed to the given group address.
 *                Note: E.g., once a status message is received and the destination address
 *                      is a group address, the model uses this API to check if it is successfully subscribed
 *                      to the given group address.
 *
 * @param[in]     model: Pointer to the model.
 * @param[in]     group_addr: Group address.
 *
 * @return        Pointer to the group address within the Subscription List of the model on success, or
 *                NULL on failure which means the model has not subscribed to the given group address.
 *                Note: With the pointer to the group address returned, you can reset the group address
 *                      to 0x0000 in order to unsubscribe the model from the group.
 *
 */
uint16_t *qcc74x_ble_mesh_is_model_subscribed_to_group(qcc74x_ble_mesh_model_t *model, uint16_t group_addr);

/**
 * @brief         Find the BLE Mesh element pointer via the element address.
 *
 * @param[in]     element_addr: Element address.
 *
 * @return        Pointer to the element on success, or NULL on failure.
 *
 */
qcc74x_ble_mesh_elem_t *qcc74x_ble_mesh_find_element(uint16_t element_addr);

/**
 * @brief         Get the number of elements that have been registered.
 *
 * @return        Number of elements.
 *
 */
uint8_t qcc74x_ble_mesh_get_element_count(void);

/**
 * @brief        Find the Vendor specific model with the given element,
 *               the company ID and the Vendor Model ID.
 *
 * @param[in]    element: Element to which the model belongs.
 * @param[in]    company_id: A 16-bit company identifier assigned by the Bluetooth SIG.
 * @param[in]    model_id: A 16-bit vendor-assigned model identifier.
 *
 * @return       Pointer to the Vendor Model on success, or NULL on failure which means the Vendor Model is not found.
 *
 */
qcc74x_ble_mesh_model_t *qcc74x_ble_mesh_find_vendor_model(const qcc74x_ble_mesh_elem_t *element,
        uint16_t company_id, uint16_t model_id);

/**
 * @brief        Find the SIG model with the given element and Model id.
 *
 * @param[in]    element: Element to which the model belongs.
 * @param[in]    model_id: SIG model identifier.
 *
 * @return       Pointer to the SIG Model on success, or NULL on failure which means the SIG Model is not found.
 *
 */
qcc74x_ble_mesh_model_t *qcc74x_ble_mesh_find_sig_model(const qcc74x_ble_mesh_elem_t *element, uint16_t model_id);

/**
 * @brief        Get the Composition data which has been registered.
 *
 * @return       Pointer to the Composition data on success, or NULL on failure which means the Composition data is not initialized.
 *
 */
const qcc74x_ble_mesh_comp_t *qcc74x_ble_mesh_get_composition_data(void);

/**
 * @brief        A local model of node or Provisioner subscribes a group address.
 *
 * @note         This function shall not be invoked before node is provisioned or Provisioner is enabled.
 *
 * @param[in]    element_addr: Unicast address of the element to which the model belongs.
 * @param[in]    company_id: A 16-bit company identifier.
 * @param[in]    model_id: A 16-bit model identifier.
 * @param[in]    group_addr: The group address to be subscribed.
 *
 * @return       QCC74x_OK on success or error code otherwise.
 *
 */
qcc74x_err_t qcc74x_ble_mesh_model_subscribe_group_addr(uint16_t element_addr, uint16_t company_id,
                                                  uint16_t model_id, uint16_t group_addr);

/**
 * @brief        A local model of node or Provisioner unsubscribes a group address.
 *
 * @note         This function shall not be invoked before node is provisioned or Provisioner is enabled.
 *
 * @param[in]    element_addr: Unicast address of the element to which the model belongs.
 * @param[in]    company_id: A 16-bit company identifier.
 * @param[in]    model_id: A 16-bit model identifier.
 * @param[in]    group_addr: The subscribed group address.
 *
 * @return       QCC74x_OK on success or error code otherwise.
 *
 */
qcc74x_err_t qcc74x_ble_mesh_model_unsubscribe_group_addr(uint16_t element_addr, uint16_t company_id,
                                                    uint16_t model_id, uint16_t group_addr);

#ifdef __cplusplus
}
#endif

#endif /* _QCC74x_BLE_MESH_LOCAL_DATA_OPERATION_API_H_ */
