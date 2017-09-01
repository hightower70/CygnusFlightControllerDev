/*****************************************************************************/
/* Communication manager functions                                           */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __comInterfaces_h
#define __comInterfaces_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/
typedef bool (*comInterfacePacketSendFunction)(uint8_t* in_packet, uint16_t in_packet_length);

typedef struct
{
	comInterfacePacketSendFunction PacketSendFunction;

} comInterfaceDescription;

#endif
