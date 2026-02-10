/*
 * bme280.h
 *
 *  Created on: Feb 9, 2026
 *      Author: danimaulana
 */

#ifndef INC_BME280_H_
#define INC_BME280_H_
#include "main.h"
#include "i2c.h"

#define BME280_ADDR (0x76 << 1)
#define REG_CTRL_HUM  0xF2
#define REG_CTRL_MEAS 0xF4
#define REG_CONFIG    0xF5
#define REG_DATA_START 0xF7


#define BME280_SIZE_DATA	8U
#define I2C_TIMEOUT			100U

void BME280_Init(I2C_HandleTypeDef *hi2c);
void BME280_Read(I2C_HandleTypeDef *hi2c, uint8_t* buffer);
#endif /* INC_BME280_H_ */
