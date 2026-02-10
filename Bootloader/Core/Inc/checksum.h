/*
 * checksum.h
 *
 *  Created on: Feb 1, 2026
 *      Author: danimaulana
 */

#ifndef INC_CHECKSUM_H_
#define INC_CHECKSUM_H_

#include "main.h"

void re_initCRC(void);
uint32_t CalculateCRC_Software(const uint8_t *pData, uint32_t Size);

#endif /* INC_CHECKSUM_H_ */
