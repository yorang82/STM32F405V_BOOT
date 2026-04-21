/*
 * Project: Common Layer
 *
 * File: crc.h
 * Author: Young Kwan CHO, Lucy
 * Last Modified: 2026-04-05
 * Description: CRC calculation interface
 */

#ifndef APP_INCLUDE_COMMON_CRC_H_
#define APP_INCLUDE_COMMON_CRC_H_

#include "main.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
  CRC_NONE = 0,
  CRC_8,
  CRC_16,
  CRC_32
} crc_type_t;

uint32_t crcCalculate(crc_type_t type, uint8_t *p_data, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif /* APP_INCLUDE_COMMON_CRC_H_ */
