/*
 * spi_cmd_processor.h
 *
 *  Created on: Sep 24, 2024
 */

#ifndef INC_SPI_CMD_PROCESSOR_H_
#define INC_SPI_CMD_PROCESSOR_H_

#include <stdint.h>

/* Commands */
#define SPI_CMD_WAKEUP_REQUEST	0x1
#define SPI_CMD_SLEEP_REQUEST	0x2
#define SPI_CMD_MAX				0x3

/* Command direction. */
#define SPI_CMD_DIR_TO_HOST		0
#define SPI_CMD_DIR_FROM_HOST	1

/* Request or Acknowledge. */
#define SPI_CMD_IS_REQ	0
#define SPI_CMD_IS_ACK	1

#define SPI_CMD_EVENT_GEN(e, __id, __dir, __ack, __seq, __len)	\
	do {					\
		(e).cmdid = __id;	\
		(e).dir = __dir;	\
		(e).ack = __ack;	\
		(e).seq = __seq;	\
		(e).len = __len;	\
	} while (0)

struct spi_cmd_event {
	uint16_t cmdid;
	uint16_t seq;
	uint8_t dir;
	uint8_t ack;
	/* Length of value in bytes. */
	uint16_t len;
	uint8_t value[1536];
};

int spi_cmd_processor_init(void);
int spi_send_cmd_event(struct spi_cmd_event *evt);

#endif /* INC_SPI_CMD_PROCESSOR_H_ */
