#include <stdbool.h>
#include "mfg_sdio.h"
#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include "qcc74x_uart.h"

#define LOCAL_FILE_NUM  (0x0009)

uint32_t sdu_port_init=0;
#define SUCCESS 0
#define ERROR 1

uint8_t g_uart_id;
struct qcc74x_device_s *uartn;
bool recved_sdio_host_msg = false;

enum {
    SDIO_MSG_HELLO,
    SDIO_MSG_UART_DATA,
};

struct __attribute__((packed)) sdio_msg {
    uint16_t type;
    uint8_t payload[];
};

int qcc74x_uart_data_send_do(uint8_t id, uint8_t data);

void out_str(const char *str)
{
    uartn = qcc74x_device_get_by_name("uart0");
    const size_t sl = strlen(str);
    for (int i = 0; i < sl; ++i) {
        //qcc74x_uart_data_send_do(g_uart_id, str[i]);
        qcc74x_uart_putchar(uartn, str[i]);
    }
}

char dbg_buf[256];
extern void mfg_deal_raw_input(uint8_t *buf,uint32_t len, int from_isr);
static void sdio_recv_cb(void *arg, const void *data_ptr, const uint16_t data_len)
{
int32_t mfg_cmd_input_from_isr(uint8_t *data, uint16_t len);
    struct sdio_msg *msg = (struct sdio_msg *)data_ptr;
    recved_sdio_host_msg = true;

    switch (msg->type) {
        case SDIO_MSG_HELLO:
            out_str("Received HELLO_MSG\r\n");
            break;
        case SDIO_MSG_UART_DATA:
            if (data_len <= sizeof(*msg)) {
                out_str("Error uart msg len\r\n");
                break;
            }
            mfg_deal_raw_input(msg->payload, data_len - sizeof(*msg), 1);
            break;
        default:
            break;
    }
}

int32_t qcc74x_sdio_init(void *data)
{
    /*sdu is ready all the time,not init or deint when interface switch */
	/* sdu_init(); */
    qcc74x_sdu_init();
    qcc74x_sdio_handshake();
    qcc74x_sdio_read_cb_register(NULL, sdio_recv_cb, NULL);
	sdu_port_init=1;
	return SUCCESS;
}

int32_t qcc74x_bootrom_sdio_handshake_poll(void *data)
{
#if 0
    int32_t ret=0;

    ret=sdu_host_check(0);
    if(ret>=0){
        return SUCCESS;
    }

	return ERROR;
#endif
    return 0;
}

void qcc74x_sdio_cmd_process(void)
{
#if 0
	uint32_t read_len=0;
	uint32_t *data;

	data=sdu_receive_data(&read_len);
	if(data!=NULL){
		mfg_deal_raw_input(data,read_len);
	}
#endif
}

int32_t qcc74x_sdio_write(uint32_t *data,uint32_t len)
{
#if 0
	sdu_send_data(data,len);

	return SUCCESS;
#endif
    return 0;
}

int qcc74x_uart_data_send(uint8_t id, uint8_t data)
{
#if 0
	if(sdu_port_init==0){
		if(-1==sdu_send_data(&data,1)){
			qcc74x_uart_data_send_do(id, 'T');
			qcc74x_uart_data_send_do(id, 'O');
			qcc74x_uart_data_send_do(id, '\r');
			qcc74x_uart_data_send_do(id, '\n');
		}
	}else{
		qcc74x_uart_data_send_do(id, data);
	}
#endif
    int ret;
    uartn = qcc74x_device_get_by_name("uart0");
    qcc74x_uart_putchar(uartn, data);
    //qcc74x_uart_data_send_do(id, data);
    //g_uart_id = id;
#if 0
    if (recved_sdio_host_msg) {
        ret = qcc74x_hal_sdio_write(NULL, &data, 1);
        if (ret) {
			qcc74x_uart_data_send_do(id, 't');
			qcc74x_uart_data_send_do(id, 'T');
			qcc74x_uart_data_send_do(id, 'O');
			qcc74x_uart_data_send_do(id, '\r');
			qcc74x_uart_data_send_do(id, '\n');
        }
    }
    #endif

	return SUCCESS;
}

int32_t qcc74x_bootrom_sdio_wait_tx_idle(uint32_t timeout)
{
	return SUCCESS;
}

int32_t qcc74x_bootrom_sdio_deinit()
{
    /*sdu is ready all the time,not init or deint when interface switch */
	return SUCCESS;
}

