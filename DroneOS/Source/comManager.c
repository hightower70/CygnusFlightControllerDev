/*****************************************************************************/
/* Communication manager (packet receiving and sending via com interfaces)   */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysRTOS.h>
#include <comManager.h>
#include <comSLIP.h>
#include <comSystemPacketDefinitions.h>
#include <comPacketBuilder.h>
#include <fileTransfer.h>
#include <crcCITT16.h>
#include <sysDateTime.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define comManager_MAX_INTERFACE_NUMBER 4
#define comManager_RECEIVER_BUFFER_COUNT 2
#define comManager_TASK_PRIORITY 2
#define comManager_TASK_MAX_CYCLE_TIME 100
#define comManager_TRANSMIT_PACKET_EXPIRE_INTERVAL 100

#define comManager_RECEIVER_PACKET_LENGTH 4096
#define comManager_TRANSMITTER_PACKET_LENGTH 1024

#define comManager_HEARTBEAT_INTERVAL 1000


/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/
typedef struct
{
	// adminstration
	bool Valid;
	comInterfaceDescription Description;

} comInterfaceState;

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/

// communication interfaces
static comInterfaceState g_com_interfaces[comManager_MAX_INTERFACE_NUMBER] = { 0 };

// receiver variables
static uint8_t l_last_received_packet_counter;

// transmitter variables
static uint8_t l_transmitter_packet_counter;

// task variables
static bool l_stop_task = false;
static sysTaskNotify l_task_event;

// packet receiver buffers
static comPacketQueue l_receiver_queue;
static uint8_t l_receiver_packet_buffer[comManager_RECEIVER_PACKET_LENGTH];

// packet transmitter buffers
static comPacketQueue l_transmitter_queue;
static uint8_t l_transmitter_packet_buffer[comManager_TRANSMITTER_PACKET_LENGTH];

// heartbeat timestamp
static sysTick l_last_heartbeat_timestamp;

/*****************************************************************************/
/* Local function prototypes                                                 */
/*****************************************************************************/
static void comManagerTask(void* in_param);
static bool comManagerSendDeviceHeartbeat(void);
static void comManagerTransmitPacket(void);
static void comManagerProcessReceivedPackets(void);
static void comProcessCommunicationPacket(comPacketInfo* in_packet_info, uint8_t* in_packet);

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initialize communication manager
void comManagerInit(void)
{
	uint32_t task_id;
	
	// initialize communication tasks
	sysTaskNotifyCreate(l_task_event);
	sysTaskCreate( comManagerTask, "comManager", sysDEFAULT_STACK_SIZE, sysNULL, comManager_TASK_PRIORITY, &task_id, comManagerTaskStop);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief  Adds interface to the list of active interfaces
/// @param in_interface Pointer to the interface description struct
uint8_t comAddInterface(comInterfaceDescription* in_interface_description)
{
	uint8_t interface_index;

	// find empty slot
	interface_index = 0;
	while (g_com_interfaces[interface_index].Valid)
		interface_index++;

	if (interface_index < comManager_MAX_INTERFACE_NUMBER)
	{
		g_com_interfaces[interface_index].Description = *in_interface_description;

		g_com_interfaces[interface_index].Valid = true;

		return interface_index;
	}
	else
	{
		// TODO: Error handling
		return comINVALID_INTERFACE_INDEX;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Stops communication manager task
void comManagerTaskStop(void)
{
	l_stop_task = true;
	sysTaskNotifyGive(l_task_event);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Communication manager task function
static void comManagerTask(void* in_param)
{
	sysTick difference;

	// initialize
	comPacketQueueInitialize(&l_receiver_queue, l_receiver_packet_buffer, comManager_RECEIVER_PACKET_LENGTH);
	comPacketQueueInitialize(&l_transmitter_queue, l_transmitter_packet_buffer, comManager_TRANSMITTER_PACKET_LENGTH);

	l_last_received_packet_counter = 0;
	l_transmitter_packet_counter = 0;

	// task loop
	while(!l_stop_task)
	{
		// wait for event
		sysTaskNotifyTake(l_task_event, comManager_TASK_MAX_CYCLE_TIME);

		// stop task is requested
		if(l_stop_task)
			break;

		// handle heartbeat message
		difference = sysGetSystemTickSince(l_last_heartbeat_timestamp);
		if (difference > comManager_HEARTBEAT_INTERVAL)
		{
			if (comManagerSendDeviceHeartbeat())
			{
				l_last_heartbeat_timestamp += difference;
			}
		}

		// handle pending transmitter messages
		comManagerTransmitPacket();

		// process received packets
		comManagerProcessReceivedPackets();
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Increases current transmitted packet counter and returns its value
/// @return Transmitter packet counter value
uint8_t comIncrementAndGetTransmittedPacketCounter(void)
{
	uint8_t transmitter_packet_counter;

	sysCriticalSectionBegin();

	transmitter_packet_counter = ++l_transmitter_packet_counter;

	sysCriticalSectionEnd();

	return transmitter_packet_counter;
}


///////////////////////////////////////////////////////////////////////////////
/// @brief Stored received packet of any interface
/// @param in_interface_index Interface index wich is received the packet
/// @param in_packet Pointer to the received packet
/// @param in_packet_size Length of the received packet
void comManagerStoreReceivedPacket(uint8_t in_interface_index, uint8_t* in_packet, uint8_t in_packet_size)
{
	comPacketHeader* packet_header;
	uint16_t push_index;
	uint8_t* packet_pointer;

	// sanity check
	if (in_packet_size == 0 || in_interface_index >= comManager_MAX_INTERFACE_NUMBER)
		return;

	// check packet counter
	packet_header = (comPacketHeader*)in_packet;

	// drop packet if the packet with same counter is already received
	if (packet_header->PacketCounter == l_last_received_packet_counter)
		return;

	// store packet
	push_index = comPacketQueuePushBegin(&l_receiver_queue, in_packet_size, in_interface_index);
	if (push_index != comINVALID_PACKET_INDEX)
	{
		// get pointer to packet data
		packet_pointer = comPacketQueueGetPacketBuffer(&l_receiver_queue, push_index);

		// copy packet content
		sysMemCopy(packet_pointer, in_packet, in_packet_size);

		// fill out header
		comPacketQueuePushEnd(&l_receiver_queue, push_index);
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sends device heartbeat packet
/// @return True if packet was transferred to at least one communication interface for sending
static bool comManagerSendDeviceHeartbeat(void)
{
	comPacketDeviceHeartbeat* heartbeat_packet;
	uint16_t push_index;

	heartbeat_packet = (comPacketDeviceHeartbeat*)comManagerTransmitPacketPushStart(sizeof(comPacketDeviceHeartbeat), comINVALID_INTERFACE_INDEX, comPT_DEVICE_HEARTBEAT, &push_index);

	if (heartbeat_packet != sysNULL)
	{
		// fill out members
		heartbeat_packet->CPULoad = sysGetCPUUsage();

		comManagerTransmitPacketPushEnd(push_index);

		return true;
	}
	else
	{
		return false;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Reserves packet storage space in the transmit buffer and fills packet header
/// @param in_packet_size Size of the packet (only packet size without CRC)
/// @param in_interface_index Interface number (where packet will be transmitted)
/// @param in_packet_type Type code of the packet
/// @param out_packet_index Pointer to a variable which will receive packet push index
/// @return Pointer to the reserved packet storage space or sysNULL when no space available
uint8_t* comManagerTransmitPacketPushStart(uint8_t in_packet_size, uint8_t in_interface_index, uint8_t in_packet_type, uint16_t *out_packet_index)
{
	uint16_t push_index = comINVALID_PACKET_INDEX;
	uint8_t* packet_pointer = sysNULL;

	// reserve storage
	push_index = comPacketQueuePushBegin(&l_transmitter_queue, in_packet_size + comCRC_BYTE_COUNT, in_interface_index);

	if (push_index != comINVALID_PACKET_INDEX)
	{
		// get pointer to packet data
		packet_pointer = comPacketQueueGetPacketBuffer(&l_transmitter_queue, push_index);

		// fill out header
		comFillPacketHeader((comPacketHeader*)packet_pointer, in_packet_type, in_packet_size);
	}

	*out_packet_index = push_index;

	return packet_pointer;
}

////////////////////////////////////////////////////////////////////////////////////
/// @brief Gets packet buffer pointer for the selected transmit packet
/// @param in_packet_index Packet index to get buffer
/// @return Pointer to the packet buffer
uint8_t* comManagerGetTransmitPacketGetBuffer(uint16_t in_packet_index)
{
	return comPacketQueueGetPacketBuffer(&l_transmitter_queue, in_packet_index);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Finished packet push operation on transmitter queue
/// @param in_packet_index Packet index to finish
void comManagerTransmitPacketPushEnd(uint16_t in_packet_index)
{
	comPacketInfo* packet_info;
	uint16_t packet_size;
	uint8_t* packet_data_buffer;
	uint16_t crc;

	// sanity check
	if (in_packet_index == comINVALID_PACKET_INDEX)
		return;

	// get packet information
	packet_info = comPacketQueueGetPacketInfo(&l_transmitter_queue, in_packet_index);
	packet_data_buffer = comPacketQueueGetPacketBuffer(&l_transmitter_queue, in_packet_index);
	packet_size = packet_info->Size;

	// calculate and update CRC
	crc = crc16CalculateForBlock(crc16_INIT_VALUE, packet_data_buffer, packet_size - comCRC_BYTE_COUNT);
	packet_data_buffer[packet_size - comCRC_BYTE_COUNT] = sysLOW(crc);
	packet_data_buffer[packet_size - comCRC_BYTE_COUNT + 1] = sysHIGH(crc);

	// finish packet preparation
	comPacketQueuePushEnd(&l_transmitter_queue, in_packet_index);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Cancels push operation on transmitter queue
/// @param in_packet_index Packet index to cancel
void comManagerTransmitPacketPushCancel(uint16_t in_packet_index)
{
	comPacketQueuePushCancel(&l_transmitter_queue, in_packet_index);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sends packet
/// @param in_queue Queue description
/// @param in_packet_index Packet index to finish
static void comManagerTransmitPacket(void)
{
	uint16_t packet_index;
	comPacketInfo* packet_info;
	uint8_t* packet_data_buffer;
	uint8_t i;
	bool packet_sent = false;

	// pop next packet from the transmitter queue, return where there no packet in the queue
	packet_index = comPacketQueuePopBegin(&l_transmitter_queue);
	if (packet_index == comINVALID_PACKET_INDEX)
		return;

	// get further packet information
	packet_info = comPacketQueueGetPacketInfo(&l_transmitter_queue, packet_index);
	packet_data_buffer = comPacketQueueGetPacketBuffer(&l_transmitter_queue, packet_index);

	if (packet_info->Interface < comManager_MAX_INTERFACE_NUMBER)
	{
		// interface is specified -> send only to the specified interface
		if (g_com_interfaces[packet_info->Interface].Valid)
		{
			if (g_com_interfaces[packet_info->Interface].Description.PacketSendFunction(packet_data_buffer, packet_info->Size))
				packet_sent = true;
		}
	}
	else
	{
		// try to send on all interfaces
		for (i = 0; i < comManager_MAX_INTERFACE_NUMBER; i++)
		{
			if (g_com_interfaces[i].Valid)
			{
				if (g_com_interfaces[i].Description.PacketSendFunction(packet_data_buffer, packet_info->Size))
					packet_sent = true;
			}
		}
	}

	// remove packet from the queue if it was sent or expired
	if(packet_sent || sysGetSystemTickSince(packet_info->Timestamp) > comManager_TRANSMIT_PACKET_EXPIRE_INTERVAL)
		comPacketQueuePopEnd(&l_transmitter_queue);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Processes received (incoming) packets
static void comManagerProcessReceivedPackets(void)
{
	uint16_t packet_index;
	comPacketInfo* packet_info;
	comPacketHeader* packet_header;

	// process all pending packets
	do
	{
		// get next pending packet
		packet_index = comPacketQueuePopBegin(&l_receiver_queue);
		if (packet_index != comINVALID_PACKET_INDEX)
		{
			// process packet
			packet_info = comPacketQueueGetPacketInfo(&l_receiver_queue, packet_index);
			packet_header = (comPacketHeader*)comPacketQueueGetPacketBuffer(&l_receiver_queue, packet_index);

			l_last_received_packet_counter = packet_header->PacketCounter;

			switch (comPT_GET_CLASS(packet_header->PacketType))
			{
				case comPT_CLASS_FILE:
					fileProcessFileTransfer(packet_info, (uint8_t*)packet_header);
					break;

				case comPT_CLASS_COMM:
					comProcessCommunicationPacket(packet_info, (uint8_t*)packet_header);
					break;

				case comPT_CLASS_CONFIG:
					break;
			}

			// remove packet from the queue
			comPacketQueuePopEnd(&l_receiver_queue);
		}
	} while (packet_index != comINVALID_PACKET_INDEX);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Process communication class packets
/// @param in_packet_info Pointer to packet information struct
/// @param in_packet Packet to process
static void comProcessCommunicationPacket(comPacketInfo* in_packet_info, uint8_t* in_packet)
{
	comPacketHeader* header = (comPacketHeader*)in_packet;

	switch (header->PacketType)
	{
		case comPT_HOST_HEARTBEAT:
		{
			comPacketHostHeartbeat* packet = (comPacketHostHeartbeat*)in_packet;
			sysDateTime host_time;
			sysDateTime device_time;
			uint32_t host_time_in_seconds;
			uint32_t device_time_in_seconds;
			int32_t diff_time;

			// get host time in second
			host_time.Year = packet->Year;
			host_time.Month = packet->Month;
			host_time.Day = packet->Day;

			host_time.Hour = packet->Hour;
			host_time.Minute = packet->Minute;
			host_time.Second = packet->Second;

			host_time_in_seconds = sysDateTimeConvertToSeconds(&host_time);

			// get device time in second
			sysDateTimeGet(&device_time);

			device_time_in_seconds = sysDateTimeConvertToSeconds(&device_time);

			// calculate difference time
			diff_time = (int32_t)(host_time_in_seconds - device_time_in_seconds);

			// if difference if igher than one second -> update RTC
			if (diff_time > 1 || diff_time < -1)
				sysDateTimeSet(&host_time);
		}
		break;
	}
}
