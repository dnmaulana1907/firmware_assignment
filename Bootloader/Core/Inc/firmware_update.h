/*
 * firmware_update.h
 *
 *  Created on: Feb 3, 2026
 *      Author: danimaulana
 */

#ifndef INC_FIRMWARE_UPDATE_H_
#define INC_FIRMWARE_UPDATE_H_

#include "stm32f4xx_hal.h"

#define	 COPY_SIZE	512U


#define	ACK			0x09U
#define	NACK		0x11U
#define ERR			0x01U
#define	PROCESSING	0x06U

#define GET_LENGTH_SIZE		4U

void download_new_firmware(void);
void backup_current_firmware(void);
void load_previous_firmware(void);
uint8_t check_firmware_validation(uint32_t address);

#endif /* INC_FIRMWARE_UPDATE_H_ */
