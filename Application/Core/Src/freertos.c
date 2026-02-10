/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "task_process.h"
#include "usart.h"
#include "iwdg.h"
#include <stdio.h>
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
extern uint8_t uart_rx_buf[UART_SIZE];

osMutexId debugMutexHandle;
/* USER CODE END Variables */
osThreadId MonitoringTaskHandle;
osThreadId TelemetryTaskHandle;
osThreadId UpdateTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
/* USER CODE END FunctionPrototypes */

void monitoring_task(void const * argument);
void telemetry_task(void const * argument);
void update_task(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
	osMutexDef(debugMutex);
	debugMutexHandle = osMutexCreate(osMutex(debugMutex));
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

  /* Create the thread(s) */
  /* definition and creation of MonitoringTask */
  osThreadDef(MonitoringTask, monitoring_task, osPriorityLow, 0, 256);
  MonitoringTaskHandle = osThreadCreate(osThread(MonitoringTask), NULL);

  /* definition and creation of TelemetryTask */
  osThreadDef(TelemetryTask, telemetry_task, osPriorityNormal, 0, 1024);
  TelemetryTaskHandle = osThreadCreate(osThread(TelemetryTask), NULL);

  /* definition and creation of UpdateTask */
  osThreadDef(UpdateTask, update_task, osPriorityNormal, 0, 256);
  UpdateTaskHandle = osThreadCreate(osThread(UpdateTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_monitoring_task */
/**
  * @brief  Function implementing the MonitoringTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_monitoring_task */
void monitoring_task(void const * argument)
{
  /* USER CODE BEGIN monitoring_task */
  /* Infinite loop */
	task_monitoring_process();
  /* USER CODE END monitoring_task */
}

/* USER CODE BEGIN Header_telemetry_task */
/**
* @brief Function implementing the TelemetryTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_telemetry_task */
void telemetry_task(void const * argument)
{
  /* USER CODE BEGIN telemetry_task */
  /* Infinite loop */
	task_telemetry_process();
  /* USER CODE END telemetry_task */
}

/* USER CODE BEGIN Header_update_task */
/**
* @brief Function implementing the UpdateTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_update_task */
void update_task(void const * argument)
{
  /* USER CODE BEGIN update_task */
	/* USER CODE BEGIN Private defines */

  /* Infinite loop */
	task_update_process();
  /* USER CODE END update_task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
