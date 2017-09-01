/*****************************************************************************/
/* I/O definitions                                                           */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __drvIODefinitions_h
#define __drvIODefinitions_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>

/*****************************************************************************/
/* UART definitions                                                          */
/*****************************************************************************/

//extern void drvESP8266UARTRxCallback(uint8_t in_char, void* in_interrupt_param);

#define halUART_MAX_COUNT 3
#define halUART_INIT_NAMES { L"\\\\.\\COM47", L"\\\\.\\COM5", L"\\\\.\\COM62" }

/*****************************************************************************/
/* Graphics definitions                                                      */
/*****************************************************************************/

// resolution
#define guiSCREEN_WIDTH 240
#define guiSCREEN_HEIGHT 320

#define guiemuZOOM 1

#define guiCOLOR_DEPTH 24

// resource address
typedef int sysResourceAddress;

#endif
