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
osThreadId_t spi_taskhandle;
const osThreadAttr_t spi_tsk_attr = {
  .name = "spi-dma",
  .priority = (osPriority_t) osPriorityRealtime7,
  .stack_size = 4096
};

osThreadId_t console_taskhandle;
const osThreadAttr_t console_tsk_attr = {
  .name = "console",
  .priority = (osPriority_t) osPriorityRealtime7,
  .stack_size = 128 * 8
};

osThreadId_t at_taskhandle;
const osThreadAttr_t at_tsk_attr = {
  .name = "at",
  .priority = (osPriority_t) osPriorityRealtime7,
  .stack_size = 4096
};

StreamBufferHandle_t uart_strm_buffer;

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 1024
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
spisync_t spisync_ctx;
uint8_t uart_rxbyte;
extern UART_HandleTypeDef huart1;
static int show_isr_stats = 0;

struct at_desc {
	const char *cmd;
	int post_delay_ms;
};

static void at_task(void *arg)
{
	int i, err, init = 0;
	struct at_desc atcmds[] = {
							/* set sta */
							{"AT+CWMODE=1\r\n", 2000},
							/* scan AP */
							{"AT+CWLAP\r\n", 10000},
							/* connect AP */
							{"AT+CWJAP=\"ChatGPT\",\"12345678\"\r\n", 18000},
							/* create tcp connection */
							{"AT+CIPMUX=1\r\n", 1000},
							{"AT+CIPSTART=0,\"TCP\",\"192.168.2.123\",8080\r\n", 2000},
						};
	/* XXX wait for spisync task is back to normal state */
	osDelay(2000);
	printf("AT task starts to run\r\n");
	while (1) {
		if (!init) {
			/* initialization sequence */
			for (i = 0; i < sizeof(atcmds)/sizeof(atcmds[0]); i++) {
				printf("Sending '%s'\r\n", atcmds[i].cmd);
				err = spisync_write(&spisync_ctx, atcmds[i].cmd, strlen(atcmds[i].cmd), 10000);
				osDelay(atcmds[i].post_delay_ms);
			}
			init = 1;
		} else {
			//osDelay(10000000);
			/* transfer is ongoing */
			const char *data = "AT+CIPSEND=0,10\r\n";
			const char *d = "0123456789";

			err = spisync_write(&spisync_ctx, data, strlen(data), 10000);
			printf("data xfer: spisync_write return %d\r\n", err);
			err = spisync_write(&spisync_ctx, d, strlen(d), 10000);
			printf("data xfer: spisync_write return %d\r\n", err);
			osDelay(10);
		}
	}
}

static int spisync_ready = 0;

struct spisync_stat {
	uint32_t xmit;
	uint32_t recv;
};

static struct spisync_stat spisync_stat;
static volatile uint32_t g_send_flag = 0;
static volatile uint32_t g_send_tick = 0;

void spisync_perf_tx_task2(void *arg)
{
	int err;
	uint8_t byte = 1;
	unsigned char txdata[SPISYNC_PAYLOADBUF_LEN];

	while (!spisync_ready) {
		printf("waiting spisync to become ready\r\n");
		osDelay(1000);
	}
	printf("spisync perf tx start\r\n");

	while (1) {
		if (g_send_flag) {
			if (((int)(xTaskGetTickCount()) - (int)g_send_tick) >= g_send_flag) {
				spisync_iperf(&spisync_ctx, 0);// for iperf log
				g_send_flag = 0;
			}
			err = spisync_write(&spisync_ctx, txdata, sizeof txdata, portMAX_DELAY);
			if (err < 0) {
				//printf("spisync write failed, %d\r\n", err);
			} else {
			 	spisync_stat.xmit++;
				memset(txdata, byte++, SPISYNC_PAYLOADBUF_LEN);
			}
		} else {
			osDelay(500);
		}
	}
}

void spisync_perf_tx_task(void *arg)
{
	int err;
	uint8_t byte = 1;
	unsigned char txdata[SPISYNC_PAYLOADBUF_LEN];

	while (!spisync_ready) {
		printf("waiting spisync to become ready\r\n");
		osDelay(1000);
	}
	printf("spisync perf tx start\r\n");

	while (1) {
		err = spisync_write(&spisync_ctx, txdata, sizeof txdata, 1000);
		if (err < 0)
			;
			//printf("spisync write failed, %d\r\n", err);
		else {
			spisync_stat.xmit++;
			memset(txdata, byte++, SPISYNC_PAYLOADBUF_LEN);
		}
	}
}

osThreadId_t spisync_tx_task;

const osThreadAttr_t spisync_tx_task_attr = {
  .name = "spisync_tx",
  .priority = (osPriority_t) osPriorityRealtime7,
  .stack_size = 4096
};

void spisync_start_perf_tx(void)
{
	printf("start perf tx\r\n");
	while (!spisync_ready) {
		printf("waiting spisync to become ready\r\n");
		osDelay(1000);
	}
	spisync_tx_task = osThreadNew(spisync_perf_tx_task2, NULL, &spisync_tx_task_attr);
	if (!spisync_tx_task) {
		printf("failed to create spisync perf tx task\r\n");
		return;
	}
}

osThreadId_t spisync_rx_task;

const osThreadAttr_t spisync_rx_task_attr = {
  .name = "spisync_rx",
  .priority = (osPriority_t) osPriorityRealtime7,
  .stack_size = 4096
};

void spisync_perf_rx_task(void *arg)
{
	int err;
	char rxbuf[SPISYNC_PAYLOADBUF_LEN];

	printf("spisync perf rx start\r\n");
	while (1) {
		err = spisync_read(&spisync_ctx, rxbuf, sizeof rxbuf, 10000);
		if (err <= 0)
			;//printf("failed to read from spisync, %d\r\n", err);
		else {
			spisync_stat.recv++;
		}
	}
}

void spisync_start_perf_rx(void)
{
	printf("start perf tx\r\n");
	while (!spisync_ready) {
		printf("waiting spisync to become ready\r\n");
		osDelay(1000);
	}
	spisync_rx_task = osThreadNew(spisync_perf_rx_task, NULL, &spisync_rx_task_attr);
	if (!spisync_rx_task) {
		printf("failed to create spisync perf rx task\r\n");
		return;
	}
}

void spisync_test_task(void *arg)
{
	int err;

	err = spisync_init(&spisync_ctx);
	if (err)
		printf("failed to init spisync context, %d\r\n", err);
	else
		printf("spisync init is done\r\n");

	//at_taskhandle = osThreadNew(at_task, NULL, &at_tsk_attr);
	osThreadNew(spisync_tx_task, NULL, &at_tsk_attr);

	/* just profiling, can be removed later */
	void get_dma_stats(unsigned int *tx_cnt, unsigned int *rx_cnt);
	unsigned int txdma_isr_cnt = 0, last_txdma_isr_cnt = 0;
	unsigned int rxdma_isr_cnt = 0, last_rxdma_isr_cnt = 0;

	while (1) {
		get_dma_stats(&txdma_isr_cnt, &rxdma_isr_cnt);
		if (show_isr_stats) {
			printf("spi txdma isr freq %u, rxdma isr freq %u\r\n",
				txdma_isr_cnt - last_txdma_isr_cnt, rxdma_isr_cnt - last_rxdma_isr_cnt);
		}
		last_txdma_isr_cnt = txdma_isr_cnt;
		last_rxdma_isr_cnt = rxdma_isr_cnt;

		char rxbuf[SPISYNC_PAYLOADBUF_LEN];

		memset(rxbuf, 0, sizeof rxbuf);
		err = spisync_read(&spisync_ctx, rxbuf, sizeof rxbuf, 10000);
		if (err <= 0)
			printf("failed to read from spisync, %d\r\n", err);
		else {
			spisync_stat.recv++;
		}
		//printf("spisync read return %d \r\n", err);
		//printf("%s\r\n", rxbuf);
	}
}

static int cli_handle_one(const char *argv)
{
	int err;

	printf("executing command %s\r\n", argv);
	if (strstr(argv, "AT+")) {
	} else if (strstr(argv, "ss_dump")) {
		spisync_dump(&spisync_ctx);
	} else if (strstr(argv, "show_isr_stats")) {
		show_isr_stats = 1;
	} else if (strstr(argv, "hide_isr_stats")) {
		show_isr_stats = 0;
	} else if (strstr(argv, "ss_start_tx")) {
		spisync_start_perf_tx();
	} else if (strstr(argv, "ss_start_rx")) {
		spisync_start_perf_rx();
	} else if (strstr(argv, "at_iperftx")) {;
		spisync_iperf(&spisync_ctx, 1);// for log
		g_send_tick = xTaskGetTickCount();
		g_send_flag = 20*1000;// default 20 s
	} else if (strstr(argv, "at_iperf")) {
		spisync_iperf(&spisync_ctx, 1);
	}
	return 0;
}

static void uart_console_task(void *param)
{
	enum {
		CLI_INPUT_STATE_COMMON,
		CLI_INPUT_STATE_CR,
	} input_state;

	size_t ret;
	char buf[256] = {0};
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
			portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);
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
	uart_strm_buffer = xStreamBufferCreate(1024, 1);
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

  int err = spisync_init(&spisync_ctx);
  if (err)
	  printf("failed to init spisync context, %d\r\n", err);
  else
	  printf("spisync init is done\r\n");
  spisync_ready = 1;

  spisync_start_perf_tx();
  spisync_start_perf_rx();
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
	int div = 0;
	uint32_t tx, rx;
	struct spisync_stat last_stat = {0};

	unsigned int txdma_isr_cnt = 0, last_txdma_isr_cnt = 0;
	unsigned int rxdma_isr_cnt = 0, last_rxdma_isr_cnt = 0;
	void get_dma_stats(unsigned int *tx_cnt, unsigned int *rx_cnt);

  for(;;)
  {
	  HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
	  HAL_GPIO_TogglePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin);
	  HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
	  osDelay(500);
#if 0
	  if (div++ & 1) {
		  tx = spisync_stat.xmit - last_stat.xmit;
		  rx = spisync_stat.recv - last_stat.recv;
		  printf("tx pkt %u, %ubps, rx pkt %u, %ubps\r\n",
				 tx, tx * 1536 * 8, rx, rx * 1536 * 8);
		  last_stat = spisync_stat;

#if 0
		  get_dma_stats(&txdma_isr_cnt, &rxdma_isr_cnt);
		  printf("spi txdma isr freq %u, rxdma isr freq %u, bandwidth %ubps\r\n",
				 txdma_isr_cnt - last_txdma_isr_cnt, rxdma_isr_cnt - last_rxdma_isr_cnt,
				 (txdma_isr_cnt - last_txdma_isr_cnt) * sizeof(spisync_slot_t) * 8);
		  last_txdma_isr_cnt = txdma_isr_cnt;
		  last_rxdma_isr_cnt = rxdma_isr_cnt;
#endif
	  }
#endif
  }
  /* USER CODE END defaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

