/*
 * app_pm.h
 *
 *  Created on: Sep 26, 2024
 */

#ifndef INC_APP_PM_H_
#define INC_APP_PM_H_

#include "power_manager.h"

struct spi_cmd_event;

int app_pm_init(void);
power_manager_t get_app_power_manager(void);
int app_pm_handle_sleep_ack(struct spi_cmd_event *evt);
int app_pm_handle_wakeup_ack(struct spi_cmd_event *evt);

#endif /* INC_APP_PM_H_ */
