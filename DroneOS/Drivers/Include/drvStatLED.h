/*****************************************************************************/
/* Status LED driver (dimming) functions                                     */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

#ifndef __drvStatLED_h
#define __drvStatLED_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
void drvStatLEDInit(void);
void drvStatLEDSetDim(uint8_t in_dim);

#endif
