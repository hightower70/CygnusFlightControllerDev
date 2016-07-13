/*****************************************************************************/
/* Communication manager functions                                           */
/*                                                                           */
/* Copyright (C) 20152016 Laszlo Arvai                                       */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

#ifndef __comManager_h
#define __comManager_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>
#include <comManager.h>
#include <comInterfaces.h>
#include <comPacketQueue.h>


/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define comCRC_BYTE_COUNT 2
#define comMAX_PACKET_SIZE 255
#define comINVALID_INTERFACE_INDEX 0xff

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void comManagerInit(void);
void comManagerTaskStop(void);

uint8_t comAddInterface(comInterfaceDescription* in_interface);

void comManagerStoreReceivedPacket(uint8_t in_interface_index, uint8_t* in_packet, uint8_t in_packet_size);

uint8_t comIncrementAndGetTransmittedPacketCounter(void);


uint8_t* comManagerTransmitPacketPushStart(uint8_t in_packet_size, uint8_t in_interface_index, uint8_t in_packet_type, uint16_t *out_packet_index);
void comManagerTransmitPacketPushEnd(uint16_t in_packet_index);
void comManagerTransmitPacketPushCancel(uint16_t in_packet_index);
uint8_t* comManagerGetTransmitPacketGetBuffer(uint16_t in_packet_index);

void comManagerGenerateEvent(void);

#endif
