/*****************************************************************************/
/* Communication packet queue handling functions                             */
/*                                                                           */
/* Copyright (C) 2014-2015 Laszlo Arvai                                      */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <comPacketQueue.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/

/*****************************************************************************/
/* Module local functions                                                    */
/*****************************************************************************/
static void comPacketQueueCallbackExecute(comPacketQueue* in_queue, comQueueEvent in_event);

/*****************************************************************************/
/* Public functions                                                          */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initialize packet queue
/// @param in_queue Packet queue description
/// @param in_packet_buffer Packet data buffer
/// @param in_buffer_size Packet data buffer size in bytes
void comPacketQueueInitialize(comPacketQueue* in_queue, uint8_t* in_packet_buffer, uint16_t in_buffer_size)
{
	in_queue->Buffer = in_packet_buffer;
	in_queue->BufferSize = in_buffer_size;
	sysMutexCreate(in_queue->PushLock);
	
	in_queue->PushIndex = 0;
	in_queue->PopIndex = 0;

	in_queue->Callback = sysNULL;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets pointer to the packet info header in the packet queue
/// @param in_queue Queue description
/// @param in_packet_index Packet index
comPacketInfo* comPacketQueueGetPacketInfo(comPacketQueue* in_queue, uint16_t in_packet_index)
{
	return (comPacketInfo*)&(in_queue->Buffer[in_packet_index]);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets pointer to the packet buffer
/// @param in_queue Queue description
/// @param in_packet_index Packet index
uint8_t* comPacketQueueGetPacketBuffer(comPacketQueue* in_queue, uint16_t in_packet_index)
{
	return ((uint8_t*)&(in_queue->Buffer[in_packet_index])) + sizeof(comPacketInfo);
}

/*****************************************************************************/
/* Queue PUSH functions                                                      */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Allocated space for a packet in the queue. Handles multiple thread access.
/// @param in_queue Queue descriptor
/// @param in_size Size of the packet to allocate
/// @param in_source_interface Source Interface number (where the packet comes)
uint16_t comPacketQueuePushBegin(comPacketQueue* in_queue, uint8_t in_size, uint8_t in_source_interface)
{
	uint16_t total_size;
	comPacketInfo* packet_info = sysNULL;
	uint16_t packet_index = comINVALID_PACKET_INDEX;

	sysASSERT(in_queue != sysNULL);

	// reserve storage from the queue
	total_size = sizeof(comPacketInfo) + in_size;

	// reserve storage in a critical section
	sysMutexTake(in_queue->PushLock, sysINFINITE_TIMEOUT);

	// reserve space from the end of the buffer
	if (in_queue->PopIndex <= in_queue->PushIndex && in_queue->PushIndex + total_size < in_queue->BufferSize)
	{
		// there is enough space at the end of the buffer
		packet_index = in_queue->PushIndex;
		packet_info = (comPacketInfo*)&(in_queue->Buffer[packet_index]);
		in_queue->PushIndex += total_size;
		packet_info->Status = comPQS_Reserved;

		// wrap around push index
		if (in_queue->PushIndex >= in_queue->BufferSize)
		{
			// there is no space beind this packet -> wrap push pointer
			in_queue->PushIndex = 0;
		}
	}
	else
	{
		// handle push pointer wrap around condition -> terminate queue
		if (in_queue->PopIndex <= in_queue->PushIndex)
		{
			// terminate end of the queue
			((comPacketInfo*)&in_queue->Buffer[in_queue->PushIndex])->Status = comPQS_EndOfQueue;
			in_queue->PushIndex = 0;
		}

		// reserve space from the beginning of the buffer
		if (in_queue->PushIndex + total_size + 1 < in_queue->PopIndex)	// +1 to never have the same value of the push and pop index except when queue is empty
		{
			packet_index = in_queue->PushIndex;
			packet_info = (comPacketInfo*)&(in_queue->Buffer[packet_index]);
			in_queue->PushIndex += total_size;
			packet_info->Status = comPQS_Reserved;
		}
		else
		{
			// no free space in the buffer
			packet_index = comINVALID_PACKET_INDEX;
		}
	}

	// release lock
	sysMutexGive(in_queue->PushLock);

	// initialize packet info
	if (packet_info != sysNULL)
	{
		packet_info->Size = in_size;
		packet_info->Interface = in_source_interface;
		packet_info->Timestamp = sysGetSystemTick();
	}

	return packet_index;
}


///////////////////////////////////////////////////////////////////////////////
/// @brief Finishes push operation
/// @param in_queue Queue description
/// @param in_packet_index Packet index to finish
void comPacketQueuePushEnd(comPacketQueue* in_queue, uint16_t in_packet_index)
{
	comPacketInfo* packet_info = sysNULL;

	sysASSERT(in_queue != sysNULL);
	sysASSERT(in_packet_index < in_queue->BufferSize);

	// mark packet as 'ready'
	packet_info = (comPacketInfo*)&(in_queue->Buffer[in_packet_index]);
	sysASSERT(packet_info->Status == comPQS_Reserved);
	packet_info->Status = comPQS_Ready;

	// execute callback
	comPacketQueueCallbackExecute(in_queue, comQE_PacketPushed);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Cancels push operation
/// @param in_queue Queue description
/// @param in_packet_index Packet index to cancel
void comPacketQueuePushCancel(comPacketQueue* in_queue, uint16_t in_packet_index)
{
	comPacketInfo* packet_info = sysNULL;

	sysASSERT(in_queue != sysNULL);
	sysASSERT(in_packet_index < in_queue->BufferSize);

	// mark packet as 'deleted'
	packet_info = (comPacketInfo*)&in_queue->Buffer[in_packet_index];
	sysASSERT(packet_info->Status == comPQS_Reserved);
	packet_info->Status = comPQS_Deleted;

	// execute callback
	comPacketQueueCallbackExecute(in_queue, comQE_PacketPushed);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Stores one byte in the given packet at the given position
/// @param in_queue Queue description
/// @param in_packet_index Packet index which will receive the new byte
/// @parma in_byte_index Position of the byte within the packet
/// @param in_data Data byte to store
void comPacketQueueStoreByte(comPacketQueue* in_queue, uint16_t in_packet_index, uint8_t in_byte_index, uint8_t in_data)
{
	comPacketInfo* packet_info = sysNULL;

	sysASSERT(in_queue != sysNULL);
	sysASSERT(in_packet_index < in_queue->BufferSize);

	packet_info = (comPacketInfo*)&(in_queue->Buffer[in_packet_index]);
	sysASSERT(packet_info->Status == comPQS_Reserved);
	sysASSERT(in_byte_index < packet_info->Size);
	in_queue->Buffer[sizeof(comPacketInfo) + in_packet_index + in_byte_index] = in_data;
}

/*****************************************************************************/
/* Queue POP functions                                                       */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Starts popping the next pecket from the queue
/// @param in_queue Queue description
/// @return Buffer in of the poping packet or comINVALID_PACKET_INDEX when no packet is available
uint16_t comPacketQueuePopBegin(comPacketQueue* in_queue)
{
	uint16_t pop_index;
	uint16_t total_size;
	comPacketInfo* packet_info = sysNULL;
	bool next_packet;

	sysASSERT(in_queue != sysNULL);

	do
	{
		next_packet = false;

		// if there is no packet in the buffer
		if (in_queue->PopIndex == in_queue->PushIndex)
		{
			packet_info = sysNULL;
			pop_index = comINVALID_PACKET_INDEX;
		}
		else
		{
			// check packet header
			packet_info = (comPacketInfo*)&in_queue->Buffer[in_queue->PopIndex];
			pop_index = in_queue->PopIndex;

			// advance pop index to the next packet when no valid packet was found
			switch(packet_info->Status)
			{
				// end of queue was found
				case comPQS_EndOfQueue:
					// wrap around
					in_queue->PopIndex = 0;
					next_packet = true;
					break;

				// deleted packet was found
				case comPQS_Deleted:
					// increment pop index
					total_size = sizeof(comPacketInfo) + packet_info->Size;

					if (in_queue->PopIndex + total_size < in_queue->BufferSize)
					{
						in_queue->PopIndex += total_size;
					}
					else
					{
						in_queue->PopIndex = 0;
					}

					next_packet = true;
					break;

				default:
					next_packet = false;
					break;
			}
		}
	} while (packet_info != sysNULL && next_packet); // skip deleted packets and end of queue

	return pop_index;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief End popping a packet and releases packet storage
/// @param in_queue Queue description
void comPacketQueuePopEnd(comPacketQueue* in_queue)
{
	uint16_t total_size;
	comPacketInfo* packet_info = sysNULL;

	packet_info = (comPacketInfo*)&in_queue->Buffer[in_queue->PopIndex];

	// increment pop index
	total_size = sizeof(comPacketInfo) + packet_info->Size;

	if (in_queue->PopIndex + total_size < in_queue->BufferSize)
	{
		in_queue->PopIndex += total_size;
	}
	else
	{
		in_queue->PopIndex = 0;
	}

	// execute callback
	comPacketQueueCallbackExecute(in_queue, comQE_PacketPushed);
}

/*****************************************************************************/
/* Module local functions                                                    */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Executes callback function of the queue
/// @param in_queue Queue description
/// @param in_event Event code of the callback
static void comPacketQueueCallbackExecute(comPacketQueue* in_queue, comQueueEvent in_event)
{
	if (in_queue->Callback != sysNULL)
		in_queue->Callback(in_event);
}
