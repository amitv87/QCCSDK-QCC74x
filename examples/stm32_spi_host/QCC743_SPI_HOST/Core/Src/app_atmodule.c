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

int spisync_ready = 0;
spisync_t spisync_ctx;


int at_iperf_udp_tx_start(spisync_t *p_spisync, char ip_addr[20], int port)
{
    char at_cmd[128] = {0};
 
    strlcpy(at_cmd, "AT+CIPMUX=0\r\n", sizeof(at_cmd));
    spisync_write(p_spisync, 0, at_cmd, strlen(at_cmd), 10000);
	osDelay(200);
    
    sprintf(at_cmd, "AT+CIPSTART=\"UDP\",\"%s\",%d\r\n", ip_addr, port);
    spisync_write(p_spisync, 0, at_cmd, strlen(at_cmd), 10000);
	osDelay(2000);

    strlcpy(at_cmd, "AT+CIPSEND\r\n", sizeof(at_cmd));
    spisync_write(p_spisync, 0, at_cmd, strlen(at_cmd), 10000);
	osDelay(200);

    return 0;
}

int at_iperf_tcp_tx_start(spisync_t *p_spisync, char ip_addr[20], int port)
{
    char at_cmd[128] = {0};
 
    strlcpy(at_cmd, "AT+CIPMUX=0\r\n", sizeof(at_cmd));
    spisync_write(p_spisync, 0, at_cmd, strlen(at_cmd), 10000);
	osDelay(200);
    
    sprintf(at_cmd, "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n", ip_addr, port);
    spisync_write(p_spisync, 0, at_cmd, strlen(at_cmd), 10000);
	osDelay(2000);

    strlcpy(at_cmd, "AT+CIPSEND\r\n", sizeof(at_cmd));
    spisync_write(p_spisync, 0, at_cmd, strlen(at_cmd), 10000);
	osDelay(200);

    return 0;
}

int at_iperf_udp_rx_start(spisync_t *p_spisync, char ip_addr[20], int port)
{
    char at_cmd[128] = {0};
/* 
    strlcpy(at_cmd, "AT+CIPMUX=1\r\n", sizeof(at_cmd));
    spisync_write(p_spisync, 0, at_cmd, strlen(at_cmd), 10000);
	osDelay(200);
    
    sprintf(at_cmd, "AT+CIPSERVER=1,%d,\"UDP\"\r\n", port);
    spisync_write(p_spisync, 0, at_cmd, strlen(at_cmd), 10000);
	osDelay(200);

    strlcpy(at_cmd, "AT+CIPSEND\r\n", sizeof(at_cmd));
    spisync_write(p_spisync, 0, at_cmd, strlen(at_cmd), 10000);
	osDelay(200);

*/
 
    strlcpy(at_cmd, "AT+CIPMUX=0\r\n", sizeof(at_cmd));
    spisync_write(p_spisync, 0, at_cmd, strlen(at_cmd), 10000);
	osDelay(200);
    
    sprintf(at_cmd, "AT+CIPSTART=\"UDP\",\"%s\",%d\r\n", ip_addr, port);
    spisync_write(p_spisync, 0, at_cmd, strlen(at_cmd), 10000);
	osDelay(2000);

    strlcpy(at_cmd, "AT+CIPSEND\r\n", sizeof(at_cmd));
    spisync_write(p_spisync, 0, at_cmd, strlen(at_cmd), 10000);
	osDelay(200);

    return 0;
}

int at_iperf_tcp_rx_start(spisync_t *p_spisync, char ip_addr[20], int port)
{
    char at_cmd[128] = {0};
/* 
    strlcpy(at_cmd, "AT+CIPMUX=1\r\n", sizeof(at_cmd));
    spisync_write(p_spisync, 0, at_cmd, strlen(at_cmd), 10000);
	osDelay(200);
    
    sprintf(at_cmd, "AT+CIPSERVER=1,%d,\"TCP\"\r\n", port);
    spisync_write(p_spisync, 0, at_cmd, strlen(at_cmd), 10000);
	osDelay(200);

    strlcpy(at_cmd, "AT+CIPSEND\r\n", sizeof(at_cmd));
    spisync_write(p_spisync, 0, at_cmd, strlen(at_cmd), 10000);
	osDelay(200);
*/
 
    strlcpy(at_cmd, "AT+CIPMUX=0\r\n", sizeof(at_cmd));
    spisync_write(p_spisync, 0, at_cmd, strlen(at_cmd), 10000);
	osDelay(200);
    
    sprintf(at_cmd, "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n", ip_addr, port);
    spisync_write(p_spisync, 0, at_cmd, strlen(at_cmd), 10000);
	osDelay(2000);

    strlcpy(at_cmd, "AT+CIPSEND\r\n", sizeof(at_cmd));
    spisync_write(p_spisync, 0, at_cmd, strlen(at_cmd), 10000);
	osDelay(200);

    return 0;
}

int at_iperf_stop(spisync_t *p_spisync)
{
    char at_cmd[128] = {0};
   
	osDelay(200);
 
    strlcpy(at_cmd, "+++", sizeof(at_cmd));
    spisync_write(p_spisync, 0, at_cmd, strlen(at_cmd), 10000);
	osDelay(100);

    strlcpy(at_cmd, "AT+CIPCLOSE\r\n", sizeof(at_cmd));
    spisync_write(p_spisync, 0, at_cmd, strlen(at_cmd), 10000);
	osDelay(200);
    strlcpy(at_cmd, "AT+CIPCLOSE=0\r\n", sizeof(at_cmd));
    spisync_write(p_spisync, 0, at_cmd, strlen(at_cmd), 10000);
	osDelay(200);

	return 0;
}

void at_wifi_connect(spisync_t *p_spisync, char ssid[32], char pswd[64])
{
    char at_cmd[128] = {0};

    strlcpy(at_cmd, "AT+CWMODE=1\r\n", sizeof(at_cmd));
    spisync_write(p_spisync, 0, at_cmd, strlen(at_cmd), 10000);
	osDelay(100);

    strlcpy(at_cmd, "+++ 0\r\n", sizeof(at_cmd));
    spisync_write(p_spisync, 0, at_cmd, strlen(at_cmd), 10000);
	osDelay(100);
    
    sprintf(at_cmd, "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, pswd?pswd:"");
    spisync_write(p_spisync, 0, at_cmd, strlen(at_cmd), 10000);
	osDelay(12000);
}

osSemaphoreId_t g_recvsem;

static int _at_to_console(uint8_t *buf, uint32_t len, void *arg)
{
    for (int i = 0; i < len; i++) {
        putchar(buf[i]);
    }
    fflush(stdout);
    return 0;
}

static int _net_data_recv(int linkid, uint8_t *buf, uint32_t size, void *arg)
{
	if (linkid == 0) {
		if (buf == NULL) {
			//recv_size = at_host_recvdata(at, linkid, recvdata, size);
			//at_ota_update(at, recvdata, recv_size);
			osSemaphoreRelease(g_recvsem);
		}
	}
	return 0;
}

static int _read_data(void *arg, void *data, int len)
{
	spisync_t *ctx = (spisync_t *)arg;

	return spisync_read(ctx, 0, data, len, 10000);
}

static int _write_data(void *arg, const void *data, int len)
{
	spisync_t *ctx = (spisync_t *)arg;

	return spisync_write(ctx, 0, data, len, 10000);
}

int at_ota_start(at_host_handle_t at, char ip_addr[20], int port)
{
	at_host_recvmode_set(at, AT_NET_RECV_MODE_PASSIVE);
	osDelay(10);

    at_host_printf(at, AT_HOST_RESP_EVT_OK, "AT+CIPMUX=1\r\n");
    osDelay(10);

	at_host_printf(at, AT_HOST_RESP_EVT_OK, "AT+CIPRECVBUF=0,4096\r\n");
	osDelay(10);

    at_host_printf(at, AT_HOST_RESP_EVT_OK, "AT+WEVT=0\r\n");
    osDelay(10);
//    at_host_printf(at, AT_HOST_RESP_EVT_OK, "AT+WIPS=0\r\n");
//    osDelay(10);
	at_host_printf(at, AT_HOST_RESP_EVT_OK, "AT+OTASTART=1\r\n");
	osDelay(10);

	at_host_net_recv_register(at, _net_data_recv, at);
	at_host_receive_register(at, NULL, NULL);

	at_host_printf(at, AT_HOST_RESP_EVT_OK, "AT+CIPSTART=0,\"TCP\",\"%s\",%d\r\n", ip_addr, port);
	osDelay(10);
}

int at_ota_finish(at_host_handle_t at)
{
	at_host_net_recv_register(at, NULL, NULL);
	at_host_receive_register(at, _at_to_console, NULL);

    at_host_printf(at, AT_HOST_RESP_EVT_OK, "AT+CIPCLOSE=0\r\n");
    osDelay(10);
    at_host_printf(at, AT_HOST_RESP_EVT_OK, "AT+WEVT=1\r\n");
    osDelay(10);
//    at_host_printf(at, AT_HOST_RESP_EVT_OK, "AT+WIPS=1\r\n");
//    osDelay(10);
    at_host_printf(at, AT_HOST_RESP_EVT_OK, "AT+OTAFIN\r\n");
    osDelay(10);
    return 0;
}


int at_ota_update(at_host_handle_t at, uint8_t *buf, uint32_t len)
{
	uint8_t *ota_buffer = buf;
    at_ota_header_t *ota_header;
    uint32_t write_len = 0;
    static uint32_t file_size, ota_size = 0;
    static uint8_t ota_ishead = 0;

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


    at_host_printf(at, AT_HOST_RESP_EVT_OK|AT_HOST_RESP_EVT_WAIT_DATA, "AT+OTASEND=%d\r\n", len);
    osDelay(10);

    if (ota_size < file_size) {
        write_len = (ota_size + len > file_size) ? (file_size - ota_size) : len;
        at_host_send(at, AT_HOST_RESP_EVT_SEND_OK, ota_buffer, write_len);
        osDelay(10);
    }
    ota_size += write_len;
    printf("OTA trans size:%d\r\n", ota_size);
    if (ota_size == file_size) {

    	ota_ishead = 0;
    	file_size = 0;
    	ota_size = 0;
        at_ota_finish(at);
    }
    return 0;
}


static void __app_task(void *arg)
{
	static uint8_t recvdata[4096];
	int recv_size;
	at_host_handle_t at = (at_host_handle_t)arg;

	g_recvsem = osSemaphoreNew(1, 0, NULL);

	while (1) {
		osSemaphoreAcquire(g_recvsem, 0xffffffff);
		while ((recv_size = at_host_recvdata(at, 0, recvdata, sizeof(recvdata))) > 0) {
			at_ota_update(at, recvdata, recv_size);
			osDelay(10);
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
	spisync_ready = 1;

	at = at_host_init(&host_drv, (void *)&spisync_ctx);

	at_host_receive_register(at, _at_to_console, NULL);

	osThreadNew(__app_task, at, &_app_task_attr);

	return at;
}

