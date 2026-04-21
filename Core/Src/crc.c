/*
 * Project: Common Layer
 *
 * File: crc.c
 * Author: Young Kwan CHO, Lucy
 * Last Modified: 2026-04-05
 * Description: CRC calculation implementation
 */

#include "crc.h"

/* -------------------------------------------------------------------------- */
/*                         GENERIC LOOKUP TABLE STORAGE                       */
/* -------------------------------------------------------------------------- */
/*
 * CRC-8   : poly 0x07
 * CRC-16  : CCITT poly 0x1021
 * CRC-32  : poly 0xEDB88320 (reflected)
 *
 * The tables below are generic standard lookup tables generated once at init
 * and reused by all channels through crcCalculate().
 */
static bool     crc_table_is_init = false;
static uint8_t  crc8_table[256];
static uint16_t crc16_table[256];
static uint32_t crc32_table[256];

static void crcInitTables(void)
{
  if (crc_table_is_init == true)
  {
    return;
  }

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

uint32_t crcCalculate(crc_type_t type, uint8_t *p_data, uint32_t length)
{
  if ((type != CRC_NONE) && ((p_data == NULL) || (length == 0U)))
  {
    return 0U;
  }

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
