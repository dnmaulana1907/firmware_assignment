/*
 * bootloader.c
 *
 *  Created on: Jan 30, 2026
 *      Author: danimaulana
 */
#include "main.h"
#include "usart.h"
#include "bootloader.h"
#include "flash_layout.h"
#include "app_header.h"
#include "authentication.h"
#include "flash.h"
#include "firmware_update.h"
#include "iwdg.h"

typedef void(*pFunction)(void);

extern uint32_t reset_flag;
uint8_t firmware_validation_status;

UpdateStatus_t UpdateStatus;

__inline static void validation_handle(uint32_t address)
{
	firmware_validation_status = RESET;
	firmware_validation_status = check_firmware_validation(address);
	printf("Firmware Validation Status : %u...\r\n", firmware_validation_status);
}

static void reset_cause_check(void)
{
	uint8_t reset_status;
    if (reset_flag & RCC_CSR_BORRSTF) {
        // Brownout reset occurred
    	reset_status = BOR_RESET;
    	printf("RESET CAUSE: BOR\r\n");
    }
    if (reset_flag & RCC_CSR_PINRSTF) {
        // Pin reset occurred
    	reset_status = PIN_RESET;
    	printf("RESET CAUSE: PIN RESET\r\n");
    }
    if(reset_flag & RCC_CSR_PORRSTF){
    	// power reset occurred
    	reset_status = POW_RESET;
    	printf("RESET CAUSE: POWER\r\n");
    }
    if (reset_flag & RCC_CSR_SFTRSTF) {
        // Software reset occurred
    	reset_status = SOFT_RESET;
    	printf("RESET CAUSE: SOFTWARE\r\n");
    }
    if (reset_flag & RCC_CSR_IWDGRSTF) {
        // Independent watchdog reset occurred
    	reset_status = IWDG_RES;
    	printf("RESET CAUSE: IWDG\r\n");
    }
    if (reset_flag & RCC_CSR_WWDGRSTF) {
        // Window watchdog reset occurred
    	reset_status = WWDG_RES;
    	printf("RESET CAUSE: WWDG\r\n");
    }
    if (reset_flag & RCC_CSR_LPWRRSTF) {
        // Low-power reset occurred
    	reset_status = LOW_POWER_RES;
    	printf("RESET CAUSE: LOW-POWER\r\n");
    }
    RCC->CSR |= RCC_CSR_RMVF;
    reset_flag = (uint32_t)reset_status;
    HAL_IWDG_Refresh(&hiwdg);
}

static void check_update_available(void)
{
	uint32_t size_backup;
	uint32_t size_main;
	uint8_t update_flag;

	update_flag = get_flag(UPDATE_FLAG_ADDRESS);
	size_main = get_size(APPLICATION_START_ADDRESS, 4);

	printf("Reset Cause Status: %lu \r\n", reset_flag);
	printf("Update Flag Status : %u\r\n", update_flag);
	printf("Size main memory: %lu Byte\r\n", size_main);

	if (update_flag != 0x01U)
	{
		if (reset_flag != SOFT_RESET && reset_flag != IWDG_RES)
		{
			printf("Normal boot...\r\n");
			UpdateStatus = NORMAL_BOOT;
		}else{

			if(reset_flag == SOFT_RESET)
			{
				size_backup = get_size(APPLICATION_BACKUP_ADDRESS, 4);
				printf("Size backup application: %lu\r\n", size_backup);
				if (size_backup == 0xFFFFFFFF)
				{
					UpdateStatus = BACKUP_MAIN_APP;
				}
				else
				{
					UpdateStatus = CHECK_MAIN_APP;
				}
			}

			if (reset_flag == IWDG_RES)
			{
				UpdateStatus = MAIN_APP_ERR;
			}

		}
	}
	else
	{
		UpdateStatus = UPDATE_FIRMWARE;
		flash_erase_range(UPDATE_FLAG_ADDRESS, 1U);
	}
	HAL_IWDG_Refresh(&hiwdg);
}


static void check_authentication(void){

	switch(UpdateStatus){
	case NORMAL_BOOT:
		/*Normal boot do nothing*/
		printf("Update status : Normal Boot...\r\n");
		break;
	case BACKUP_MAIN_APP:
		/*Backup Main Flash to Backup Memory*/
		printf("Update status : Backup Main Application...\r\n");
		backup_current_firmware();
		validation_handle(APPLICATION_BACKUP_ADDRESS);
		break;
	case CHECK_MAIN_APP:
		printf("Update status : Verify Main Application...\r\n");
		validation_handle(APPLICATION_START_ADDRESS);
		/*Validate Main Application*/
		break;
	case MAIN_APP_ERR:
		printf("Update status : Main Flash Error...\r\n");
		load_previous_firmware();
		check_firmware_validation(APPLICATION_START_ADDRESS);
		/*Copy Backup Memory to Main Address*/
		break;
	case UPDATE_FIRMWARE:
		printf("Update status : Update Firmware...\r\n");
		download_new_firmware();
		check_firmware_validation(APPLICATION_START_ADDRESS);
		if(firmware_validation_status != SUCCESS)
		{
			backup_current_firmware();
			HAL_NVIC_SystemReset();
		}
		HAL_NVIC_SystemReset();
		/*Writing Code To Main Application*/
		break;
	default:
		break;
	}
	HAL_IWDG_Refresh(&hiwdg);

}

static void start_application(void){
	uint32_t app_stack;
	pFunction app_entry;

	/* Read application stack pointer */
	app_stack = *(volatile uint32_t*)(APPLICATION_START_ADDRESS + VEC_OFFSET + 4);

	/*Read Reset Handler*/
	app_entry = (pFunction)app_stack;

	/*disable interrupt*/

	__disable_irq();

	/*STOP SysTick*/
	SysTick ->CTRL = 0;
	SysTick ->LOAD = 0;
	SysTick ->VAL = 0;

	/* Set main stack pointer */
	__set_MSP(app_stack);

	/*Jump to Appliaction*/
	app_entry();
}

void run_bootloader(void)
{
	reset_cause_check();
	check_update_available();
	check_authentication();

	start_application();
}
