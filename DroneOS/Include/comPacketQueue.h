/*****************************************************************************/
/* Communication packet queue handling functions                             */
/*                                                                           */
/* Copyright (C) 2014-2015 Laszlo Arvai                                      */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __comPacketQueue_h
#define __comPacketQueue_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>
#include <sysRTOS.h>


/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define comINVALID_PACKET_INDEX 0xffff

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/

/// Packet status code in the queue
typedef enum
{
	comPQS_EndOfQueue,

	comPQS_Reserved,
	comPQS_Ready,
	comPQS_Deleted

} comPacketStatus;

/// Information about the stored packet
typedef struct
{
	comPacketStatus Status;
	uint8_t Size;
	sysTick Timestamp;
	uint8_t Interface;
} comPacketInfo;

/// Event code for queue event handler function
typedef enum
{
	comQE_PacketPushed,
	comQE_PacketPopped
} comQueueEvent;

/// Type of the queue event handler callback function
typedef void(*comQueueNotificationFunction)(comQueueEvent in_event);

/// Packet queue
typedef struct
{
	uint8_t* Buffer;
	uint16_t BufferSize;

	sysMutex PushLock;

	uint16_t PushIndex;
	uint16_t PopIndex;

	comQueueNotificationFunction Callback;

} comPacketQueue;


///////////////////////////////////////////////////////////////////////////////
// Function prototypes
void comPacketQueueInitialize(comPacketQueue* in_queue, uint8_t* in_packet_buffer, uint16_t in_buffer_size);

uint16_t comPacketQueuePushBegin(comPacketQueue* in_queue, uint8_t in_size, uint8_t in_source_interface);
void comPacketQueuePushEnd(comPacketQueue* in_queue, uint16_t in_packet_index);
void comPacketQueuePushCancel(comPacketQueue* in_queue, uint16_t in_packet_index);
void comPacketQueueStoreByte(comPacketQueue* in_queue, uint16_t in_packet_index, uint8_t in_byte_index, uint8_t in_data);

uint16_t comPacketQueuePopBegin(comPacketQueue* in_queue);
void comPacketQueuePopEnd(comPacketQueue* in_queue);

comPacketInfo* comPacketQueueGetPacketInfo(comPacketQueue* in_queue, uint16_t in_packet_index);
uint8_t* comPacketQueueGetPacketBuffer(comPacketQueue* in_queue, uint16_t in_packet_index);


#endif