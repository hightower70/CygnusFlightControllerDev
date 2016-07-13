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
#include <drvUDP.h>
#include <strString.h>
#include <comPacketBuilder.h>
#include <cfgStorage.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define comUDP_TASK_PRIORITY 2
#define comUDP_DEVICE_ANNOUNCE_PERIOD 1000
#define comUDP_HOST_PACKET_TIMEOUT 5000

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
static void comUDPSendDeviceAnounce(uint8_t* in_transmit_buffer);
static void comUDPProcessHostAnnouncePacket(comPacketHostAnnounce* in_packet);

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

	drvUDPInit();
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
	if (packet_header->PacketType == comPT_DEVICE_ANNOUNCE)
		return;

	// check CRC
	crc = crc16_INIT_VALUE;
	crc = crc16CalculateForBlock(crc, in_packet, in_packet_size - comCRC_BYTE_COUNT);

	if (sysLOW(crc) != in_packet[in_packet_size - 2] || sysHIGH(crc) != in_packet[in_packet_size - 1])
		return;

	// store timestamp of the last received packet
	l_host_last_packet_received = sysGetSystemTick();

	// size and CRC is valid -> process packet
	if (packet_header->PacketType == comPT_HOST_ANNOUNCE)
	{
		// process host info packet
		comUDPProcessHostAnnouncePacket((comPacketHostAnnounce*)in_packet);
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

	sysASSERT(in_packet_length <= comMAX_PACKET_SIZE);
	sysASSERT(in_packet != sysNULL);

	// check if connected to the host
	if (!drvUDPIsConnected() ||	l_host_address == 0)
		return false;

	// prepare data content
	transmit_buffer = drvUDPAllocTransmitBuffer();

	if (transmit_buffer == sysNULL)
		return false;

	// store packet
	sysMemCopy(transmit_buffer, in_packet, in_packet_length);

	// send packet
	drvUDPTransmitData(in_packet_length, l_host_address);

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
		if (ellapsed_time > comUDP_DEVICE_ANNOUNCE_PERIOD && drvUDPIsConnected())
		{
			uint8_t* transmit_buffer = drvUDPAllocTransmitBuffer();

			if (transmit_buffer != sysNULL)
			{
				l_device_announce_time_stamp += ellapsed_time;

				// send device announce packet
				comUDPSendDeviceAnounce(transmit_buffer);
			}
		}
	}
	else
	{
		if (sysGetSystemTickSince(l_host_last_packet_received) > comUDP_HOST_PACKET_TIMEOUT)
		{
			// no paacket received from the host for a while
			uint8_t* transmit_buffer = drvUDPAllocTransmitBuffer();

			// reset host address
			l_host_address = 0;

			// restart device annoucing
			if (transmit_buffer != sysNULL)
			{
				l_device_announce_time_stamp = sysGetSystemTick();

				// send device announce packet
				comUDPSendDeviceAnounce(transmit_buffer);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sends device information announce packet
/// @param in_transmit_buffer Buffer for data storage
static void comUDPSendDeviceAnounce(uint8_t* in_transmit_buffer)
{
	uint16_t crc;
	comPacketDeviceAnnounce* info = (comPacketDeviceAnnounce*)in_transmit_buffer;
	uint32_t ip_address = drvUDPGetLocalIPAddress();

	// fill packet header
	comFillPacketHeader(&info->Header, comPT_DEVICE_ANNOUNCE, sizeof(comPacketDeviceAnnounce));

	// fill packet data members
	strCopyString(info->Name, comDEVICE_NAME_LENGTH, 0, cfgGetStringValue(cfgVAL_SYS_NAME));
	info->UniqueID = cfgGetUInt32Value(cfgVAL_SYS_UID);
	info->Address = ip_address;

	// calculate CRC
	crc = crc16CalculateForBlock(crc16_INIT_VALUE, in_transmit_buffer, sizeof(comPacketDeviceAnnounce));
	in_transmit_buffer[sizeof(comPacketDeviceAnnounce)] = sysLOW(crc);
	in_transmit_buffer[sizeof(comPacketDeviceAnnounce) + 1] = sysHIGH(crc);

	// send packet (Broadcast)
	drvUDPTransmitData(sizeof(comPacketDeviceAnnounce) + comCRC_BYTE_COUNT, comUDP_MAKE_BROADCAST_ADDRESS(ip_address));
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Process host announce packet
/// @param in_packet Pointer to the host announce packet
static void comUDPProcessHostAnnouncePacket(comPacketHostAnnounce* in_packet)
{
	// update address only when no host is detected
	if (l_host_address != 0)
		return;

	// update host information
	l_host_address = in_packet->Address;
}

