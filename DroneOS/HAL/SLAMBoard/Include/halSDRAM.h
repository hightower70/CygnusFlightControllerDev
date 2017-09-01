/*****************************************************************************/
/* Hardware Abstraction Layer for SDRAM interface                            */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __halSDRAM_h
#define __halSDRAM_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>
#include <halIODefinitions.h>
#include <stm32f4xx_hal.h>

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
bool halSDRAMInitialize(void);

#define halSDRAMWriteByte(address, value)       (*(__IO uint8_t *) (halSDRAM_START_ADDRESS + (address)) = (value))
#define halSDRAMReadByte(address)               (*(__IO uint8_t *) (halSDRAM_START_ADDRESS + (address)))

#endif
