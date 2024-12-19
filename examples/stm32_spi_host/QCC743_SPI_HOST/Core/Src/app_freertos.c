/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os2.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "spisync.h"
#include "app_atmodule.h"
#include "power_manager.h"
#include "app_pm.h"
#include "spi_cmd_processor.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
osThreadId_t console_taskhandle;

StreamBufferHandle_t uart_strm_buffer;

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

uint8_t uart_rxbyte;
extern UART_HandleTypeDef huart1;

struct at_desc {
	const char *cmd;
	int post_delay_ms;
};
static at_host_handle_t g_at_handle;

static char *argc_parse(const char *argv)
{
	char *s = strstr(argv, " ");
	char *end = strstr(argv, "\r\n");
	if (end) {
		*end = '\0';
	}
	return s ? (s + 1) : NULL;
}

void qcc74x_lp_gpio_wakeup()
{
    HAL_GPIO_WritePin(QCC74X_LP_WAKEUP_GPIO_Port, QCC74X_LP_WAKEUP_Pin, 1);
	osDelay(1);
    HAL_GPIO_WritePin(QCC74X_LP_WAKEUP_GPIO_Port, QCC74X_LP_WAKEUP_Pin, 0);
}

static void spisync_tx_perf(void *arg)
{
    int type = (long)arg;
    uint8_t buffer[SPISYNC_PAYLOADBUF_LEN];

    spisync_msg_t msg;
    spisync_build_msg_zerocopy(&msg, type, buffer, sizeof(buffer), portMAX_DELAY);

    uint32_t success_count = 0;
    uint32_t byte_count = 0;
    uint32_t last_tick = osKernelGetTickCount();
    uint32_t start_tick = last_tick;

    printf("spisync tx perf task for type %d started successfully.\n", type);
    while (1) {
        int ret = spisync_write(g_at_handle->arg, &msg, 0);
        if (ret > 0) {
            success_count++;
            if (type >= 0 && type <= 2) {
                byte_count += sizeof(buffer);
            } else if (type >= 3 && type <= 5) {
                byte_count += ret;
            }
        } else if (ret == 0) {
            printf("Timeout occurred while writing for type %d.\n", type);
            osDelay(500);
            continue;
        } else if (ret < 0) {
            printf("Error occurred while writing for type %d.\n", type);
            osDelay(500);
            continue;
        }

        uint32_t current_tick = osKernelGetTickCount();
        if (current_tick - start_tick >= 30 * 1000) {
        	printf("type %d tx perf task exited\r\n", type);
            break;
        }

        if ((current_tick - last_tick) >= 1000) {
            printf("Type %d: TX %"PRIu32" packets, %"PRIu32
            		" bps, elapsed %"PRIu32" ticks.\n",
                    type, success_count, byte_count * 8, current_tick - last_tick);

            last_tick = current_tick;
            success_count = 0;
            byte_count = 0;
        }
    }
    osThreadExit();
}

static void spisync_start_tx_perf(const char *arg)
{
    int type = atoi(arg + strlen("ss_tx_perf "));

    if (type < 0 || type > 5) {
        printf("Invalid data type %d\n", type);
        return;
    }

    osThreadId_t task_handle;
    osThreadAttr_t task_attr = {
        .name = "ss_tx_perf",
        .priority = osPriorityRealtime,
        .stack_size = 4096
    };

    task_handle = osThreadNew(spisync_tx_perf, (void *)(long)type, &task_attr);
    if (!task_handle) {
        printf("Failed to create task for type %d.\n", type);
        return;
    }
}

static void spisync_rx_perf(void *arg)
{
    int type = (long)arg;
    uint8_t buffer[2048];

    spisync_msg_t msg;
    /* FIXME */
    spisync_build_msg_needcopy(&msg, type, buffer, sizeof(buffer), portMAX_DELAY);

    uint32_t success_count = 0;
    uint32_t byte_count = 0;
    uint32_t last_tick = osKernelGetTickCount();

    printf("spisync rx perf task for type %d started\r\n", type);
    while (1) {
        int ret = spisync_read(g_at_handle->arg, &msg, 0);

        if (ret > 0) {
            success_count++;
            if (type >= 0 && type <= 2) {
                byte_count += msg.buf_len;
            } else if (type >= 3 && type <= 5) {
                byte_count += ret;
            }
        } else if (ret == 0) {
            printf("Timeout occurred while reading for type %d.\r\n", type);
            continue;
        } else if (ret < 0) {
            printf("Error occurred while reading for type %d.\r\n", type);
            continue;
        }

        uint32_t current_tick = osKernelGetTickCount();
        if ((current_tick - last_tick) >= 1000) {
            printf("Type %d: RX %"PRIu32" packets, %"PRIu32
            		" bps, elapsed %"PRIu32" ticks\r\n",
                    type, success_count, byte_count * 8, current_tick - last_tick);
            last_tick = current_tick;
            success_count = 0;
            byte_count = 0;
        }
    }
}

static void spisync_start_rx_perf(const char *arg)
{
	static int task_status[6] = {0};

    int type = atoi(arg + strlen("ss_rx_perf "));

    if (type < 0 || type > 5) {
    	printf("invalid data type %d\r\n", type);
    	return;
    }

    if (task_status[type] != 0) {
        printf("The spisync rx perf task for type %d is already running.\n", type);
        return;
    }

    task_status[type] = 1;
    osThreadId_t task_handle;
    osThreadAttr_t task_attr = {
      .name = "ss_rx_perf",
	  .priority = 55,
      .stack_size = 6 * 1024,
    };

    task_handle = osThreadNew(spisync_rx_perf, (void *)(long)type, &task_attr);
    if (!task_handle) {
        printf("Failed to create task for type %d.\n", type);
        task_status[type] = 0;
        return;
    }
}

static void spisync_rx_once(const char *arg)
{
	int ret;
	uint8_t buf[SPISYNC_PAYLOADBUF_LEN];
	spisync_msg_t msg;

	int type = atoi(arg + strlen("ss_rx_once "));

	if (type < 0 || type > 5) {
		printf("Invalid type %d\r\n", type);
		return;
	}

	memset(buf, 0x88, sizeof(buf));
	/* FIXME */
	spisync_build_msg_needcopy(&msg, type, buf, sizeof(buf), 5000);
	ret = spisync_read(g_at_handle->arg, &msg, 0);
	printf("spisync_read ret %d, msg len %"PRIu32"\r\n", ret, msg.buf_len);
}

static void spisync_tx_once(const char *arg)
 {
	int ret;
	uint8_t buf[SPISYNC_PAYLOADBUF_LEN];
	spisync_msg_t msg;

	int type = atoi(arg + strlen("ss_tx_once "));

	if (type < 0 || type > 5) {
		printf("Invalid type %d\r\n", type);
		return;
	}

	memset(buf, 0x88, sizeof(buf));
	/* FIXME */
	spisync_build_msg_needcopy(&msg, type, buf, sizeof(buf), 5000);
	ret = spisync_write(g_at_handle->arg, &msg, 0);
	printf("spisync_write ret %d\r\n", ret);
}

static void do_ss_dump(const char *arg)
{
	spisync_status(g_at_handle->arg);
}

static void do_ss_tx_once(const char *arg)
{
	spisync_tx_once(arg);
}

static void do_ss_rx_once(const char *arg)
{
	spisync_rx_once(arg);
}

static void do_ss_tx_perf(const char *arg)
{
	spisync_start_tx_perf(arg);
}

static void do_ss_rx_perf(const char *arg)
{
	spisync_start_rx_perf(arg);
}

static void do_ips(const char *arg)
{
	char *ip_addr = NULL;

	ip_addr = argc_parse(arg);
	at_iperf_tcp_rx_start(g_at_handle, ip_addr, 5001);
}

static void do_ipus(const char *arg)
{
	char *ip_addr = NULL;

	ip_addr = argc_parse(arg);
	at_iperf_udp_rx_start(g_at_handle, ip_addr, 5001);
}

static void do_ipu(const char *arg)
{
	char *ip_addr = NULL;

	ip_addr = argc_parse(arg);
	at_iperf_udp_tx_start(g_at_handle, ip_addr, 5001);
}

static void do_ipc(const char *arg)
{
	char *ip_addr = NULL;

	ip_addr = argc_parse(arg);
	at_iperf_tcp_tx_start(g_at_handle, ip_addr, 5001);
}

static void do_at_iperf_stop(const char *arg)
{
	at_iperf_stop(g_at_handle, 0);
}

static void do_ota_start(const char *arg)
{
	char *ip_addr = NULL;

	ip_addr = argc_parse(arg);
	at_ota_start(g_at_handle, ip_addr, 3365);
}

static void do_ota_stop(const char *arg)
{
	at_ota_finish(g_at_handle);
}

static void do_sleep(const char *arg)
{
	int ret;

	printf("Requesting to sleep\r\n");
	ret = power_manager_request_sleep(get_app_power_manager());
	if (ret < 0)
		printf("failed to request sleep, %d\r\n", ret);
}

static void do_wakeup(const char *arg)
{
	int ret;

	printf("Requesting to wakeup\r\n");
	ret = power_manager_request_wakeup(get_app_power_manager());
	if (ret < 0)
		printf("failed to request wakeup, %d\r\n", ret);
}

static void do_mock_sleep_ack(const char *arg)
{
	int ret;

	ret = power_manager_handle_sleep_ack(get_app_power_manager());
	if (ret < 0)
		printf("Failed to handle sleep ack, %d\r\n", ret);
}

static void do_mock_wakeup_ack(const char *arg)
{
	int ret;

	ret = power_manager_handle_wakeup_ack(get_app_power_manager());
	if (ret < 0)
		printf("Failed to handle wakeup ack, %d\r\n", ret);
}

struct cmd_entry {
	const char *name;
	void (*func)(const char *arg);
};

static const struct cmd_entry cmds[] = {
	{"ss_dump", do_ss_dump},
	{"ss_tx_once", do_ss_tx_once},
	{"ss_rx_once", do_ss_rx_once},
	{"ss_tx_perf", do_ss_tx_perf},
	{"ss_rx_perf", do_ss_rx_perf},
	{"ips", do_ips},
	{"ipus", do_ipus},
	{"ipc", do_ipc},
	{"ipu", do_ipu},
	{"iperf_stop", do_at_iperf_stop},
	{"ota_start", do_ota_start},
	{"ota_stop", do_ota_stop},
	{"wakeup", do_wakeup},
	{"sleep", do_sleep},
	{"mock_sleep_ack", do_mock_sleep_ack},
	{"mock_wakeup_ack", do_mock_wakeup_ack},
};

static int cli_handle_one(const char *argv)
{
	int i;
    int cmd_len;

	for (i = 0; i < sizeof(cmds) / sizeof(cmds[0]); i++) {
		const struct cmd_entry *cmd = &cmds[i];

        cmd_len = strlen(cmd->name);
		if ((!strncmp(argv, cmd->name, cmd_len)) && 
            ((argv[cmd_len] == ' ') || (argv[cmd_len] == '\r') || (argv[cmd_len] == '\n'))) {
			printf("executing command %s\r\n", argv);
			if (cmd->func)
				cmd->func(argv);
		}
	}
	return 0;
}

static int _console_to_at(const char *buf, uint32_t len)
{
    int ret = at_host_send(g_at_handle, 0, (uint8_t *)buf, len, 0);

    if (ret < len) {
        printf("%s send fail\r\n", __func__);
    }
    return 0;
}

#define CONSOLE_AT_ENABLE 1
#define HOSTCMD_KEYWORD "HOSTCMD "

static int console_cli_run(const char *cmd)
{
#if CONSOLE_AT_ENABLE
	int off = 0;

	off = strlen(HOSTCMD_KEYWORD);
    if (!strncmp(cmd, HOSTCMD_KEYWORD, off)) {
        cmd = (char *)cmd + off;
		cli_handle_one(cmd);
        return 0;
    }

    return _console_to_at(cmd, strlen(cmd));
#else
    cli_handle_one(cmd);
#endif
}

static void uart_console_task(void *param)
{
	enum {
		CLI_INPUT_STATE_COMMON,
		CLI_INPUT_STATE_CR,
	} input_state;

	size_t ret;
	char buf[512] = {0};
	char cmd[512];
	HAL_StatusTypeDef status;
	unsigned int in = 0, out = 0;
	input_state = CLI_INPUT_STATE_COMMON;

#define BUF_SPACE()	(sizeof(buf) - in)
#define BUF_EMPTY()	(in == out)

	status = HAL_UART_Receive_IT(&huart1, (uint8_t *)&uart_rxbyte, 1);
	if (status != HAL_OK)
		printf("failed to setup uart reception, %d\r\n", status);

	while (1) {
		ret = xStreamBufferReceive(uart_strm_buffer, &buf[in],
								BUF_SPACE(), portMAX_DELAY);
		if (ret) {
			/* new data arrives */
			in += ret;

            if (strstr(buf, "+++") != NULL) {
                _console_to_at(buf, 3);
                out = 3;
                in -= out;
            }
			/* parse received data */
			while (in > out) {
				char ch = buf[out++];

				switch (input_state) {
				case CLI_INPUT_STATE_COMMON:
					if (ch == '\r')
						input_state = CLI_INPUT_STATE_CR;
					break;

				case CLI_INPUT_STATE_CR:
					if (ch == '\n') {
						/*
						 * Copy the current command to buffer and make sure
						 * it has a terminating NULL.
						 */
						memcpy(cmd, buf, out);
						cmd[out] = '\0';
						console_cli_run(cmd);
						/* Shift out the old command. */
						memmove(buf, &buf[out], in - out);
						in -= out;
						out = 0;
					}
					input_state = CLI_INPUT_STATE_COMMON;
					break;
				}
			}
		}
	}
}

extern UART_HandleTypeDef huart1;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	HAL_StatusTypeDef status;
	BaseType_t pxHigherPriorityTaskWoken = 0;

	if (huart->Instance == USART1)
	{
		if (uart_strm_buffer) {
			xStreamBufferSendFromISR(uart_strm_buffer,
							&uart_rxbyte, 1, &pxHigherPriorityTaskWoken);
			//portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
		}
	}
	/* set up next uart reception */
	status = HAL_UART_Receive_IT(&huart1, (uint8_t *)&uart_rxbyte, 1);
	if (status != HAL_OK)
		printf("failed to setup next uart reception, %d\r\n", status);
}

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
	int ret;

	osThreadAttr_t defaultTask_attributes = {
	  .name = "defaultTask",
	  .priority = (osPriority_t) osPriorityNormal,
	  .stack_size = 128 * 4
	};
	osThreadAttr_t console_tsk_attr = {
	  .name = "console",
	  .priority = (osPriority_t) osPriorityRealtime7,
	  .stack_size = 1024*4,
	};
	uart_strm_buffer = xStreamBufferCreate(1024 * 3, 1);
	if (!uart_strm_buffer)
		printf("failed to create stream buffer for uart\r\n");
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  console_taskhandle = osThreadNew(uart_console_task, NULL, &console_tsk_attr);
  if (!console_taskhandle) {
	  printf("failed to create console task\r\n");
	  while (1);
  }

  g_at_handle = at_spisync_init();

  ret = spi_cmd_processor_init();
  if (ret < 0) {
	  printf("Failed to init spi command processor, %d\r\n", ret);
	  while (1);
  }

  ret = app_pm_init();
  if (ret < 0) {
	  printf("Failed to init power manager app\r\n");
	  while (1);
  }
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}
/* USER CODE BEGIN Header_StartDefaultTask */
/**
* @brief Function implementing the defaultTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN defaultTask */
  /* Infinite loop */
  for(;;)
  {
	  HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
	  HAL_GPIO_TogglePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin);
	  HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
	  osDelay(500);
  }
  /* USER CODE END defaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

