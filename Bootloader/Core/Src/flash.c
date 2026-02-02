/*
 * flash.c
 *
 *  Created on: Feb 1, 2026
 *      Author: danimaulana
 */
#include "flash.h"
#include <string.h>


static uint32_t GetSector(uint32_t Address) {
    uint32_t sector = 0;

    if((Address < 0x08004000) && (Address >= 0x08000000)) {
        sector = FLASH_SECTOR_0;  // 16 KB
    } else if((Address < 0x08008000) && (Address >= 0x08004000)) {
        sector = FLASH_SECTOR_1;  // 16 KB
    } else if((Address < 0x0800C000) && (Address >= 0x08008000)) {
        sector = FLASH_SECTOR_2;  // 16 KB
    } else if((Address < 0x08010000) && (Address >= 0x0800C000)) {
        sector = FLASH_SECTOR_3;  // 16 KB
    } else if((Address < 0x08020000) && (Address >= 0x08010000)) {
        sector = FLASH_SECTOR_4;  // 64 KB
    } else if((Address < 0x08040000) && (Address >= 0x08020000)) {
        sector = FLASH_SECTOR_5;  // 128 KB
    } else if((Address < 0x08060000) && (Address >= 0x08040000)) {
        sector = FLASH_SECTOR_6;  // 128 KB
    } else if((Address < 0x08080000) && (Address >= 0x08060000)) {
        sector = FLASH_SECTOR_7;  // 128 KB
    } else if((Address < 0x080A0000) && (Address >= 0x08080000)) {
        sector = FLASH_SECTOR_8;  // 128 KB
    } else if((Address < 0x080C0000) && (Address >= 0x080A0000)) {
        sector = FLASH_SECTOR_9;  // 128 KB
    } else if((Address < 0x080E0000) && (Address >= 0x080C0000)) {
        sector = FLASH_SECTOR_10; // 128 KB
    } else if((Address < 0x08100000) && (Address >= 0x080E0000)) {
        sector = FLASH_SECTOR_11; // 128 KB
    }
    return sector;
}

FlashOp_Status_t Flash_Erase_Range(uint32_t startAddr, uint32_t length) {
    FlashOp_Status_t status = FLASH_OK_STATUS;
    uint32_t FirstSector, NbOfSectors, SectorError;
    FLASH_EraseInitTypeDef EraseInitStruct;


    HAL_FLASH_Unlock();

    FirstSector = GetSector(startAddr);
    uint32_t LastSector = GetSector(startAddr + length - 1);
    NbOfSectors = LastSector - FirstSector + 1;

    // 3. Konfigurasi Erase
    EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
    EraseInitStruct.Sector        = FirstSector;
    EraseInitStruct.NbSectors     = NbOfSectors;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK) {
        status = FLASH_ERR_ERASE;
    }

    HAL_FLASH_Lock();

    return status;
}

FlashOp_Status_t Flash_Write_Data(uint32_t destAddr, const uint8_t *data, uint32_t length) {
    FlashOp_Status_t status = FLASH_OK_STATUS;

    HAL_FLASH_Unlock();

    for (uint32_t i = 0; i < length; i += 4) {
        uint32_t currentAddr = destAddr + i;
        uint32_t wordData = 0xFFFFFFFF;

        uint32_t bytesRemaining = length - i;
        if (bytesRemaining >= 4) {
            memcpy(&wordData, &data[i], 4);
        } else {
            memcpy(&wordData, &data[i], bytesRemaining);
        }

        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, currentAddr, wordData) != HAL_OK) {
            status = FLASH_ERR_WRITE;
            break;
        }

        if (*(volatile uint32_t*)currentAddr != wordData) {
            status = FLASH_ERR_WRITE;
            break;
        }
    }

    HAL_FLASH_Lock();
    return status;
}

void Flash_Read_Data(uint32_t srcAddr, uint8_t *buffer, uint32_t length) {
    memcpy(buffer, (void*)srcAddr, length);
}


