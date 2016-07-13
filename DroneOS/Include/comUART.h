/*****************************************************************************/
/* UART communication driver                                                 */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

#ifndef __comUART_h
#define __comUART_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void comUARTInit(void);
void comUARTDeinit(void);

bool comUARTSendPacket(uint8_t* in_packet, uint16_t in_packet_length);

#endif
