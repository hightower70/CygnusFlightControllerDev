/*****************************************************************************/
/* RTOS Wrapper functions                                                    */
/*                                                                           */
/* Copyright (C) 2014-2015 Laszlo Arvai                                      */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/
#ifndef __sysRTOS_h
#define __sysRTOS_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>
#include <string.h>

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/
typedef uint32_t sysTick;

/*****************************************************************************/
/* RTOS independent function prototypes                                      */
/*****************************************************************************/
sysTick sysGetSystemTick(void);
uint32_t sysGetSystemTickSince(sysTick in_start_tick);
uint8_t sysGetCPUUsage(void);
void sysDelay(uint32_t in_delay_in_ms);

#define sysMemZero(dst, siz)  memset(dst, 0, siz)
#define sysMemCopy(dst, src, siz) memcpy(dst, src, siz)

/*****************************************************************************/
/* RTOS dependent function prototypes                                        */
/*****************************************************************************/
#if defined(_WIN32)
#include <sysRTOS_Win32.h>
#elif defined(__linux)
#include <sysRTOS_Linux.h>
#else
#include <sysRTOS_FreeRTOS.h>
#endif

#endif
