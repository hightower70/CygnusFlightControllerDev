/*****************************************************************************/
/* UDP Communication routines                                                */
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
#include <comUDP.h>
#include <sysRTOS.h>
#include <comInterfaces.h>
#include <comManager.h>
#include <comSystemPacketDefinitions.h>
#include <crcCITT16.h>
#include <cfgValues.h>
#include <halUDP.h>
#include <strString.h>
#include <comPacketBuilder.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define comUDP_TASK_PRIORITY 2
#define comUDP_DEVICE_ANNOUNCE_PERIOD 1000
#define comUDP_HOST_PACKET_TIMEOUT 2000

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static uint8_t l_interface_index;
static sysTick l_device_announce_time_stamp;
static sysTick l_host_last_packet_received;
static uint32_t l_host_address;
static uint32_t l_device_address;

/*****************************************************************************/
/* Local functions prototypes                                                */
/*****************************************************************************/
static void comUDPSendDeviceInformation(uint8_t* in_transmit_buffer);
static void comUDPProcessHostInfoPacket(comPacketHostInformation* in_packet);

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initialize communication manager
void comUDPInit(void)
{
	comInterfaceDescription interface_description;

	interface_description.PacketSendFunction = comUDPSendPacket;

	// add this interface to the list of communication interfaces
	l_interface_index = comAddInterface(&interface_description);
	l_device_announce_time_stamp = sysGetSystemTick();
	l_host_address = 0;

	halUDPInit();
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Processes received UDP packets
/// @param in_paacket Poitner to packet buffer
/// @param in_packet_size Length of the packet in bytes
void comUDPProcessReceivedPacket(uint8_t* in_packet, uint8_t in_packet_size)
{
	comPacketHeader* packet_header = (comPacketHeader*)in_packet;
	uint16_t crc;

	// check size
	if (packet_header->PacketLength != in_packet_size)
		return;

	// drop own broadcast message
	if (packet_header->PacketType == comPT_DEVICE_INFO)
		return;

	// check CRC
	crc = crc16_INIT_VALUE;
	crc = crc16CalculateForBlock(crc, in_packet, in_packet_size - comCRC_BYTE_COUNT);

	if (sysLOW(crc) != in_packet[in_packet_size - 2] || sysHIGH(crc) != in_packet[in_packet_size - 1])
		return;

	// store timestamp of the last received packet
	l_host_last_packet_received = sysGetSystemTick();

	// size and CRC is valid -> process packet
	if (packet_header->PacketType == comPT_HOST_INFO)
	{
		// process host info packet
		comUDPProcessHostInfoPacket((comPacketHostInformation*)in_packet);
	}
	else
	{
		// further process other packets
		comManagerStoreReceivedPacket(l_interface_index, in_packet, in_packet_size);
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sends packet over UDP communication line
/// @param in_packet Pointer to the packet
/// @param in_packet_length Length of the packet in bytes (total length, including CRC and header)
/// @return True if packet sending was started, false 
bool comUDPSendPacket(uint8_t* in_packet, uint16_t in_packet_length)
{
	uint8_t* transmit_buffer;

	sysASSERT(in_packet_length < comMAX_PACKET_LENGTH);
	sysASSERT(in_packet != sysNULL);

	// check if connected to the host
	if (!halUDPIsConnected() ||	l_host_address == 0)
		return false;

	// prepare data content
	transmit_buffer = halUDPAllocTransmitBuffer();

	if (transmit_buffer == 0)
		return false;

	// store packet
	sysMemCopy(transmit_buffer, in_packet, in_packet_length);

	// send packet
	halUDPTransmitData(in_packet_length, l_host_address);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Handles periodic tasks
void comUDPPeriodicCallback(void)
{
	sysTick ellapsed_time;

	if (l_host_address == 0)
	{
		// handle device announce period
		ellapsed_time = sysGetSystemTickSince(l_device_announce_time_stamp);
		if (ellapsed_time > comUDP_DEVICE_ANNOUNCE_PERIOD)
		{
			uint8_t* transmit_buffer = halUDPAllocTransmitBuffer();

			if (transmit_buffer != sysNULL)
			{
				l_device_announce_time_stamp += ellapsed_time;

				// send device announce packet
				comUDPSendDeviceInformation(transmit_buffer);
			}
		}
	}
	else
	{
		// Handle host heartbeat timeout
		if (sysGetSystemTickSince(l_host_last_packet_received) > comUDP_HOST_PACKET_TIMEOUT)
		{
			uint8_t* transmit_buffer = halUDPAllocTransmitBuffer();

			// reset host address
			l_host_address = 0;

			// send announce message
			if (transmit_buffer != sysNULL)
			{
				l_device_announce_time_stamp = sysGetSystemTick();

				comUDPSendDeviceInformation(transmit_buffer);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sends device information announce packet
/// @param in_transmit_buffer Buffer for data storage
static void comUDPSendDeviceInformation(uint8_t* in_transmit_buffer)
{
	uint16_t crc;
	comPacketDeviceInformation* info = (comPacketDeviceInformation*)in_transmit_buffer;
	uint32_t ip_address = halUDPGetLocalIPAddress();

	// fill packet header
	comFillPacketHeader(&info->Header, comPT_DEVICE_INFO, sizeof(comPacketDeviceInformation));

	// fill packet data members
	strCopyString(info->Name, comDEVICE_NAME_LENGTH, 0, cfgGetStringValue(cfgVAL_DEVICE_NAME));
	info->UniqueID = cfgGetUInt32Value(cfgVAL_DEVICE_UNIQUE_ID);
	info->Address = ip_address;

	// calculate CRC
	crc = crc16CalculateForBlock(crc16_INIT_VALUE, in_transmit_buffer, sizeof(comPacketDeviceInformation));
	in_transmit_buffer[sizeof(comPacketDeviceInformation)] = sysLOW(crc);
	in_transmit_buffer[sizeof(comPacketDeviceInformation) + 1] = sysHIGH(crc);

	// send packet (Broadcast)
	halUDPTransmitData(sizeof(comPacketDeviceInformation) + comCRC_BYTE_COUNT, comUDP_MAKE_BROADCAST_ADDRESS(ip_address));
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Process host information packet
/// @param in_packet Pointer to the host information packet
static void comUDPProcessHostInfoPacket(comPacketHostInformation* in_packet)
{
	// update address only when no host is detected
	if (l_host_address != 0)
		return;

	// update host information
	l_host_address = in_packet->Address;
}

