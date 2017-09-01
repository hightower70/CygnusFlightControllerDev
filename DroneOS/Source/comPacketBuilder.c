/*****************************************************************************/
/* Communication packet builder functions                                    */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/
#include <comSystemPacketDefinitions.h>
#include <comManager.h>

/*****************************************************************************/
/* Function implementations                                                  */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Fills packet header
/// @param in_header Pointer to the packet header to fill
/// @param in_type Packet type code
/// @param in_length Length of the packet in bytes
void comFillPacketHeader(comPacketHeader* in_header, uint8_t in_type, uint8_t in_length)
{
	in_header->PacketLength = in_length + comCRC_BYTE_COUNT;
	in_header->PacketType = in_type;
	in_header->PacketCounter = comIncrementAndGetTransmittedPacketCounter();
}