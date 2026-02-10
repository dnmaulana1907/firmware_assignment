/*
 * aunthentication.c
 *
 *  Created on: Feb 1, 2026
 *      Author: danimaulana
 */

#include "authentication.h"
#include <string.h>
#include "bootloader.h"
#include <stdint.h>
#include "flash.h"
#include "checksum.h"
#include <stdio.h>
cmox_sha256_handle_t sha256_ctx;



static void read_byte(uint8_t *result, uint32_t address, uint8_t length, uint8_t offset){
	uint8_t buffer[512];
	flash_read_data(address + offset, buffer, length);
	memcpy(result, buffer, length);
}


void get_version(uint8_t *major, uint8_t *minor, uint8_t *revision, uint32_t address){
	uint8_t buffer[4];
	read_byte(buffer, address, 4U, 0);
	*major = buffer[0];
	*minor = buffer[1];
	*revision = buffer[2];
}

uint32_t get_size(uint32_t address, uint8_t length){
	uint8_t buffer[4];
	read_byte(buffer, address, length, 4);
	return ((buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3]);
}

void get_signature(uint8_t *signature, uint32_t address, uint8_t length){
	read_byte(signature, address, length, 8);
}

uint8_t get_flag(uint32_t address){
	uint8_t buff;
	read_byte(&buff, address, 1, 0);
	return buff;
}

int calculate_authentication(uint8_t *crc, uint8_t *signature, uint8_t major_ver, uint8_t minor_ver, uint8_t revision_ver){
	uint8_t sig_computed[CMOX_SHA256_SIZE];
	cmox_hash_retval_t value_return;
	size_t sig_size;

	if(cmox_initialize(NULL) != CMOX_INIT_SUCCESS){
		//Error Case
	}

	uint16_t major = *(uint16_t *)&major_ver;
	uint16_t shift_right = major%2;
	uint8_t loops = 1 + (minor_ver%3);

	value_return = cmox_hash_compute(CMOX_SHA256_ALGO, crc, sizeof(crc), sig_computed, CMOX_SHA256_SIZE, &sig_size);

	for (int i = 0; i < CMOX_SHA256_SIZE; i++){
		sig_computed[i] = sig_computed[i] >> shift_right;
	}


	for (int i = 1; i <= loops; i++){
		value_return = cmox_hash_compute(CMOX_SHA256_ALGO, sig_computed, sizeof(sig_computed), sig_computed, CMOX_SHA256_SIZE, &sig_size);
	}

	if(value_return != CMOX_HASH_SUCCESS){
		/*ERROR*/
		printf("Error cmox has\r\n....");
	}

	if(sig_size != CMOX_SHA256_SIZE){
		/*ERROR*/
		printf("Error cmoxs size\r\n....");
	}

	int retval = memcmp(signature, sig_computed, CMOX_SHA256_SIZE);
	return retval;
}

int apps_verificaton(uint32_t address, uint32_t size){
	uint8_t crc_buffer[4];
	uint8_t apps_major = 0;
	uint8_t apps_minor = 0;
	uint8_t apps_revision = 0;
	uint8_t apps_signature[CMOX_SHA256_SIZE];

	memset(apps_signature, 0x00, CMOX_SHA256_SIZE);
	re_initCRC();
	HAL_Delay(20);

	uint32_t calculated_crc = CalculateCRC_Software(((uint8_t *)address + 0x200), size);

	crc_buffer[0] = (calculated_crc >> 24) & 0xFF;
	crc_buffer[1] = (calculated_crc >> 16) & 0xFF;
	crc_buffer[2] = (calculated_crc >> 8) & 0XFF;
	crc_buffer[3] = (calculated_crc) & 0XFF;


	get_version(&apps_major, &apps_minor, &apps_revision, address);
	    /* --- TAMBAHAN PRINTF DI SINI --- */
	printf("Read Version from Flash: %d.%d.%d\r\n", apps_major, apps_minor, apps_revision);

	get_signature(apps_signature, address, CMOX_SHA256_SIZE);


	int ret = calculate_authentication(crc_buffer, apps_signature, apps_major, apps_minor, apps_revision);
	return ret;
}
