/*****************************************************************************/
/* UDP HAL driver                                                            */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

#ifndef __drvUDP_h
#define __drvUDP_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void drvUDPInit(void);

uint8_t* drvUDPAllocTransmitBuffer(void);
void drvUDPTransmitData(uint16_t in_data_length, uint32_t in_destination_address);
uint32_t drvUDPGetLocalIPAddress(void);
bool drvUDPIsConnected(void);

#endif