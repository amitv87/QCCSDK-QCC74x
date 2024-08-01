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
#include <string.h>
#include "spisync.h"
#include "app_atmodule.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define DEMO_AT 1
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
osThreadId_t spi_taskhandle;

osThreadId_t console_taskhandle;
const osThreadAttr_t console_tsk_attr = {
  .name = "console",
  .priority = (osPriority_t) osPriorityRealtime7,
  .stack_size = 1024*4,
};


StreamBufferHandle_t uart_strm_buffer;

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};

#define UART_STRM_BUFFER_SIZE     (3*1024)
#define CONSOLE_AT_ENABLE         (1)

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

uint8_t uart_rxbyte;
extern UART_HandleTypeDef huart1;

struct at_desc {
	const char *cmd;
	int post_delay_ms;
};
static at_host_handle_t g_at_handle;
static volatile uint32_t g_send_flag = 0;
static volatile uint32_t g_send_tick = 0;

static char *argc_parse(const char *argv)
{
	char *s = strstr(argv, " ");
	char *end = strstr(argv, "\r\n");
	if (end) {
		*end = '\0';
	}
	return s ? (s + 1) : NULL;
}


void qcc74x_lp_gpio_weakup()
{
    HAL_GPIO_WritePin(QCC74X_LP_WEAKUP_GPIO_Port, QCC74X_LP_WEAKUP_Pin, 1);
	osDelay(1);
    HAL_GPIO_WritePin(QCC74X_LP_WEAKUP_GPIO_Port, QCC74X_LP_WEAKUP_Pin, 0);
}

static int cli_handle_one(const char *argv)
{
	int ret;
	static uint8_t buf[1536];

	printf("executing command %s\r\n", argv);
	if (strstr(argv, "ss_dump")) {
		spisync_dump(g_at_handle->arg);
	} else if (strstr(argv, "ss_tx_once")) {
   		ret = spisync_write(g_at_handle->arg, 1, buf, sizeof(buf), 0);
    	printf("spisync_write ret %d\r\n", ret);
	} else if (strstr(argv, "at_iperftx")) {
		spisync_iperf(g_at_handle->arg, 1);// for log
		g_send_tick = xTaskGetTickCount();
		g_send_flag = 20*1000;// default 20 s
	} else if (strstr(argv, "at_iperf")) {
		spisync_iperf(g_at_handle->arg, 1);
    } else if (strstr(argv, "spisync_type")) {
		int spisync_type_test(spisync_t *spisync, uint32_t per, uint32_t t);
		spisync_type_test(g_at_handle->arg, 1, 600);
	} else if (strstr(argv, "spisync_iperf 0 1")) {
		int spisync_iperf_test(spisync_t *spisync, uint8_t tx_en, uint8_t type, uint32_t per, uint32_t t);
		spisync_iperf_test(g_at_handle->arg, 0, 1, 1, 60);
	} else if (strstr(argv, "spisync_iperf 1 1")) {
		int spisync_iperf_test(spisync_t *spisync, uint8_t tx_en, uint8_t type, uint32_t per, uint32_t t);
		spisync_iperf_test(g_at_handle->arg, 1, 1, 1, 30);
	} else if (strstr(argv, "ips")) {
		char *ip_addr = NULL;
		ip_addr = argc_parse(argv);
		at_iperf_tcp_rx_start(g_at_handle->arg, ip_addr, 5001); //port defaulf 5001
	} else if (strstr(argv, "ipus")) {
		char *ip_addr = NULL;
		ip_addr = argc_parse(argv);
		at_iperf_udp_rx_start(g_at_handle->arg, ip_addr, 5001); //port defaulf 5001
	} else if (strstr(argv, "ipc")) {
		char *ip_addr = NULL;
		ip_addr = argc_parse(argv);
		at_iperf_tcp_tx_start(g_at_handle->arg, ip_addr, 5001); //port defaulf 8888
	} else if (strstr(argv, "ipu")) {
		char *ip_addr = NULL;
		ip_addr = argc_parse(argv);
		at_iperf_udp_tx_start(g_at_handle->arg, ip_addr, 5001); //port defaulf 8888
	} else if (strstr(argv, "iperf_stop")) {
		at_iperf_stop(g_at_handle->arg);
	} else if (strstr(argv, "ota_start")) {
		char *ip_addr = NULL;
		ip_addr = argc_parse(argv);
		at_ota_start(g_at_handle, ip_addr, 3365); //OTA port defaulf 3365
	} else if (strstr(argv, "wakeup 28")) {
        qcc74x_lp_gpio_weakup();
    }
	return 0;
}

static int _console_to_at(const char *buf, uint32_t len)
{
    int ret = at_host_send(g_at_handle, 0, (uint8_t *)buf, len);

    if (ret < len) {
        printf("%s send fail\r\n", __func__);
    }
    return 0;
}

#define HOSTCMD_KEYWORD "HOSTCMD "

static int console_cli_run(const char *cmd)
{
    char *cmd_parse = NULL;

    if (strstr(cmd, HOSTCMD_KEYWORD) != NULL) {
        cmd_parse = (char *)cmd + strlen(HOSTCMD_KEYWORD);
		cli_handle_one(cmd_parse);
        return 0;
    }
    return -1;
}


static void uart_console_task(void *param)
{
	enum {
		CLI_INPUT_STATE_COMMON,
		CLI_INPUT_STATE_CR,
	} input_state;

	size_t ret;
	static char buf[512] = {0};
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
	
#if CONSOLE_AT_ENABLE 
            if (console_cli_run(&buf[in]) != 0) {
            	_console_to_at(&buf[in], ret);
            }
            in = 0;
#else 
		    /* echo */
			for (int i = 0; i < ret; i++) {
				putchar(buf[in + i]);
				fflush(stdout);
			}

			/* new data arrives */
			in += ret;
			/* parse received data */
			while (in > out) {
				char *cmd = NULL;
				char ch = buf[out++];

				switch (input_state) {
				case CLI_INPUT_STATE_COMMON:
					if (ch == '\r')
						input_state = CLI_INPUT_STATE_CR;
					break;

				case CLI_INPUT_STATE_CR:
					if (ch == '\n') {
						cmd = buf;
						cli_handle_one(cmd);
						memmove(buf, &buf[out], in - out);
						in -= out;
						out = 0;
					}
					input_state = CLI_INPUT_STATE_COMMON;
					break;
				}
			}
#endif
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
	uart_strm_buffer = xStreamBufferCreate(UART_STRM_BUFFER_SIZE, 1);
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
  if (!console_taskhandle)
	  printf("failed to create console task\r\n");

  g_at_handle = at_spisync_init();

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

