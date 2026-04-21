/*
 * Project: STM32F405V 펌웨어 업데이트 예제
 *
 * File: crc.c
 * Author: Young Kwan CHO, Lucy
 * Last Modified: 2026-04-21
 * Description: CRC 계산 함수 구현 (8/16/32비트)
 */


#include "crc.h"
#include <stdbool.h>

/* -------------------------------------------------------------------------- */
/*                         CRC 테이블 저장소 및 초기화                         */
/* -------------------------------------------------------------------------- */
/*
 * CRC-8   : poly 0x07
 * CRC-16  : CCITT poly 0x1021
 * CRC-32  : poly 0xEDB88320 (reflected)
 *
 * 아래 테이블은 최초 1회만 생성되며, crcCalculate()에서 재사용됩니다.
 */

static bool     crc_table_is_init = false;      // 테이블 초기화 여부
static uint8_t  crc8_table[256];                // CRC-8 테이블
static uint16_t crc16_table[256];               // CRC-16 테이블
static uint32_t crc32_table[256];               // CRC-32 테이블


/**
 * @brief  CRC 테이블(8/16/32) 1회 초기화
 */
static void crcInitTables(void)
{
  if (crc_table_is_init == true)
    return;

  for (uint32_t i = 0U; i < 256U; i++)
  {
    uint8_t crc8 = (uint8_t)i;
    uint16_t crc16 = (uint16_t)(i << 8);
    uint32_t crc32 = i;

    for (uint32_t bit = 0U; bit < 8U; bit++)
    {
      crc8 = (crc8 & 0x80U) ? (uint8_t)((crc8 << 1) ^ 0x07U) : (uint8_t)(crc8 << 1);
      crc16 = (crc16 & 0x8000U) ? (uint16_t)((crc16 << 1) ^ 0x1021U) : (uint16_t)(crc16 << 1);
      crc32 = (crc32 & 1U) ? ((crc32 >> 1) ^ 0xEDB88320U) : (crc32 >> 1);
    }

    crc8_table[i] = crc8;
    crc16_table[i] = crc16;
    crc32_table[i] = crc32;
  }

  crc_table_is_init = true;
}


/**
 * @brief  지정 타입의 CRC 계산 (8/16/32)
 * @param  type    CRC 종류 (CRC_8, CRC_16, CRC_32)
 * @param  p_data  데이터 포인터
 * @param  length  데이터 길이 (byte)
 * @retval 계산된 CRC 값
 */
uint32_t crcCalculate(crc_type_t type, uint8_t *p_data, uint32_t length)
{
  if ((type != CRC_NONE) && ((p_data == NULL) || (length == 0U)))
    return 0U;

  crcInitTables();

  switch (type)
  {
    case CRC_8:
    {
      uint8_t crc = 0x00U;
      for (uint32_t i = 0U; i < length; i++)
      {
        crc = crc8_table[crc ^ p_data[i]];
      }
      return crc;
    }

    case CRC_16:
    {
      uint16_t crc = 0xFFFFU;
      for (uint32_t i = 0U; i < length; i++)
      {
        uint8_t index = (uint8_t)((crc >> 8) ^ p_data[i]);
        crc = (uint16_t)((crc << 8) ^ crc16_table[index]);
      }
      return crc;
    }

    case CRC_32:
    {
      uint32_t crc = 0xFFFFFFFFU;
      for (uint32_t i = 0U; i < length; i++)
      {
        uint8_t index = (uint8_t)(crc ^ p_data[i]);
        crc = (crc >> 8) ^ crc32_table[index];
      }
      return (crc ^ 0xFFFFFFFFU);
    }

    case CRC_NONE:
    default:
      return 0U;
  }
}
