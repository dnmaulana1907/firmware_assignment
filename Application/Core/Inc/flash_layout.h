/*
 * flash_layout.h
 *
 *  Created on: Jan 30, 2026
 *      Author: danimaulana
 */

#ifndef INC_FLASH_LAYOUT_H_
#define INC_FLASH_LAYOUT_H_

#define BOOTLOADER_START_ADDRESS	0x08000000
#define VEC_OFFSET					0x200
#define	APPLICATION_START_ADDRESS	0x08008000
#define BACKUP_FLASH_ADDRESS
#define SECOND_APP_ADDRESS

#define APPLICATION_MAX_SIZE		992 * 1024
#endif /* INC_FLASH_LAYOUT_H_ */
