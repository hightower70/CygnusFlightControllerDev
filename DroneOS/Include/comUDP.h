/*****************************************************************************/
/* UDP Communication routines                                                */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __comUDP_h
#define __comUDP_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define comUDP_PERIODIC_CALLBACK_TIME 100

#define comUDP_MAKE_BROADCAST_ADDRESS(x) (x | 0x000000ff)


/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void comUDPInit(void);
void comUDPProcessReceivedPacket(uint8_t* in_packet, uint8_t in_packet_size);
void comUDPPeriodicCallback(void);
bool comUDPSendPacket(uint8_t* in_packet, uint16_t in_packet_length);

#endif