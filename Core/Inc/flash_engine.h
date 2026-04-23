/*
 * Project: STM32F405V 펌웨어 업데이트 예제
 *
 * File: flash_engine.h
 * Author: Young Kwan CHO, Lucy
 * Last Modified: 2026-04-21
 * Description: 내장 Flash 메모리 업데이트/플래그 관리 API
 */


#ifndef INC_FLASH_ENGINE_H_
#define INC_FLASH_ENGINE_H_


#include "main.h"
#include <stdbool.h>


// ==================================================
//                  매크로 정의
// ==================================================
#define UPDATE_FILENAME   "derma.bin" // 예: 12345678.bin


// ------------------- 플래그 관련 -------------------
#define FLAG_ADDR         0x08010000   // 플래그 저장 주소
#define FLAG_SECTOR       FLASH_SECTOR_4 // 플래그 저장 섹터 (64KB, Sector 4)
#define FLAG_PASS         0x50415353   // 'PASS'
#define FLAG_NEW          0x4E455720   // 'NEW '
#define FLAG_READY        0x52454144   // 'READY'
#define FLAG_ING          0x494E4720   // 'ING '

#define FLASH_CHUNK_SIZE  128         // Flash 1회 읽기/쓰기 크기 (128B 권장)

// ------------------- 앱 영역 (Sector 5~6) -------------------
#define APP_START_ADDR    0x08020000   // Sector 5 시작 주소
#define APP_SECTOR_START  FLASH_SECTOR_5 // 앱 영역 시작 섹터
#define APP_SECTOR_COUNT  2            // [변경] Sector 5, 6만 사용 (256KB)


// ==================================================
//                  외부 API
// ==================================================

/**
 * @brief  Application 영역 Flash 전체 삭제
 * @retval HAL_OK: 성공, HAL_ERROR: 실패
 */
HAL_StatusTypeDef Erase_App_Sectors(void);

/**
 * @brief  Flash에 데이터 쓰기
 * @param  DestAddr  Flash 목적지 주소
 * @param  pData     데이터 포인터
 * @param  DataLen   데이터 길이 (byte)
 * @retval HAL_OK: 성공, HAL_ERROR: 실패
 */
HAL_StatusTypeDef Write_Flash(uint32_t DestAddr, uint8_t *pData, uint32_t DataLen);

/**
 * @brief  플래그 값 업데이트 (Sector 4)
 * @param  flag  기록할 플래그 값 (FLAG_PASS, FLAG_ING 등)
 */
void Update_Flag(uint32_t flag);

/**
 * @brief  Application 영역 유효성 검사
 * @retval true: 유효, false: 무효
 */
bool Is_App_Valid(void);

/**
 * @brief  지정 주소로 코드 점프 (Application 실행)
 * @param  address  점프할 시작 주소
 */
void Jump_To_Application(uint32_t address);

/**
 * @brief  현재 저장된 플래그 값 반환
 * @retval 플래그 값 (FLAG_PASS, FLAG_ING 등)
 */
uint32_t Get_Flag(void);



#endif /* INC_FLASH_ENGINE_H_ */
