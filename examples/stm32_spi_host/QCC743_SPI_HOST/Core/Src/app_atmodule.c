/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os2.h"
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
extern int spisync_ready;
uint8_t g_iperf_tx_runing = 0;
uint8_t g_iperf_rx_runing = 0;
uint32_t rx_len = 0;
uint8_t g_ota_start = 0;

typedef struct ota_header {
    union {
        struct {
            uint8_t header[16];

            uint8_t type[4]; //RAW XZ
            uint32_t len;    //body len
            uint8_t pad0[8];

            uint8_t ver_hardware[16];
            uint8_t ver_software[16];
    
            uint8_t sha256[32];
        } s;
        uint8_t _pad[512];
    } u;
} at_ota_header_t;

static int spisync_wait_ok(spisync_t *p_spisync)
{
	int err;
	uint8_t buffer[10];
	err = spisync_read(p_spisync, 0, buffer, sizeof buffer, 10000);
	if (err <= 0) {
		return -1;
	}
	if (strstr(buffer, "OK\r\n") == NULL) {
		return -1;
	}
	return 0;
}

static int spisync_buffer_clear(spisync_t *p_spisync)
{
	int err;
	uint8_t buffer[128];
	while (1) {
		err = spisync_read(p_spisync, 0, buffer, sizeof buffer, 100);
		if (err <= 0) {
			break;
		}
	}

	return 0;
}

static uint8_t ota_ishead = 0;

static void qcc74x_ota_update(spisync_t *p_spisync, uint8_t *buf, uint32_t len)
{
	char at_cmd[64] = {0};
	uint8_t *ota_buffer = buf;
    at_ota_header_t *ota_header;
    uint32_t write_len = 0;
    static uint32_t file_size, ota_size = 0;

	if (!ota_ishead) {
		/*
		 * Here's the echo after a successful TCP connection in a single connection scenario.
		 * This test sends the OTA bin immediately after the TCP connection is successful.
		 */
		ota_buffer = strstr(buf, "QCC74X_OTA");
		if (!ota_buffer) {
			return;
		}
        ota_header = (at_ota_header_t *)ota_buffer;

        /* If the size of the OTA bin is already known,
         * there is no need to parse the bin size using the ota_head_t type.
         */
        file_size = ota_header->u.s.len + sizeof(at_ota_header_t);

		len = len - (uint32_t)(ota_buffer - buf);
		ota_ishead = 1;
        ota_size = 0;

        snprintf(at_cmd, sizeof(at_cmd), "AT+OTA-S=%d\r\n", file_size);
        spisync_write(p_spisync, 0, at_cmd, strlen(at_cmd), 10000);
    	osDelay(200);
	}
    
    if (ota_size < file_size) {
        write_len = (ota_size + len > file_size) ? (file_size - ota_size) : len;
	    spisync_write(p_spisync, 0, ota_buffer, write_len, 10000);
    }
    ota_size += write_len;
    if (ota_size == file_size) {
        printf("OTA finish size:%d\r\n", ota_size);
        at_qcc74x_ota_finish(p_spisync);
    }
}

static void __perf_rx_task(void *arg)
{
	int err;
	char rxbuf[SPISYNC_PAYLOADBUF_LEN];
	uint32_t count = 0, diff = 0;

	spisync_t *p_spisync = (spisync_t *)arg;

	printf("spisync perf rx task start\r\n");
	while (g_iperf_rx_runing) {
		//memset(rxbuf, 0, sizeof rxbuf);
		err = spisync_read(p_spisync, 0, rxbuf, sizeof rxbuf, 1000);
		if (err <= 0)
			;//printf("failed to read from spisync, %d\r\n", err);
		else {
            if (g_ota_start) {
            	qcc74x_ota_update(p_spisync, rxbuf, err);
            }
			rx_len += err;
		}
		diff = osKernelGetTickCount() - count;
		if (diff >= 1000) {
			count = osKernelGetTickCount();
			printf("RX: %f Mbps\r\n", (float)rx_len*8/1000/1000);
			rx_len = 0;
			diff = 0;
		}
	}
	printf("perf rx task exit\r\n");
    osThreadExit();
}

typedef struct {
    int32_t flags;
    int32_t numThreads;
    int32_t mPort;
    int32_t bufferlen;
    int32_t mWindowSize;
    int32_t mAmount;
    int32_t mRate;
    int32_t mUDPRateUnits;
    int32_t mRealtime;
} iperf_client_hdr_t;
#define HEADER_VERSION1 0x80000000
#define RUN_NOW         0x00000001
#define UNITS_PPS       0x00000002

#define PP_HTONL(x) ((((x) & (uint32_t)0x000000ffUL) << 24) | \
                    (((x) & (uint32_t)0x0000ff00UL) <<  8) | \
                    (((x) & (uint32_t)0x00ff0000UL) >>  8) | \
                    (((x) & (uint32_t)0xff000000UL) >> 24))

uint32_t htonl(uint32_t n)
{
  return PP_HTONL(n);
}

static void send_dual_header(spisync_t *p_spisync)
{
    iperf_client_hdr_t hdr = {0};

    hdr.flags = htonl(HEADER_VERSION1 | RUN_NOW);
    hdr.numThreads = htonl(1);
    hdr.mPort = htonl(5001);
    hdr.mAmount = htonl((100));

    spisync_write(p_spisync, 0, &hdr, sizeof(hdr), 10000);
}

static void __perf_tx_task(void *arg)
{
	spisync_t *p_spisync = (spisync_t *)arg;
    uint8_t txbuf[1536];
	int err;
	uint32_t  pkt_cnt = 0;
	uint32_t *pkt_id_p = (uint32_t *)txbuf;

	printf("spisync perf tx task start\r\n");

    if (g_iperf_rx_runing) {
        memset(txbuf, 'R', sizeof txbuf);
    } else {
        memset(txbuf, 0, sizeof txbuf);
    }

    //send_dual_header(p_spisync);

    while (g_iperf_tx_runing) {
    	*pkt_id_p = htonl(pkt_cnt);
        err = spisync_write(p_spisync, 0, txbuf, sizeof(txbuf), 10000);
        if (err <= 0)
        	printf("data xfer: spisync_write return %d\r\n", err);
        pkt_cnt++;
        if ((rx_len > 0) && (g_iperf_rx_runing)) {
        	printf("perf tx task exit\r\n");
            break;
        }
    }
	printf("perf tx task exit %d\r\n", pkt_cnt);
    osThreadExit();
}

static void at_start_perf_rx(spisync_t *p_spisync)
{
	osThreadId_t spisync_rx_task = NULL;

	while (!spisync_ready) {
		printf("waiting spisync to become ready\r\n");
		osDelay(1000);
	}

	if (!g_iperf_rx_runing) {
        g_iperf_rx_runing = 1;
		spisync_rx_task = osThreadNew(__perf_rx_task, p_spisync, &at_rx_task_attr);
		if (!spisync_rx_task) {
			printf("failed to create spisync perf rx task\r\n");
			return;
		}
	}
}

static void at_start_perf_tx(spisync_t *p_spisync)
{
	osThreadId_t spisync_tx_task = NULL;

	while (!spisync_ready) {
		printf("waiting spisync to become ready\r\n");
		osDelay(1000);
	}

	if (!g_iperf_tx_runing) {
        g_iperf_tx_runing = 1;
		spisync_tx_task = osThreadNew(__perf_tx_task, p_spisync, &at_tx_task_attr);
		if (!spisync_tx_task) {
			printf("failed to create spisync perf rx task\r\n");
			return;
		}
	}
}

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

	at_start_perf_tx(p_spisync);
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

	at_start_perf_tx(p_spisync);
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

	at_start_perf_tx(p_spisync);
	at_start_perf_rx(p_spisync);
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

	at_start_perf_rx(p_spisync);
	at_start_perf_tx(p_spisync);
    return 0;
}

int at_iperf_stop(spisync_t *p_spisync)
{
    char at_cmd[128] = {0};
   
    g_iperf_rx_runing = 0;
    g_iperf_tx_runing = 0;
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

void at_qcc74x_ota_start(spisync_t *p_spisync, char ip_addr[20], int port)
{
    char at_cmd[128] = {0};

    //at_iperf_stop(p_spisync);

    strlcpy(at_cmd, "AT+CIPMUX=0\r\n", sizeof(at_cmd));
    spisync_write(p_spisync, 0, at_cmd, strlen(at_cmd), 10000);
    //spisync_wait_ok(p_spisync);
    osDelay(100);

    strlcpy(at_cmd, "AT+WEVT=0\r\n", sizeof(at_cmd));
    spisync_write(p_spisync, 0, at_cmd, strlen(at_cmd), 10000);
    //spisync_wait_ok(p_spisync);
    osDelay(100);

    strlcpy(at_cmd, "AT+WIPS=0\r\n", sizeof(at_cmd));
    spisync_write(p_spisync, 0, at_cmd, strlen(at_cmd), 10000);
    //spisync_wait_ok(p_spisync);
    osDelay(100);

    sprintf(at_cmd, "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n", ip_addr, port);
    spisync_write(p_spisync, 0, at_cmd, strlen(at_cmd), 10000);
    spisync_wait_ok(p_spisync);
    osDelay(500);

	g_ota_start = 1;
	at_start_perf_rx(p_spisync);

}

int at_qcc74x_ota_finish(spisync_t *p_spisync)
{
	char at_cmd[128] = {0};
    
    g_iperf_rx_runing = 0;
    g_iperf_tx_runing = 0;
	osDelay(200);

	spisync_buffer_clear(p_spisync);

    strlcpy(at_cmd, "+++", sizeof(at_cmd));
    spisync_write(p_spisync, 0, at_cmd, strlen(at_cmd), 10000);
	osDelay(100);

    strlcpy(at_cmd, "AT+CIPCLOSE\r\n", sizeof(at_cmd));
    spisync_write(p_spisync, 0, at_cmd, strlen(at_cmd), 10000);
	osDelay(200);
    
    strlcpy(at_cmd, "AT+WEVT=1\r\n", sizeof(at_cmd));
    spisync_write(p_spisync, 0, at_cmd, strlen(at_cmd), 10000);
    //spisync_wait_ok(p_spisync);
    osDelay(100);

    strlcpy(at_cmd, "AT+WIPS=1\r\n", sizeof(at_cmd));
    spisync_write(p_spisync, 0, at_cmd, strlen(at_cmd), 10000);
    //spisync_wait_ok(p_spisync);
    osDelay(100);

    strlcpy(at_cmd, "AT+OTA-R\r\n", sizeof(at_cmd));
    spisync_write(p_spisync, 0, at_cmd, strlen(at_cmd), 10000);
	osDelay(200);
    ota_ishead = 0;
	g_ota_start = 0;
    return 0;
}
