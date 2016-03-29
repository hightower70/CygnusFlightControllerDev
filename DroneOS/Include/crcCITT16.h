/*****************************************************************************/
/* CITT16 CRC calculation routines                                           */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/
#ifndef __crcCITT16_h
#define __sysCITT16_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <stdint.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define crc16_INIT_VALUE 0xffff

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
uint16_t crc16Calculate(uint16_t in_crc, uint8_t in_data);
uint16_t crc16CalculateForBlock(uint16_t in_crc, uint8_t* in_buffer, uint16_t in_buffer_length);

#endif
