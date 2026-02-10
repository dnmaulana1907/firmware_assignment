/*
 * bootloader.h
 *
 *  Created on: Jan 30, 2026
 *      Author: danimaulana
 */

#ifndef INC_BOOTLOADER_H_
#define INC_BOOTLOADER_H_


typedef enum {
	BOR_RESET = 0,
	PIN_RESET,
	POW_RESET,
	SOFT_RESET,
	IWDG_RES,
	WWDG_RES,
	LOW_POWER_RES,

}ResetCause_t;


typedef enum {
	NORMAL_BOOT,
	BACKUP_MAIN_APP,
	CHECK_MAIN_APP,
	MAIN_APP_ERR,
	UPDATE_FIRMWARE
}UpdateStatus_t;
extern uint8_t write_permit;
extern uint8_t process_data;
void run_bootloader(void);
#endif /* INC_BOOTLOADER_H_ */
