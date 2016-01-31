/*****************************************************************************/
/* Task for scheduling telemetry data transmission                           */
/*                                                                           */
/* Copyright (C) 2014 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <RTOS.h>
#include <CRC16.h>
#include <SLIP.h>
#include <PacketDefinitions.h>
#include <TelemetryPacketInfo.h>
#include <TelemetryPacketBuilder.h>
#include <Telemetry.h>

///////////////////////////////////////////////////////////////////////////////
// Constants
#define PACKET_BUFFER_LENGTH 64

///////////////////////////////////////////////////////////////////////////////
// Types


///////////////////////////////////////////////////////////////////////////////
// Function prototypes
static void BuildTelemetryPacket(uint8_t in_packet_index);

///////////////////////////////////////////////////////////////////////////////
// Module local variables
static uint8_t l_packet_buffer[PACKET_BUFFER_LENGTH];
static slipEncoderState l_encoder_state;
static uint8_t l_packet_counter = 0;

extern PacketInfoEntryType g_telemetry_packet_info[];

///////////////////////////////////////////////////////////////////////////////
// Task for sending telemetry data
void telemetryTask(void* pvParameters)
{
	uint32_t delay_time;
	uint32_t packet_delay_time;
	uint32_t ellapsed_time;
	uint8_t packet_index;

	while (rtosTaskLoop())
	{
		// Find the next packet to send
		packet_index = 0;
		delay_time  = UINT32_MAX;
		while(g_telemetry_packet_info[packet_index].TransmitPeriod != 0)
		{
			ellapsed_time = rtosGetSystemTickSince(g_telemetry_packet_info[packet_index].LastTransmissionTimestamp);
			if(ellapsed_time >= g_telemetry_packet_info[packet_index].TransmitPeriod)
			{
				if(telemetryIsConnected())
				{
					// wait if needed
					telemetryWaitForSenderReady();

					// Create packet
					BuildTelemetryPacket(packet_index);

					// send packet
					telemetrySendPacket(l_encoder_state.TargetBuffer, l_encoder_state.TargetBufferPos);
				}

				// update timestamp
				g_telemetry_packet_info[packet_index].LastTransmissionTimestamp = rtosGetSystemTick();
				packet_delay_time = g_telemetry_packet_info[packet_index].TransmitPeriod;
			}
			else
			{
				packet_delay_time = g_telemetry_packet_info[packet_index].TransmitPeriod - ellapsed_time;
			}

			// determine delay
			if(packet_delay_time < delay_time)
			{
				delay_time = packet_delay_time;
			}

			packet_index++;
		}

		if(delay_time < UINT32_MAX)
			rtosDelay(delay_time);
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Builds data packet and and header/railer, calculates CRC and SLIP encodes it
/// @param Index of the packet to build (in g_telemetry_packet_info array)
static void BuildTelemetryPacket(uint8_t in_packet_index)
{
	uint8_t packet_buffer[PACKET_BUFFER_LENGTH];
	PacketHeaderType packet_header;

	// init
	l_encoder_state.CyclicRedundancyCheck = crc16_INIT_VALUE;
	l_encoder_state.TargetBuffer = l_packet_buffer;
	l_encoder_state.TargetBufferPos = 1;

	// packet header
	l_packet_buffer[0] = slip_END;

	packet_header.PacketLength	= g_telemetry_packet_info[in_packet_index].PacketLength + sizeof(packet_header)+ 2; // packet length + header length + CRC length
	packet_header.PacketType		= g_telemetry_packet_info[in_packet_index].PacketType;
	packet_header.PacketCounter = l_packet_counter++;
	slipEncodeBlock(&l_encoder_state, (uint8_t*)&packet_header, sizeof(packet_header));

	// create and store packet
	g_telemetry_packet_info[in_packet_index].PacketBuilder(packet_buffer);
	slipEncodeBlock(&l_encoder_state, packet_buffer, g_telemetry_packet_info[in_packet_index].PacketLength);

	// store CRC
	slipEncodeByte(&l_encoder_state, LOW(l_encoder_state.CyclicRedundancyCheck));
	slipEncodeByte(&l_encoder_state, HIGH(l_encoder_state.CyclicRedundancyCheck));

	l_packet_buffer[l_encoder_state.TargetBufferPos++] = slip_END;

	// update time
	g_telemetry_packet_info[in_packet_index].LastTransmissionTimestamp += g_telemetry_packet_info[in_packet_index].TransmitPeriod;

	TelemetryUpdateByteCounter(l_encoder_state.TargetBufferPos);
}

