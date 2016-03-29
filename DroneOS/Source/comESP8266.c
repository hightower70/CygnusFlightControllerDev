/*****************************************************************************/
/* ESP8266 WiFi chip driver                                                  */
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
#include <drvUART.h>
#include <sysString.h>
#include <comESP8266.h>
#include <comPacketBuilder.h>
#include <cfgValues.h>
#include <sysCRC16.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define comESP8266_TRANSMITTER_DATA_BUFFER_LENGTH 512
#define comESP8266_RECEIVER_BUFFER_LENGTH 512
#define comESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH 64
#define comESP8266_PARSER_BUFFER_LENGTH 32
#define comESP8266_DATA_HEADER_PARSER_BUFFER_LENGTH 16

#define comESP8266_MAKE_BROADCAST_ADDRESS(x) (x | 0xff)

// timing
#define comESP8266_TASK_EVENT_MAX_TIMEOUT 100
#define comESP8266_COMMAND_TIMEOUT 100
#define comESP8266_DATA_SEND_TIMEOUT 200
#define comESP8266_DEVICE_INFO_SEND_DELAY 1000

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/

/// Communication status values
typedef enum
{
	comESP8266_CS_Idle,

	comESP8266_CS_Initializing,
	comESP8266_CS_Initialized,

	comESP8266_CS_WifiConnecting,
	comESP8266_CS_WifiConnected,
	comESP8266_CS_WifiDisconnected,
	comESP8266_CS_WifiConnectionQuery,

	comESP8266_CS_ConnectionInitialize,
	comESP8266_CS_Connected,

	comESP8266_CS_SendingCommand,
	comESP8266_CS_SendingData,
	comESP8266_CS_SendingFinishing,

	comESP8266_CS_Unknown

} comESP8266CommunicationState;


/// Determies how to interpret data from the modem
typedef enum
{
	comESP8266_RS_Command,
	comESP8266_RS_DataHeaderConnection,
	comESP8266_RS_DataHeaderLength,
	comESP8266_RS_Data
}	comESP8266ReceiverStatus;

/// Result of the tokenizing received data block
typedef enum
{
	comESP8266_RTT_Unknown,

	comESP8266_RTT_Empty,

	comESP8266_RTT_NotTokenized,

	comESP8266_RTT_String,
	comESP8266_RTT_Data,

	comESP8266_RTT_OK,
	comESP8266_RTT_NoChange,
	comESP8266_RTT_Fail,
	comESP8266_RTT_WifiConnected,
	comESP8266_RTT_WifiDisconnected,
	comESP8266_RTT_GotIP,
	comESP8266_RTT_StationIP,
	comESP8266_RTT_SendPrompt,
	comESP8266_RTT_SendOK,
	comESP8266_RTT_Ready,

	comESP8266_RTT_Error,

	comESP8266_RTT_ErrorOrOK

} comESP8266ResponseBlockType;

typedef struct
{
	sysConstString Token;
	comESP8266ResponseBlockType Type;
} comESP8266ResponseTokenTableEntry;

/// Reason of the command callback calling
typedef enum
{
	comESP8266_CCR_CommandPrepare,
	comESP8266_CCR_Response,
	comESP8266_CCR_Timeout
} comESP8266CommandCallbackReason;

typedef void (*comESP8266CommandCallbackFunction)(uint8_t* inout_communication_state_step, comESP8266CommandCallbackReason in_reason);

/// Entry for the command sequence table
typedef struct
{
	sysConstString Command;
	uint32_t Timeout;
	comESP8266ResponseBlockType ExpectedResponse;
	comESP8266CommandCallbackFunction Callback;
} comESP8266CommandTableEntry;

/*****************************************************************************/
/* Module local functions                                                    */
/*****************************************************************************/
static void comESP8266SendCommand(sysConstString in_command);
static void comESP8266SendCommandWithTimeout(sysConstString in_command, uint32_t in_timeout);
static void comESP8266FlushCommand(uint16_t in_timeout);
static void comESP8266ClearReceiverBuffer(void);
static void comESP8266SwitchToNextReceivedBlock(void);
static void comESP8266StartTimeout(uint32_t in_delay);
static void comESP8266StartCommandSequence(comESP8266CommunicationState in_new_communiation_state, comESP8266CommandTableEntry* in_command_table, comESP8266CommunicationState in_success_communication_state, comESP8266CommunicationState in_timeout_communication_state);
static void comESP8266SendSequenceCommand(comESP8266CommandTableEntry* in_command_table, uint8_t in_sequence_index);
static void comESP8266Thread(void* in_param);
static void comESP8266StartPacketSending(void);
static void comESP8266SendDeviceInformation(void);

void comESP8266UARTRxCallback(uint8_t in_char, void* in_interrupt_param);
void comESP8266UARTTxEmptyCallback(void* in_interrupt_param);

static void comESP8266PushReceivedCharacter(uint8_t in_char);
static comESP8266ResponseBlockType comESP8266TokenizeResponse(void);
static void comESP8266CopyStringFromReceivedBlock(sysChar* in_buffer, uint16_t in_buffer_length, uint16_t in_bytes_to_copy);
static void comESP8266IncrementPopPointer(uint16_t* inout_pop_pointer);

static void comESP8266ProcessCommunicationState(void);
static void comESP8266ProcessCommunicationTimeout(void);
static void comESP8266StartWifiConnectionQuery(void);
static void comESP8266ProcessResponse(void);
static sysStringLength comESP8266AppendIPAddresToCommand(sysStringLength in_pos, uint32_t in_ip);


static void comESP8266ModemResetCallback(uint8_t* inout_communication_state_step, comESP8266CommandCallbackReason in_reason);
static void comESP8266UDPConnectionPrepareCallback(uint8_t* inout_communication_state_step, comESP8266CommandCallbackReason in_reason);
static void comESP8266TCPServerConnectionPrepareCallback(uint8_t* inout_communication_state_step, comESP8266CommandCallbackReason in_reason);

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static volatile comESP8266CommunicationState l_communication_state;
static uint8_t l_communication_state_step;
static comESP8266CommandTableEntry* l_command_sequence_table;
static comESP8266CommunicationState l_communication_state_timeout;
static comESP8266CommunicationState l_communication_state_success;
static sysTaskNotify l_task_event = sysNULL;
static uint32_t l_timeout_value;
static sysTick l_timeout_timestamp;
static bool l_stop_task = false;

// ethernet data
static bool l_wifi_connection_changed;
static uint32_t l_local_ip_address;
static uint32_t l_remote_ip_address;
static sysTick l_last_deviceinfo_send_timestamp;

// transmitter variables
static sysChar l_transmitter_data_buffer[comESP8266_TRANSMITTER_DATA_BUFFER_LENGTH];
static uint16_t l_transmitter_data_buffer_length;
static sysChar l_transmitter_command_buffer[comESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH];
static uint16_t l_transmitter_command_buffer_length;

// receiver variables
static volatile comESP8266ReceiverStatus l_receiver_mode = comESP8266_RS_Command;
static uint8_t l_receiver_buffer[comESP8266_RECEIVER_BUFFER_LENGTH];
static uint16_t l_receiver_buffer_pop_index;
static uint16_t l_receiver_buffer_push_index;
static uint16_t l_receiver_buffer_block_start_index;

static sysChar l_parser_buffer[comESP8266_PARSER_BUFFER_LENGTH];

static sysChar l_data_header_parser_buffer[comESP8266_DATA_HEADER_PARSER_BUFFER_LENGTH];
static uint8_t l_data_header_parser_buffer_pos;
static uint16_t l_data_size;
static uint16_t l_data_expected_size;

// uart variable
static uint8_t l_uart_index = 0;

static const comESP8266ResponseTokenTableEntry l_response_token_table[] =
{
	{ "OK", comESP8266_RTT_OK },
	{ "no change", comESP8266_RTT_NoChange },
	{ "FAIL", comESP8266_RTT_Fail },
	{ "Error", comESP8266_RTT_Error },
	{ "ERROR", comESP8266_RTT_Error },
	{ "WIFI CONNECTED", comESP8266_RTT_WifiConnected },
	{ "WIFI GOT IP", comESP8266_RTT_GotIP },
	{ "WIFI DISCONNECT", comESP8266_RTT_WifiDisconnected },
	{ "+CIFSR:STAIP,", comESP8266_RTT_StationIP },
	{ "ready", comESP8266_RTT_Ready},
	{ "SEND OK", comESP8266_RTT_SendOK},

	{ sysNULL, comESP8266_RTT_Unknown }
};

static comESP8266CommandTableEntry l_modem_init_table[] =
{
	{ "AT", comESP8266_COMMAND_TIMEOUT, comESP8266_RTT_OK, sysNULL },
	{ "AT+RST", 5000, comESP8266_RTT_Ready, comESP8266ModemResetCallback },
	{ "ATE0", comESP8266_COMMAND_TIMEOUT, comESP8266_RTT_OK, sysNULL },
	{ "AT+CWQAP", comESP8266_COMMAND_TIMEOUT, comESP8266_RTT_OK, sysNULL },
	{ "AT+CIPMUX=1", comESP8266_COMMAND_TIMEOUT, comESP8266_RTT_OK, sysNULL },
	{ "AT+CWMODE_CUR=1", comESP8266_COMMAND_TIMEOUT, comESP8266_RTT_OK, sysNULL },
	{ "AT+CWAUTOCONN=1", comESP8266_COMMAND_TIMEOUT, comESP8266_RTT_OK, sysNULL },

	{ sysNULL, 0, comESP8266_RTT_Unknown, sysNULL }
};

static comESP8266CommandTableEntry l_modem_connect_table[] =
{
	{ "AT+CIPCLOSE=5", comESP8266_COMMAND_TIMEOUT, comESP8266_RTT_ErrorOrOK,  sysNULL },
	{ "AT+CIPSERVER=0", comESP8266_COMMAND_TIMEOUT, comESP8266_RTT_ErrorOrOK,  sysNULL },
	{ "", comESP8266_COMMAND_TIMEOUT, comESP8266_RTT_OK, comESP8266UDPConnectionPrepareCallback },

	{ sysNULL, 0, comESP8266_RTT_Unknown, sysNULL }
};


static sysConstString l_data_header = "+IPD,";

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/
#pragma region Public functions
// <editor-fold defaultstate="collapsed" desc="Public functions">

///////////////////////////////////////////////////////////////////////////////
/// @brief 
void comESP8266Init(void)
{
	uint32_t task_id;

	sysTaskCreate(comESP8266Thread, "comESP8266", sysDEFAULT_STACK_SIZE, sysNULL, 2, &task_id, comESP8266Deinit);
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Stops communication manager task
void comESP8266Deinit(void)
{
	l_stop_task = true;
	sysTaskNotifyGive(l_task_event);
}

// </editor-fold>
#pragma endregion

/*****************************************************************************/
/* Local functions                                                           */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Main communication thread
static void comESP8266Thread(void* in_param)
{
	comESP8266ResponseBlockType block_type;
	drvUARTConfigInfo uart_config;
	uint32_t event_timeout;

	// UART init
	drvUARTConfigInfoInit(&uart_config);
	uart_config.RxReceivedCallback = comESP8266UARTRxCallback;
	uart_config.TxEmptyCallback = comESP8266UARTTxEmptyCallback;
	drvUARTConfig(l_uart_index, &uart_config);
	drvUARTSetBaudRate(l_uart_index, 115200);

	// task variables init
	l_communication_state = comESP8266_CS_Idle;
	l_receiver_mode = comESP8266_RS_Command;
	l_receiver_buffer_pop_index = 0;
	l_receiver_buffer_push_index = 0;
	l_receiver_buffer_block_start_index = 0;
	l_timeout_value = 0;
	l_command_sequence_table = sysNULL;
	l_wifi_connection_changed = false;
	l_remote_ip_address = 0;
	l_last_deviceinfo_send_timestamp = 0;
	
	// task notofication init
	sysTaskNotifyCreate(l_task_event);
	sysTaskNotifyGive(l_task_event);

	event_timeout = comESP8266_TASK_EVENT_MAX_TIMEOUT;
	while (!l_stop_task)
	{
		// wait for event
		sysTaskNotifyTake(l_task_event, event_timeout);

		if (l_stop_task)
			break;

		// process received response
		do
		{
			// tokenize response
			block_type = comESP8266TokenizeResponse();

			// preprocess response
			switch (block_type)
			{
				// no response yet
				case comESP8266_RTT_GotIP:
				case comESP8266_RTT_WifiConnected:
				case comESP8266_RTT_WifiDisconnected:
					l_wifi_connection_changed = true;
						break;
			}

			// process response
			if (block_type != comESP8266_RTT_Empty)
			{
				comESP8266ProcessResponse();
				comESP8266SwitchToNextReceivedBlock();
			}

		} while (block_type != comESP8266_RTT_Empty);
		
		// handle timeout
		if (l_timeout_value > 0 && sysGetSystemTickSince(l_timeout_timestamp) > l_timeout_value)
		{
			// disable timeout
			l_timeout_value = 0;

			// process timeout event
			comESP8266ProcessCommunicationTimeout();
		}

		// process communication state
		comESP8266ProcessCommunicationState();

		// setup thread lock timeout
		if (l_timeout_value > 0)
		{
			event_timeout = sysGetSystemTickSince(l_timeout_timestamp);

			if (l_timeout_value > event_timeout)
				event_timeout = l_timeout_value - event_timeout;
			else
				event_timeout = 0;

			if (event_timeout > comESP8266_TASK_EVENT_MAX_TIMEOUT)
				event_timeout = comESP8266_TASK_EVENT_MAX_TIMEOUT;
		}
		else
		{
			event_timeout = comESP8266_TASK_EVENT_MAX_TIMEOUT;
		}
	}

	// clean up
	if (l_task_event != sysNULL)
		sysTaskNotifyDelete(l_task_event);

	l_task_event = sysNULL;
}

/*****************************************************************************/
/* Response processing functions                                             */
/*****************************************************************************/

#pragma region Modem response processing functions
// <editor-fold defaultstate="collapsed" desc="Modem response processing functions">

///////////////////////////////////////////////////////////////////////////////
/// @brief processing responses from the modem
static void comESP8266ProcessResponse(void)
{
	uint16_t receive_buffer_pop_index;
	comESP8266ResponseBlockType block_type;
	uint8_t block_length;
	uint8_t new_communication_state_step;
	sysStringLength parser_buffer_pos;

	// check if there is data in the receiver buffer
	if (l_receiver_buffer_pop_index == l_receiver_buffer_push_index)
		return;

	// cache pop index
	receive_buffer_pop_index = l_receiver_buffer_pop_index;

	// get block length and type
	block_type = l_receiver_buffer[receive_buffer_pop_index];
	comESP8266IncrementPopPointer(&receive_buffer_pop_index);

	block_length = l_receiver_buffer[receive_buffer_pop_index];
	comESP8266IncrementPopPointer(&receive_buffer_pop_index);

	// check if command sequency is active
	if (l_command_sequence_table != sysNULL)
	{
		// process response according the current command seqency table entry
		new_communication_state_step = l_communication_state_step;
		switch (l_command_sequence_table[new_communication_state_step].ExpectedResponse)
		{
			case comESP8266_RTT_Unknown:
				// use callback to parse response
				if (l_command_sequence_table[new_communication_state_step].Callback != sysNULL)
				{
					l_command_sequence_table[new_communication_state_step].Callback(&new_communication_state_step, comESP8266_CCR_Response);
				}
				break;

			case comESP8266_RTT_ErrorOrOK:
				// check for 'error' or 'ok'
				if(block_type == comESP8266_RTT_OK || block_type == comESP8266_RTT_Error)
					new_communication_state_step++;
				break;

			default:
				// check response type 
				if(block_type == l_command_sequence_table[new_communication_state_step].ExpectedResponse)
					new_communication_state_step++;
				break;
		}

		// move to the next step
		if (new_communication_state_step != l_communication_state_step)
		{
			l_communication_state_step = new_communication_state_step;

			if (l_command_sequence_table[new_communication_state_step].Command != sysNULL)
			{
				// start next step
				comESP8266SendSequenceCommand(l_command_sequence_table, new_communication_state_step);
			}
			else
			{
				// there is no more step
				l_communication_state = l_communication_state_success;
				l_command_sequence_table = sysNULL;
				l_timeout_value = 0;
			}
		}
	}
	else
	{
		// process response when command sequency is not active
		switch (l_communication_state)
		{
			// Process responses while wifi is connection
			case comESP8266_CS_WifiConnecting:
				switch (block_type)
				{
					case comESP8266_RTT_OK:
						comESP8266StartWifiConnectionQuery();
						break;

					case comESP8266_RTT_Fail:
						l_communication_state = comESP8266_CS_Initialized;
						l_timeout_value = 0;
						break;
				}
				break;

			// waiting for send prompt
			case comESP8266_CS_SendingCommand:
				switch (block_type)
				{
					// send prompt received 
					case comESP8266_RTT_SendPrompt:
						l_communication_state = comESP8266_CS_SendingData;

						// send command
						drvUARTSendBlock(l_uart_index, (uint8_t*)l_transmitter_data_buffer, l_transmitter_data_buffer_length);
						break;
				}
				break;

			// Waiting for send response
			case comESP8266_CS_SendingFinishing:
				if (block_type == comESP8266_RTT_SendOK)
				{
					l_timeout_value = 0;
					l_communication_state = comESP8266_CS_Connected;
				}
				break;

			// process responses while wifi connection query is active
			case comESP8266_CS_WifiConnectionQuery:
				switch (block_type)
				{
					case comESP8266_RTT_StationIP:
					{
						// parse IP address
						uint8_t byte1;
						uint8_t byte2;
						uint8_t byte3;
						uint8_t byte4;
						bool success = true;

						comESP8266CopyStringFromReceivedBlock(l_parser_buffer, comESP8266_PARSER_BUFFER_LENGTH, block_length - 2);
						parser_buffer_pos = 14;

						strStringToByte(l_parser_buffer, comESP8266_PARSER_BUFFER_LENGTH, &parser_buffer_pos, &success, &byte1);
						strCheckForSeparator(l_parser_buffer, comESP8266_PARSER_BUFFER_LENGTH, &parser_buffer_pos, &success, '.');
						strStringToByte(l_parser_buffer, comESP8266_PARSER_BUFFER_LENGTH, &parser_buffer_pos, &success, &byte2);
						strCheckForSeparator(l_parser_buffer, comESP8266_PARSER_BUFFER_LENGTH, &parser_buffer_pos, &success, '.');
						strStringToByte(l_parser_buffer, comESP8266_PARSER_BUFFER_LENGTH, &parser_buffer_pos, &success, &byte3);
						strCheckForSeparator(l_parser_buffer, comESP8266_PARSER_BUFFER_LENGTH, &parser_buffer_pos, &success, '.');
						strStringToByte(l_parser_buffer, comESP8266_PARSER_BUFFER_LENGTH, &parser_buffer_pos, &success, &byte4);

						if (success)
							l_local_ip_address = (((uint32_t)byte1) << 24) + (byte2 << 16) + (byte3 << 8) + byte4;
						else
							l_local_ip_address = 0;
					}
					break;

					case comESP8266_RTT_OK:
						l_wifi_connection_changed = false;
						l_timeout_value = 0;

						if (l_local_ip_address == 0)
							l_communication_state = comESP8266_CS_WifiDisconnected;
						else
						{
							l_communication_state = comESP8266_CS_ConnectionInitialize;
						}
						break;
				}
				break;
		}
	}
}


// </editor-fold>
#pragma endregion

/*****************************************************************************/
/* Communication state handler functions                                     */
/*****************************************************************************/

#pragma region Communication state handlers
// <editor-fold defaultstate="collapsed" desc="Communication state handlers">

///////////////////////////////////////////////////////////////////////////////
/// @brief Processes communication state
static void comESP8266ProcessCommunicationState(void)
{
	sysStringLength pos;

	// process communication state 
	switch (l_communication_state)
	{
		// modem is idle -> start initialization and connection procedure
		case comESP8266_CS_Idle:
			comESP8266StartCommandSequence(comESP8266_CS_Initializing, l_modem_init_table, comESP8266_CS_Initialized, comESP8266_CS_Idle);
			break;

		// modem is initialized -> start wifi connection (connection to AP) 
		case comESP8266_CS_Initialized:
			l_communication_state = comESP8266_CS_WifiConnecting;

			pos = strCopyConstString(l_transmitter_command_buffer, comESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, 0, (sysConstString)"AT+CWJAP=\"");
			pos = strCopyString(l_transmitter_command_buffer, comESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos, /* TODO g_system_settings.WiFiSSID*/ "CygnusTelemetry");
			pos = strCopyConstString(l_transmitter_command_buffer, comESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos, (sysConstString)"\",\"");
			pos = strCopyString(l_transmitter_command_buffer, comESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos, /* TODO g_system_settings.WiFiPassword*/"cygnustelemetry");
			pos = strCopyConstString(l_transmitter_command_buffer, comESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos, (sysConstString)"\"");

			comESP8266FlushCommand(20000);
			break;

		// handle wifi connection state change
		case comESP8266_CS_WifiConnected:
		case comESP8266_CS_WifiDisconnected:
			// check for connection event
			if (l_wifi_connection_changed)
				comESP8266StartWifiConnectionQuery();
			break;

		// wifi connection is active, prepare for TCP/UDP connection
		case comESP8266_CS_ConnectionInitialize:
			l_remote_ip_address = 0;
			comESP8266StartCommandSequence(comESP8266_CS_Initializing, l_modem_connect_table, comESP8266_CS_Connected, comESP8266_CS_Initialized);
			break;

		// connection is prepared
		case comESP8266_CS_Connected:
			// check for connection event
			if (l_wifi_connection_changed)
			{
				comESP8266StartWifiConnectionQuery();
			}
			else
			{
				// check if remote device is connected
				if (l_remote_ip_address == 0)
				{
					// there is no remote device -> broadcast this device information

					if (sysGetSystemTickSince(l_last_deviceinfo_send_timestamp) > comESP8266_DEVICE_INFO_SEND_DELAY)
					{
						l_last_deviceinfo_send_timestamp = sysGetSystemTick();
						comESP8266SendDeviceInformation();
					}
				}
				else
				{
					// remote device is active -> send pending data


				}
			}
			break;

		default:
			break;
	}
}

static void comESP8266StartWifiConnectionQuery(void)
{
	l_communication_state = comESP8266_CS_WifiConnectionQuery;
	comESP8266SendCommand("AT+CIFSR");
}

static void comESP8266StartPacketSending(void)
{
	sysStringLength pos;

	if (l_communication_state != comESP8266_CS_Connected)
		return;
	 
	l_communication_state = comESP8266_CS_SendingCommand;

	pos = strCopyConstString(l_transmitter_command_buffer, comESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, 0, (sysConstString)"AT+CIPSEND=0,");
	pos = strWordToStringPos(l_transmitter_command_buffer, comESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos, l_transmitter_data_buffer_length, 0, 0, 0);

	l_transmitter_command_buffer_length = pos;

	comESP8266FlushCommand(comESP8266_DATA_SEND_TIMEOUT);
}

static void comESP8266SendDeviceInformation(void)
{
	uint16_t crc;
	comDeviceInformation* info = (comDeviceInformation*)l_transmitter_data_buffer;

	// fill packet header
	comFillPacketHeader(&info->Header, comPT_DEVICE_INFO, sizeof(comDeviceInformation));

	strCopyString(info->Name, comDEVICE_NAME_LENGTH, 0, cfgGetStringValue(cfgVAL_DEVICE_NAME));

	info->Address = l_local_ip_address;

	// calculate CRC
	crc = crc16CalculateForBlock(crc16_INIT_VALUE, l_transmitter_data_buffer, sizeof(comDeviceInformation));
	l_transmitter_data_buffer[sizeof(comDeviceInformation)] = sysLOW(crc);
	l_transmitter_data_buffer[sizeof(comDeviceInformation)+1] = sysHIGH(crc);

	// update packet length
	l_transmitter_data_buffer_length = info->Header.PacketLength;

	// send packet
	comESP8266StartPacketSending();
}


// </editor-fold>
#pragma endregion


/*****************************************************************************/
/* Communication timeout handler functions                                   */
/*****************************************************************************/

#pragma region Communication timeout handlers
// <editor-fold defaultstate="collapsed" desc="Communication timeout handlers">

///////////////////////////////////////////////////////////////////////////////
/// @brief Processes communicaiton timeout of the wifi modem
static void comESP8266ProcessCommunicationTimeout(void)
{
	uint8_t new_communication_state_step = l_communication_state_step;

	// if in command sequence mode
	if (l_command_sequence_table != sysNULL)
	{
		// call callback if it exists
		if (l_command_sequence_table[l_communication_state_step].Callback != sysNULL)
		{
			l_command_sequence_table[l_communication_state_step].Callback(&new_communication_state_step, comESP8266_CCR_Timeout);

			// move to the next step
			if (new_communication_state_step != l_communication_state_step)
			{
				l_communication_state_step = new_communication_state_step;
				comESP8266SendCommandWithTimeout(l_command_sequence_table[new_communication_state_step].Command, l_command_sequence_table[new_communication_state_step].Timeout);
			}
		}
		else
		{
			// there is no callback, set communication stage to the specified timeout value
			l_communication_state = l_communication_state_timeout;
			l_command_sequence_table = sysNULL;
		}
	}
	else
	{
		// TODO
	}
}

// </editor-fold>
#pragma endregion

/*****************************************************************************/
/* Command callback functions                                                */
/*****************************************************************************/

#pragma region Command callback functions
// <editor-fold defaultstate="collapsed" desc="Command callback functions">

///////////////////////////////////////////////////////////////////////////////
/// @brief Handles timeout for resetting modem
/// @param inout_communication_state_step Communication step index
static void comESP8266ModemResetCallback(uint8_t* inout_communication_state_step, comESP8266CommandCallbackReason in_reason)
{
	switch (in_reason)
	{
		case comESP8266_CCR_Response:
		case comESP8266_CCR_Timeout:
			comESP8266ClearReceiverBuffer();

			(*inout_communication_state_step)++;
			break;
	}
}

static void comESP8266UDPConnectionPrepareCallback(uint8_t* inout_communication_state_step, comESP8266CommandCallbackReason in_reason)
{
	sysStringLength pos;

	if (in_reason != comESP8266_CCR_CommandPrepare)
		return;

	pos = strCopyConstString(l_transmitter_command_buffer, comESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, 0, (sysConstString)"AT+CIPSTART=0,\"UDP\",\"");
	pos = comESP8266AppendIPAddresToCommand(pos, comESP8266_MAKE_BROADCAST_ADDRESS(l_local_ip_address));
	pos = strCopyConstString(l_transmitter_command_buffer, comESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos, (sysConstString)"\",");
	pos = strWordToStringPos(l_transmitter_command_buffer, comESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos, 9601, 0, 0, 0);
	pos = strCopyCharacter(l_transmitter_command_buffer, comESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos, ',');
	pos = strWordToStringPos(l_transmitter_command_buffer, comESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos, 9601, 0, 0, 0);
	pos = strCopyConstString(l_transmitter_command_buffer, comESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos, (sysConstString)",0");
}

static void comESP8266TCPServerConnectionPrepareCallback(uint8_t* inout_communication_state_step, comESP8266CommandCallbackReason in_reason)
{
	sysStringLength pos;

	if (in_reason != comESP8266_CCR_CommandPrepare)
		return;

	pos = strCopyConstString(l_transmitter_command_buffer, comESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, 0, (sysConstString)"AT+CIPSERVER=1,");
	pos = strWordToStringPos(l_transmitter_command_buffer, comESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos, 9602, 0, 0, 0);
}


// </editor-fold>
#pragma endregion

/*****************************************************************************/
/* UART Callback functions                                                   */
/*****************************************************************************/

#pragma region Callback functions
// <editor-fold defaultstate="collapsed" desc="Callback functions">

///////////////////////////////////////////////////////////////////////////////
/// @brief ESP8266 UART Character Received Callback
void comESP8266UARTRxCallback(uint8_t in_char, void* in_interrupt_param)
{
	uint8_t block_length;
	uint16_t pop_pointer;
	uint16_t pos;

	switch (l_receiver_mode)
	{
		// receive command response
		case comESP8266_RS_Command:
			switch (in_char)
			{
				// drop new line character
				case '\n':
					break;

					// process response line end
				case '\r':
					// skip empty lines
					if (l_receiver_buffer_push_index != l_receiver_buffer_block_start_index)
					{
						// mark buffer as non tokenized buffer
						l_receiver_buffer[l_receiver_buffer_block_start_index] = comESP8266_RTT_NotTokenized;

						if (l_receiver_buffer_push_index > l_receiver_buffer_block_start_index)
							block_length = l_receiver_buffer_push_index - l_receiver_buffer_block_start_index;
						else
							block_length = comESP8266_RECEIVER_BUFFER_LENGTH - l_receiver_buffer_block_start_index + l_receiver_buffer_push_index;

						// set buffer size
						if ((l_receiver_buffer_block_start_index + 1) >= comESP8266_RECEIVER_BUFFER_LENGTH)
							l_receiver_buffer[0] = block_length;
						else
							l_receiver_buffer[l_receiver_buffer_block_start_index + 1] = block_length;

						// prepare for the next block
						l_receiver_buffer_block_start_index = l_receiver_buffer_push_index;

						// notify thread about the received block
						sysTaskNotifyGiveFromISR(l_task_event, in_interrupt_param);
					}
					break;

					// store character
				default:
					// create block header (store block type and length before storing the first character)
					if (l_receiver_buffer_push_index == l_receiver_buffer_block_start_index)
					{
						if (in_char == '>')
						{
							comESP8266PushReceivedCharacter(comESP8266_RTT_SendPrompt); // block type
							comESP8266PushReceivedCharacter(3); // block length
							comESP8266PushReceivedCharacter(in_char);

							// notify thread about the received block
							sysTaskNotifyGiveFromISR(l_task_event, in_interrupt_param);
						}
						else
						{
							comESP8266PushReceivedCharacter(comESP8266_RTT_Unknown); // block type
							comESP8266PushReceivedCharacter(0); // block length

							// store received character
							comESP8266PushReceivedCharacter(in_char);
						}
					}
					else
					{
						comESP8266PushReceivedCharacter(in_char);

						// determine current block length
						if (l_receiver_buffer_push_index > l_receiver_buffer_block_start_index)
							block_length = l_receiver_buffer_push_index - l_receiver_buffer_block_start_index;
						else
							block_length = comESP8266_RECEIVER_BUFFER_LENGTH - l_receiver_buffer_block_start_index + l_receiver_buffer_push_index;

						// search for received data header
						if (block_length == 6)
						{
							// check for data header
							pop_pointer = l_receiver_buffer_block_start_index + 2;
							if (pop_pointer >= comESP8266_RECEIVER_BUFFER_LENGTH)
								pop_pointer = 0;

							pos = 0;
							while (l_data_header[pos] != '\0' && l_receiver_buffer[pop_pointer] == l_data_header[pos])
							{
								comESP8266IncrementPopPointer(&pop_pointer);
								pos++;
							}

							// data header found -> switch to data processing
							if (l_data_header[pos] == '\0')
							{
								l_receiver_mode = comESP8266_RS_DataHeaderConnection;
							}
						}
					}

					break;
			}
			break;

		// receive data header connection ID
		case comESP8266_RS_DataHeaderConnection:
			// store received character
			comESP8266PushReceivedCharacter(in_char);

			// switch to data header length
			if (in_char == ',')
			{
				l_receiver_mode = comESP8266_RS_DataHeaderLength;
				l_data_header_parser_buffer_pos = 0;
			}
			break;

		// receive data length
		case comESP8266_RS_DataHeaderLength:
		{
			bool success;
			uint16_t size;
			sysStringLength pos;

			// store received character
			comESP8266PushReceivedCharacter(in_char);

			if (in_char == ':')
			{
				// parse data length
				l_data_header_parser_buffer[l_data_header_parser_buffer_pos] = '\0';
				success = true;
				pos = 0;
				size = 0;
				strStringToWord(l_data_header_parser_buffer, comESP8266_DATA_HEADER_PARSER_BUFFER_LENGTH, &pos, &success, &size);

				l_data_size = 0;
				l_data_expected_size = size;
				l_receiver_mode = comESP8266_RS_Data;
			}
			else
			{
				if (l_data_header_parser_buffer_pos < comESP8266_DATA_HEADER_PARSER_BUFFER_LENGTH - 1)
				{
					l_data_header_parser_buffer[l_data_header_parser_buffer_pos++] = in_char;
				}
			}
		}
		break;

		// receiving binary data
		case comESP8266_RS_Data:
			// store data
			comESP8266PushReceivedCharacter(in_char);

			l_data_size++;
			if (l_data_size == l_data_expected_size)
			{
				// mark buffer as non tokenized buffer
				l_receiver_buffer[l_receiver_buffer_block_start_index] = comESP8266_RTT_Data;

				if (l_receiver_buffer_push_index > l_receiver_buffer_block_start_index)
					block_length = l_receiver_buffer_push_index - l_receiver_buffer_block_start_index;
				else
					block_length = comESP8266_RECEIVER_BUFFER_LENGTH - l_receiver_buffer_block_start_index + l_receiver_buffer_push_index;

				// set buffer size
				if ((l_receiver_buffer_block_start_index + 1) >= comESP8266_RECEIVER_BUFFER_LENGTH)
					l_receiver_buffer[0] = block_length;
				else
					l_receiver_buffer[l_receiver_buffer_block_start_index + 1] = block_length;

				// prepare for the next block
				l_receiver_buffer_block_start_index = l_receiver_buffer_push_index;
				l_receiver_mode = comESP8266_RS_Command;

				// notify thread about the received block
				sysTaskNotifyGiveFromISR(l_task_event, in_interrupt_param);
			}

			break;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief ESP8266 UART Transmitter Empty Callback

void comESP8266UARTTxEmptyCallback(void* in_interrupt_param)
{
	// flag transmitter buffer free
	if (l_communication_state == comESP8266_CS_SendingData)
	{
		l_transmitter_data_buffer_length = 0;
		l_communication_state = comESP8266_CS_SendingFinishing;
	}
	else
	{
		l_transmitter_command_buffer_length = 0;
	}

	// notify thread about the received block
	sysTaskNotifyGiveFromISR(l_task_event, in_interrupt_param);
}

// </editor-fold>
#pragma endregion

/*****************************************************************************/
/* Communication helper functions                                            */
/*****************************************************************************/

#pragma region Communication helpers
// <editor-fold defaultstate="collapsed" desc="Communication helpers">

///////////////////////////////////////////////////////////////////////////////
/// @brief Clears receive buffer content
static void comESP8266ClearReceiverBuffer(void)
{
	sysCriticalSectionBegin();

	l_receiver_buffer_pop_index = 0;
	l_receiver_buffer_block_start_index = 0;
	l_receiver_buffer_push_index = 0;

	// set block length and type
	l_receiver_buffer[0] = comESP8266_RTT_Empty;
	l_receiver_buffer[1] = 0;

	sysCriticalSectionEnd();
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Copies string from the receiver buffer to the specified buffer and null terminates it.
/// @param in_buffer Buffer to copy the string
/// @param in_buffer_lenth Length of the buffer
/// @param in_bytes_to_copy Number of bytes to copy
static void comESP8266CopyStringFromReceivedBlock(sysChar* in_buffer, uint16_t in_buffer_length, uint16_t in_bytes_to_copy)
{
	uint16_t pos = 0;
	uint16_t receive_buffer_pop_index;
	comESP8266ResponseBlockType block_type;
	uint8_t string_length;

	// terminate buffer
	in_buffer[0] = '\0';

	// check if there is data in the receiver buffer
	if (l_receiver_buffer_pop_index == l_receiver_buffer_push_index)
		return;

	// cache pop index
	receive_buffer_pop_index = l_receiver_buffer_pop_index;

	// get block length and type
	block_type = l_receiver_buffer[receive_buffer_pop_index];
	comESP8266IncrementPopPointer(&receive_buffer_pop_index);

	string_length = l_receiver_buffer[receive_buffer_pop_index] - 2;
	comESP8266IncrementPopPointer(&receive_buffer_pop_index);

	while (pos < string_length && pos < in_buffer_length - 1)
	{
		in_buffer[pos++] = l_receiver_buffer[receive_buffer_pop_index]; 
		comESP8266IncrementPopPointer(&receive_buffer_pop_index);
	}

	in_buffer[pos] = '\0';
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Tokenizes response string based on the token table and updates receiver buffer content when maching token is found
static comESP8266ResponseBlockType comESP8266TokenizeResponse(void)
{
	uint16_t receive_buffer_pop_index;
	uint16_t string_start_pop_index;
	uint8_t token_pos;
	uint8_t token_index;
	comESP8266ResponseBlockType block_type;
	uint8_t block_length;
	uint8_t block_pos;

	// check if there is data in the receiver buffer
	if (l_receiver_buffer_pop_index == l_receiver_buffer_push_index)
		return comESP8266_RTT_Empty;

	// cache pop index
	receive_buffer_pop_index = l_receiver_buffer_pop_index;

	// get block length and type
	block_type = l_receiver_buffer[receive_buffer_pop_index];
	comESP8266IncrementPopPointer(&receive_buffer_pop_index);

	block_length = l_receiver_buffer[receive_buffer_pop_index];
	comESP8266IncrementPopPointer(&receive_buffer_pop_index);

	if (block_length == 0)
		return comESP8266_RTT_Empty;

	if (block_type != comESP8266_RTT_NotTokenized)
		return block_type;

	// tokenize block
	string_start_pop_index = receive_buffer_pop_index;

	// process data
	token_index = 0;
	while (l_response_token_table[token_index].Token != sysNULL)
	{
		// compare buffer content to the token string
		token_pos = 0;
		receive_buffer_pop_index = string_start_pop_index;
		block_pos = 0;
		while (block_pos < block_length && l_response_token_table[token_index].Token[token_pos]!= '\0' && l_receiver_buffer[receive_buffer_pop_index] == l_response_token_table[token_index].Token[token_pos])
		{
			token_pos++;
			block_pos;
			comESP8266IncrementPopPointer(&receive_buffer_pop_index);
		}

		// if token matches
		if (l_response_token_table[token_index].Token[token_pos] == '\0')
			break;

		// next token
		token_index++;
	}

	// update block header
	if (l_response_token_table[token_index].Token != sysNULL)
	{
		// token was found
		block_type = l_response_token_table[token_index].Type;
	}
	else
	{
		// token was not found
		block_type = comESP8266_RTT_String;
	}

	// store block type in the header
	l_receiver_buffer[l_receiver_buffer_pop_index] = block_type;

	return block_type;
}


///////////////////////////////////////////////////////////////////////////////
/// @brief Increments receiver buffer pop pointer (and handles wrap around)
static void comESP8266IncrementPopPointer(uint16_t* inout_pop_pointer)
{
	uint16_t pop_pointer = *inout_pop_pointer;

	pop_pointer++;
	if (pop_pointer >= comESP8266_RECEIVER_BUFFER_LENGTH)
		pop_pointer = 0;

	*inout_pop_pointer = pop_pointer;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Switches to the processing of the next received block
static void comESP8266SwitchToNextReceivedBlock(void)
{
	uint16_t receive_buffer_pop_index;
	uint8_t block_length;

	receive_buffer_pop_index = l_receiver_buffer_pop_index;
	comESP8266IncrementPopPointer(&receive_buffer_pop_index);

	block_length = l_receiver_buffer[receive_buffer_pop_index];
	comESP8266IncrementPopPointer(&receive_buffer_pop_index);

	l_receiver_buffer_pop_index += block_length;

	if (l_receiver_buffer_pop_index >= comESP8266_RECEIVER_BUFFER_LENGTH)
		l_receiver_buffer_pop_index -= comESP8266_RECEIVER_BUFFER_LENGTH;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Stores received character in the receiver queue
/// @param in_mode New receiver mode
static void comESP8266PushReceivedCharacter(uint8_t in_char)
{
	l_receiver_buffer[l_receiver_buffer_push_index++] = in_char;

	if (l_receiver_buffer_push_index >= comESP8266_RECEIVER_BUFFER_LENGTH)
		l_receiver_buffer_push_index = 0;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Send command string to the ESP8266
static void comESP8266SendCommand(sysConstString in_command)
{
	strCopyConstString(l_transmitter_command_buffer, comESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, 0, in_command);

	// send command
	comESP8266FlushCommand(comESP8266_COMMAND_TIMEOUT);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Send command string to the ESP8266 with the specified timeout
static void comESP8266SendCommandWithTimeout(sysConstString in_command, uint32_t in_timeout)
{
	strCopyConstString(l_transmitter_command_buffer, comESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, 0, in_command);

	// send command
	comESP8266FlushCommand(in_timeout);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Flushed command string from the command buffer
static void comESP8266FlushCommand(uint16_t in_timeout)
{
	sysStringLength pos;

	// terminate command
	pos = strAppendConstString(l_transmitter_command_buffer, comESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, "\r\n");
	l_transmitter_command_buffer_length = pos;

	// setup timeout
	l_timeout_value = in_timeout;
	l_timeout_timestamp = sysGetSystemTick();

	// send command
	drvUARTSendBlock(l_uart_index, (uint8_t*)l_transmitter_command_buffer, pos);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Waits for the specified time
/// @param in_delay Delay time (in ms)
static void comESP8266StartTimeout(uint32_t in_delay)
{
	l_timeout_value = in_delay;
	l_timeout_timestamp = sysGetSystemTick();
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Changes communication stage to a new value and starts sending sequency of commands to the modem
/// @param in_new_communiation_state New communication stage. The stage step wil be set to zero.
static void comESP8266StartCommandSequence(comESP8266CommunicationState in_new_communiation_state, comESP8266CommandTableEntry* in_command_table, comESP8266CommunicationState in_success_communication_state, comESP8266CommunicationState in_timeout_communication_state)
{
	l_command_sequence_table = in_command_table;
	l_communication_state = in_new_communiation_state;
	l_communication_state_step = 0;
	l_communication_state_success = in_success_communication_state;
	l_communication_state_timeout = in_timeout_communication_state;

	// start with the first command
	comESP8266SendSequenceCommand(in_command_table, 0);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sends one command from a sequence
/// @param in_command_table Pointer to the command sequence table
/// @param in_sequency_index Index within the command table
static void comESP8266SendSequenceCommand(comESP8266CommandTableEntry* in_command_table, uint8_t in_sequence_index)
{
	// prepare command
	if (l_command_sequence_table[in_sequence_index].Command[0] != '\0')
	{
		// if command specified then copy into the transmit buffer
		strCopyConstString(l_transmitter_command_buffer, comESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, 0, l_command_sequence_table[in_sequence_index].Command);
	}
	else
	{
		// clear command buffer
		l_transmitter_command_buffer[0] = '\0';

		// command is not specified -> call callback
		if (l_command_sequence_table[in_sequence_index].Callback != sysNULL)
			l_command_sequence_table[in_sequence_index].Callback(&l_communication_state_step, comESP8266_CCR_CommandPrepare);
	}

	// send command
	comESP8266FlushCommand(l_command_sequence_table[in_sequence_index].Timeout);
}


///////////////////////////////////////////////////////////////////////////////
/// @brief Appends IP address to the command string in the command buffer
/// @param in_pos Position where IP address will be written to
/// @param in_ip IP addres to write
/// @return New end of string position
sysStringLength comESP8266AppendIPAddresToCommand(sysStringLength in_pos, uint32_t in_ip)
{
	sysStringLength pos = in_pos;

	pos = strWordToStringPos(l_transmitter_command_buffer, comESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos, (in_ip >> 24) & 0xff, 0, 0, 0);
	pos = strCopyCharacter(l_transmitter_command_buffer, comESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos, '.');
	pos = strWordToStringPos(l_transmitter_command_buffer, comESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos, (in_ip >> 16) & 0xff, 0, 0, 0);
	pos = strCopyCharacter(l_transmitter_command_buffer, comESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos, '.');
	pos = strWordToStringPos(l_transmitter_command_buffer, comESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos, (in_ip >> 8) & 0xff, 0, 0, 0);
	pos = strCopyCharacter(l_transmitter_command_buffer, comESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos, '.');
	pos = strWordToStringPos(l_transmitter_command_buffer, comESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos, (in_ip) & 0xff, 0, 0, 0);

	return pos;
}

// </editor-fold>
#pragma endregion

