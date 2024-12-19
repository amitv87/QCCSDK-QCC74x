/*
 * app_pm.c
 *
 *  Created on: Sep 20, 2024
 */
#include <stdio.h>
#include <stdint.h>
#include "main.h"
#include "spisync.h"
#include "power_manager.h"
#include "spi_cmd_processor.h"

uint16_t txn_id = 0;
static power_manager_t pm;

power_manager_t get_app_power_manager(void)
{
	return pm;
}

int app_pm_handle_sleep_ack(struct spi_cmd_event *evt)
{
	int ret;

	if (!evt) {
		printf("Invalid incoming event in %s\r\n", __func__);
		return -1;
	}

	printf("Peer acked sleep request, seq %d\r\n", evt->seq);
	ret = power_manager_handle_sleep_ack(get_app_power_manager());
	if (ret < 0) {
		printf("power manager failed to handle sleep ack, %d\r\n", ret);
		return ret;
	}

	/* ACK-ACK */
	struct spi_cmd_event resp;

	SPI_CMD_EVENT_GEN(resp, SPI_CMD_SLEEP_REQUEST, SPI_CMD_DIR_FROM_HOST,
						SPI_CMD_IS_ACK, txn_id, 0);
	ret = spi_send_cmd_event(&resp);
	if (ret < 0)
		printf("Failed to request peer to sleep, %d\n", ret);
	return ret;
}

int app_pm_handle_wakeup_ack(struct spi_cmd_event *evt)
{
	int ret;

	if (!evt) {
		printf("Invalid incoming event in %s\r\n", __func__);
		return -1;
	}

	printf("Peer acked wakeup request, seq %d\r\n", evt->seq);
	ret = power_manager_handle_wakeup_ack(get_app_power_manager());
	if (ret < 0) {
		printf("Failed to handle wakeup ack, %d\r\n", ret);
		return ret;
	}

	/* ACK-ACK */
	struct spi_cmd_event resp;

	SPI_CMD_EVENT_GEN(resp, SPI_CMD_WAKEUP_REQUEST, SPI_CMD_DIR_FROM_HOST,
						SPI_CMD_IS_ACK, evt->seq, 0);
	ret = spi_send_cmd_event(&resp);
	if (ret < 0)
		printf("Failed to request peer to sleep, %d\n", ret);
	return ret;
}

static int pm_request_peer_wakeup(struct power_manager *pm)
{
	printf("Requesting peer to wake up\r\n");
	HAL_GPIO_WritePin(QCC74X_LP_WAKEUP_GPIO_Port, QCC74X_LP_WAKEUP_Pin, 0);
	HAL_GPIO_WritePin(QCC74X_LP_WAKEUP_GPIO_Port, QCC74X_LP_WAKEUP_Pin, 1);
	osDelay(50);
	HAL_GPIO_WritePin(QCC74X_LP_WAKEUP_GPIO_Port, QCC74X_LP_WAKEUP_Pin, 0);
	return 0;
}

static int pm_request_peer_sleep(struct power_manager *pm)
{
	int ret;
	struct spi_cmd_event evt;

	printf("Requesting peer to sleep, seq %d\r\n", txn_id);
	SPI_CMD_EVENT_GEN(evt, SPI_CMD_SLEEP_REQUEST, SPI_CMD_DIR_FROM_HOST,
						SPI_CMD_IS_REQ, txn_id, 0);
	ret = spi_send_cmd_event(&evt);
	if (ret < 0) {
		printf("Failed to request peer to sleep, %d\n", ret);
		return ret;
	}
	return 0;
}

extern SPI_HandleTypeDef hspi1;

static int pm_pre_sleep_hook(void *arg)
{
	txn_id++;
	return 0;
}

static int pm_post_sleep_hook(void *arg)
{
	txn_id++;
	return 0;
}

int app_pm_init(void)
{
	int ret;
	struct power_manager_config config;

	config.request_peer_sleep = pm_request_peer_sleep;
	config.request_peer_wakeup = pm_request_peer_wakeup;
	pm = power_manager_create(&config);
	if (!pm) {
		printf("Failed to create power manager\r\n");
		return -1;
	}
	printf("Power manager is created successfully\r\n");

	struct power_manager_hook pre_sleep_hook;

	pre_sleep_hook.hook = pm_pre_sleep_hook;
	pre_sleep_hook.userdata = pm;
	ret = power_manager_register_hook(pm, PM_HOOK_PRE_SLEEP, PM_HOOK_PRIO_LOW, &pre_sleep_hook);
	if (ret < 0) {
		printf("Failed to register pre-sleep hook, %d\r\n", ret);
		goto err_hook;
	}

	struct power_manager_hook post_sleep_hook;

	post_sleep_hook.hook = pm_post_sleep_hook;
	post_sleep_hook.userdata = pm;
	ret = power_manager_register_hook(pm, PM_HOOK_POST_SLEEP, PM_HOOK_PRIO_LOW, &post_sleep_hook);
	if (ret < 0) {
		printf("Failed to register post-sleep hook, %d\r\n", ret);
		goto err_hook;
	}
	return 0;

err_hook:
	power_manager_destroy(pm);
	return -1;
}
