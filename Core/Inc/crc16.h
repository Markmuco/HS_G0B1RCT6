/*!
 * \file  crc16.h
 *
 * \brief This software module contains functions to calculate 16 bit CRC.
 */
#ifndef _CRC16_H_
#define _CRC16_H_

// Public function prototypes
uint16_t crc16_calc(uint8_t *p_data, uint32_t size, uint16_t crc);

#endif
