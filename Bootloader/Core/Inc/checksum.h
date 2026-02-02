/*
 * checksum.h
 *
 *  Created on: Feb 1, 2026
 *      Author: danimaulana
 */

#ifndef INC_CHECKSUM_H_
#define INC_CHECKSUM_H_

#include "main.h"

uint32_t CalculateCRC(uint8_t *Data, uint32_t Length);
void re_initCRC(void);


#endif /* INC_CHECKSUM_H_ */
