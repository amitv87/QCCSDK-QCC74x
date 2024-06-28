/**
 ****************************************************************************************
 *
 * @file uart.c
 *
 * @brief UART driver
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup UART
 * @{
 ****************************************************************************************
 */
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "btblecontroller_port_uart.h" // uart definition
#include "qcc74x_uart.h"
#include "qcc74x_gpio.h"
#if defined(CFG_IOT_SDK)
#include "qcc74x_irq.h"
#endif
#include <stdint.h>
#include <stdbool.h>

/*
 * DEFINES
 *****************************************************************************************
 */

/*
 * STRUCT DEFINITIONS
 *****************************************************************************************
 */
/* TX and RX channel class holding data used for asynchronous read and write data
 * transactions
 */
/// UART TX RX Channel
struct uart_txchannel {
    /// call back function pointer
    void (*callback)(void *, uint8_t);
    /// Dummy data pointer returned to callback when operation is over.
    void *dummy;
    uint32_t remain_size;
    const uint8_t *remain_data;
};

struct uart_rxchannel {
    /// call back function pointer
    void (*callback)(void *, uint8_t);
    /// Dummy data pointer returned to callback when operation is over.
    void *dummy;
    uint32_t remain_size;
    uint8_t *remain_data;
};

/// UART environment structure
struct uart_env_tag {
    /// tx channel
    struct uart_txchannel tx;
    /// rx channel
    struct uart_rxchannel rx;
};

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

#define UART_GLITCH_TEST_ENABLE

/// uart environment structure
static struct uart_env_tag uart_env;
static struct qcc74x_device_s *btble_uart;
static volatile uint8_t uart_id;

static void uart_isr(int irq, void *arg)
{
    uint8_t *p;
    void (*callback)(void *, uint8_t) = NULL;
    void *data = NULL;

    uint32_t intstatus = qcc74x_uart_get_intstatus(btble_uart);

    if (intstatus & UART_INTSTS_RX_FIFO) {
        while (qcc74x_uart_rxavailable(btble_uart) && uart_env.rx.remain_size) {
            p = uart_env.rx.remain_data;
            *p = qcc74x_uart_getchar(btble_uart);
            p++;
            uart_env.rx.remain_size--;
            uart_env.rx.remain_data++;
        }

        if (uart_env.rx.remain_size == 0) {
            qcc74x_uart_rxint_mask(btble_uart, true);

            callback = uart_env.rx.callback;
            data = uart_env.rx.dummy;
            if (callback != NULL) {
                // Clear callback pointer
                uart_env.rx.callback = NULL;
                uart_env.rx.dummy = NULL;

                // Call handler
                callback(data, 0);
            }
        }
    }
    if (intstatus & UART_INTSTS_RTO) {
        while (qcc74x_uart_rxavailable(btble_uart) && uart_env.rx.remain_size) {
            p = uart_env.rx.remain_data;
            *p = qcc74x_uart_getchar(btble_uart);
            p++;
            uart_env.rx.remain_size--;
            uart_env.rx.remain_data++;
        }

        if (uart_env.rx.remain_size == 0) {
            qcc74x_uart_rxint_mask(btble_uart, true);

            callback = uart_env.rx.callback;
            data = uart_env.rx.dummy;
            if (callback != NULL) {
                // Clear callback pointer
                uart_env.rx.callback = NULL;
                uart_env.rx.dummy = NULL;

                // Call handler
                callback(data, 0);
            }
        }
        qcc74x_uart_int_clear(btble_uart, UART_INTCLR_RTO);
    }
    if (intstatus & UART_INTSTS_TX_FIFO) {
        while (qcc74x_uart_txready(btble_uart) && uart_env.tx.remain_size) { 
            p = (uint8_t *)uart_env.tx.remain_data;
            qcc74x_uart_putchar(btble_uart, *p);
            p++;
            uart_env.tx.remain_size--;
            uart_env.tx.remain_data++;
        }
        if (uart_env.tx.remain_size == 0) {
            qcc74x_uart_txint_mask(btble_uart, true);

            callback = uart_env.tx.callback;
            data = uart_env.tx.dummy;
            if (callback != NULL) {
                // Clear callback pointer
                uart_env.tx.callback = NULL;
                uart_env.tx.dummy = NULL;

                // Call handler
                callback(data, 0);
            }
        }
    }
}

/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/*
 * EXPORTED FUNCTION DEFINITIONS
 ****************************************************************************************
 */

__attribute__((weak)) void btble_uart_pin_config(uint8_t uartid, uint8_t tx, uint8_t rx, uint8_t cts, uint8_t rts)
{
    struct qcc74x_device_s *gpio;
    gpio = qcc74x_device_get_by_name("gpio");

    if (uartid == 0) {
        qcc74x_gpio_uart_init(gpio, tx, GPIO_UART_FUNC_UART0_TX);
        qcc74x_gpio_uart_init(gpio, rx, GPIO_UART_FUNC_UART0_RX);

        if (cts != 0xff) {
            qcc74x_gpio_uart_init(gpio, cts, GPIO_UART_FUNC_UART0_CTS);
        }
        if (rts != 0xff) {
            qcc74x_gpio_uart_init(gpio, rts, GPIO_UART_FUNC_UART0_RTS);
        }
    } else if (uartid == 1) {
        qcc74x_gpio_uart_init(gpio, tx, GPIO_UART_FUNC_UART1_TX);
        qcc74x_gpio_uart_init(gpio, rx, GPIO_UART_FUNC_UART1_RX);

        if (cts != 0xff) {
            qcc74x_gpio_uart_init(gpio, cts, GPIO_UART_FUNC_UART1_CTS);
        }
        if (rts != 0xff) {
            qcc74x_gpio_uart_init(gpio, rts, GPIO_UART_FUNC_UART1_RTS);
        }
    }
}

__attribute__((weak)) void btble_uart_init(uint8_t uartid)
{
    char uart_name[64];
    struct qcc74x_uart_config_s cfg;

    uart_id = uartid;
    snprintf((char *)uart_name,sizeof(uart_name)-1, "uart%d", uartid);
    btble_uart = qcc74x_device_get_by_name(uart_name);

    qcc74x_uart_txint_mask(btble_uart, true);
    qcc74x_uart_rxint_mask(btble_uart, true);

    cfg.baudrate = 115200;
    cfg.data_bits = UART_DATA_BITS_8;
    cfg.stop_bits = UART_STOP_BITS_1;
    cfg.parity = UART_PARITY_NONE;
    cfg.flow_ctrl = 0;
    cfg.tx_fifo_threshold = 7;
    cfg.rx_fifo_threshold = 7;
    qcc74x_uart_init(btble_uart, &cfg);
    #ifdef UART_GLITCH_TEST_ENABLE
    qcc74x_uart_feature_control(btble_uart, UART_CMD_SET_DEGLITCH_CNT, 12);
    #endif
    #if defined(CFG_IOT_SDK)
    qcc74x_irq_register_with_ctx(btble_uart->irq_num, uart_isr, NULL);
    qcc74x_irq_enable(btble_uart->irq_num);
    #else
    qcc74x_irq_attach(btble_uart->irq_num, uart_isr, NULL);
    qcc74x_irq_enable(btble_uart->irq_num);
    #endif 
}

__attribute__((weak)) int8_t btble_uart_reconfig(uint32_t baudrate, uint8_t flow_ctl_en, uint8_t cts_pin, uint8_t rts_pin)
{
    struct qcc74x_device_s *gpio;
    gpio = qcc74x_device_get_by_name("gpio");

    qcc74x_uart_disable(btble_uart);
    qcc74x_uart_feature_control(btble_uart, UART_CMD_SET_BAUD_RATE, baudrate);
    qcc74x_uart_enable(btble_uart);
    if (flow_ctl_en) {
        if (uart_id == 0) {
            qcc74x_gpio_uart_init(gpio, cts_pin, GPIO_UART_FUNC_UART0_CTS);
            qcc74x_gpio_uart_init(gpio, rts_pin, GPIO_UART_FUNC_UART0_RTS);
        } else if (uart_id == 1) {
            qcc74x_gpio_uart_init(gpio, cts_pin, GPIO_UART_FUNC_UART1_CTS);
            qcc74x_gpio_uart_init(gpio, rts_pin, GPIO_UART_FUNC_UART1_RTS);
        }
        btble_uart_flow_on();
    } else {
        btble_uart_flow_off();
    }
    return 0;
}

__attribute__((weak)) void btble_uart_flow_on(void)
{
    qcc74x_uart_feature_control(btble_uart, UART_CMD_SET_SW_RTS_CONTROL, false);
    //qcc74x_uart_feature_control(btble_uart, UART_CMD_SET_CTS_EN, true);
}

__attribute__((weak)) bool btble_uart_flow_off(void)
{
    qcc74x_uart_feature_control(btble_uart, UART_CMD_SET_SW_RTS_CONTROL, true);
    qcc74x_uart_feature_control(btble_uart, UART_CMD_SET_CTS_EN, false);
    return true;
}

__attribute__((weak)) void btble_uart_write(const uint8_t *bufptr, uint32_t size, void (*callback)(void *, uint8_t), void *dummy)
{
    if (!bufptr || !size) {
        return;
    }
    uart_env.tx.remain_data = bufptr;
    uart_env.tx.remain_size = size;
    uart_env.tx.callback = callback;
    uart_env.tx.dummy = dummy;

    qcc74x_uart_txint_mask(btble_uart, false);
}

__attribute__((weak)) void btble_uart_read(uint8_t *bufptr, uint32_t size, void (*callback)(void *, uint8_t), void *dummy)
{
    if (!bufptr || !size) {
        return;
    }
    uart_env.rx.remain_data = bufptr;
    uart_env.rx.remain_size = size;
    uart_env.rx.callback = callback;
    uart_env.rx.dummy = dummy;

    if (size < 8) {
        qcc74x_uart_feature_control(btble_uart, UART_CMD_SET_RX_FIFO_THREHOLD, size - 1);
    } else {
        qcc74x_uart_feature_control(btble_uart, UART_CMD_SET_RX_FIFO_THREHOLD, 7);
    }
    qcc74x_uart_rxint_mask(btble_uart, false);
}

__attribute__((weak)) void btble_uart_finish_transfers(void)
{
    qcc74x_uart_feature_control(btble_uart, UART_CMD_SET_SW_RTS_CONTROL, true);
    while (!qcc74x_uart_txempty(btble_uart)) {}
}
