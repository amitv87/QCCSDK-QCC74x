/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os2.h"
#include "app_atmodule.h"
/* Private includes ----------------------------------------------------------*/

/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "spisync.h"
/* USER CODE END Includes */

const osThreadAttr_t at_tx_task_attr = {
  .name = "at_tx",
  .priority = (osPriority_t) osPriorityRealtime7,
  .stack_size = 4096
};

const osThreadAttr_t at_rx_task_attr = {
  .name = "at_rx",
  .priority = (osPriority_t) osPriorityRealtime7,
  .stack_size = 4096
};

static spisync_t spisync_ctx;

static int _at_to_console(uint8_t *buf, uint32_t len, void *arg);
static int _read_data(void *arg, void *data, int len);

void at_gpio_debug()
{
	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_7);
	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_7);
}

static int __at_iperf_statis(uint8_t *buf, uint32_t size, void *arg)
{
//	int linkid, data_len = 0;
	static uint32_t last = 0;
	static uint32_t totle_len = 0;

//	if (strncmp("\r\n+IPD:", buf, strlen("\r\n+IPD:")) != 0) {
//		return 0;
//	}
//    sscanf(buf, "\r\n+IPD:%d,%d", &linkid, &data_len);

	totle_len += size;
	if (!last) {
		last = osKernelGetTickCount();
	}
	if (osKernelGetTickCount() - last >= 1000) {
		last = osKernelGetTickCount();
		printf("RX Bandwidth: %f Mbps\r\n", (float)totle_len*8/1000/1000);
		totle_len = 0;
	}
	return 0;
}
#define TEST_BUFFER_SIZE   (6000)
int at_iperf_udp_tx_start(at_host_handle_t at, char ip_addr[20], int port)
{
	static uint32_t last = 0;
	static uint32_t totle_len = 0;
	int ret, run_count = 0;
	uint8_t *tx_buf;

	at_host_receive_register(at, NULL, NULL);

    at_host_printf(at, AT_HOST_RESP_EVT_OK, 100, "AT+CIPMUX=1\r\n");
    osDelay(100);

    at_host_printf(at, AT_HOST_RESP_EVT_OK, 100, "AT+CIPSTART=0,\"UDP\",\"%s\",%d\r\n", ip_addr, port);
    osDelay(100);

    tx_buf = pvPortMalloc(TEST_BUFFER_SIZE);
	memset(tx_buf, 0x55, TEST_BUFFER_SIZE);

	last = osKernelGetTickCount();

    while (run_count <= 10) {

    	at_host_printf(at, AT_HOST_RESP_EVT_OK|AT_HOST_RESP_EVT_WAIT_DATA, 100, "AT+CIPSEND=0,%d\r\n", TEST_BUFFER_SIZE);

    	ret = at_host_send(at, AT_HOST_RESP_EVT_RECV_BTYES, tx_buf, TEST_BUFFER_SIZE, 100);
    	if (ret > 0) {
    		totle_len += ret;
    	}
    	if (osKernelGetTickCount() - last >= 1000) {
    		last = osKernelGetTickCount();
    		printf("TX Bandwidth: %f Mbps\r\n", (float)totle_len*8/1000/1000);
    		totle_len = 0;
    		run_count++;
    	}
    }
    osDelay(200);
    vPortFree(tx_buf);
    at_iperf_stop(at, 1);
    return 0;
}

#define AT_RESP_OK        "\r\nOK\r\n"
#define AT_RESP_WAIT_DATA "\r\n>"
#define AT_RESP_RECV_CFM  "Recv 7680 bytes\r\n"
#define AT_RESP_SEND_OK   "\r\nSEND OK\r\n"

int at_iperf_tcp_tx_start(at_host_handle_t at, char ip_addr[20], int port)
{
	static uint32_t last = 0;
	static uint32_t totle_len = 0;
	int ret, run_count = 0;
	uint8_t *tx_buf;

	at_host_receive_register(at, NULL, NULL);

    at_host_printf(at, AT_HOST_RESP_EVT_OK, 100, "AT+CIPMUX=1\r\n");
    osDelay(100);

    at_host_printf(at, AT_HOST_RESP_EVT_OK, 100, "AT+CIPSTART=0,\"TCP\",\"%s\",%d\r\n", ip_addr, port);
    osDelay(100);

    tx_buf = pvPortMalloc(TEST_BUFFER_SIZE);
	memset(tx_buf, 0x55, TEST_BUFFER_SIZE);

	last = osKernelGetTickCount();

    while (run_count <= 10) {
    	at_host_printf(at, AT_HOST_RESP_EVT_OK|AT_HOST_RESP_EVT_WAIT_DATA, 1000, "AT+CIPSEND=0,%d\r\n", TEST_BUFFER_SIZE);

    	ret = at_host_send(at, AT_HOST_RESP_EVT_RECV_BTYES, tx_buf, TEST_BUFFER_SIZE, 1000);
    	if (ret > 0) {
    		totle_len += ret;
    	}
    	if (osKernelGetTickCount() - last >= 1000) {
    		last = osKernelGetTickCount();
    		printf("TX Bandwidth: %f Mbps\r\n", (float)totle_len*8/1000/1000);
    		totle_len = 0;
    		run_count++;
    	}
    }

    vPortFree(tx_buf);
    osDelay(200);
    at_iperf_stop(at, 1);
    return 0;
}

int at_iperf_udp_rx_start(at_host_handle_t at, char ip_addr[20], int port)
{
    at_host_printf(at, AT_HOST_RESP_EVT_OK, 100, "AT+CIPMUX=1\r\n");
    osDelay(100);

    at_host_printf(at, AT_HOST_RESP_EVT_OK, 100, "AT+CIPSERVER=1,%d,\"UDP\"\r\n", port);
    osDelay(100);

    //at_host_printf(at, AT_HOST_RESP_EVT_OK, "AT+CIPSEND\r\n");
    //osDelay(100);
	at_host_receive_register(at, __at_iperf_statis, NULL);

    return 0;
}

int at_iperf_tcp_rx_start(at_host_handle_t at, char ip_addr[20], int port)
{
    at_host_printf(at, AT_HOST_RESP_EVT_OK, 100, "AT+CIPMUX=1\r\n");
    osDelay(100);

    at_host_printf(at, AT_HOST_RESP_EVT_OK, 100, "AT+CIPSERVER=1,%d,\"TCP\"\r\n", port);
    osDelay(100);

    //at_host_printf(at, AT_HOST_RESP_EVT_OK, "AT+CIPSEND\r\n");
    //osDelay(100);

	at_host_receive_register(at, __at_iperf_statis, NULL);

    return 0;
}

int at_iperf_stop(at_host_handle_t at, int is_cli)
{
    //at_host_printf(at, 0, "+++");
    //osDelay(100);

    if (is_cli) {
        at_host_printf(at, AT_HOST_RESP_EVT_OK, 100, "AT+CIPCLOSE=0\r\n");
        osDelay(100);
    } else {
        at_host_printf(at, AT_HOST_RESP_EVT_OK, 100, "AT+CIPSERVER=0,1\r\n");
        osDelay(100);
    }
	at_host_receive_register(at, _at_to_console, NULL);

	return 0;
}

SemaphoreHandle_t g_recvsem;

static int _at_to_console(uint8_t *buf, uint32_t len, void *arg)
{
    for (int i = 0; i < len; i++) {
        putchar(buf[i]);
    }
    fflush(stdout);
    return len;
}

static int _net_data_recv(int linkid, uint8_t *buf, uint32_t size, void *arg)
{
	if (linkid == 0) {
		if (buf == NULL) {
			//recv_size = at_host_recvdata(at, linkid, recvdata, size);
			//at_ota_update(at, recvdata, recv_size);
			//xSemaphoreGive(g_recvsem);
		}
	}
	return 0;
}

static int _read_data(void *arg, void *data, int len)
{
	spisync_msg_t msg;
	spisync_t *ctx = (spisync_t *)arg;
	int ret;

	spisync_build_msg_zerocopy(&msg, SPISYNC_TYPESTREAM_AT, data, len, 10000);
	ret = spisync_read(ctx, &msg, 0);

#if 0
	if (ret > 0) {
		printf("[read] %d===>", len);
		for (int i = 0; i < 25; i++) {
			printf("%c", ((char *)data)[i]);
		}
		printf("\r\n");
	}
#endif
	return ret;
}

static int _write_data(void *arg, const void *data, int len)
{
	spisync_msg_t msg;
	spisync_t *ctx = (spisync_t *)arg;
	int ret;

#if 0
	printf("[write] %d===>", len);
	for (int i = 0; i < 25; i++) {
		printf("%c", ((char *)data)[i]);
	}
	printf("\r\n");
#endif

	spisync_build_msg_zerocopy(&msg, SPISYNC_TYPESTREAM_AT, data, len, 10000);
	ret = spisync_write(ctx, &msg, 0);
	return ret;
}


static uint8_t ota_started = 0;

int at_ota_start(at_host_handle_t at, char ip_addr[20], int port)
{
	if (ota_started) {
		return -1;
	}
	at_host_recvmode_set(at, AT_NET_RECV_MODE_PASSIVE);
	osDelay(10);

    at_host_printf(at, AT_HOST_RESP_EVT_OK, 100, "AT+CIPMUX=1\r\n");
    osDelay(10);

	at_host_printf(at, AT_HOST_RESP_EVT_OK, 100, "AT+CIPRECVBUF=0,8192\r\n");
	osDelay(10);

    at_host_printf(at, AT_HOST_RESP_EVT_OK, 100, "AT+CWEVT=0\r\n");
    osDelay(10);
    at_host_printf(at, AT_HOST_RESP_EVT_OK, 100, "AT+CIPEVT=0\r\n");
    osDelay(10);

	at_host_printf(at, AT_HOST_RESP_EVT_OK, 100, "AT+OTASTART=1\r\n");
	osDelay(10);

	at_host_net_recv_register(at, _net_data_recv, at);
	at_host_receive_register(at, NULL, NULL);

	at_host_printf(at, AT_HOST_RESP_EVT_OK, 100, "AT+CIPSTART=0,\"TCP\",\"%s\",%d\r\n", ip_addr, port);
	osDelay(100);

    ota_started = 1;
    xSemaphoreGive(g_recvsem);
	return 0;
}

static uint32_t file_size, ota_size = 0;
static uint8_t ota_ishead = 0;

int at_ota_finish(at_host_handle_t at)
{
	ota_ishead = 0;
	file_size = 0;
	ota_size = 0;

    at_host_printf(at, AT_HOST_RESP_EVT_NONE, 100, "AT+CIPCLOSE=0\r\n");
    osDelay(10);
    at_host_printf(at, AT_HOST_RESP_EVT_OK, 100, "AT+CWEVT=1\r\n");
    osDelay(10);
    at_host_printf(at, AT_HOST_RESP_EVT_OK, 100, "AT+CIPEVT=1\r\n");
    osDelay(10);
    at_host_printf(at, AT_HOST_RESP_EVT_OK, 100, "AT+OTAFIN\r\n");
    osDelay(500);
	ota_started = 0;

    at_host_net_recv_register(at, NULL, NULL);
	at_host_receive_register(at, _at_to_console, NULL);

    return 0;
}

int at_ota_update(at_host_handle_t at, uint8_t *buf, uint32_t len)
{
	uint8_t *ota_buffer = buf;
    at_ota_header_t *ota_header;
    uint32_t write_len = 0;

	if (!ota_ishead) {
		/*
		 * Here's the echo after a successful TCP connection in a single connection scenario.
		 * This test sends the OTA bin immediately after the TCP connection is successful.
		 */
		ota_buffer = strstr(buf, "QCC74X_OTA");
		if (!ota_buffer) {
			return -1;
		}
        ota_header = (at_ota_header_t *)ota_buffer;

        /* If the size of the OTA bin is already known,
         * there is no need to parse the bin size using the ota_head_t type.
         */
        file_size = ota_header->u.s.len + sizeof(at_ota_header_t);

		len = len - (uint32_t)(ota_buffer - buf);
		ota_ishead = 1;
        ota_size = 0;
        printf("OTA start file_size:%d\r\n", file_size);
	}


    at_host_printf(at, AT_HOST_RESP_EVT_OK|AT_HOST_RESP_EVT_WAIT_DATA, 1000, "AT+OTASEND=%d\r\n", len);
    //osDelay(10);

    if (ota_size < file_size) {
        write_len = (ota_size + len > file_size) ? (file_size - ota_size) : len;
        at_host_send(at, AT_HOST_RESP_EVT_SEND_OK, ota_buffer, write_len, 1000);
        //osDelay(10);
    }
    ota_size += write_len;
    printf("OTA trans size:%d\r\n", ota_size);
    if (ota_size == file_size) {

        at_ota_finish(at);
        return 1;
    }
    return 0;
}

static void __app_task(void *arg)
{
	static uint8_t recvdata[4096];
	int recv_size;
	at_host_handle_t at = (at_host_handle_t)arg;

	g_recvsem = xSemaphoreCreateBinary();

	while (1) {
		xSemaphoreTake(g_recvsem, 0);
		xSemaphoreTake(g_recvsem, 0xffffffff);
		while (ota_started) {
			recv_size = at_host_recvdata(at, 0, recvdata, sizeof(recvdata), 1000);
			if (recv_size <= 0) {
				osDelay(5);
				//printf("wait recvdata\r\n");
				continue;
			}
			if (at_ota_update(at, recvdata, recv_size) == 1) {
				break;
			}
			//osDelay(10);
		}
	}
}

static const osThreadAttr_t _app_task_attr = {
  .name = "app_task",
  .priority = (osPriority_t) osPriorityRealtime7,
  .stack_size = 4096
};

at_host_handle_t at_spisync_init(void)
{
	static const at_host_drv_t host_drv = {
		.f_init_device = NULL,
		.f_deinit_device = NULL,
		.f_read_data = _read_data,
		.f_write_data = _write_data,
	};
	at_host_handle_t at;

	int err = spisync_init(&spisync_ctx, 0);
	if (err)
	  printf("failed to init spisync context, %d\r\n", err);
	else
	  printf("spisync init is done\r\n");

	at = at_host_init(&host_drv, (void *)&spisync_ctx);

	at_host_receive_register(at, _at_to_console, NULL);

	osThreadNew(__app_task, at, &_app_task_attr);

	return at;
}

