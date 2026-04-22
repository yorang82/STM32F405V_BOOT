/*
 * Project: STM32F405V 펌웨어 업데이트 예제
 *
 * File: flash_engine.c
 * Author: Young Kwan CHO, Lucy
 * Last Modified: 2026-04-21
 * Description: 내장 Flash 메모리 업데이트/플래그 관리 함수 구현
 */


#include "flash_engine.h"


/* -------------------------------------------------------------------------- */
/*                        Application 영역 Flash 삭제                         */
/* -------------------------------------------------------------------------- */
/**
 * @brief  섹터 5, 6 (App 영역) 전체 삭제
 * @retval HAL_OK: 성공, HAL_ERROR: 실패
 */
HAL_StatusTypeDef Erase_App_Sectors(void)
{
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t SectorError;

    EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
    EraseInitStruct.Sector        = APP_SECTOR_START; // 시작 섹터
    EraseInitStruct.NbSectors     = APP_SECTOR_COUNT;              // Sector 5, 6 총 2개

    return HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);
}


/* -------------------------------------------------------------------------- */
/*                           Flash 데이터 기록 함수                           */
/* -------------------------------------------------------------------------- */
/**
 * @brief  Flash에 데이터 쓰기
 * @param  DestAddr  Flash 목적지 주소
 * @param  pData     데이터 포인터
 * @param  DataLen   데이터 길이 (byte)
 * @retval HAL_OK: 성공, HAL_ERROR: 실패
 */
HAL_StatusTypeDef Write_Flash(uint32_t DestAddr, uint8_t *pData, uint32_t DataLen)
{
    HAL_StatusTypeDef status = HAL_OK;
    // Flash Unlock
    if (HAL_FLASH_Unlock() != HAL_OK) {
        return HAL_ERROR;
    }

    for (uint32_t i = 0; i < DataLen; i += 4) {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, DestAddr + i, *(uint32_t*)(pData + i)) != HAL_OK) {
            status = HAL_ERROR;
            break;
        }
    }

    // Flash Lock
    HAL_FLASH_Lock();

    return status;
}


/* -------------------------------------------------------------------------- */
/*                        Sector 플래그 기록 함수                               */
/* -------------------------------------------------------------------------- */
/**
 * @brief  플래그 값(FLAG_PASS/ING 등) 기록
 * @param  flag  기록할 플래그 값
 */
void Update_Flag(uint32_t flag)
{
    HAL_FLASH_Unlock();
    
    // 기록 전 부저 짧게 ON
    LL_GPIO_SetOutputPin(BUZZER_GPIO_Port, BUZZER_Pin);

    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t SectorError;
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.Sector    = FLAG_SECTOR; // FLASH_SECTOR_4
    EraseInitStruct.NbSectors = 1;
    EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) == HAL_OK) {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLAG_ADDR, flag);
    }

    // 기록 후 부저 OFF
    LL_GPIO_ResetOutputPin(BUZZER_GPIO_Port, BUZZER_Pin);
    
    HAL_FLASH_Lock();
}


/* -------------------------------------------------------------------------- */
/*                        앱 실행 유효성 검사 함수                            */
/* -------------------------------------------------------------------------- */
/**
 * @brief  Application 영역 유효성 검사 (MSP 값 RAM 범위 체크)
 * @retval true: 유효, false: 무효
 */
bool Is_App_Valid(void)
{
    uint32_t msp = *(volatile uint32_t*)APP_START_ADDR;
    // RAM 영역 주소인지 확인 (0x20000000 ~ 0x20020000)
    return (msp >= 0x20000000 && msp <= 0x20020000);
}


/* -------------------------------------------------------------------------- */
/*                        앱으로 점프 (실행) 함수                              */
/* -------------------------------------------------------------------------- */
/**
 * @brief  지정 주소로 코드 점프 (Application 실행)
 * @param  address  점프할 시작 주소
 */
void Jump_To_Application(uint32_t address) {
    // 안차장님 코드의 핵심: USB 관련 인터럽트와 클럭을 완전히 끄기
    HAL_NVIC_DisableIRQ(OTG_FS_IRQn);
    __HAL_RCC_USB_OTG_FS_CLK_DISABLE();

    HAL_RCC_DeInit();
    HAL_DeInit();

    // 모든 인터럽트 마스킹 및 메모리 장벽 설정
    __set_PRIMASK(1);
    __disable_irq();
    __DSB();
    __ISB();

    uint32_t jumpAddress = *(__IO uint32_t*)(address + 4);
    void (*pJump)(void) = (void (*)(void))jumpAddress;

    __set_MSP(*(__IO uint32_t*)address);
    SCB->VTOR = address; // 벡터 테이블 재배치
    pJump();
}


/* -------------------------------------------------------------------------- */
/*                        플래그 값 반환 함수                                  */
/* -------------------------------------------------------------------------- */
/**
 * @brief  현재 저장된 플래그 값 반환
 * @retval 플래그 값 (FLAG_PASS, FLAG_ING 등)
 */
uint32_t Get_Flag(void)
{
    // FLAG_ADDR(0x08060000) 위치의 값을 32비트로 읽어서 반환합니다.
    return *(volatile uint32_t*)FLAG_ADDR;
}
