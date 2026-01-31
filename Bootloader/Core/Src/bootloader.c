/*
 * bootloader.c
 *
 *  Created on: Jan 24, 2026
 *      Author: danimaulana
 */
#include "main.h"
#include "bootloader.h"
#include "flash_layout.h"
#include "app_header.h"


#define APP_MAGIC	0xABCDEFAB
typedef void(*pFunction)(void);

void start_bootloader(void){
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

