/*
 * flash_engine.c
 *
 *  Created on: 2026. 4. 21.
 *      Author: YK_Cho
 */

#include "flash_engine.h"
#include <stdbool.h>

// 섹터 5, 6 (App 영역) 지우기
HAL_StatusTypeDef Erase_App_Sectors(void) {
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t SectorError;

    EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
    EraseInitStruct.Sector        = FLASH_SECTOR_5; // 시작 섹터
    EraseInitStruct.NbSectors     = 2;              // Sector 5, 6 총 2개

    return HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);
}

// 데이터 기록 함수
HAL_StatusTypeDef Write_Flash(uint32_t DestAddr, uint8_t *pData, uint32_t DataLen) {
    for (uint32_t i = 0; i < DataLen; i += 4) {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, DestAddr + i, *(uint32_t*)(pData + i)) != HAL_OK) {
            return HAL_ERROR;
        }
    }
    return HAL_OK;
}

// 2. Sector 7 플래그 기록 함수 (공용)
void Update_Flag(uint32_t flag) {
    HAL_FLASH_Unlock();
    // Sector 7 전체를 지우고 새로 써야 합니다 (128KB)
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t SectorError;
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.Sector = FLASH_SECTOR_7;
    EraseInitStruct.NbSectors = 1;
    EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) == HAL_OK) {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLAG_ADDR, flag);
    }
    HAL_FLASH_Lock();
}

// 3. 앱 실행 유효성 검사 (MSP 값 범위 체크)
bool Is_App_Valid(void) {
    uint32_t msp = *(volatile uint32_t*)APP_START_ADDR;
    // RAM 영역 주소인지 확인 (0x20000000 ~ 0x20020000)
    return (msp >= 0x20000000 && msp <= 0x20020000);
}

// 4. 앱으로 점프 (안전 로직 추가)
void Jump_To_Application(uint32_t address) {
    typedef void (*pFunction)(void);

    // [보완] 점프 전 모든 인터럽트를 비활성화합니다.
    __disable_irq();

    // [보완] 시스틱 타이머 정지 (HAL_Delay 등 영향 방지)
    SysTick->CTRL = 0;
    SysTick->VAL = 0;

    // [보완] 주변 장치 인터럽트 클리어 (필요 시 NVIC_ICER 등 사용)

    uint32_t JumpAddress = *(volatile uint32_t*)(address + 4);
    pFunction JumpToApp = (pFunction)JumpAddress;

    // Stack Pointer 설정 및 점프
    __set_MSP(*(volatile uint32_t*)address);
    JumpToApp();
}
