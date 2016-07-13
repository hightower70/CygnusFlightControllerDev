/*****************************************************************************/
/* UART communication driver                                                 */
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
#include <halUART.h>
#include <cfgStorage.h>
#include <crcCITT16.h>
#include <comPacketBuilder.h>
#include <comInterfaces.h>
#include <comManager.h>
#include <comUART.h>
#include <comSLIP.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define comUART_TRANSMITTER_BUFFER_LENGTH 512
#define comUART_RECEIVER_BUFFER_LENGTH 256

#define comUART_TRANSMITTER_BUFFER_RESEVED 0xffff

#define comUART_TASK_PRIORITY 2

#define comUART_TASK_MAX_CYCLE_TIME 100

/*****************************************************************************/
/* Module local functions                                                    */
/*****************************************************************************/
static void comUARTThread(void* in_param);

static void comUARTRxCallback(uint8_t in_char, void* in_interrupt_param);
static void comUARTTxEmptyCallback(void* in_interrupt_param);
static uint8_t* comUARTAllocTransmitBuffer(void);


/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/

// task variables
static sysTaskNotify l_task_event = sysNULL; 
static bool l_stop_task = false;
static uint8_t l_interface_index;
static bool l_tx_buffer_empty_event;
// uart variables
static uint8_t l_uart_index = 1;

// receiver variables
static uint8_t l_receive_buffer[comUART_RECEIVER_BUFFER_LENGTH];
static uint16_t l_received_packet_length;
static volatile bool l_packet_received;
static slipDecoderState l_slip_decoder_state;

// transmitter variables
static uint8_t l_transmitter_buffer[comUART_TRANSMITTER_BUFFER_LENGTH];
static uint16_t l_transmitter_buffer_length;

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

#pragma region Public functions
// <editor-fold defaultstate="collapsed" desc="Public functions">

///////////////////////////////////////////////////////////////////////////////
/// @brief 
void comUARTInit(void)
{
	sysTask task_id;

	comInterfaceDescription interface_description;

	interface_description.PacketSendFunction = comUARTSendPacket;

	// add this interface to the list of communication interfaces
	l_interface_index = comAddInterface(&interface_description);

	sysTaskCreate(comUARTThread, "comUART", sysDEFAULT_STACK_SIZE, sysNULL, comUART_TASK_PRIORITY, &task_id, comUARTDeinit);
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Stops communication manager task
void comUARTDeinit(void)
{
	l_stop_task = true;
	sysTaskNotifyGive(l_task_event);
}

// </editor-fold>
#pragma endregion

#pragma region Transmitter functions
// <editor-fold defaultstate="collapsed" desc="Trasmitter functions">

///////////////////////////////////////////////////////////////////////////////
/// @brief Allocates and reserves transmitter buffer
/// @return Transmitter data buffer address or sysNULL if transmitter is not available
static uint8_t* comUARTAllocTransmitBuffer(void)
{
	uint8_t* buffer;

	sysCriticalSectionBegin();

	if (l_transmitter_buffer_length == 0)
	{
		l_transmitter_buffer_length = comUART_TRANSMITTER_BUFFER_RESEVED;
		buffer = l_transmitter_buffer;
	}
	else
	{
		buffer = sysNULL;
	}

	sysCriticalSectionEnd();

	return buffer;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sends packet over UART communication line
/// @param in_packet Pointer to the packet
/// @param in_packet_length Length of the packet in bytes (total length, including CRC and header)
/// @return True if packet sending was started, false 
bool comUARTSendPacket(uint8_t* in_packet, uint16_t in_packet_length)
{
	slipEncoderState slip_encoder_state;
	uint8_t* transmit_buffer;

	sysASSERT(in_packet_length <= comMAX_PACKET_SIZE);
	sysASSERT(in_packet != sysNULL);

	// prepare data content
	transmit_buffer = comUARTAllocTransmitBuffer();

	if (transmit_buffer == sysNULL)
		return false;

	slip_encoder_state.TargetBuffer = transmit_buffer;
	slip_encoder_state.TargetBufferSize = comUART_TRANSMITTER_BUFFER_LENGTH;
	slip_encoder_state.TargetBufferPos = 0;

	if (slipEncodeBlock(&slip_encoder_state, in_packet, (uint8_t)in_packet_length))
	{
		halUARTSendBlock(l_uart_index, transmit_buffer, slip_encoder_state.TargetBufferPos);
		return true;
	}
	else
	{
		return false;
	}
}

// </editor-fold>
#pragma endregion

/*****************************************************************************/
/* Local functions                                                           */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Main communication thread
static void comUARTThread(void* in_param)
{
	halUARTConfigInfo uart_config;
	uint8_t packet_length;
	comPacketHeader* packet_header;
	uint16_t crc;

	sysUNUSED(in_param);

	sysTaskNotifyCreate(l_task_event);

	// UART init
	l_uart_index = 1;

	halUARTConfigInfoInit(&uart_config);
	uart_config.RxReceivedCallback = comUARTRxCallback;
	uart_config.TxEmptyCallback = comUARTTxEmptyCallback;
	halUARTConfig(l_uart_index, &uart_config);
	halUARTSetBaudRate(l_uart_index, 115200);

	// SLIP init
	slipDecodeInitialize(&l_slip_decoder_state);
	l_slip_decoder_state.TargetBuffer = l_receive_buffer;
	l_slip_decoder_state.TargetBufferSize = comUART_RECEIVER_BUFFER_LENGTH;

	// receiver init
	l_packet_received = false;

	// task loop
	while (!l_stop_task)
	{
		// wait for event
		sysTaskNotifyTake(l_task_event, comUART_TASK_MAX_CYCLE_TIME);

		// stop task is requested
		if (l_stop_task)
			break;

		// process received packets
		if (l_packet_received)
		{
			packet_header = (comPacketHeader*)l_receive_buffer;
			packet_length = packet_header->PacketLength;

			// check packet length
			if (packet_length == l_received_packet_length)
			{
				// check CRC
				crc = crc16_INIT_VALUE;
				crc = crc16CalculateForBlock(crc, l_receive_buffer, packet_length - comCRC_BYTE_COUNT);

				if (sysLOW(crc) == l_receive_buffer[packet_length - 2] && sysHIGH(crc) == l_receive_buffer[packet_length - 1])
				{
					// further process other packets
					comManagerStoreReceivedPacket(l_interface_index, l_receive_buffer, packet_length);
				}
			}

			l_packet_received = false;
		}

		// process TX buffer empty event
		if (l_tx_buffer_empty_event)
		{
			l_tx_buffer_empty_event = false;

			// notify com manager about the empty buffer
			comManagerGenerateEvent();
		}
	}

	sysTaskNotifyDelete(l_task_event);
	l_task_event = sysNULL;
}

/*****************************************************************************/
/* UART Callback functions                                                   */
/*****************************************************************************/
#pragma region Callback functions
// <editor-fold defaultstate="collapsed" desc="Callback functions">

///////////////////////////////////////////////////////////////////////////////
/// @brief UART Character Received Callback
void comUARTRxCallback(uint8_t in_char, void* in_interrupt_param)
{
	sysUNUSED(in_interrupt_param);

	if (slipDecodeByte(&l_slip_decoder_state, in_char))
	{
		// drop packet if previous is not processed yet
		if (!l_packet_received)
		{
			l_received_packet_length = l_slip_decoder_state.LastPacketLength;
			l_packet_received = true;

			// notify thread about the received block
			sysTaskNotifyGiveFromISR(l_task_event, in_interrupt_param);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief ESP8266 UART Transmitter Empty Callback
void comUARTTxEmptyCallback(void* in_interrupt_param)
{
	l_transmitter_buffer_length = 0;

	// notify thread about the received block
	sysTaskNotifyGiveFromISR(l_task_event, in_interrupt_param);
}

// </editor-fold>
#pragma endregion
