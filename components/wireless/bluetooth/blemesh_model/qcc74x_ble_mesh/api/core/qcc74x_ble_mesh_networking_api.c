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

#include <stdint.h>
#include <string.h>

#include "qcc74x_err.h"

#include "btc_ble_mesh_prov.h"
#include "qcc74x_ble_mesh_networking_api.h"


#define QCC74x_BLE_MESH_TX_SDU_MAX ((CONFIG_BT_MESH_ADV_BUF_COUNT - 3) * 12)

static qcc74x_err_t ble_mesh_model_send_msg(qcc74x_ble_mesh_model_t *model,
        qcc74x_ble_mesh_msg_ctx_t *ctx,
        uint32_t opcode,
        btc_ble_mesh_model_act_t act,
        uint16_t length, uint8_t *data,
        int32_t msg_timeout, bool need_rsp,
        qcc74x_ble_mesh_dev_role_t device_role)
{
    btc_ble_mesh_model_args_t arg = {0};
    uint8_t op_len = 0, mic_len = 0;
    uint8_t *msg_data = NULL;
    btc_msg_t msg = {0};
    qcc74x_err_t status = QCC74x_OK;

    QCC74x_BLE_HOST_STATUS_CHECK(QCC74x_BLE_HOST_STATUS_ENABLED);

    if (ctx && ctx->addr == QCC74x_BLE_MESH_ADDR_UNASSIGNED) {
        BT_ERR("%s, Invalid destination address 0x0000", __func__);
        return QCC74x_ERR_INVALID_ARG;
    }

    if (device_role > ROLE_FAST_PROV) {
        BT_ERR("%s, Invalid device role 0x%02x", __func__, device_role);
        return QCC74x_ERR_INVALID_ARG;
    }

    /* When data is NULL, it is mandatory to set length to 0 to prevent users from misinterpreting parameters. */
    if (data == NULL) {
        length = 0;
    }

    if (opcode < 0x100) {
        op_len = 1;
    } else if (opcode < 0x10000) {
        op_len = 2;
    } else {
        op_len = 3;
    }

    if (act == BTC_BLE_MESH_ACT_MODEL_PUBLISH) {
        if (op_len + length > model->pub->msg->size) {
            BT_ERR("%s, Model publication msg size %d is too small", __func__, model->pub->msg->size);
            return QCC74x_ERR_INVALID_ARG;
        }
    }

    if (act == BTC_BLE_MESH_ACT_MODEL_PUBLISH) {
        mic_len = QCC74x_BLE_MESH_MIC_SHORT;
    } else {
        mic_len = ctx->send_rel ? QCC74x_BLE_MESH_MIC_LONG : QCC74x_BLE_MESH_MIC_SHORT;
    }

    if (op_len + length + mic_len > MIN(QCC74x_BLE_MESH_SDU_MAX_LEN, QCC74x_BLE_MESH_TX_SDU_MAX)) {
        BT_ERR("%s, Data length %d is too large", __func__, length);
        return QCC74x_ERR_INVALID_ARG;
    }

    if (act == BTC_BLE_MESH_ACT_MODEL_PUBLISH) {
        bt_mesh_model_msg_init(model->pub->msg, opcode);
        net_buf_simple_add_mem(model->pub->msg, data, length);
    } else {
        msg_data = (uint8_t *)bt_mesh_malloc(op_len + length);
        if (msg_data == NULL) {
            return QCC74x_ERR_NO_MEM;
        }
        qcc74x_ble_mesh_model_msg_opcode_init(msg_data, opcode);
        memcpy(msg_data + op_len, data, length);
    }

    msg.sig = BTC_SIG_API_CALL;
    msg.pid = BTC_PID_MODEL;
    msg.act = act;

    if (act == BTC_BLE_MESH_ACT_MODEL_PUBLISH) {
        arg.model_publish.model = model;
        arg.model_publish.device_role = device_role;
    } else {
        arg.model_send.model = model;
        arg.model_send.ctx = ctx;
        arg.model_send.need_rsp = need_rsp;
        arg.model_send.opcode = opcode;
        arg.model_send.length = op_len + length;
        arg.model_send.data = msg_data;
        arg.model_send.device_role = device_role;
        arg.model_send.msg_timeout = msg_timeout;
    }

    status = (btc_transfer_context(&msg, &arg, sizeof(btc_ble_mesh_model_args_t), NULL)
              == BT_STATUS_SUCCESS ? QCC74x_OK : QCC74x_FAIL);

    bt_mesh_free(msg_data);

    return status;
}

qcc74x_err_t qcc74x_ble_mesh_register_custom_model_callback(qcc74x_ble_mesh_model_cb_t callback)
{
    QCC74x_BLE_HOST_STATUS_CHECK(QCC74x_BLE_HOST_STATUS_ENABLED);

    return (btc_profile_cb_set(BTC_PID_MODEL, callback) == 0 ? QCC74x_OK : QCC74x_FAIL);
}

qcc74x_err_t qcc74x_ble_mesh_model_msg_opcode_init(uint8_t *data, uint32_t opcode)
{
    uint16_t val = 0;

    if (data == NULL) {
        return QCC74x_ERR_INVALID_ARG;
    }

    if (opcode < 0x100) {
        /* 1-byte OpCode */
        data[0] = opcode & 0xff;
        return QCC74x_OK;
    }

    if (opcode < 0x10000) {
        /* 2-byte OpCode, big endian */
        val = sys_cpu_to_be16 (opcode);
        memcpy(data, &val, 2);
        return QCC74x_OK;
    }

    /* 3-byte OpCode, note that little endian for the least 2 bytes(Company ID) of opcode */
    data[0] = (opcode >> 16) & 0xff;
    val = sys_cpu_to_le16(opcode & 0xffff);
    memcpy(&data[1], &val, 2);

    return QCC74x_OK;
}

qcc74x_err_t qcc74x_ble_mesh_client_model_init(qcc74x_ble_mesh_model_t *model)
{
    if (model == NULL) {
        return QCC74x_ERR_INVALID_ARG;
    }

    QCC74x_BLE_HOST_STATUS_CHECK(QCC74x_BLE_HOST_STATUS_ENABLED);

    return btc_ble_mesh_client_model_init(model);
}

qcc74x_err_t qcc74x_ble_mesh_client_model_deinit(qcc74x_ble_mesh_model_t *model)
{
    if (model == NULL) {
        return QCC74x_ERR_INVALID_ARG;
    }

    QCC74x_BLE_HOST_STATUS_CHECK(QCC74x_BLE_HOST_STATUS_ENABLED);

    return btc_ble_mesh_client_model_deinit(model);
}

qcc74x_err_t qcc74x_ble_mesh_server_model_send_msg(qcc74x_ble_mesh_model_t *model,
        qcc74x_ble_mesh_msg_ctx_t *ctx, uint32_t opcode,
        uint16_t length, uint8_t *data)
{
    if (model == NULL || ctx == NULL ||
        ctx->net_idx == QCC74x_BLE_MESH_KEY_UNUSED ||
        ctx->app_idx == QCC74x_BLE_MESH_KEY_UNUSED) {
        return QCC74x_ERR_INVALID_ARG;
    }

    return ble_mesh_model_send_msg(model, ctx, opcode, BTC_BLE_MESH_ACT_SERVER_MODEL_SEND,
                                   length, data, 0, false, ROLE_NODE);
}

qcc74x_err_t qcc74x_ble_mesh_client_model_send_msg(qcc74x_ble_mesh_model_t *model,
        qcc74x_ble_mesh_msg_ctx_t *ctx, uint32_t opcode,
        uint16_t length, uint8_t *data, int32_t msg_timeout,
        bool need_rsp, qcc74x_ble_mesh_dev_role_t device_role)
{
    if (model == NULL || ctx == NULL ||
        ctx->net_idx == QCC74x_BLE_MESH_KEY_UNUSED ||
        ctx->app_idx == QCC74x_BLE_MESH_KEY_UNUSED) {
        return QCC74x_ERR_INVALID_ARG;
    }

    return ble_mesh_model_send_msg(model, ctx, opcode, BTC_BLE_MESH_ACT_CLIENT_MODEL_SEND,
                                   length, data, msg_timeout, need_rsp, device_role);
}

qcc74x_err_t qcc74x_ble_mesh_model_publish(qcc74x_ble_mesh_model_t *model, uint32_t opcode,
                                     uint16_t length, uint8_t *data,
                                     qcc74x_ble_mesh_dev_role_t device_role)
{
    if (model == NULL || model->pub == NULL || model->pub->msg == NULL ||
        model->pub->addr == QCC74x_BLE_MESH_ADDR_UNASSIGNED) {
        return QCC74x_ERR_INVALID_ARG;
    }

    return ble_mesh_model_send_msg(model, NULL, opcode, BTC_BLE_MESH_ACT_MODEL_PUBLISH,
                                   length, data, 0, false, device_role);
}

qcc74x_err_t qcc74x_ble_mesh_server_model_update_state(qcc74x_ble_mesh_model_t *model,
        qcc74x_ble_mesh_server_state_type_t type,
        qcc74x_ble_mesh_server_state_value_t *value)
{
    btc_ble_mesh_model_args_t arg = {0};
    btc_msg_t msg = {0};

    if (!model || !value || type >= QCC74x_BLE_MESH_SERVER_MODEL_STATE_MAX) {
        return QCC74x_ERR_INVALID_ARG;
    }

    QCC74x_BLE_HOST_STATUS_CHECK(QCC74x_BLE_HOST_STATUS_ENABLED);

    arg.model_update_state.model = model;
    arg.model_update_state.type = type;
    arg.model_update_state.value = value;

    msg.sig = BTC_SIG_API_CALL;
    msg.pid = BTC_PID_MODEL;
    msg.act = BTC_BLE_MESH_ACT_SERVER_MODEL_UPDATE_STATE;

    return (btc_transfer_context(&msg, &arg, sizeof(btc_ble_mesh_model_args_t), NULL)
            == BT_STATUS_SUCCESS ? QCC74x_OK : QCC74x_FAIL);
}

qcc74x_err_t qcc74x_ble_mesh_node_local_reset(void)
{
    btc_msg_t msg = {0};

    QCC74x_BLE_HOST_STATUS_CHECK(QCC74x_BLE_HOST_STATUS_ENABLED);

    msg.sig = BTC_SIG_API_CALL;
    msg.pid = BTC_PID_PROV;
    msg.act = BTC_BLE_MESH_ACT_NODE_RESET;

    return (btc_transfer_context(&msg, NULL, 0, NULL) == BT_STATUS_SUCCESS ? QCC74x_OK : QCC74x_FAIL);
}

#if (CONFIG_BLE_MESH_PROVISIONER)

qcc74x_err_t qcc74x_ble_mesh_provisioner_set_node_name(uint16_t index, const char *name)
{
    btc_ble_mesh_prov_args_t arg = {0};
    btc_msg_t msg = {0};

    if (!name || (strlen(name) > QCC74x_BLE_MESH_NODE_NAME_MAX_LEN)) {
        return QCC74x_ERR_INVALID_ARG;
    }

    QCC74x_BLE_HOST_STATUS_CHECK(QCC74x_BLE_HOST_STATUS_ENABLED);

    msg.sig = BTC_SIG_API_CALL;
    msg.pid = BTC_PID_PROV;
    msg.act = BTC_BLE_MESH_ACT_PROVISIONER_SET_NODE_NAME;

    arg.set_node_name.index = index;
    memset(arg.set_node_name.name, 0, sizeof(arg.set_node_name.name));
    strlcpy(arg.set_node_name.name, name, QCC74x_BLE_MESH_NODE_NAME_MAX_LEN);

    return (btc_transfer_context(&msg, &arg, sizeof(btc_ble_mesh_prov_args_t), NULL)
            == BT_STATUS_SUCCESS ? QCC74x_OK : QCC74x_FAIL);
}

const char *qcc74x_ble_mesh_provisioner_get_node_name(uint16_t index)
{
    return bt_mesh_provisioner_get_node_name(index);
}

uint16_t qcc74x_ble_mesh_provisioner_get_node_index(const char *name)
{
    if (!name || (strlen(name) > QCC74x_BLE_MESH_NODE_NAME_MAX_LEN)) {
        return QCC74x_BLE_MESH_INVALID_NODE_INDEX;
    }

    return bt_mesh_provisioner_get_node_index(name);
}

qcc74x_err_t qcc74x_ble_mesh_provisioner_store_node_comp_data(uint16_t unicast_addr, uint8_t *data, uint16_t length)
{
    btc_ble_mesh_prov_args_t arg = {0};
    btc_msg_t msg = {0};

    if (!QCC74x_BLE_MESH_ADDR_IS_UNICAST(unicast_addr) || !data || length <= 14) {
        return QCC74x_ERR_INVALID_ARG;
    }

    QCC74x_BLE_HOST_STATUS_CHECK(QCC74x_BLE_HOST_STATUS_ENABLED);

    msg.sig = BTC_SIG_API_CALL;
    msg.pid = BTC_PID_PROV;
    msg.act = BTC_BLE_MESH_ACT_PROVISIONER_STORE_NODE_COMP_DATA;

    arg.store_node_comp_data.unicast_addr = unicast_addr;
    arg.store_node_comp_data.length = length;
    arg.store_node_comp_data.data = data;
    return (btc_transfer_context(&msg, &arg, sizeof(btc_ble_mesh_prov_args_t), NULL)
            == BT_STATUS_SUCCESS ? QCC74x_OK : QCC74x_FAIL);
}

qcc74x_ble_mesh_node_t *qcc74x_ble_mesh_provisioner_get_node_with_uuid(const uint8_t uuid[16])
{
    if (!uuid) {
        return NULL;
    }

    return btc_ble_mesh_provisioner_get_node_with_uuid(uuid);
}

qcc74x_ble_mesh_node_t *qcc74x_ble_mesh_provisioner_get_node_with_addr(uint16_t unicast_addr)
{
    if (!QCC74x_BLE_MESH_ADDR_IS_UNICAST(unicast_addr)) {
        return NULL;
    }

    return btc_ble_mesh_provisioner_get_node_with_addr(unicast_addr);
}

qcc74x_err_t qcc74x_ble_mesh_provisioner_delete_node_with_uuid(const uint8_t uuid[16])
{
    btc_ble_mesh_prov_args_t arg = {0};
    btc_msg_t msg = {0};

    if (!uuid) {
        return QCC74x_ERR_INVALID_ARG;
    }

    QCC74x_BLE_HOST_STATUS_CHECK(QCC74x_BLE_HOST_STATUS_ENABLED);

    msg.sig = BTC_SIG_API_CALL;
    msg.pid = BTC_PID_PROV;
    msg.act = BTC_BLE_MESH_ACT_PROVISIONER_DELETE_NODE_WITH_UUID;

    memcpy(arg.delete_node_with_uuid.uuid, uuid, 16);

    return (btc_transfer_context(&msg, &arg, sizeof(btc_ble_mesh_prov_args_t), NULL)
            == BT_STATUS_SUCCESS ? QCC74x_OK : QCC74x_FAIL);
}

qcc74x_err_t qcc74x_ble_mesh_provisioner_delete_node_with_addr(uint16_t unicast_addr)
{
    btc_ble_mesh_prov_args_t arg = {0};
    btc_msg_t msg = {0};

    if (!QCC74x_BLE_MESH_ADDR_IS_UNICAST(unicast_addr)) {
        return QCC74x_ERR_INVALID_ARG;
    }

    QCC74x_BLE_HOST_STATUS_CHECK(QCC74x_BLE_HOST_STATUS_ENABLED);

    msg.sig = BTC_SIG_API_CALL;
    msg.pid = BTC_PID_PROV;
    msg.act = BTC_BLE_MESH_ACT_PROVISIONER_DELETE_NODE_WITH_ADDR;

    arg.delete_node_with_addr.unicast_addr = unicast_addr;

    return (btc_transfer_context(&msg, &arg, sizeof(btc_ble_mesh_prov_args_t), NULL)
            == BT_STATUS_SUCCESS ? QCC74x_OK : QCC74x_FAIL);
}

qcc74x_err_t qcc74x_ble_mesh_provisioner_add_local_app_key(const uint8_t app_key[16],
        uint16_t net_idx, uint16_t app_idx)
{
    btc_ble_mesh_prov_args_t arg = {0};
    btc_msg_t msg = {0};

    QCC74x_BLE_HOST_STATUS_CHECK(QCC74x_BLE_HOST_STATUS_ENABLED);

    msg.sig = BTC_SIG_API_CALL;
    msg.pid = BTC_PID_PROV;
    msg.act = BTC_BLE_MESH_ACT_PROVISIONER_ADD_LOCAL_APP_KEY;

    arg.add_local_app_key.net_idx = net_idx;
    arg.add_local_app_key.app_idx = app_idx;
    if (app_key) {
        memcpy(arg.add_local_app_key.app_key, app_key, 16);
    } else {
        bzero(arg.add_local_app_key.app_key, 16);
    }
    return (btc_transfer_context(&msg, &arg, sizeof(btc_ble_mesh_prov_args_t), NULL)
            == BT_STATUS_SUCCESS ? QCC74x_OK : QCC74x_FAIL);
}

qcc74x_err_t qcc74x_ble_mesh_provisioner_update_local_app_key(const uint8_t app_key[16],
            uint16_t net_idx, uint16_t app_idx)
{
    btc_ble_mesh_prov_args_t arg = {0};
    btc_msg_t msg = {0};

    if (app_key == NULL) {
        return QCC74x_ERR_INVALID_ARG;
    }

    QCC74x_BLE_HOST_STATUS_CHECK(QCC74x_BLE_HOST_STATUS_ENABLED);

    msg.sig = BTC_SIG_API_CALL;
    msg.pid = BTC_PID_PROV;
    msg.act = BTC_BLE_MESH_ACT_PROVISIONER_UPDATE_LOCAL_APP_KEY;

    memcpy(arg.update_local_app_key.app_key, app_key, 16);
    arg.update_local_app_key.net_idx = net_idx;
    arg.update_local_app_key.app_idx = app_idx;
    return (btc_transfer_context(&msg, &arg, sizeof(btc_ble_mesh_prov_args_t), NULL)
            == BT_STATUS_SUCCESS ? QCC74x_OK : QCC74x_FAIL);
}

const uint8_t *qcc74x_ble_mesh_provisioner_get_local_app_key(uint16_t net_idx, uint16_t app_idx)
{
    return bt_mesh_provisioner_local_app_key_get(net_idx, app_idx);
}

qcc74x_err_t qcc74x_ble_mesh_provisioner_bind_app_key_to_local_model(uint16_t element_addr, uint16_t app_idx,
        uint16_t model_id, uint16_t company_id)
{
    btc_ble_mesh_prov_args_t arg = {0};
    btc_msg_t msg = {0};

    if (!QCC74x_BLE_MESH_ADDR_IS_UNICAST(element_addr)) {
        return QCC74x_ERR_INVALID_ARG;
    }

    QCC74x_BLE_HOST_STATUS_CHECK(QCC74x_BLE_HOST_STATUS_ENABLED);

    msg.sig = BTC_SIG_API_CALL;
    msg.pid = BTC_PID_PROV;
    msg.act = BTC_BLE_MESH_ACT_PROVISIONER_BIND_LOCAL_MOD_APP;

    arg.local_mod_app_bind.elem_addr = element_addr;
    arg.local_mod_app_bind.app_idx = app_idx;
    arg.local_mod_app_bind.model_id = model_id;
    arg.local_mod_app_bind.cid = company_id;
    return (btc_transfer_context(&msg, &arg, sizeof(btc_ble_mesh_prov_args_t), NULL)
            == BT_STATUS_SUCCESS ? QCC74x_OK : QCC74x_FAIL);
}

qcc74x_err_t qcc74x_ble_mesh_provisioner_add_local_net_key(const uint8_t net_key[16], uint16_t net_idx)
{
    btc_ble_mesh_prov_args_t arg = {0};
    btc_msg_t msg = {0};

    if (net_idx == QCC74x_BLE_MESH_KEY_PRIMARY) {
        return QCC74x_ERR_INVALID_ARG;
    }

    QCC74x_BLE_HOST_STATUS_CHECK(QCC74x_BLE_HOST_STATUS_ENABLED);

    msg.sig = BTC_SIG_API_CALL;
    msg.pid = BTC_PID_PROV;
    msg.act = BTC_BLE_MESH_ACT_PROVISIONER_ADD_LOCAL_NET_KEY;

    arg.add_local_net_key.net_idx = net_idx;
    if (net_key) {
        memcpy(arg.add_local_net_key.net_key, net_key, 16);
    } else {
        bzero(arg.add_local_net_key.net_key, 16);
    }
    return (btc_transfer_context(&msg, &arg, sizeof(btc_ble_mesh_prov_args_t), NULL)
            == BT_STATUS_SUCCESS ? QCC74x_OK : QCC74x_FAIL);
}

qcc74x_err_t qcc74x_ble_mesh_provisioner_update_local_net_key(const uint8_t net_key[16], uint16_t net_idx)
{
    btc_ble_mesh_prov_args_t arg = {0};
    btc_msg_t msg = {0};

    if (net_key == NULL) {
        return QCC74x_ERR_INVALID_ARG;
    }

    QCC74x_BLE_HOST_STATUS_CHECK(QCC74x_BLE_HOST_STATUS_ENABLED);

    msg.sig = BTC_SIG_API_CALL;
    msg.pid = BTC_PID_PROV;
    msg.act = BTC_BLE_MESH_ACT_PROVISIONER_UPDATE_LOCAL_NET_KEY;

    memcpy(arg.update_local_net_key.net_key, net_key, 16);
    arg.update_local_net_key.net_idx = net_idx;
    return (btc_transfer_context(&msg, &arg, sizeof(btc_ble_mesh_prov_args_t), NULL)
            == BT_STATUS_SUCCESS ? QCC74x_OK : QCC74x_FAIL);
}

const uint8_t *qcc74x_ble_mesh_provisioner_get_local_net_key(uint16_t net_idx)
{
    return bt_mesh_provisioner_local_net_key_get(net_idx);
}

uint16_t qcc74x_ble_mesh_provisioner_get_prov_node_count(void)
{
    return btc_ble_mesh_provisioner_get_prov_node_count();
}
int qcc74x_ble_mesh_fast_prov_group_address(uint16_t model_id, uint16_t group_addr)
{
    struct qcc74x_ble_mesh_comp_t *comp = NULL;
    qcc74x_ble_mesh_elem_t *element = NULL;
    qcc74x_ble_mesh_model_t *model = NULL;
    int i, j;

    if (!QCC74x_BLE_MESH_ADDR_IS_UNICAST(group_addr)) {
        return QCC74x_ERR_INVALID_ARG;
    }

    comp = qcc74x_ble_mesh_get_composition_data();
    if (!comp) 
    {
        return QCC74x_FAIL;
    }

    for (i = 0; i < comp->element_count; i++) {
        element = &comp->elements[i];
        model = qcc74x_ble_mesh_find_sig_model(element, model_id);
        if (!model) {
            continue;
        }
        for (j = 0; j < ARRAY_SIZE(model->groups); j++) {
            if (model->groups[j] == group_addr) {
                break;
            }
        }
        if (j != ARRAY_SIZE(model->groups)) {
            BT_WARN("%s: Group address already exists, element index: %d", __func__, i);
            continue;
        }
        for (j = 0; j < ARRAY_SIZE(model->groups); j++) {
            if (model->groups[j] == QCC74x_BLE_MESH_ADDR_UNASSIGNED) {
                model->groups[j] = group_addr;
                break;
            }
        }
        if (j == ARRAY_SIZE(model->groups)) {
            BT_WARN("%s: Model is full of group addresses, element index: %d", __func__, i);
        }
    }

    return QCC74x_OK;
}
#endif /* CONFIG_BLE_MESH_PROVISIONER */

#if (CONFIG_BLE_MESH_FAST_PROV)
const uint8_t *qcc74x_ble_mesh_get_fast_prov_app_key(uint16_t net_idx, uint16_t app_idx)
{
    return bt_mesh_get_fast_prov_app_key(net_idx, app_idx);
}
#endif  /* CONFIG_BLE_MESH_FAST_PROV */

