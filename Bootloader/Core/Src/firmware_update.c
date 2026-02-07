/*
 * firmware_update.c
 *
 *  Created on: Feb 3, 2026
 *      Author: danimaulana
 */

#include "firmware_update.h"
#include "flash_layout.h"
#include "flash.h"
#include "usart.h"
#include "main.h"
#include <string.h>
#include "checksum.h"
#include "authentication.h"
#include "iwdg.h"

#define DATA_FIRMWARE		512U

#define CRC_INDEX			515U
#define SIZE_INDEX			5U


extern CRC_HandleTypeDef hcrc;

extern FirmwareProcessData_s FirmwareProcessData;
extern uint8_t uart_rx_buf[UART_SIZE];

__inline static void send_command(uint8_t cmd)
{
	uint8_t cmd_set = cmd;
	HAL_UART_Transmit(&huart1, &cmd_set, 1U, 100);

}

__inline static void enable_receive_dma(void)
{
	memset(uart_rx_buf, 0x0, UART_SIZE);
	HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart_rx_buf, UART_SIZE);
	__HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
}


__inline static void disable_receive_dma(void)
{
	memset(uart_rx_buf, 0x0, UART_SIZE);
	HAL_UART_DMAStop(&huart1);
}

static void copy_flash_app(uint32_t src_add, uint32_t dst_add, uint32_t size){
	uint8_t buff[COPY_SIZE];

	flash_erase_range(dst_add, size);

	for (uint32_t offset = 0; offset < size; offset += COPY_SIZE)
	{
		uint32_t byte_to_process = size - offset;

		if (byte_to_process > COPY_SIZE) byte_to_process = COPY_SIZE;

		flash_read_data(src_add + offset, buff, byte_to_process);

		flash_write_data(dst_add + offset, buff, byte_to_process);
	}
	HAL_IWDG_Refresh(&hiwdg);
}

static void start_write_firmware(uint32_t app_add, uint8_t* buff)
{
	uint32_t total_firmware_size = 0;
	printf("Waiting for firmware...\r\n");
	enable_receive_dma();

	while(1)
	{

		if(FirmwareProcessData.write_permission == SET)
		{
			FirmwareProcessData.write_permission = RESET;
			FirmwareProcessData.current_offset = 0;
			total_firmware_size = 0;
			send_command(ACK);
			enable_receive_dma();
			HAL_IWDG_Refresh(&hiwdg);
		}

		if(FirmwareProcessData.processing_data == SET)
		{
			FirmwareProcessData.processing_data = RESET;

			if(FirmwareProcessData.current_offset == RESET)
			{
				uint32_t flash_firmware_size = 	((uint32_t)buff[5] << 24)|
												((uint32_t)buff[6] << 16)|
												((uint32_t)buff[7] << 8)|
												(uint32_t)buff[8];

				total_firmware_size = flash_firmware_size + 0x200U;
				if(total_firmware_size >= APP_MAX_SIZE)
				{
					send_command(NACK);
					disable_receive_dma();
					return;
				}
				printf("Header detected. Total FW Size: %lu bytes\r\n", total_firmware_size);
				flash_erase_range(app_add, total_firmware_size);

			}
			uint32_t cal_crc = CalculateCRC_Software(buff, CRC_INDEX);
			uint32_t rx_crc = ((uint32_t)buff[CRC_INDEX ] << 24) |
							  ((uint32_t)buff[CRC_INDEX + 1] << 16) |
							  ((uint32_t)buff[CRC_INDEX + 2] << 8)  |
			                  ((uint32_t)buff[CRC_INDEX + 3]);

			if (cal_crc != rx_crc)
			{
			 printf("ERROR: CRC Fail! Calc: %08lX != Rx: %08lX\r\n", cal_crc, rx_crc);
			 send_command(NACK);
			 enable_receive_dma();
			 continue;
			 }

			uint32_t target_address = app_add + FirmwareProcessData.current_offset;
			flash_write_data(target_address, &buff[1], DATA_FIRMWARE);
			FirmwareProcessData.current_offset += DATA_FIRMWARE;

			enable_receive_dma();
			send_command(ACK);


			if (total_firmware_size > 0 && FirmwareProcessData.current_offset >= total_firmware_size)
			{
				printf("All data received. Write complete.\r\n");
				HAL_Delay(50);
				disable_receive_dma();
				break;
			}
			HAL_IWDG_Refresh(&hiwdg);
		}

	}
	printf("Done write the process...\r\n");

}

void download_new_firmware(void)
{
	start_write_firmware(APPLICATION_START_ADDRESS, uart_rx_buf);
}

void backup_current_firmware(void)
{
	uint32_t size = get_size(APPLICATION_START_ADDRESS, 4) + 0x200U;
	copy_flash_app(APPLICATION_START_ADDRESS, APPLICATION_BACKUP_ADDRESS, size);
}

void load_previous_firmware(void)
{
	uint32_t size = get_size(APPLICATION_BACKUP_ADDRESS, 4) + 0x200U;
	copy_flash_app(APPLICATION_BACKUP_ADDRESS, APPLICATION_START_ADDRESS, size);
}

uint8_t check_firmware_validation(uint32_t address)
{
	uint32_t size_app;

	size_app = get_size(address, GET_LENGTH_SIZE);
	if (apps_verificaton(address, size_app) != SUCCESS)
	{
		printf("Application is not valid in Address : 0x%08lX\r\n", address);
		return ERROR;

	}
	else
	{
		printf("Application in Address Address : 0x%08lX is valid\r\n", address);
		return SUCCESS;
	}
}
