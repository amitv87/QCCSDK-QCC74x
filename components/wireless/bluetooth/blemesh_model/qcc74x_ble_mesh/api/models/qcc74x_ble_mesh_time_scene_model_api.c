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

#include "btc/btc_manage.h"

#include "btc_ble_mesh_time_scene_model.h"
#include "qcc74x_ble_mesh_time_scene_model_api.h"

qcc74x_err_t qcc74x_ble_mesh_register_time_scene_client_callback(qcc74x_ble_mesh_time_scene_client_cb_t callback)
{
    QCC74x_BLE_HOST_STATUS_CHECK(QCC74x_BLE_HOST_STATUS_ENABLED);

    return (btc_profile_cb_set(BTC_PID_TIME_SCENE_CLIENT, callback) == 0 ? QCC74x_OK : QCC74x_FAIL);
}

qcc74x_err_t qcc74x_ble_mesh_time_scene_client_get_state(qcc74x_ble_mesh_client_common_param_t *params,
        qcc74x_ble_mesh_time_scene_client_get_state_t *get_state)
{
    btc_ble_mesh_time_scene_client_args_t arg = {0};
    btc_msg_t msg = {0};

    if (params == NULL || params->model == NULL ||
        params->ctx.net_idx == QCC74x_BLE_MESH_KEY_UNUSED ||
        params->ctx.app_idx == QCC74x_BLE_MESH_KEY_UNUSED ||
        params->ctx.addr == QCC74x_BLE_MESH_ADDR_UNASSIGNED ||
        (params->opcode == QCC74x_BLE_MESH_MODEL_OP_SCHEDULER_ACT_GET && get_state == NULL)) {
        return QCC74x_ERR_INVALID_ARG;
    }

    QCC74x_BLE_HOST_STATUS_CHECK(QCC74x_BLE_HOST_STATUS_ENABLED);

    msg.sig = BTC_SIG_API_CALL;
    msg.pid = BTC_PID_TIME_SCENE_CLIENT;
    msg.act = BTC_BLE_MESH_ACT_TIME_SCENE_CLIENT_GET_STATE;
    arg.time_scene_client_get_state.params = params;
    arg.time_scene_client_get_state.get_state = get_state;

    return (btc_transfer_context(&msg, &arg, sizeof(btc_ble_mesh_time_scene_client_args_t), btc_ble_mesh_time_scene_client_arg_deep_copy)
            == BT_STATUS_SUCCESS ? QCC74x_OK : QCC74x_FAIL);
}

qcc74x_err_t qcc74x_ble_mesh_time_scene_client_set_state(qcc74x_ble_mesh_client_common_param_t *params,
        qcc74x_ble_mesh_time_scene_client_set_state_t *set_state)
{
    btc_ble_mesh_time_scene_client_args_t arg = {0};
    btc_msg_t msg = {0};

    if (params == NULL || params->model == NULL || set_state == NULL ||
        params->ctx.net_idx == QCC74x_BLE_MESH_KEY_UNUSED ||
        params->ctx.app_idx == QCC74x_BLE_MESH_KEY_UNUSED ||
        params->ctx.addr == QCC74x_BLE_MESH_ADDR_UNASSIGNED) {
        return QCC74x_ERR_INVALID_ARG;
    }

    QCC74x_BLE_HOST_STATUS_CHECK(QCC74x_BLE_HOST_STATUS_ENABLED);

    msg.sig = BTC_SIG_API_CALL;
    msg.pid = BTC_PID_TIME_SCENE_CLIENT;
    msg.act = BTC_BLE_MESH_ACT_TIME_SCENE_CLIENT_SET_STATE;
    arg.time_scene_client_set_state.params = params;
    arg.time_scene_client_set_state.set_state = set_state;

    return (btc_transfer_context(&msg, &arg, sizeof(btc_ble_mesh_time_scene_client_args_t), btc_ble_mesh_time_scene_client_arg_deep_copy)
            == BT_STATUS_SUCCESS ? QCC74x_OK : QCC74x_FAIL);
}

qcc74x_err_t qcc74x_ble_mesh_register_time_scene_server_callback(qcc74x_ble_mesh_time_scene_server_cb_t callback)
{
    QCC74x_BLE_HOST_STATUS_CHECK(QCC74x_BLE_HOST_STATUS_ENABLED);

    return (btc_profile_cb_set(BTC_PID_TIME_SCENE_SERVER, callback) == 0 ? QCC74x_OK : QCC74x_FAIL);
}

