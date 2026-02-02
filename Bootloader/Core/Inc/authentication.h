/*
 * aunthentification.h
 *
 *  Created on: Feb 1, 2026
 *      Author: danimaulana
 */

#ifndef INC_AUTHENTICATION_H_
#define INC_AUTHENTICATION_H_

#include "main.h"

int apps_verificaton(uint32_t address, uint32_t size);
int calculate_autentication(uint8_t *crc, uint8_t *signature, uint8_t major_ver, uint8_t minor_ver, uint8_t revision_ver);
void get_version(uint8_t *major, uint8_t *minor, uint8_t *revision, uint8_t *update_flag,uint32_t address);
void get_signature(uint8_t *signature, uint32_t address, uint8_t length);
uint32_t get_size(uint32_t address, uint8_t length);

#endif /* INC_AUTHENTICATION_H_ */
