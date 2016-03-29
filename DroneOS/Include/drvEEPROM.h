/*****************************************************************************/
/* EEPROM driver                                                             */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
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
void drvEEPROMReadBlock(uint16_t in_address, uint8_t* out_buffer, uint16_t in_length);
void drvEEPROMWriteBlock(uint16_t in_address, uint8_t* in_buffer, uint16_t in_length);
bool drvEEPROMVerifyBlock(uint16_t in_address, uint8_t* in_buffer, uint16_t in_length);

#endif
