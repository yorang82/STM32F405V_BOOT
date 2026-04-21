/*
 * flash_engine.h
 *
 *  Created on: 2026. 4. 21.
 *      Author: YK_Cho
 */

#ifndef INC_FLASH_ENGINE_H_
#define INC_FLASH_ENGINE_H_

#include "main.h"

#define UPDATE_FILENAME "update.bin" //예: 12345678.bin
#define APP_START_ADDR  0x08020000  // Sector 5
#define FLASH_CHUNK_SIZE 2048       // 한 번에 읽을 크기 (2KB 권장)

// 플래그 관련 (Sector 7)
#define FLAG_ADDR       0x08060000
#define FLAG_PASS       0x50415353  // 'PASS'
#define FLAG_ING        0x494E4720  // 'ING '

HAL_StatusTypeDef Erase_App_Sectors(void);
HAL_StatusTypeDef Write_Flash(uint32_t DestAddr, uint8_t *pData, uint32_t DataLen);
void Update_Flag(uint32_t flag);
bool Is_App_Valid(void);
void Jump_To_Application(uint32_t address);



#endif /* INC_FLASH_ENGINE_H_ */
