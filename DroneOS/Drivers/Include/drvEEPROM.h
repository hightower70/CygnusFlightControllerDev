/*****************************************************************************/
/* EEPROM driver                                                             */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __drvEEPROM_h
#define __drvEEPROM_h

/*****************************************************************************/
/* Include files                                                             */
/*****************************************************************************/
#include <sysTypes.h>

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void drvEEPROMInit(void);
bool drvEEPROMReadBlock(uint16_t in_address, uint8_t* out_buffer, uint16_t in_length);
bool drvEEPROMWriteBlock(uint16_t in_address, uint8_t* in_buffer, uint16_t in_length);

#endif
