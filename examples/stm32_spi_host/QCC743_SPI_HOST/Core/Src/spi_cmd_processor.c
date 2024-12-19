/*
 * spi_cmd_processor.c
 *
 *  Created on: Sep 24, 2024
 */

#include <stdio.h>
#include <string.h>
#include "cmsis_os2.h"
#include "spisync.h"
#include "app_pm.h"
#include "spi_cmd_processor.h"

struct spi_cmd_processor {
	int tsk_stop;
	osThreadId_t tsk_handle;
};

#define SPI_TLV_TAG_RNM	0x1
#define SPI_TLV_TAG_CMD	0x2

struct spi_tlv_header {
	uint32_t tag;
	/* Length of value. */
	uint32_t len;
	uint8_t value[];
} __attribute__((packed));

struct spi_cmd_header {
	uint32_t id : 10;
	uint32_t rsvd : 20;
	uint32_t dir : 1;
	uint32_t ack : 1;
	uint16_t seq;
	uint16_t len;
} __attribute__((packed));

struct spi_cmd_handler {
	int (*req_cb)(struct spi_cmd_processor *proc, struct spi_cmd_event *evt);
	int (*ack_cb)(struct spi_cmd_processor *proc, struct spi_cmd_event *evt);
};

static struct spi_cmd_processor processor;

static int spi_sleep_ackcb(struct spi_cmd_processor *proc, struct spi_cmd_event *evt)
{
	return app_pm_handle_sleep_ack(evt);
}

static int spi_wakeup_ackcb(struct spi_cmd_processor *proc, struct spi_cmd_event *evt)
{
	return app_pm_handle_wakeup_ack(evt);
}

static const struct spi_cmd_handler handlers[] = {
	[SPI_CMD_WAKEUP_REQUEST] = {NULL, spi_wakeup_ackcb},
	[SPI_CMD_SLEEP_REQUEST] = {NULL, spi_sleep_ackcb},
};

int spi_send_cmd_event(struct spi_cmd_event *evt)
{
	int ret, off = 0;
	spisync_msg_t msg;
	uint8_t buffer[SPISYNC_PAYLOADBUF_LEN];
	struct spi_tlv_header *tlv_hdr;
	struct spi_cmd_header *cmd_hdr;

	if (!evt || evt->len >= sizeof(evt->value))
		return -1;

	/* Build TLV header. */
	tlv_hdr = (struct spi_tlv_header *)&buffer[off];
	tlv_hdr->tag = SPI_TLV_TAG_CMD;
	tlv_hdr->len = evt->len + sizeof(struct spi_cmd_header);
	off += sizeof(struct spi_tlv_header);
	/* Build command header. */
	cmd_hdr = (struct spi_cmd_header *)&buffer[off];
	cmd_hdr->ack = evt->ack == SPI_CMD_IS_ACK;
	cmd_hdr->dir = evt->dir == SPI_CMD_DIR_FROM_HOST;
	cmd_hdr->len = evt->len;
	cmd_hdr->id = evt->cmdid;
	cmd_hdr->seq = evt->seq;
	cmd_hdr->rsvd = 0;
	off += sizeof(struct spi_cmd_header);
	if (evt->len > 0) {
		memcpy(&buffer[off], evt->value, evt->len);
		off += evt->len;
	}
	spisync_build_msg_needcopy(&msg, SPISYNC_TYPEMSG_SYSCTRL, buffer, off, 5000);
	ret = spisync_write(get_spisync_handle(), &msg, 0);
	if (ret <= 0) {
		printf("Failed to send cmd event due to spisync_write, %d\r\n", ret);
		return -1;
	}
	return 0;
}

static int spi_parse_cmd(uint8_t *buffer, int len, struct spi_cmd_event *evt)
{
	struct spi_cmd_header cmd;
	uint8_t *ptr = buffer;
	struct spi_tlv_header tlv;

	/* Parse TLV header. */
	if (len < sizeof(tlv))
		return -1;

	memcpy(&tlv, ptr, sizeof(tlv));
	if (tlv.tag != SPI_TLV_TAG_CMD)
		return -2;

	if (!tlv.len || tlv.len < sizeof(cmd))
		return -3;

	ptr += sizeof(tlv);
	len -= sizeof(tlv);
	/* Parse command header. */
	if (len < sizeof(cmd))
		return -4;

	memcpy(&cmd, ptr, sizeof(cmd));
	if (cmd.id >= SPI_CMD_MAX)
		return -5;
	if (cmd.len > sizeof(evt->value))
		return -6;

	evt->cmdid = cmd.id;
	evt->seq = cmd.seq;
	evt->dir = cmd.dir;
	evt->ack = cmd.ack;
	evt->len = cmd.len;

	ptr += sizeof(cmd);
	len -= sizeof(cmd);
	/* Check command payload. */
	if (len < cmd.len)
		return -7;

	if (cmd.len)
		memcpy(evt->value, ptr, cmd.len);
	return 0;
}

static int spi_process_cmd(struct spi_cmd_processor *proc, struct spi_cmd_event *evt)
{
	const struct spi_cmd_handler *handler;

	if (evt->cmdid >= SPI_CMD_MAX) {
		printf("Invalid spi cmd id 0x%x\r\n", evt->cmdid);
		return -1;
	}

	handler = &handlers[evt->cmdid];
	if (!handler) {
		printf("No handler for spi cmd 0x%x\r\n", evt->cmdid);
		return -2;
	}

	if (evt->dir)
		printf("Warning: received spi cmd with wrong direction\r\n");

	if (evt->ack) {
		if (handler->ack_cb)
			handler->ack_cb(proc, evt);
	} else {
		if (handler->req_cb)
			handler->req_cb(proc, evt);
	}
	return 0;
}

static void spi_cmd_processor_task(void *arg)
{
	int ret;
	spisync_msg_t msg;
	struct spi_cmd_event evt;
	struct spi_cmd_processor *proc = arg;
	uint8_t buffer[SPISYNC_PAYLOADBUF_LEN];

	while (!proc->tsk_stop) {
		spisync_build_msg_zerocopy(&msg, SPISYNC_TYPEMSG_SYSCTRL, buffer, sizeof(buffer), 2000);
		ret = spisync_read(get_spisync_handle(), &msg, 0);
		if (ret < 0) {
			printf("Failed to read spi command, %d\r\n", ret);
			osDelay(1000);
		}

		if (!ret)
			continue;

		ret = spi_parse_cmd(buffer, msg.buf_len, &evt);
		if (ret < 0) {
			printf("Failed to parse incoming spi command %d\r\n", ret);
			continue;
		}
		spi_process_cmd(proc, &evt);
	}
}

int spi_cmd_processor_init(void)
{
	osThreadAttr_t tsk_attr = {
		.name = "spi_cmd_processor",
		.priority = osPriorityRealtime,
		.stack_size = 1024 * 8,
	};

	processor.tsk_stop = 0;
    processor.tsk_handle = osThreadNew(spi_cmd_processor_task, &processor, &tsk_attr);
    if (!processor.tsk_handle) {
    	printf("Failed to create spi cmd processor task\r\n");
    	return -1;
    }
    return 0;
}
