#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef void *qcctl_handle_t;

/**
 * @brief Init qcctl.
 *
 * @param handle Handle to return
 *
 * @return 0 on success
 */
int qcctl_init(qcctl_handle_t *handle);

int qcctl_init2(qcctl_handle_t *handle, bool daemon);

typedef void (*qcctl_wildcard_msg_handler_t)(void *msg, void *arg);

/**
 * @brief Register wildcard message handler.
 * A wildcard message is one that is not a response to any sent command.
 * The handler is mainly used for customer defined data channel to receive
 * async messages that can be sent from emb to host at any time.
 *
 * Note: the callback shall free the message. See example demo_user_ext().
 *
 * @param handle       qcctl Handle
 * @param callback     Callback
 * @param callback_arg Callback arg
 *
 * @return 0 on success
 */
int qcctl_register_wildcard_msg_handler(qcctl_handle_t *handle, qcctl_wildcard_msg_handler_t callback, void *callback_arg);

/**
 * @brief Deinit qcctl.
 *
 * Remember to deinit qcctl otherwise resource leak would occur.
 *
 * @param handle qcctl handle
 */
void qcctl_deinit(qcctl_handle_t handle);

/**
 * @brief Receive a control port message.
 *
 * @param handle     qcctl handle
 * @param rsp        Response of control port message.
 * @param timeout_ms Timeout
 *
 * @return 0 on success, 1 on error
 */
int qcctl_msg_recv(qcctl_handle_t handle, void **rsp, size_t timeout_ms);

/**
 * @brief Send a RNM message and expect its response.
 *
 * Note that the msg should be modifiable.
 *
 * @param handle     qcctl handle
 * @param msg        RNM message to send
 * @param msg_len    RNM message length
 * @param response   Response to return
 * @param timeout_ms Timeout to receive response
 *
 * @return 0 on success
 */
int qcctl_rnm_msg_expect(qcctl_handle_t handle, void *msg, size_t msg_len, void **response, size_t timeout_ms);

/**
 * @brief Send a RNM message and expect its response.
 *
 * Note that the msg should be modifiable.
 *
 * Only status of response is returned to caller.
 *
 * @param handle     qcctl handle
 * @param msg        RNM message to send
 * @param msg_len    RNM message length
 * @param status     Status code of response. See cmd_status_t
 * @param timeout_ms Timeout to receive response
 *
 * @return 0 on success
 */
int qcctl_rnm_msg_expect_simple(qcctl_handle_t handle, void *msg, size_t msg_len, uint16_t *status, size_t timeout_ms);

/**
 * @brief Start OTA
 *
 * @param handle qcctl handle
 *
 * @return 0 on success
 */
int qcctl_ota_start(qcctl_handle_t handle);

/**
 * @brief Abort OTA
 *
 * @param handle qcctl handle
 *
 * @return 0 on success
 */
int qcctl_ota_abort(qcctl_handle_t handle);

/**
 * @brief Program a part.
 * WARNING: Restricted to pass 1024 bytes at a time, so
 * offset should be a multiple of 1024.
 *
 * @param handle   qcctl handle
 * @param offset   offset of OTA bin
 * @param data     Data
 * @param data_len Data length
 *
 * @return 0 on success
 */
int qcctl_ota_program_part(qcctl_handle_t handle, uint32_t offset, const uint8_t *data, uint32_t data_len);

/**
 * @brief Commit OTA.
 * Switch active partition.
 *
 * @param handle qcctl handle
 *
 * @return 0 on success
 */
int qcctl_ota_commit(qcctl_handle_t handle);

/**
 * @brief OTA demo.
 *
 * @param handle      qcctl handle
 * @param fw_bin_path Path to OTA bin
 *
 * @return 0 on succes
 */
int qcctl_ota(qcctl_handle_t handle, const char *fw_bin_path);

/**
 * @brief Convert RNM Wi-Fi authentication method to descripting string
 *
 * @param auth Wi-Fi authentication method
 *
 * @return Corresponding string
 */
const char *qcctl_auth_to_str(uint8_t auth);

/**
 * @brief Convert RNM Wi-Fi cipher to descripting string
 *
 * @param cipher Wi-Fi cipher
 *
 * @return Corresponding string
 */
const char *qcctl_cipher_to_str(uint8_t cipher);

/**
 * @brief Send a user extension message to device
 *
 * @param handle  qcctl handle
 * @param msg     User ext msg payload
 * @param msg_len msg length
 *
 * @return 0 on success
 */
int qcctl_rnm_user_ext_send(qcctl_handle_t handle, const void *msg, size_t msg_len);

/**
 * @brief Send a response message
 *
 * @param handle  qcctl handle
 * @param cmdid   the acking command ID
 * @param msgid   the acking message ID
 * @param msg     User msg payload
 * @param msg_len User msg length in bytes
 *
 * @return 0 on success
 */
int qcctl_rnm_send_resp(qcctl_handle_t handle, uint16_t cmdid, uint16_t msgid,
        const void *msg, size_t msg_len);

/**
 * @brief Send a user extension message to device
 *
 * @param handle  qcctl handle
 * @param msg     User ext msg payload
 * @param msg_len msg length
 * @param cmd     user cmd
 *
 * @return 0 on success
 */
int qcctl_rnm_user_send_extension(qcctl_handle_t handle, const void *msg, size_t msg_len, uint16_t cmd);
