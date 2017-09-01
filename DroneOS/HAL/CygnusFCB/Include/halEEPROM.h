/*****************************************************************************/
/* SPI EEPROM HAL                                                            */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __halEEPROM_h
#define __halEEPROM_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void halEEPROMInit(void);
bool halEEPROMWriteAndReadBlock(uint8_t* in_write_block, uint16_t in_write_block_length, uint8_t* out_read_block, uint16_t in_read_block_length);
bool halEEPROMWriteAndWriteBlock(uint8_t* in_write_block1, uint16_t in_write_block1_length, uint8_t* in_write_block2, uint16_t in_write_block2_length);

#endif

