/*
 * Project: STM32F405V 펌웨어 업데이트 예제
 *
 * File: crc.h
 * Author: Young Kwan CHO, Lucy
 * Last Modified: 2026-04-05
 * Description: CRC 계산 인터페이스
 */


#ifndef APP_INCLUDE_COMMON_CRC_H_
#define APP_INCLUDE_COMMON_CRC_H_


#include "main.h"


#ifdef __cplusplus
extern "C" {
#endif

// ==================================================
//                  타입 정의
// ==================================================


/**
 * @brief  CRC 종류 열거형
 */
typedef enum
{
  CRC_NONE = 0,   // CRC 사용 안 함
  CRC_8,          // 8비트 CRC
  CRC_16,         // 16비트 CRC
  CRC_32          // 32비트 CRC
} crc_type_t;


// ==================================================
//                  외부 API
// ==================================================

/**
 * @brief  지정한 타입의 CRC 계산
 * @param  type    CRC 종류 (CRC_8, CRC_16, CRC_32)
 * @param  p_data  데이터 포인터
 * @param  length  데이터 길이 (byte)
 * @retval 계산된 CRC 값
 */
uint32_t crcCalculate(crc_type_t type, uint8_t *p_data, uint32_t length);


#ifdef __cplusplus
}
#endif


#endif /* APP_INCLUDE_COMMON_CRC_H_ */
