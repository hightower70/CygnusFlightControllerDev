/*****************************************************************************/
/* UDP HAL driver                                                            */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

#ifndef __halUDP_h
#define __halUDP_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void halUDPInit(void);

uint8_t* halUDPAllocTransmitBuffer(void);
void halUDPTransmitData(uint16_t in_data_length, uint32_t in_destination_address);
uint32_t halUDPGetLocalIPAddress(void);
bool halUDPIsConnected(void);

#endif