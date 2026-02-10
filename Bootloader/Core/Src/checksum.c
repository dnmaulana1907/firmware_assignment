/*
 * checksum.c
 *
 *  Created on: Feb 1, 2026
 *      Author: danimaulana
 */
#include "main.h"
#include "checksum.h"
#include "crc.h"

extern CRC_HandleTypeDef hcrc;

void re_initCRC(void)
{
	MX_CRC_Init();
}

uint32_t CalculateCRC_Software(const uint8_t *pData, uint32_t Size)
{
    uint32_t crc = 0xFFFFFFFF;
    uint32_t poly = 0xEDB88320;

    for (uint32_t i = 0; i < Size; i++)
    {
        uint8_t data = pData[i];
        crc ^= data;

        for (int j = 0; j < 8; j++)
        {
            if (crc & 1)
            {
                crc = (crc >> 1) ^ poly;
            }
            else
            {
                crc = (crc >> 1);
            }
        }
    }

    return ~crc;
}
