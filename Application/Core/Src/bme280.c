/*
 * bme280.c
 *
 *  Created on: Feb 9, 2026
 *      Author: danimaulana
 */

#include "bme280.h"
void BME280_Init(I2C_HandleTypeDef *hi2c){
	uint8_t configData[2];


	configData[0] = REG_CTRL_HUM;
	configData[1] = 0x01;
	HAL_I2C_Master_Transmit(hi2c, BME280_ADDR, configData, 2U, I2C_TIMEOUT);

	configData[0] = REG_CTRL_MEAS;
	configData[1] = 0x27;
	HAL_I2C_Master_Transmit(hi2c, BME280_ADDR, configData, 2U, I2C_TIMEOUT);
}


void BME280_Read(I2C_HandleTypeDef *hi2c, uint8_t* buffer)
{
	uint8_t register_address = REG_DATA_START;
	HAL_I2C_Master_Transmit(hi2c, BME280_ADDR, & register_address, 1U, I2C_TIMEOUT);
	HAL_I2C_Master_Receive(hi2c, BME280_ADDR, buffer, BME280_SIZE_DATA, I2C_TIMEOUT);
}
