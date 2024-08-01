#include <wifi_pkt_hooks.h>

#include <stdio.h>

qcc74x_pkt_eth_input_hook_cb_t qcc74x_wifi_pkt_eth_input_hook = NULL;
void *qcc74x_wifi_pkt_eth_input_hook_arg = NULL;

void qcc74x_pkt_eth_input_hook_register(qcc74x_pkt_eth_input_hook_cb_t cb, void *cb_arg)
{
    qcc74x_wifi_pkt_eth_input_hook = cb;
    qcc74x_wifi_pkt_eth_input_hook_arg = cb_arg;
}

void qcc74x_pkt_eth_input_hook_unregister(void)
{
    qcc74x_wifi_pkt_eth_input_hook = NULL;
    qcc74x_wifi_pkt_eth_input_hook_arg = NULL;
}

qcc74x_pkt_eth_output_hook_cb_t qcc74x_wifi_pkt_eth_output_hook = NULL;
void *qcc74x_wifi_pkt_eth_output_hook_arg = NULL;

void qcc74x_pkt_eth_output_hook_register(qcc74x_pkt_eth_output_hook_cb_t cb, void *cb_arg)
{
    qcc74x_wifi_pkt_eth_output_hook = cb;
    qcc74x_wifi_pkt_eth_output_hook_arg = cb_arg;
}

void qcc74x_pkt_eth_output_hook_unregister(void)
{
    qcc74x_wifi_pkt_eth_output_hook = NULL;
    qcc74x_wifi_pkt_eth_output_hook_arg = NULL;
}
