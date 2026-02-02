/*
 * checksum.c
 *
 *  Created on: Feb 1, 2026
 *      Author: danimaulana
 */
#include "checksum.h"
#include "crc.h"

uint32_t CalculateCRC(uint8_t *Data, uint32_t Length){
	return HAL_CRC_Calculate(&hcrc,(uint32_t*)Data, Length);
}
void re_initCRC(void)
{
	MX_CRC_Init();
}
