#include "qcc74x_mtimer.h"
#include "board.h"
#include "log.h"

#include "lwip/api.h"
#include "lwip/arch.h"
#include "lwip/opt.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "FreeRTOS.h"
#include "task.h"

#include "eth_iperf_server.h"

#define IPERF_PORT     (5001)
#define IPERF_BUF_SIZE (1024 * 4)

static StackType_t iperf_server_stack[2048];
static StaticTask_t iperf_server_handle;

void iperf_server_netconn(void *arg)
{
    struct netconn *conn, *newconn;

    err_t err;
    void *recv_data;

    recv_data = pvPortMalloc(IPERF_BUF_SIZE);

    if (recv_data == NULL) {
        printf("iperf malloc failed\n");
    }

    conn = netconn_new(NETCONN_TCP);
    netconn_bind(conn, IP_ADDR_ANY, IPERF_PORT);
    printf("listen port!\r\n");
    /* tell connection to go into listening mode. */
    netconn_listen(conn);

    while (1) {
        /* grab net connection */
        err = netconn_accept(conn, &newconn);
        if (err == ERR_OK) {
            struct netbuf *inbuf;
            u16_t len;

            while ((err = netconn_recv(newconn, &inbuf)) == ERR_OK) {
                // recve data
                // printf("recv!\r\n");
                do {
                    netbuf_data(inbuf, &recv_data, &len);
                } while (netbuf_next(inbuf) >= 0);
                netbuf_delete(inbuf);
            }
            netconn_close(newconn);
            netconn_delete(newconn);
        }
    }
}

void iperf_server_socket(void *arg)
{
    uint8_t *recv_data;
    socklen_t sin_size;
    uint32_t tick1, tick2;
    int sock = -1, connected, bytes_recv;
    uint64_t recvlen;
    struct sockaddr_in server_addr, client_addr;
    fd_set readset;
    struct timeval timeout;

    recv_data = (uint8_t *)pvPortMalloc(IPERF_BUF_SIZE);
    if (recv_data == NULL) {
        printf("iperf malloc failed!\r\n");
        return;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("iperf socket failed!\r\n");
        vPortFree(recv_data);
        return;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(IPERF_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(server_addr.sin_zero), 0x0, sizeof(server_addr.sin_zero));

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        printf("iperf bind failed!\r\n");
        close(sock);
        vPortFree(recv_data);
        return;
    }

    if (listen(sock, 5) == -1) {
        printf("iperf listen failed!\r\n");
        close(sock);
        vPortFree(recv_data);
        return;
    }

    timeout.tv_sec = 3;
    timeout.tv_usec = 0;

    printf("iperf_server!\r\n");
    while (1) {
        FD_ZERO(&readset);
        FD_SET(sock, &readset);
        if (select(sock + 1, &readset, NULL, NULL, &timeout) == 0) {
            continue;
        }
        printf("iperf_server!\r\n");

        sin_size = sizeof(struct sockaddr_in);

        connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);

        printf("new clinet connected from (%s, %d) \r\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        {
            int flag = 1;

            setsockopt(connected, IPPROTO_IP, TCP_NODELAY, (void *)&flag, sizeof(int));
        }

        recvlen = 0;
        tick1 = xTaskGetTickCount();
        while (1) {
            bytes_recv = recv(connected, recv_data, IPERF_BUF_SIZE, 0);
            if (bytes_recv <= 0) break;

            recvlen += bytes_recv;
            tick2 = xTaskGetTickCount();
            if (tick2 - tick1 >= configTICK_RATE_HZ * 5) {
                float f;
                f = (float)(recvlen * configTICK_RATE_HZ/125/(tick2 - tick1));
                f /= 1000.0f;

                tick1 = tick2;
                recvlen = 0;
            }
        }
        if (connected >= 0 ) closesocket(connected);
        connected = -1;
    }
}

/**
  * @brief  Initialize the iperf server (start its thread)
  * @param  none
  * @retval None
  */
void iperf_server_init(void)
{
    xTaskCreateStatic((void *)iperf_server_netconn, (char *)"iperf_server", sizeof(iperf_server_stack) / 4, NULL, osPriorityHigh, iperf_server_stack, &iperf_server_handle);
}
