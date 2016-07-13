/*****************************************************************************/
/* System time functions                                                     */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

#ifndef __sysTimer_h
#define __sysTimer_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysRTOS.h>

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
uint32_t sysGetSystemTickSince(sysTick in_start_tick);

#endif