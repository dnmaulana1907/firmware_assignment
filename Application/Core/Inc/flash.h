/*
 * flash.h
 *
 *  Created on: Feb 1, 2026
 *      Author: danimaulana
 */

#ifndef INC_FLASH_H_
#define INC_FLASH_H_

#include "stm32f4xx_hal.h"
#include "flash_layout.h"


typedef enum {
	FLASH_OK_STATUS = 0,
	FLASH_ERR_LOCKED,
	FLASH_ERR_WRITE,
	FLASH_ERR_ERASE,
	FLASH_ERR_ALIGNMENT
} FlashOp_Status_t;

FlashOp_Status_t Flash_Erase_Range(uint32_t startAddr, uint32_t length);
FlashOp_Status_t Flash_Write_Data(uint32_t destAddr, const uint8_t *data, uint32_t length);
void Flash_Read_Data(uint32_t srcAddr, uint8_t *buffer, uint32_t length);
FlashOp_Status_t Flash_Erase_Address(uint32_t address);
#endif /* INC_FLASH_H_ */
