/*
 * app_header.h
 *
 *  Created on: Jan 30, 2026
 *      Author: danimaulana
 */

#ifndef INC_APP_HEADER_H_
#define INC_APP_HEADER_H_

#include "flash_layout.h"

typedef struct
{
	uint8_t major;
	uint8_t minor;
	uint8_t version;
	uint8_t reserve;
} fw_version_t;

#endif /* INC_APP_HEADER_H_ */
