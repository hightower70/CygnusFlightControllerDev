/*****************************************************************************/
/* I/O definitions                                                           */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
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

extern void drvESP8266UARTRxCallback(uint8_t in_char, void* in_interrupt_param);

#define drvUART_MAX_COUNT 1
#define drvUART_INIT_NAMES { L"\\\\.\\COM10" }



#endif
