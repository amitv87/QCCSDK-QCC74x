#include <stdio.h>
#include "qcctl.h"
#include "../rnm_msg.h"

struct rnm_msg_cb {
    int (*req_cb)(qcctl_handle_t handle, rnm_base_msg_t *msg, void *data, int len);
    int (*ack_cb)(qcctl_handle_t handle, rnm_base_msg_t *msg, void *data, int len);
};

#if THIS_IS_AN_EXAMPLE
static int on_example_cmd_req(void *handle, rnm_ack_msg_t *msg, void *data, int len)
{
    qcctl_handle_t qcctl_handle = handle;

    do_something_with(handle, msg);
    /* send response if necessary */
    qcctl_rnm_send_resp(qcctl_handle, msg->cmd, msg->msg_id, resp_data, resp_len);
    return 0;
}

static int on_example_cmd_ack(void *handle, rnm_ack_msg_t *msg, void *data, int len)
{
    qcctl_handle_t qcctl_handle = handle;

    do_something_with(handle, msg);
    return 0;
}
/* THIS_IS_AN_EXAMPLE */
#endif
static int example_cmd_handle_rsp(qcctl_handle_t handle, rnm_base_msg_t *msg, void *data, int len)
{
    (void)handle;
    (void)msg;
    (void)len;
    printf("recv: %s\n", (char *)data);
    
    const char resp_msg[] = "response from host!";
    qcctl_rnm_send_resp(handle, QC_CMD_USER_EXT_RSP, 0, resp_msg, sizeof(resp_msg));
    
    return 0;
}

static int example_cmd_handle_no_rsp(qcctl_handle_t handle, rnm_base_msg_t *msg, void *data, int len)
{
    (void)handle;
    (void)msg;
    (void)len;
    printf("recv: %s\n", (char *)data);
    
    return 0;
}

static int example_cmd_handle_ack(qcctl_handle_t handle, rnm_base_msg_t *msg, void *data, int len)
{
    (void)handle;
    (void)msg;
    (void)len;
    printf("recv: %s\n", (char *)data);
    
    return 0;
}

static int on_dummy_cmd_req(qcctl_handle_t handle, rnm_base_msg_t *msg, void *data, int len)
{
    (void)handle;
    (void)msg;
    (void)data;
    (void)len;
    return 0;
}

static int on_dummy_cmd_ack(qcctl_handle_t handle, rnm_base_msg_t *msg, void *data, int len)
{
    (void)handle;
    (void)msg;
    (void)data;
    (void)len;
    return 0;
}

static struct rnm_msg_cb msg_handlers[] = {
    [QC_CMD_USER_EXT_NO_RSP] = {example_cmd_handle_no_rsp, NULL},
    [QC_CMD_USER_EXT_RSP] = {example_cmd_handle_rsp, example_cmd_handle_ack},
    /* this dummy entry here is just hanging out to make the array bigger */
    [QC_CMD_MAX] = {on_dummy_cmd_req, on_dummy_cmd_ack},
};

int qcctl_handle_one_rnm(qcctl_handle_t handle, rnm_base_msg_t *msg, void *data, int len)
{
    int ret = -1;
    struct rnm_msg_cb *cb;

    if (!handle || !msg)
        return -1;

    if (msg->cmd >= QC_CMD_MAX)
        return -1;

    cb = &msg_handlers[msg->cmd];
    if (msg->flags & RNM_MSG_FLAG_ACK) {
        if (cb->ack_cb)
            ret = cb->ack_cb(handle, msg, data, len);
    } else {
        if (cb->req_cb)
            ret = cb->req_cb(handle, msg, data, len);
    }
    return ret;
}
