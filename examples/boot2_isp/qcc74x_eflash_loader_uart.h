#ifndef __QCC74x_EFLASH_LOADER_UART_H__
#define __QCC74x_EFLASH_LOADER_UART_H__

#include "stdint.h"

int32_t qcc74x_eflash_loader_uart_init();

int32_t qcc74x_eflash_loader_uart_handshake_poll(uint32_t timeout);

uint32_t *qcc74x_eflash_loader_uart_recv(uint32_t *recv_len, uint32_t maxlen, uint32_t timeout);

int32_t qcc74x_eflash_loader_uart_send(uint32_t *data, uint32_t len);

int32_t qcc74x_eflash_loader_usart_wait_tx_idle(uint32_t timeout);

int32_t qcc74x_eflash_loader_uart_change_rate(uint32_t oldval, uint32_t newval);

int32_t qcc74x_eflash_loader_uart_deinit(void);

#define AUTO_BAUDRATE_CHECK_TIME_MAX_MS 2
#if defined(CHIP_QCC74x_undef)
#define QCC74x_EFLASH_LOADER_IF_UART_RX_TIMEOUT 90 /*ms*/
#else
#define QCC74x_EFLASH_LOADER_IF_UART_RX_TIMEOUT 8000 /*ms*/
#endif

#endif
