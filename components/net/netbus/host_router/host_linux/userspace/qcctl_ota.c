#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "../rnm_msg.h"
#include "qcctl.h"
#include "utils.h"
#include "qcctl_log.h"

#define OTA_START_TO        (100 * 1000)
#define OTA_ABORT_TO        100
#define OTA_PROGRAM_PART_TO 1000
#define OTA_COMMIT_TO       1000

#ifdef QCC74x_TAG
#undef QCC74x_TAG
#define QCC74x_TAG  "qcc74x_ota"
#endif

int qcctl_ota_start(qcctl_handle_t handle)
{
    int ret;
    rnm_ota_msg_t msg;
    rnm_ota_msg_t *omsg;

    memset(&msg, 0, sizeof(msg));
    msg.hdr.cmd = QC_CMD_OTA;
    msg.msg_type = QCC74x_OTA_MSG_START;

    if ((ret = qcctl_rnm_msg_expect(handle, &msg, sizeof(msg), (void *)&omsg, OTA_START_TO)))
        return ret;
    if (omsg->status.status_type == QCC74x_OTA_START_ACK) {
        free(omsg);
        return 0;
    }
    free(omsg);
    return -1;
}

int qcctl_ota_abort(qcctl_handle_t handle)
{
    int ret;
    rnm_ota_msg_t msg;
    rnm_ota_msg_t *omsg;

    memset(&msg, 0, sizeof(msg));
    msg.hdr.cmd = QC_CMD_OTA;
    msg.msg_type = QCC74x_OTA_MSG_ABORT;

    if ((ret = qcctl_rnm_msg_expect(handle, &msg, sizeof(msg), (void *)&omsg, OTA_ABORT_TO)))
        return ret;
    if (omsg->status.status_type == QCC74x_OTA_ABORT_ACK) {
        free(omsg);
        return 0;
    }
    free(omsg);
    return -1;
}

int qcctl_ota_program_part(qcctl_handle_t handle, uint32_t offset, const uint8_t *data, uint32_t data_len)
{
    int ret;
    uint8_t msg_buf[sizeof(rnm_ota_msg_t) + RNM_OTA_PART_SIZE];
    rnm_ota_msg_t *msg = (rnm_ota_msg_t *)msg_buf;
    rnm_ota_msg_t *omsg;
    const size_t msg_len = sizeof(*msg) + data_len;
    uint16_t ota_status;

    memset(msg_buf, 0, sizeof(msg_buf));
    msg->hdr.cmd = QC_CMD_OTA;
    msg->msg_type = QCC74x_OTA_MSG_PROGRAM_PART;
    msg->program_part.offset = offset;
    msg->program_part.data_len = data_len;
    memcpy(&msg->program_part.data, data, data_len);

    if ((ret = qcctl_rnm_msg_expect(handle, msg, msg_len, (void *)&omsg, OTA_PROGRAM_PART_TO)))
        return ret;
    ota_status = omsg->status.status_type;
    free(omsg);
    if (ota_status == QCC74x_OTA_PROGRAM_ACK || ota_status == QCC74x_OTA_FIN_ACK)
        return 0;
    return -1;
}

int qcctl_ota_commit(qcctl_handle_t handle)
{
    int ret;
    rnm_ota_msg_t msg;
    rnm_ota_msg_t *omsg;

    memset(&msg, 0, sizeof(msg));
    msg.hdr.cmd = QC_CMD_OTA;
    msg.msg_type = QCC74x_OTA_MSG_COMMIT;

    if ((ret = qcctl_rnm_msg_expect(handle, &msg, sizeof(msg), (void *)&omsg, OTA_COMMIT_TO)))
        return ret;
    if (omsg->status.status_type == QCC74x_OTA_COMMIT_ACK) {
        free(omsg);
        return 0;
    }
    free(omsg);
    return -1;
}

int qcctl_ota(qcctl_handle_t handle, const char *fw_bin_path)
{
    int ret = -1;
    uint8_t read_buf[RNM_OTA_PART_SIZE];
    FILE *fp = NULL;
    long bin_size;
    uint32_t write_offset = 0;

    if (!(fp = fopen(fw_bin_path, "rb"))) {
        qcc74x_loge("failed to open %s, %d, %s\n",
                fw_bin_path, errno, strerror(errno));
        goto cleanup;
    }

    if ((bin_size = get_file_len(fp)) < 0) {
        qcc74x_loge("failed to get file len, %d, %s\n", errno, strerror(errno));
        goto cleanup;
    }

    if ((ret = qcctl_ota_start(handle))) {
        qcc74x_loge("START OTA failed, %d\n", ret);
        goto cleanup;
    }
    qcc74x_logi("OTA START DONE\n");

    while (write_offset < bin_size) {
        bool is_last_part = false;
        uint32_t program_size = RNM_OTA_PART_SIZE;
        if (bin_size - write_offset <= RNM_OTA_PART_SIZE) {
            is_last_part = true;
            if (bin_size - write_offset < RNM_OTA_PART_SIZE) {
                program_size = bin_size - write_offset;
            }
        }
        size_t read_ret = fread(read_buf, 1, program_size, fp);
        if (read_ret == program_size) {
            qcc74x_logi("To program offset %u/%lu%s, ", write_offset, bin_size, is_last_part ? "(LAST)" : "");
            if ((ret = qcctl_ota_program_part(handle, write_offset, read_buf, program_size))) {
                qcc74x_loge("program erred with code %d\n", ret);
                goto cleanup;
            }
            qcc74x_logi("Done\n");
            write_offset += program_size;
        } else {
            qcc74x_loge("read ota file error\n");
            goto cleanup;
        }
    }

    if ((ret = qcctl_ota_commit(handle))) {
        qcc74x_loge("failed to commit, %d", ret);
        goto cleanup;
    } else {
        qcc74x_logi("OTA finished\n");
        ret = 0;
    }

cleanup:
    if (fp)
        fclose(fp);
    return ret;
}