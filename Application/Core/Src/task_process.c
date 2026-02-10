/*
 * task_process.c
 *
 *  Created on: Feb 8, 2026
 *      Author: danimaulana
 */

#include "task_process.h"
#include "main.h"
#include "cmsis_os.h"
#include <stdio.h>
#include <string.h>
#include "iwdg.h"
#include "adc.h"
#include "gpio.h"
#include "bme280.h"
#include "lg290p_gps.h"
#include "rtc.h"

#define TASK_UPDATE_TIME		3000
#define TASK_MONITOR_TIME		1000
#define TASK_TELE_TIME			500

#define WDT_TASK_TELE			(1 << 0)
#define WDT_TASK_UPDATE			(1 << 1)

#define WDT_ALL_TASK			WDT_TASK_TELE | WDT_TASK_UPDATE

#define SENSOR_INTERVAL_MS		5000
#define IDLE_TIME_OUT_MS		10000
#define IDLE_WAITING_MS			1000
#define ADC_POLLING_MS			20
#define RTC_WAKEUP_COUNTER		15999

extern void SystemClock_Config(void);


extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_rx;

extern osThreadId UpdateTaskHandle;
extern osThreadId MonitoringTaskHandle;
extern osThreadId TelemetryTaskHandle;


osTimerDef(SensorTimer, SensorTimerCallback);
osTimerId SensorTimerHandle;


uint8_t uart_rx_buf[UART_SIZE];

__inline static void enable_update_flag(void)
{
	  uint8_t flag = 0x01U;
	  flash_erase_range(UPDATE_FLAG_ADDRESS, 1);
	  flash_write_data(UPDATE_FLAG_ADDRESS, &flag, 1);
}

__inline static void start_receive_gps(void)
{
	volatile uint32_t tmp;
	    tmp = huart1.Instance->SR;
	    tmp = huart1.Instance->DR;
	    (void)tmp;

	    __HAL_UART_CLEAR_OREFLAG(&huart1);
	    __HAL_UART_CLEAR_NEFLAG(&huart1);
	    __HAL_UART_CLEAR_FEFLAG(&huart1);
	    __HAL_UART_CLEAR_IDLEFLAG(&huart1);

	    HAL_UART_DMAStop(&huart1);

	    if (HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart_rx_buf, UART_SIZE) != HAL_OK)
	    {
	        printf("[ERROR] Failed to start GPS DMA\r\n");
	    }

	    __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
}


__inline static void reuse_uart_to_exti(void)
{
	HAL_UART_DeInit(&huart1);
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	GPIO_InitStruct.Pin = GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

__inline static void enter_stop_mode(void)
{
	osThreadSuspendAll();
	HAL_SuspendTick();
	HAL_PWR_EnableSleepOnExit();
	HAL_PWR_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);

}

__inline static void wakeup_form_stop(void)
{
	SystemClock_Config();
	HAL_ResumeTick();
	HAL_Delay(5);
	re_init_uart();
	start_receive_gps();
	osThreadResumeAll();
}

void SensorTimerCallback(void const * argument)
{
  if (TelemetryTaskHandle != NULL)
  {
      osSignalSet(TelemetryTaskHandle, WAKEUP_SIGNAL);
  }
}

void task_update_process(void)
{
	printf("---[UPDATE_TASK]: Start Task---\r\n");
	for(;;)
	{
		osEvent updateEvent = osSignalWait(UPDATE_SIGNAL, TASK_UPDATE_TIME);
		osSignalSet(MonitoringTaskHandle, WDT_TASK_UPDATE);
		if(updateEvent.status == osEventSignal)
		{
			if (updateEvent.value.signals & UPDATE_SIGNAL)
			{
				printf("[UPDATE_TASK]: Toggle RED LED...\r\n");
				osDelay(50);
				osSignalWait(UPDATE_SIGNAL, RESET);
				enable_update_flag();
				HAL_NVIC_SystemReset();
			  }
		  }
	}
}

void task_monitoring_process(void)
{
	uint32_t current_flag = RESET;
	osEvent monitoringEvent;

	printf("---[MONITORING_TASK]: Start Task---\r\n");

	for(;;)
	{
		monitoringEvent = osSignalWait(WDT_TASK_UPDATE | WDT_TASK_TELE, TASK_MONITOR_TIME);

		if (monitoringEvent.status == osEventSignal)
		{
			current_flag |= (monitoringEvent.value.signals);
		}

		if(current_flag == (WDT_ALL_TASK))
		{
			HAL_IWDG_Refresh(&hiwdg);
			HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin);
			printf("[MONITORING_TASK]: Reported--\r\n");
			current_flag = RESET;
		}
	}
}

void task_telemetry_process(void)
{
	printf("---[TELEMETRY_TASK]: Start Task---\r\n");
	StateMAchine_t currentState = STATE_INIT;
	osEvent telemetryEvent;
	uint32_t last_tick = RESET;
	uint8_t bme_data[BME280_SIZE_DATA];
	uint16_t fuel_raw_adc = RESET;
	float fuel_percentage = 0.0f;

	uint32_t temperature_adc;
	uint32_t pressure_adc;
	uint16_t humidity_adc;

	LG290P_RMC_t gps_data;

	float temp;
	float press;
	float hum;

	for(;;)
	{
		if(MonitoringTaskHandle != NULL)
		{
			osSignalSet(MonitoringTaskHandle, WDT_TASK_TELE);
		}

		switch(currentState)
		{
		case STATE_INIT:
			printf("[TELEMETRY_TASK] Init telemetry and timer\r\n");

			SensorTimerHandle = osTimerCreate(osTimer(SensorTimer), osTimerPeriodic, NULL);
			osTimerStart(SensorTimerHandle, SENSOR_INTERVAL_MS);
			BME280_Init(&hi2c1);
			start_receive_gps();
			printf("[TELEMETRY_TASK] Start time 5s\r\n");
			last_tick = HAL_GetTick();
			currentState = STATE_IDLE;
			break;
		case STATE_IDLE:
			osSignalSet(MonitoringTaskHandle, WDT_TASK_TELE);
			telemetryEvent = osSignalWait(UART_SIGNAL | I2C_SIGNAL | WAKEUP_SIGNAL, IDLE_WAITING_MS);

			if(telemetryEvent.status == osEventSignal)
			{
				last_tick = HAL_GetTick();
				currentState = STATE_READ;
			}

			else if(telemetryEvent.status == osEventTimeout)
			{
				if((HAL_GetTick() - last_tick) > IDLE_TIME_OUT_MS){

					currentState = STATE_STOP;
				}
			}
			break;
		case STATE_READ:
			printf("[TELEMETRY_TASK] Read Data GPS...\r\n");
			if(telemetryEvent.value.signals & UART_SIGNAL)
			{
				printf("[TELEMETRY_TASK] Read GPS Data ...\r\n");
				last_tick = HAL_GetTick();
				if (lg290p_validatechecksum((const char*)uart_rx_buf))
				{
					lg290p_parse_rmc((char*)uart_rx_buf, &gps_data);
					memset(uart_rx_buf, 0x0, UART_SIZE);
					start_receive_gps();
				}
				else
				{
					printf("[TELEMETRY_TASK] Error parsing GPS Data ...\r\n");
					currentState = STATE_ERROR;
				}
			}

			printf("[TELEMETRY_TASK] Read BME280...\r\n");

			uint8_t reg_add = REG_DATA_START;

			if(HAL_I2C_Master_Transmit(&hi2c1, BME280_ADDR, &reg_add, 1U, I2C_TIMEOUT)!= HAL_OK)
			{
//				currentState = STATE_ERROR;
//				printf("[TELEMETRY_TASK] Error get data BME sensor...\r\n");
			}
			HAL_I2C_Master_Receive(&hi2c1, BME280_ADDR, (uint8_t*)bme_data, BME280_SIZE_DATA, I2C_TIMEOUT);

			pressure_adc = (uint32_t)((bme_data[0] << 12)|(bme_data[1] << 4) |(bme_data[2] >> 4));
			temperature_adc = (uint32_t)((bme_data[3] << 12)|(bme_data[4] << 4) |(bme_data[5] >> 4));
			humidity_adc = (uint16_t)((bme_data[6] << 8)|bme_data[7]) ;

			press = (float)pressure_adc/25600.0; //Pa
			temp = (float)temperature_adc/100.0; // Celcius
			hum = (float)humidity_adc/1024.0; // %RH

			printf("[TELEMETRY_TASK] Reset ADC DMA buffer...\r\n");

			HAL_ADC_Start(&hadc2);
			if(HAL_ADC_PollForConversion(&hadc2, ADC_POLLING_MS)== HAL_OK)
			{
				fuel_raw_adc = HAL_ADC_GetValue(&hadc2);
				fuel_percentage = (float)fuel_raw_adc / 4095.0f*100.0f;
			}
			else
			{
				currentState = STATE_ERROR;
			}
			HAL_ADC_Stop(&hadc2);



			currentState = STATE_TRANSMIT;
			break;
		case STATE_TRANSMIT:
			printf("---[TELEMETRY_TASK] DATA TRANSMIT---\r\n");
			printf("------------------------------------r\n");
			printf("---[TELEMETRY_TASK] DATA FUEL---\r\n");
			printf(" Fuel 		: %.1f%% Percent\r\n", fuel_percentage);
			printf(" ------------------------------------r\n");
			printf("---[TELEMETRY_TASK] DATA BME280---\r\n");
			printf(" Pressure 	: %.2f%%  Pascal\r\n", press);
			printf(" Temperature: %.1f%%  Celsius\r\n", temp);
			printf(" Humidity	: %.2f%%  Percent\r\n", hum);
			printf("------------------------------------r\n");
			printf("---[TELEMETRY_TASK] DATA GPS QUECTEL---\r\n");
			printf("Time UTC	: %s\r\n", gps_data.utc_time);
			printf("Status		: %c\r\n", gps_data.status);
			printf("Coordinate	: %.8f %c, %.8f %c\r\n", gps_data.latitude,gps_data.ns_indicator,gps_data.longitude,gps_data.ew_indicator);
			printf("Speed		: %.3f Knots\r\n", gps_data.speed_knot);
			printf("Date		: %s \r\n", gps_data.date);
			printf("Mode		: %d \r\n", gps_data.mode_indicator);
			printf("------------------------------------r\n");
			currentState = STATE_IDLE;
			break;
		case STATE_STOP:

			printf("[TELEMETRY_TASK] No GPS Activity more than 30s. Entering STOP Mode \r\n");
			osDelay(20);
			HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, RTC_WAKEUP_COUNTER, RTC_WAKEUPCLOCK_RTCCLK_DIV16);
			reuse_uart_to_exti();
			enter_stop_mode();
			wakeup_form_stop();
			HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);
			printf("[TELEMETRY_TASK] WakeUp from STOP MODE \r\n");
			last_tick = HAL_GetTick();
			currentState = STATE_IDLE;
			break;
		case STATE_ERROR:
			break;
		default:
			currentState = STATE_INIT;
			break;

		}
	}
}
