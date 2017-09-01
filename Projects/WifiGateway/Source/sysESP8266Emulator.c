/*****************************************************************************/
/* ESP8266 WiFi chip emulator driver                                         */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysRTOS.h>
#include <halUART.h>
#include <drvUDP.h>
#include <sysString.h>
#include <comUDP.h>
#include <drvUDP.h>
#include "cfgConstants.h"

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define drvESP8266_UART_RX_BUFFER_LENGTH 1500
#define drvESP8266_UART_TX_BUFFER_LENGTH 1500
#define drvESP8266_STRING_BUFFER_LENGTH 64
#define drvESP8266_MAX_SOCKET_NUMBER 5

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/

typedef struct
{
	uint32_t RemoteIPAddress;
	uint16_t RemotePort;
} drvESP8266SocketInfo;

/// UART Input mode
typedef enum
{
	drvESP8266Emulator_UIM_Command,
	drvESP8266Emulator_UIM_Data
} drvESP8266EmulatorUARTImputMode;

typedef enum
{
	drvESP8266Emulator_CR_Unknown,
	drvESP8266Emulator_CR_OK,
	drvESP8266Emulator_CR_ready

} drvESP8266EmulatorCommandResponse;

typedef void(*drvESP8266CommandCallbackFunction)(sysStringLength in_pasing_position);

/// Entry for the command sequence table
typedef struct
{
	sysConstString Command;
	drvESP8266EmulatorCommandResponse ExpectedResponse;
	drvESP8266CommandCallbackFunction Callback;
} drvESP8266EmulatorCommandTableEntry;

/*****************************************************************************/
/* Command parser function prototypes                                        */
/*****************************************************************************/
static void drvESP8266CWJAPParser(sysStringLength in_parsing_position);
static void drvESP8266CIFSRParser(sysStringLength in_parsing_position);
static void drvESP8266CIPMUXParser(sysStringLength in_parsing_position);
static void drvESP8266CIPSTARTParser(sysStringLength in_parsing_position);

/*****************************************************************************/
/* Command processing table                                                  */
/*****************************************************************************/
static const drvESP8266EmulatorCommandTableEntry l_command_processing_table[] =
{
	{ "AT", drvESP8266Emulator_CR_OK, sysNULL },
	{ "ATE0", drvESP8266Emulator_CR_OK, sysNULL },
	{ "AT+CWQAP", drvESP8266Emulator_CR_OK, sysNULL },
	{ "AT+CIPMUX=", drvESP8266Emulator_CR_Unknown, drvESP8266CIPMUXParser },
	{ "AT+CWMODE_CUR=1", drvESP8266Emulator_CR_OK, sysNULL },
	{ "AT+CWAUTOCONN=1", drvESP8266Emulator_CR_OK, sysNULL },
	{ "AT+RST", drvESP8266Emulator_CR_ready, sysNULL },
	{ "AT+CWJAP=", drvESP8266Emulator_CR_Unknown, drvESP8266CWJAPParser },
	{ "AT+CIFSR", drvESP8266Emulator_CR_Unknown, drvESP8266CIFSRParser },
	{ "AT+CIPCLOSE=5", drvESP8266Emulator_CR_OK, sysNULL },
	{ "AT+CIPSTART", drvESP8266Emulator_CR_Unknown, drvESP8266CIPSTARTParser },

	{ sysNULL, drvESP8266Emulator_CR_Unknown }
};


/*****************************************************************************/
/* Module local functions                                                    */
/*****************************************************************************/
static sysTaskRetval drvESP8266EmulatorThread(void* in_param);
static void drvESP8266EmulatorDeinit(void);
void drvESP8266EmulatorUARTRxCallback(uint8_t in_char, void* in_interrupt_param);
void drvESP8266EmulatorUARTTxEmptyCallback(void* in_interrupt_param);
static void drvESP8266EmulatorCommandProcessor(void);


/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static uint8_t l_communication_state_step;
static sysTaskNotify l_task_event = sysNULL;
static uint32_t l_timeout_value;
static sysTick l_timeout_timestamp;
static bool l_stop_task = false;
static sysTick l_periodic_timestamp;
static uint8_t l_mux_mode = 0;

// UART RX
static drvESP8266EmulatorUARTImputMode l_uart_rx_mode;
static sysChar l_uart_rx_buffer[drvESP8266_UART_RX_BUFFER_LENGTH];
static uint16_t l_uart_rx_pos;
static volatile bool l_uart_rx_is_ready;
static uint16_t l_uart_rx_expected_data_length;

// UART TX
static sysChar l_uart_tx_buffer[drvESP8266_UART_TX_BUFFER_LENGTH];
static uint16_t l_uart_tx_length;
static volatile bool l_uart_tx_ready;

// uart variable
static uint8_t l_uart_index = 0;

///////////////////////////////////////////////////////////////////////////////
/// @brief Initialize ESP8266 Emulator library
void drvESP8266EmulatorInit(void)
{
	sysTask task_id;

	sysTaskCreate(drvESP8266EmulatorThread, "drvESP8266", sysDEFAULT_STACK_SIZE, sysNULL, 2, &task_id, drvESP8266EmulatorDeinit);
}

/*****************************************************************************/
/* Thread function                                                           */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Main communication thread
static sysTaskRetval drvESP8266EmulatorThread(void* in_param)
{
	halUARTConfigInfo uart_config;
	sysTick event_timeout;
	sysTick ellapsed_time;

	sysUNUSED(in_param);

	// UART init
	halUARTConfigInfoInit(&uart_config);
	uart_config.RxReceivedCallback = drvESP8266EmulatorUARTRxCallback;
	uart_config.TxEmptyCallback = drvESP8266EmulatorUARTTxEmptyCallback;
	halUARTConfig(l_uart_index, &uart_config);
	halUARTSetBaudRate(l_uart_index, 115200);

	// UART input mode init
	l_uart_rx_pos = 0;
	l_uart_rx_mode = drvESP8266Emulator_UIM_Command;
	l_uart_rx_is_ready = false;

	l_uart_tx_ready = true;

	while (!l_stop_task)
	{
		// wait for event
		sysTaskNotifyTake(l_task_event, sysINFINITE_TIMEOUT);

		// handle UART->Socket data flow
		if (l_uart_rx_is_ready && l_uart_tx_ready)
		{
			// Process commands
			drvESP8266EmulatorCommandProcessor();
		}

		// handle Socket->UART data flow


		if (l_stop_task)
			break;
	}

	// clean up
	if (l_task_event != sysNULL)
		sysTaskNotifyDelete(l_task_event);

	l_task_event = sysNULL;

	return 0;
}
 

static void drvESP8266EmulatorCommandProcessor(void)
{
	uint8_t command_found_index = 0xff;
	uint8_t command_index;
	uint8_t pos;

	// skip  empty lines
	if (l_uart_rx_pos == 0)
		return;

	printf(l_uart_rx_buffer);
	printf("\n");

	// find command
	command_index = 0;
	while (l_command_processing_table[command_index].Command != sysNULL && command_found_index == 0xff)
	{
		pos = 0;
		while (l_command_processing_table[command_index].Command[pos] == l_uart_rx_buffer[pos] && command_found_index == 0xff)
		{
			// if completely matches with the command
			if (l_command_processing_table[command_index].Command[pos] == '\0')
			{
				command_found_index = command_index;
				break;
			}

			pos++;

			// partial matches if callback defined
			if (l_command_processing_table[command_index].Command[pos] == '\0' && l_command_processing_table[command_index].Callback != sysNULL)
			{
				command_found_index = command_index;
				break;
			}
		}

		command_index++;
	}

	// if command found
	if (command_found_index != 0xff)
	{
		// call callback (if defined)
		if (l_command_processing_table[command_found_index].Callback != sysNULL)
		{
			l_command_processing_table[command_found_index].Callback(pos);
		}

		// prepare response
		switch (l_command_processing_table[command_found_index].ExpectedResponse)
		{
			case drvESP8266Emulator_CR_OK:
				l_uart_tx_length = sysCopyConstString(l_uart_tx_buffer, drvESP8266_UART_TX_BUFFER_LENGTH, 0, (sysConstString)"OK\n\r");
				break;

			case drvESP8266Emulator_CR_ready:
				l_uart_tx_length = sysCopyConstString(l_uart_tx_buffer, drvESP8266_UART_TX_BUFFER_LENGTH, 0, (sysConstString)"ESP8266 Emulator\n\rready\n\r");
				break;

			default:
				break;
		}
	}
	else
	{
		l_uart_tx_length = sysCopyConstString(l_uart_tx_buffer, drvESP8266_UART_TX_BUFFER_LENGTH, 0, (sysConstString)"ERROR\n\r");
	}

	// restart receiver
	l_uart_rx_is_ready = false;
	l_uart_rx_pos = 0;
	l_uart_rx_mode = drvESP8266Emulator_UIM_Command;

	// start uart transmission
	if (l_uart_tx_length > 0)
	{
		l_uart_tx_ready = false;
		halUARTSendBlock(l_uart_index, l_uart_tx_buffer, l_uart_tx_length);
		printf(l_uart_tx_buffer);
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Parser AT+CWJAP command
static void drvESP8266CWJAPParser(sysStringLength in_parsing_position)
{
	bool success = true;
	sysChar ssid[drvESP8266_STRING_BUFFER_LENGTH];
	sysChar pwd[drvESP8266_STRING_BUFFER_LENGTH];
	sysStringLength pos;

	// get SSID
	sysCheckForConstString(l_uart_rx_buffer, drvESP8266_UART_RX_BUFFER_LENGTH, &in_parsing_position, &success, "\"");

	pos = 0;
	while (success && l_uart_rx_buffer[in_parsing_position] != '\"')
	{
		if (l_uart_rx_buffer[in_parsing_position] == '\0')
			success = false;

		if (pos < drvESP8266_STRING_BUFFER_LENGTH - 1)
			ssid[pos++] = l_uart_rx_buffer[in_parsing_position++];
		else
			success = false;
	}
	ssid[pos] = '\0';

	sysCheckForConstString(l_uart_rx_buffer, drvESP8266_UART_RX_BUFFER_LENGTH, &in_parsing_position, &success, "\",\"");

	// get password
	pos = 0;
	while (success && l_uart_rx_buffer[in_parsing_position] != '\"')
	{
		if (l_uart_rx_buffer[in_parsing_position] == '\0')
			success = false;

		if (pos < drvESP8266_STRING_BUFFER_LENGTH - 1)
			pwd[pos++] = l_uart_rx_buffer[in_parsing_position++];
		else
			success = false;
	}
	pwd[pos] = '\0';

	sysCheckForConstString(l_uart_rx_buffer, drvESP8266_UART_RX_BUFFER_LENGTH, &in_parsing_position, &success, "\"");

	if (success)
	{
		l_uart_tx_length = sysCopyConstString(l_uart_tx_buffer, drvESP8266_UART_TX_BUFFER_LENGTH, 0, "OK\r\nWIFI CONNECTED\r\nWIFI GOT IP\r\n");
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Parser AT+CIFSR command
static void drvESP8266CIFSRParser(sysStringLength in_parsing_position)
{
	uint32_t ip_address = drvUDPGetLocalIPAddress();
	sysStringLength pos = 0;

	pos = sysCopyConstString(l_uart_tx_buffer, drvESP8266_UART_TX_BUFFER_LENGTH, pos, "+CIFSR:STAIP,\"");
	pos = sysWordToStringPos(l_uart_tx_buffer, drvESP8266_UART_TX_BUFFER_LENGTH, pos, (ip_address >> 24) & 0xff, 0, 0, 0);
	l_uart_tx_buffer[pos++] = '.';
	pos = sysWordToStringPos(l_uart_tx_buffer, drvESP8266_UART_TX_BUFFER_LENGTH, pos, (ip_address >> 16) & 0xff, 0, 0, 0);
	l_uart_tx_buffer[pos++] = '.';
	pos = sysWordToStringPos(l_uart_tx_buffer, drvESP8266_UART_TX_BUFFER_LENGTH, pos, (ip_address >> 8) & 0xff, 0, 0, 0);
	l_uart_tx_buffer[pos++] = '.';
	pos = sysWordToStringPos(l_uart_tx_buffer, drvESP8266_UART_TX_BUFFER_LENGTH, pos, ip_address & 0xff, 0, 0, 0);

	pos = sysCopyConstString(l_uart_tx_buffer, drvESP8266_UART_TX_BUFFER_LENGTH, pos, "\"\n\rOK\r\n");

	l_uart_tx_buffer[pos] = '\0';

	l_uart_tx_length = pos;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief ESP8266 UART Character Received Callback
void drvESP8266EmulatorUARTRxCallback(uint8_t in_char, void* in_interrupt_param)
{
	// drop characters if previous command is not processed
	if (l_uart_rx_is_ready)
		return;

	// store characters if there is space 
	switch (l_uart_rx_mode)
	{
		// process incoming characters in command mode
		case drvESP8266Emulator_UIM_Command:
			switch (in_char)
			{
					// drop LF
				case sysASCII_LF:
					break;

					// command received
				case sysASCII_CR:
					l_uart_rx_buffer[l_uart_rx_pos] = '\0'; // terminate input buffer
					l_uart_rx_is_ready = true; // notify task about the incoming command
					sysTaskNotifyGive(l_task_event);
					break;

					// store any other character if there is space in the buffer
				default:
					if (l_uart_rx_pos < drvESP8266_UART_RX_BUFFER_LENGTH - 1)
					{
						// skip leading spaces
						if (l_uart_rx_pos != 0 || in_char != ' ')
							l_uart_rx_buffer[l_uart_rx_pos++] = in_char;
					}
					break;
			}
			break;

			case drvESP8266Emulator_UIM_Data:
				break;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief ESP8266 UART Transmitter Empty Callback
void drvESP8266EmulatorUARTTxEmptyCallback(void* in_interrupt_param)
{
	l_uart_tx_ready = true;
	sysTaskNotifyGive(l_task_event);
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Stops communication manager task
static void drvESP8266EmulatorDeinit(void)
{
	l_stop_task = true;
	sysTaskNotifyGive(l_task_event);
}

uint16_t cfgGetUInt16Value(uint16_t in_value_index)
{
	switch (in_value_index)
	{
		case cfgVAL_WIFI_LOCAL:
			return 9601;

		case cfgVAL_WIFI_REMOTE:
			return 9602;
	}

	return 0;
}

void comManagerGenerateEvent()
{

}

void comUDPProcessReceivedPacket(uint8_t* in_packet, uint8_t in_packet_size)
{

}


void comUDPPeriodicCallback(void)
{

}
