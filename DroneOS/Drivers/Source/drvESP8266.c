/*****************************************************************************/
/* ESP8266 WiFi chip driver                                                  */
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
#include <cfgStorage.h>
#include <comManager.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define drvESP8266_TRANSMITTER_DATA_BUFFER_LENGTH 512
#define drvESP8266_RECEIVER_BUFFER_LENGTH 512
#define drvESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH 64
#define drvESP8266_PARSER_BUFFER_LENGTH 32
#define drvESP8266_DATA_HEADER_PARSER_BUFFER_LENGTH 16
#define drvESP8266_RECEIVER_BUFFER_BLOCK_HEADER_LENGTH 2
#define drvESP8266_BLOCK_LENGTH_MAX 255

#define drvESP8266_MAKE_BROADCAST_ADDRESS(x) (x | 0xff)

// timing
#define drvESP8266_TASK_EVENT_MAX_TIMEOUT 100
#define drvESP8266_COMMAND_TIMEOUT 100
#define drvESP8266_DATA_SEND_TIMEOUT 200
#define drvESP8266_DEVICE_ANNOUNCE_PERIOD 1000

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/

/// Communication status values
typedef enum
{
	drvESP8266_CS_Idle,

	drvESP8266_CS_Initializing,
	drvESP8266_CS_Initialized,

	drvESP8266_CS_WifiConnecting,
	drvESP8266_CS_WifiConnected,
	drvESP8266_CS_WifiDisconnected,
	drvESP8266_CS_WifiConnectionQuery,

	drvESP8266_CS_ConnectionInitialize,
	drvESP8266_CS_Connected,

	drvESP8266_CS_Unknown

} drvESP8266CommunicationState;

/// Determies how to interpret data from the modem
typedef enum
{
	drvESP8266_RM_Command,
	drvESP8266_RM_DataHeaderConnection,
	drvESP8266_RM_DataHeaderLength,
	drvESP8266_RM_Data,
	drvESP8266_RM_DropData
}	drvESP8266ReceiverMode;

/// Transmitter mode while transmitting UDP packets
typedef enum
{
	drvESP8266_TM_Idle,
	drvESP8266_TM_Reserved,
	drvESP8266_TM_ReadyToSend,
	drvESP8266_TM_SendingCommand,
	drvESP8266_TM_SendingData,
	drvESP8266_TM_SendingFinishing
} drvESP8266TransmitterMode;

/// Result of the tokenizing received data block
typedef enum
{
	drvESP8266_RBT_Unknown,

	drvESP8266_RBT_Empty,

	drvESP8266_RBT_Deleted,

	drvESP8266_RBT_NotTokenized,

	drvESP8266_RBT_String,
	drvESP8266_RBT_Data,

	drvESP8266_RBT_OK,
	drvESP8266_RBT_NoChange,
	drvESP8266_RBT_Fail,
	drvESP8266_RBT_WifiConnected,
	drvESP8266_RBT_WifiDisconnected,
	drvESP8266_RBT_GotIP,
	drvESP8266_RBT_StationIP,
	drvESP8266_RBT_SendPrompt,
	drvESP8266_RBT_SendOK,
	drvESP8266_RBT_Ready,

	drvESP8266_RBT_Error,

	drvESP8266_RBT_ErrorOrOK

} drvESP8266ResponseBlockType;

// response token table entry
typedef struct
{
	sysConstString Token;
	drvESP8266ResponseBlockType Type;
} drvESP8266ResponseTokenTableEntry;

/// Prompt token types
typedef enum
{
	drvESP8266_PTT_Unknown,

	drvESP8266_PTT_SendPrompt,
	drvESP8266_PTT_ReceiveDataHeader
} drvESP8266PrompTokenType;

// prompt token table entry
typedef struct
{
	sysConstString Token;
	uint8_t TokenLength;
	drvESP8266PrompTokenType Type;
} drvESP8266PromptTokenTableEntry;

/// Reason of the command callback calling
typedef enum
{
	drvESP8266_CCR_CommandPrepare,
	drvESP8266_CCR_Response,
	drvESP8266_CCR_Timeout
} drvESP8266CommandCallbackReason;

typedef void (*drvESP8266CommandCallbackFunction)(uint8_t* inout_communication_state_step, drvESP8266CommandCallbackReason in_reason);

/// Entry for the command sequence table
typedef struct
{
	sysConstString Command;
	uint32_t Timeout;
	drvESP8266ResponseBlockType ExpectedResponse;
	drvESP8266CommandCallbackFunction Callback;
} drvESP8266CommandTableEntry;

/*****************************************************************************/
/* Module local functions                                                    */
/*****************************************************************************/
static void drvESP8266CommandSend(sysConstString in_command);
static void drvESP8266CommandSendWithTimeout(sysConstString in_command, uint32_t in_timeout);
static void drvESP8266CommandFlush(uint32_t in_timeout);
static void drvESP8266ReceiverBufferClear(void);
static void drvESP8266TransmitterDataBufferClear(void);
static void drvESP8266SwitchToNextReceivedBlock(void);
//static void drvESP8266StartTimeout(uint32_t in_delay);
static void drvESP8266CommandSequenceStart(drvESP8266CommunicationState in_new_communiation_state, drvESP8266CommandTableEntry* in_command_table, drvESP8266CommunicationState in_success_communication_state, drvESP8266CommunicationState in_timeout_communication_state);
static void drvESP8266SequenceCommandSend(drvESP8266CommandTableEntry* in_command_table, uint8_t in_sequence_index);
static void drvESP8266Thread(void* in_param);
static void drvESP8266Deinit(void);

static void drvESP8266HandleTransmitter(void);

static void drvESP8266ReceivedBlockPushStart(void);
static void drvESP8266ReceivedBlockPushEnd(drvESP8266ResponseBlockType in_type);
static void drvESP8266ReceivedBlockPush(uint8_t in_char);
static uint16_t drvESP8266ReceivedBlockGetLength(void);

void drvESP8266UARTRxCallback(uint8_t in_char, void* in_interrupt_param);
void drvESP8266UARTTxEmptyCallback(void* in_interrupt_param);

static drvESP8266ResponseBlockType drvESP8266TokenizeResponse(void);
static void drvESP8266CopyStringFromReceivedBlock(sysChar* in_buffer, uint16_t in_buffer_length, uint16_t in_bytes_to_copy);
static void drvESP8266IncrementPopPointer(uint16_t* inout_pop_pointer);

static void drvESP8266ProcessCommunicationState(void);
static void drvESP8266ProcessCommunicationTimeout(void);
static void drvESP8266StartWifiConnectionQuery(void);
static void drvESP8266ProcessResponse(void);
static sysStringLength drvESP8266AppendIPAddresToCommand(sysStringLength in_pos, uint32_t in_ip);

static void drvESP8266ModemResetCallback(uint8_t* inout_communication_state_step, drvESP8266CommandCallbackReason in_reason);
static void drvESP8266UDPConnectionPrepareCallback(uint8_t* inout_communication_state_step, drvESP8266CommandCallbackReason in_reason);

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static volatile drvESP8266CommunicationState l_communication_state;
static uint8_t l_communication_state_step;
static drvESP8266CommandTableEntry* l_command_sequence_table;
static drvESP8266CommunicationState l_communication_state_timeout;
static drvESP8266CommunicationState l_communication_state_success;
static sysTaskNotify l_task_event = sysNULL;
static uint32_t l_timeout_value;
static sysTick l_timeout_timestamp;
static bool l_stop_task = false;
static sysTick l_periodic_timestamp;

// ethernet data
static bool l_wifi_connection_changed;
static uint32_t l_local_ip_address;
static uint32_t l_remote_ip_address;
static sysTick l_device_announce_time_stamp;

// transmitter variables
static sysChar l_transmitter_data_buffer[drvESP8266_TRANSMITTER_DATA_BUFFER_LENGTH];
static uint16_t l_transmitter_data_buffer_length;
static sysChar l_transmitter_command_buffer[drvESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH];
static uint16_t l_transmitter_command_buffer_length;
static drvESP8266TransmitterMode l_transmitter_mode;
static sysTick l_transmitter_timestamp;
static uint32_t l_transmitter_ip_address;

// receiver variables
static volatile drvESP8266ReceiverMode l_receiver_mode = drvESP8266_RM_Command;
static uint8_t l_receiver_buffer[drvESP8266_RECEIVER_BUFFER_LENGTH];
static uint16_t l_receiver_buffer_pop_index;
static uint16_t l_receiver_buffer_push_index;
static uint16_t l_receiver_buffer_block_start_index;

static sysChar l_parser_buffer[drvESP8266_PARSER_BUFFER_LENGTH];

static sysChar l_data_header_parser_buffer[drvESP8266_DATA_HEADER_PARSER_BUFFER_LENGTH];
static uint8_t l_data_header_parser_buffer_pos;
static uint16_t l_data_size;
static uint16_t l_data_expected_size;

// uart variable
static uint8_t l_uart_index = 1;

uint16_t l_last_prompt_pos;

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/

// Modem response strings for tokeninizing
static const drvESP8266ResponseTokenTableEntry l_response_token_table[] =
{
	{ "OK", drvESP8266_RBT_OK },
	{ "no change", drvESP8266_RBT_NoChange },
	{ "FAIL", drvESP8266_RBT_Fail },
	{ "Error", drvESP8266_RBT_Error },
	{ "ERROR", drvESP8266_RBT_Error },
	{ "WIFI CONNECTED", drvESP8266_RBT_WifiConnected },
	{ "WIFI GOT IP", drvESP8266_RBT_GotIP },
	{ "WIFI DISCONNECT", drvESP8266_RBT_WifiDisconnected },
	{ "+CIFSR:STAIP,", drvESP8266_RBT_StationIP },
	{ "ready", drvESP8266_RBT_Ready},
	{ "SEND OK", drvESP8266_RBT_SendOK},

	{ sysNULL, drvESP8266_RBT_Unknown }
};

// modem initialization command sequence
static drvESP8266CommandTableEntry l_modem_init_table[] =
{
	{ "AT", drvESP8266_COMMAND_TIMEOUT, drvESP8266_RBT_OK, sysNULL },
	{ "AT+RST", 5000, drvESP8266_RBT_Ready, drvESP8266ModemResetCallback },
	{ "ATE0", drvESP8266_COMMAND_TIMEOUT, drvESP8266_RBT_OK, sysNULL },
	{ "AT+CWQAP", drvESP8266_COMMAND_TIMEOUT, drvESP8266_RBT_OK, sysNULL },
	{ "AT+CIPMUX=1", drvESP8266_COMMAND_TIMEOUT, drvESP8266_RBT_OK, sysNULL },
	{ "AT+CWMODE_CUR=1", drvESP8266_COMMAND_TIMEOUT, drvESP8266_RBT_OK, sysNULL },
	{ "AT+CWAUTOCONN=1", drvESP8266_COMMAND_TIMEOUT, drvESP8266_RBT_OK, sysNULL },

	{ sysNULL, 0, drvESP8266_RBT_Unknown, sysNULL }
};

// connection init command sequence
static drvESP8266CommandTableEntry l_modem_connect_table[] =
{
	{ "AT+CIPCLOSE=5", drvESP8266_COMMAND_TIMEOUT, drvESP8266_RBT_ErrorOrOK,  sysNULL },
	{ "AT+CIPSERVER=0", drvESP8266_COMMAND_TIMEOUT, drvESP8266_RBT_ErrorOrOK,  sysNULL },
	{ "", drvESP8266_COMMAND_TIMEOUT, drvESP8266_RBT_OK, drvESP8266UDPConnectionPrepareCallback },

	{ sysNULL, 0, drvESP8266_RBT_Unknown, sysNULL }
};

// tooken table for prompt tokenizing
static drvESP8266PromptTokenTableEntry l_prompt_token_table[] =
{
	{ "+IPD,", 5, drvESP8266_PTT_ReceiveDataHeader },
	{ "> ", 2, drvESP8266_PTT_SendPrompt },

	{ sysNULL, 0, drvESP8266_PTT_Unknown }
};

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

/*****************************************************************************/
/* Driver interface functions                                                */
/*****************************************************************************/
#pragma region Driver interface functions
// <editor-fold defaultstate="collapsed" desc="Driver interface functions">

///////////////////////////////////////////////////////////////////////////////
/// @brief 
void drvUDPInit(void)
{
	sysTask task_id;

	sysTaskCreate(drvESP8266Thread, "drvESP8266", sysDEFAULT_STACK_SIZE, sysNULL, 2, &task_id, drvESP8266Deinit);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Allocates and reserves transmitter buffer
/// @return Transmitter data buffer address or sysNULL if transmitter is not available
uint8_t* drvUDPAllocTransmitBuffer(void)
{
	uint8_t* buffer = sysNULL;

	// prepare for atomic access
	sysCriticalSectionBegin();

	// transmitter should be in idle mode 
	if (l_transmitter_mode == drvESP8266_TM_Idle)
	{
		// reserve transmitter buffer
		l_transmitter_mode = drvESP8266_TM_Reserved;
	}

	// exit atommic operation
	sysCriticalSectionEnd();

	// if buffer could be reserved
	if (l_transmitter_mode == drvESP8266_TM_Reserved)
	{
		buffer = (uint8_t*)l_transmitter_data_buffer;
		l_transmitter_timestamp = sysGetSystemTick();
	}

	return buffer;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Transmits data which is already placed in the transmitter buffer
/// @param in_data_length Number of bytes to send
/// @param in_destination_address IP address of the destination
void drvUDPTransmitData(uint16_t in_data_length, uint32_t in_destination_address)
{
	// transmitter buffer must be reserved first
	if (l_transmitter_mode != drvESP8266_TM_Reserved)
		return;

	// store length
	l_transmitter_data_buffer_length = in_data_length;
	l_transmitter_ip_address = in_destination_address;

	l_transmitter_mode = drvESP8266_TM_ReadyToSend;
	sysTaskNotifyGive(l_task_event);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Returns true when connected to the network media
bool drvUDPIsConnected(void)
{
	return (l_local_ip_address != 0);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets local IP address
/// @return Local IP address
uint32_t drvUDPGetLocalIPAddress(void)
{
	return l_local_ip_address;
}

// </editor-fold>
#pragma endregion

/*****************************************************************************/
/* Thread function                                                           */
/*****************************************************************************/
#pragma region Thread function
// <editor-fold defaultstate="collapsed" desc="Thread function">

///////////////////////////////////////////////////////////////////////////////
/// @brief Main communication thread
static void drvESP8266Thread(void* in_param)
{
	drvESP8266ResponseBlockType block_type;
	halUARTConfigInfo uart_config;
	sysTick event_timeout;
	sysTick ellapsed_time;

	sysUNUSED(in_param);

	// UART init
	halUARTConfigInfoInit(&uart_config);
	uart_config.RxReceivedCallback = drvESP8266UARTRxCallback;
	uart_config.TxEmptyCallback = drvESP8266UARTTxEmptyCallback;
	halUARTConfig(l_uart_index, &uart_config);
	halUARTSetBaudRate(l_uart_index, 115200);

	// task variables init
	l_communication_state = drvESP8266_CS_Idle;
	l_receiver_mode = drvESP8266_RM_Command;
	l_receiver_buffer_pop_index = 0;
	l_receiver_buffer_push_index = 0;
	l_receiver_buffer_block_start_index = 0;
	l_transmitter_data_buffer_length = 0;
	l_transmitter_mode = drvESP8266_TM_Idle;
	l_timeout_value = 0;
	l_command_sequence_table = sysNULL;
	l_wifi_connection_changed = false;
	l_remote_ip_address = 0;
	l_local_ip_address = 0;
	l_device_announce_time_stamp = sysGetSystemTick();

	// task notofication init
	sysTaskNotifyCreate(l_task_event);
	sysTaskNotifyGive(l_task_event);

	event_timeout = drvESP8266_TASK_EVENT_MAX_TIMEOUT;
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
			block_type = drvESP8266TokenizeResponse();

			// process response
			if (block_type != drvESP8266_RBT_Empty)
			{
				drvESP8266ProcessResponse();
				drvESP8266SwitchToNextReceivedBlock();
			}

		} while (block_type != drvESP8266_RBT_Empty);
		
		// handle timeout
		if (l_timeout_value > 0 && sysGetSystemTickSince(l_timeout_timestamp) > l_timeout_value)
		{
			// disable timeout
			l_timeout_value = 0;

			// process timeout event
			drvESP8266ProcessCommunicationTimeout();
		}

		// process communication state
		drvESP8266ProcessCommunicationState();

		// transmitter handler
		drvESP8266HandleTransmitter();

		// setup thread lock timeout
		if (l_timeout_value > 0)
		{
			event_timeout = sysGetSystemTickSince(l_timeout_timestamp);

			if (l_timeout_value > event_timeout)
				event_timeout = l_timeout_value - event_timeout;
			else
				event_timeout = 0;

			if (event_timeout > drvESP8266_TASK_EVENT_MAX_TIMEOUT)
				event_timeout = drvESP8266_TASK_EVENT_MAX_TIMEOUT;
		}
		else
		{
			event_timeout = drvESP8266_TASK_EVENT_MAX_TIMEOUT;
		}

		// handle periodic callback
		ellapsed_time = sysGetSystemTickSince(l_periodic_timestamp);
		if (ellapsed_time > comUDP_PERIODIC_CALLBACK_TIME)
		{
			l_periodic_timestamp += ellapsed_time;
			comUDPPeriodicCallback();
		}
	}

	// clean up
	if (l_task_event != sysNULL)
		sysTaskNotifyDelete(l_task_event);

	l_task_event = sysNULL;
}

// </editor-fold>
#pragma endregion

/*****************************************************************************/
/* Response processing functions                                             */
/*****************************************************************************/
#pragma region Modem response processing functions
// <editor-fold defaultstate="collapsed" desc="Modem response processing functions">

///////////////////////////////////////////////////////////////////////////////
/// @brief processing responses from the modem
static void drvESP8266ProcessResponse(void)
{
	uint16_t receive_buffer_pop_index;
	drvESP8266ResponseBlockType block_type;
	uint8_t data_length;
	uint8_t new_communication_state_step;
	sysStringLength parser_buffer_pos;

	// check if there is data in the receiver buffer
	if (l_receiver_buffer_pop_index == l_receiver_buffer_push_index)
		return;

	// cache pop index
	receive_buffer_pop_index = l_receiver_buffer_pop_index;

	// get block length and type
	block_type = l_receiver_buffer[receive_buffer_pop_index];
	drvESP8266IncrementPopPointer(&receive_buffer_pop_index);

	data_length = l_receiver_buffer[receive_buffer_pop_index];
	drvESP8266IncrementPopPointer(&receive_buffer_pop_index);

	// preprocess response
	switch (block_type)
	{
		case drvESP8266_RBT_WifiDisconnected:
			l_local_ip_address = 0;
			return;

		case drvESP8266_RBT_Data:
			comUDPProcessReceivedPacket(&l_receiver_buffer[receive_buffer_pop_index], data_length);
			return;

		case drvESP8266_RBT_Deleted:
			return;

		case drvESP8266_RBT_SendPrompt:
			l_last_prompt_pos = l_receiver_buffer_block_start_index;
			l_transmitter_mode = drvESP8266_TM_SendingData;

			// send data
			halUARTSendBlock(l_uart_index, (uint8_t*)l_transmitter_data_buffer, l_transmitter_data_buffer_length);
			return;

		case drvESP8266_RBT_SendOK:
			l_transmitter_mode = drvESP8266_TM_Idle;

			// notify com manager about the empty buffer
			comManagerGenerateEvent();
			return;

		default:
		  break;
	}

	// check if command sequency is active
	if (l_command_sequence_table != sysNULL)
	{
		// process response according the current command seqency table entry
		new_communication_state_step = l_communication_state_step;
		switch (l_command_sequence_table[new_communication_state_step].ExpectedResponse)
		{
			case drvESP8266_RBT_Unknown:
				// use callback to parse response
				if (l_command_sequence_table[new_communication_state_step].Callback != sysNULL)
				{
					l_command_sequence_table[new_communication_state_step].Callback(&new_communication_state_step, drvESP8266_CCR_Response);
				}
				break;

			case drvESP8266_RBT_ErrorOrOK:
				// check for 'error' or 'ok'
				if(block_type == drvESP8266_RBT_OK || block_type == drvESP8266_RBT_Error)
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
				drvESP8266SequenceCommandSend(l_command_sequence_table, new_communication_state_step);
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
			case drvESP8266_CS_WifiConnecting:
				switch (block_type)
				{
					case drvESP8266_RBT_OK:
						drvESP8266StartWifiConnectionQuery();
						break;

					case drvESP8266_RBT_Fail:
						l_communication_state = drvESP8266_CS_Initialized;
						l_timeout_value = 0;
						break;

					default:
					  break;
				}
				break;

			// process responses while wifi connection query is active
			case drvESP8266_CS_WifiConnectionQuery:
				switch (block_type)
				{
					case drvESP8266_RBT_StationIP:
					{
						// parse IP address
						uint8_t byte1;
						uint8_t byte2;
						uint8_t byte3;
						uint8_t byte4;
						bool success = true;

						drvESP8266CopyStringFromReceivedBlock(l_parser_buffer, drvESP8266_PARSER_BUFFER_LENGTH, data_length);
						parser_buffer_pos = 14;

						sysStringToByte(l_parser_buffer, drvESP8266_PARSER_BUFFER_LENGTH, &parser_buffer_pos, &success, &byte1);
						sysCheckForSeparator(l_parser_buffer, drvESP8266_PARSER_BUFFER_LENGTH, &parser_buffer_pos, &success, '.');
						sysStringToByte(l_parser_buffer, drvESP8266_PARSER_BUFFER_LENGTH, &parser_buffer_pos, &success, &byte2);
						sysCheckForSeparator(l_parser_buffer, drvESP8266_PARSER_BUFFER_LENGTH, &parser_buffer_pos, &success, '.');
						sysStringToByte(l_parser_buffer, drvESP8266_PARSER_BUFFER_LENGTH, &parser_buffer_pos, &success, &byte3);
						sysCheckForSeparator(l_parser_buffer, drvESP8266_PARSER_BUFFER_LENGTH, &parser_buffer_pos, &success, '.');
						sysStringToByte(l_parser_buffer, drvESP8266_PARSER_BUFFER_LENGTH, &parser_buffer_pos, &success, &byte4);

						if (success)
							l_local_ip_address = (((uint32_t)byte1) << 24) + (byte2 << 16) + (byte3 << 8) + byte4;
						else
							l_local_ip_address = 0;
					}
					break;

					case drvESP8266_RBT_OK:
						l_wifi_connection_changed = false;
						l_timeout_value = 0;

						if (l_local_ip_address == 0)
							l_communication_state = drvESP8266_CS_WifiDisconnected;
						else
						{
							l_communication_state = drvESP8266_CS_ConnectionInitialize;
						}
						break;

					default:
					  break;
				}
				break;

				default:
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
static void drvESP8266ProcessCommunicationState(void)
{
	sysStringLength pos;

	// process communication state 
	switch (l_communication_state)
	{
		// modem is idle -> start initialization and connection procedure
		case drvESP8266_CS_Idle:
			drvESP8266CommandSequenceStart(drvESP8266_CS_Initializing, l_modem_init_table, drvESP8266_CS_Initialized, drvESP8266_CS_Idle);
			break;

		// modem is initialized -> start wifi connection (connection to AP) 
		case drvESP8266_CS_Initialized:
			l_communication_state = drvESP8266_CS_WifiConnecting;

			pos = sysCopyConstString(l_transmitter_command_buffer, drvESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, 0, (sysConstString)"AT+CWJAP=\"");
			pos = sysCopyString(l_transmitter_command_buffer, drvESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos, cfgGetStringValue(cfgVAL_WIFI_SSID));
			pos = sysCopyConstString(l_transmitter_command_buffer, drvESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos, (sysConstString)"\",\"");
			pos = sysCopyString(l_transmitter_command_buffer, drvESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos, cfgGetStringValue(cfgVAL_WIFI_PWD));
			pos = sysCopyConstString(l_transmitter_command_buffer, drvESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos, (sysConstString)"\"");

			drvESP8266CommandFlush(20000);
			break;

		// handle wifi connection state change
		case drvESP8266_CS_WifiConnected:
		case drvESP8266_CS_WifiDisconnected:
			// check for connection event
			if (l_wifi_connection_changed)
				drvESP8266StartWifiConnectionQuery();
			break;

		// wifi connection is active, prepare for TCP/UDP connection
		case drvESP8266_CS_ConnectionInitialize:
			l_remote_ip_address = 0;
			drvESP8266CommandSequenceStart(drvESP8266_CS_Initializing, l_modem_connect_table, drvESP8266_CS_Connected, drvESP8266_CS_Initialized);
			break;

		default:
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Starts querying local IP address 
static void drvESP8266StartWifiConnectionQuery(void)
{
	l_communication_state = drvESP8266_CS_WifiConnectionQuery;
	drvESP8266CommandSend("AT+CIFSR");
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
static void drvESP8266ProcessCommunicationTimeout(void)
{
	uint8_t new_communication_state_step = l_communication_state_step;

	// if in command sequence mode
	if (l_command_sequence_table != sysNULL)
	{
		// call callback if it exists
		if (l_command_sequence_table[l_communication_state_step].Callback != sysNULL)
		{
			l_command_sequence_table[l_communication_state_step].Callback(&new_communication_state_step, drvESP8266_CCR_Timeout);

			// move to the next step
			if (new_communication_state_step != l_communication_state_step)
			{
				l_communication_state_step = new_communication_state_step;
				drvESP8266CommandSendWithTimeout(l_command_sequence_table[new_communication_state_step].Command, l_command_sequence_table[new_communication_state_step].Timeout);
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
/* Transmitter handlers                                                      */
/*****************************************************************************/
#pragma region Transmiter handlers
// <editor-fold defaultstate="collapsed" desc="Transmiter handlers">

///////////////////////////////////////////////////////////////////////////////
/// @breief Handles transmitter section
static void drvESP8266HandleTransmitter(void)
{
	sysStringLength pos;

	// do nothing when transmitter is idle
	if (l_transmitter_mode == drvESP8266_TM_Idle)
		return;

	switch (l_transmitter_mode)
	{
		case drvESP8266_TM_ReadyToSend:
			// packet is ready to send, check is modem is connected
			if (l_communication_state == drvESP8266_CS_Connected)
			{
				// start packet sending
				if (l_transmitter_data_buffer_length == 0)
				{
					l_transmitter_mode = drvESP8266_TM_Idle;
				}
				else
				{
					l_transmitter_mode = drvESP8266_TM_SendingCommand;

					pos = sysCopyConstString(l_transmitter_command_buffer, drvESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, 0, (sysConstString)"AT+CIPSEND=0,");
					pos = sysWordToStringPos(l_transmitter_command_buffer, drvESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos, l_transmitter_data_buffer_length, 0, 0, 0);
					pos = sysCopyConstString(l_transmitter_command_buffer, drvESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos, (sysConstString)",\"");
					pos = drvESP8266AppendIPAddresToCommand(pos, l_transmitter_ip_address);
					pos = sysCopyConstString(l_transmitter_command_buffer, drvESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos, (sysConstString)"\",");
					pos = sysWordToStringPos(l_transmitter_command_buffer, drvESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos, cfgGetUInt16Value(cfgVAL_WIFI_REMOTE), 0, 0, 0);

					l_transmitter_command_buffer_length = pos;

					drvESP8266CommandFlush(drvESP8266_DATA_SEND_TIMEOUT);
				}
			}
			break;

		default:
		  break;
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
static void drvESP8266ModemResetCallback(uint8_t* inout_communication_state_step, drvESP8266CommandCallbackReason in_reason)
{
	switch (in_reason)
	{
		case drvESP8266_CCR_Response:
		case drvESP8266_CCR_Timeout:
			drvESP8266ReceiverBufferClear();

			(*inout_communication_state_step)++;
			break;

		default:
		  break;
	}
}

static void drvESP8266UDPConnectionPrepareCallback(uint8_t* inout_communication_state_step, drvESP8266CommandCallbackReason in_reason)
{
	sysStringLength pos;

	sysUNUSED(inout_communication_state_step);

	if (in_reason != drvESP8266_CCR_CommandPrepare)
		return;

	pos = sysCopyConstString(l_transmitter_command_buffer, drvESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, 0, (sysConstString)"AT+CIPSTART=0,\"UDP\",\"");
	pos = drvESP8266AppendIPAddresToCommand(pos, drvESP8266_MAKE_BROADCAST_ADDRESS(l_local_ip_address));		// remote IP (any IP addres on the same subnet)
	pos = sysCopyConstString(l_transmitter_command_buffer, drvESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos, (sysConstString)"\",");
	pos = sysWordToStringPos(l_transmitter_command_buffer, drvESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos,  cfgGetUInt16Value(cfgVAL_WIFI_REMOTE), 0, 0, 0); // remote port
	pos = sysCopyCharacter(l_transmitter_command_buffer, drvESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos, ',');
	pos = sysWordToStringPos(l_transmitter_command_buffer, drvESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos, cfgGetUInt16Value(cfgVAL_WIFI_LOCAL), 0, 0, 0);
	pos = sysCopyConstString(l_transmitter_command_buffer, drvESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos, (sysConstString)",0");
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
void drvESP8266UARTRxCallback(uint8_t in_char, void* in_interrupt_param)
{
	uint16_t block_length;
	uint16_t pop_pointer;
	uint16_t new_push_index;
	uint8_t prompt_token_index;
	drvESP8266PrompTokenType prompt_token_type;
	sysStringLength pos;

	switch (l_receiver_mode)
	{
		// receive command response
		case drvESP8266_RM_Command:
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
						// mark current block as non tokenized data
						drvESP8266ReceivedBlockPushEnd(drvESP8266_RBT_NotTokenized);

						// notify thread about the received block
						sysTaskNotifyGiveFromISR(l_task_event, in_interrupt_param);
					}
					break;

					// store character
				default:
					// create block header (store block type and length before storing the first character)
					if (l_receiver_buffer_push_index == l_receiver_buffer_block_start_index)
					{
						// create block header (type, length)
						drvESP8266ReceivedBlockPushStart();

//						printf("\n%d:", l_receiver_buffer_block_start_index);
					}

					// store current character
					drvESP8266ReceivedBlockPush(in_char);

					// try to check for prompt

					// determine current block length
					block_length = drvESP8266ReceivedBlockGetLength();

					// search for prompt string
					prompt_token_index = 0;
					prompt_token_type = drvESP8266_PTT_Unknown;
					while (l_prompt_token_table[prompt_token_index].Token != sysNULL && prompt_token_type == drvESP8266_PTT_Unknown)
					{
						if (block_length == l_prompt_token_table[prompt_token_index].TokenLength + drvESP8266_RECEIVER_BUFFER_BLOCK_HEADER_LENGTH)
						{
							// check for data header
							pop_pointer = l_receiver_buffer_block_start_index + drvESP8266_RECEIVER_BUFFER_BLOCK_HEADER_LENGTH;
							if (pop_pointer >= drvESP8266_RECEIVER_BUFFER_LENGTH)
								pop_pointer = pop_pointer - drvESP8266_RECEIVER_BUFFER_LENGTH;

							// compare buffer content with the token string
							pos = 0;
							while (l_prompt_token_table[prompt_token_index].Token[pos] != '\0' && l_receiver_buffer[pop_pointer] == l_prompt_token_table[prompt_token_index].Token[pos])
							{
								drvESP8266IncrementPopPointer(&pop_pointer);
								pos++;
							}

							// if token is found in the buffer
							if (l_prompt_token_table[prompt_token_index].Token[pos] == '\0')
							{
								prompt_token_type = l_prompt_token_table[prompt_token_index].Type;
							}
						}

						prompt_token_index++;
					}

					// check token
					switch (prompt_token_type)
					{
						// send prompt received
						case drvESP8266_PTT_SendPrompt:
							// set buffer type to send prompt
							drvESP8266ReceivedBlockPushEnd(drvESP8266_RBT_SendPrompt);

							// notify thread about the received prompt
							sysTaskNotifyGiveFromISR(l_task_event, in_interrupt_param);
							break;

						// data header received of the incoming packet
						case drvESP8266_PTT_ReceiveDataHeader:
							l_receiver_mode = drvESP8266_RM_DataHeaderConnection;
							break;

						default:
							break;
					}
			}
			break;

		// incoming packet, receive data header connection ID
		case drvESP8266_RM_DataHeaderConnection:
			// store received character
			drvESP8266ReceivedBlockPush(in_char);

			// switch to data header length
			if (in_char == ',')
			{
				l_receiver_mode = drvESP8266_RM_DataHeaderLength;
				l_data_header_parser_buffer_pos = 0;
			}
			break;

		// incoming data, receive data length
		case drvESP8266_RM_DataHeaderLength:
		{
			bool success;
			uint16_t size;

			// store received character
			drvESP8266ReceivedBlockPush(in_char);

			if (in_char == ':')
			{
				// parse data length
				l_data_header_parser_buffer[l_data_header_parser_buffer_pos] = '\0';
				success = true;
				pos = 0;
				size = 0;
				sysStringToWord(l_data_header_parser_buffer, drvESP8266_DATA_HEADER_PARSER_BUFFER_LENGTH, &pos, &success, &size);

				// prepare for data receiveing
				l_data_size = 0;
				l_data_expected_size = size;
				l_receiver_mode = drvESP8266_RM_Data;

				// remove modem data header from the buffer and keep binary data only in the buffer
				new_push_index = l_receiver_buffer_block_start_index + drvESP8266_RECEIVER_BUFFER_BLOCK_HEADER_LENGTH;

				if (new_push_index >= drvESP8266_RECEIVER_BUFFER_LENGTH)
					new_push_index -= drvESP8266_RECEIVER_BUFFER_LENGTH;

				// prepare buffer for data receiving (reserve buffer in one linear region of buffer memory without wrapping)
				if (new_push_index + size >= drvESP8266_RECEIVER_BUFFER_LENGTH)
				{
					// there is not enough space (without warpping) at then end of the buffer
					// check space at the beginning
					if (l_receiver_buffer_pop_index > l_receiver_buffer_block_start_index)
					{
						// no free space in the buffer -> drop incoming data
						l_receiver_mode = drvESP8266_RM_DropData;
						drvESP8266ReceivedBlockPushEnd(drvESP8266_RBT_Deleted);
					}
					else
					{
						// set remaining part of the buffer to deleted
						l_receiver_buffer_push_index = 0;
						drvESP8266ReceivedBlockPushEnd(drvESP8266_RBT_Deleted);

						// start a new buffer from the beginning of the physical buffer memory
						drvESP8266ReceivedBlockPushStart();
					}
				}
				else
				{
					l_receiver_buffer_push_index = new_push_index;
				}
			}
			else
			{
				if (l_data_header_parser_buffer_pos < drvESP8266_DATA_HEADER_PARSER_BUFFER_LENGTH - 1)
				{
					l_data_header_parser_buffer[l_data_header_parser_buffer_pos++] = in_char;
				}
			}
		}
		break;

		// receiving binary data
		case drvESP8266_RM_Data:
			// store data
			drvESP8266ReceivedBlockPush(in_char);

			l_data_size++;
			if (l_data_size == l_data_expected_size)
			{
				// mark buffer as data buffer and store length
				drvESP8266ReceivedBlockPushEnd(drvESP8266_RBT_Data);

				// prepare for receiving the next block
				l_receiver_mode = drvESP8266_RM_Command;

				// notify thread about the received block
				sysTaskNotifyGiveFromISR(l_task_event, in_interrupt_param);
			}

			break;

		// drops received data because there was no space in the buffer
		case drvESP8266_RM_DropData:
			l_data_size++;
			if (l_data_size == l_data_expected_size)
			{
				l_receiver_buffer_block_start_index = l_receiver_buffer_push_index;
				l_receiver_mode = drvESP8266_RM_Command;
			}
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief ESP8266 UART Transmitter Empty Callback
void drvESP8266UARTTxEmptyCallback(void* in_interrupt_param)
{
	// flag transmitter buffer free
	if (l_transmitter_mode == drvESP8266_TM_SendingData)
	{
		l_transmitter_data_buffer_length = 0;
		l_transmitter_mode = drvESP8266_TM_SendingFinishing;
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
static void drvESP8266ReceiverBufferClear(void)
{
	sysCriticalSectionBegin();

	l_receiver_buffer_pop_index = 0;
	l_receiver_buffer_block_start_index = 0;
	l_receiver_buffer_push_index = 0;

	// set block length and type
	l_receiver_buffer[0] = drvESP8266_RBT_Empty;
	l_receiver_buffer[1] = 0;

	sysCriticalSectionEnd();
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Clear transmitter data buffer
static void drvESP8266TransmitterDataBufferClear(void)
{
	l_transmitter_data_buffer_length = 0;
	l_transmitter_mode = drvESP8266_TM_Idle;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Copies string from the receiver buffer to the specified buffer and null terminates it.
/// @param in_buffer Buffer to copy the string
/// @param in_buffer_lenth Length of the buffer
/// @param in_bytes_to_copy Number of bytes to copy
static void drvESP8266CopyStringFromReceivedBlock(sysChar* in_buffer, uint16_t in_buffer_length, uint16_t in_bytes_to_copy)
{
	uint16_t pos = 0;
	uint16_t receive_buffer_pop_index;
	uint8_t chars_to_copy;
	uint8_t block_type;

	// terminate buffer
	in_buffer[0] = '\0';

	// check if there is data in the receiver buffer
	if (l_receiver_buffer_pop_index == l_receiver_buffer_push_index)
		return;

	// cache pop index
	receive_buffer_pop_index = l_receiver_buffer_pop_index;

	// get block length and type
	block_type = l_receiver_buffer[receive_buffer_pop_index];
	drvESP8266IncrementPopPointer(&receive_buffer_pop_index);

	sysUNUSED(block_type);

	chars_to_copy = l_receiver_buffer[receive_buffer_pop_index];
	drvESP8266IncrementPopPointer(&receive_buffer_pop_index);

	if (in_buffer_length < chars_to_copy)
		chars_to_copy = (uint8_t)in_buffer_length;

	if (in_bytes_to_copy < chars_to_copy)
		chars_to_copy = (uint8_t)in_bytes_to_copy;

	while (pos < chars_to_copy && pos < in_buffer_length - 1)
	{
		in_buffer[pos++] = l_receiver_buffer[receive_buffer_pop_index]; 
		drvESP8266IncrementPopPointer(&receive_buffer_pop_index);
	}

	in_buffer[pos] = '\0';
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Tokenizes response string based on the token table and updates receiver buffer content when maching token is found
static drvESP8266ResponseBlockType drvESP8266TokenizeResponse(void)
{
	uint16_t receive_buffer_pop_index;
	uint16_t string_start_pop_index;
	uint8_t token_pos;
	uint8_t token_index;
	drvESP8266ResponseBlockType block_type;
	uint8_t block_length;
	uint8_t block_pos;

	// check if there is data in the receiver buffer
	if (l_receiver_buffer_pop_index == l_receiver_buffer_push_index)
		return drvESP8266_RBT_Empty;

	// cache pop index
	receive_buffer_pop_index = l_receiver_buffer_pop_index;

	// get block length and type
	block_type = l_receiver_buffer[receive_buffer_pop_index];
	drvESP8266IncrementPopPointer(&receive_buffer_pop_index);

	block_length = l_receiver_buffer[receive_buffer_pop_index];
	drvESP8266IncrementPopPointer(&receive_buffer_pop_index);

	if (block_length == 0)
		return drvESP8266_RBT_Empty;

	if (block_type != drvESP8266_RBT_NotTokenized)
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
			block_pos++;
			drvESP8266IncrementPopPointer(&receive_buffer_pop_index);
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
		block_type = drvESP8266_RBT_String;
	}

	// store block type in the header
	l_receiver_buffer[l_receiver_buffer_pop_index] = block_type;

	return block_type;
}


///////////////////////////////////////////////////////////////////////////////
/// @brief Increments receiver buffer pop pointer (and handles wrap around)
static void drvESP8266IncrementPopPointer(uint16_t* inout_pop_pointer)
{
	uint16_t pop_pointer = *inout_pop_pointer;

	pop_pointer++;
	if (pop_pointer >= drvESP8266_RECEIVER_BUFFER_LENGTH)
		pop_pointer = 0;

	*inout_pop_pointer = pop_pointer;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Switches to the processing of the next received block
static void drvESP8266SwitchToNextReceivedBlock(void)
{
	uint16_t receive_buffer_pop_index;
	uint8_t data_length;

	receive_buffer_pop_index = l_receiver_buffer_pop_index;
	drvESP8266IncrementPopPointer(&receive_buffer_pop_index);

	data_length = l_receiver_buffer[receive_buffer_pop_index];
	drvESP8266IncrementPopPointer(&receive_buffer_pop_index);

	l_receiver_buffer_pop_index += data_length + drvESP8266_RECEIVER_BUFFER_BLOCK_HEADER_LENGTH;

	if (l_receiver_buffer_pop_index >= drvESP8266_RECEIVER_BUFFER_LENGTH)
		l_receiver_buffer_pop_index -= drvESP8266_RECEIVER_BUFFER_LENGTH;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes received block (fills out header)
static void drvESP8266ReceivedBlockPushStart(void)
{
	uint16_t type_index;
	uint16_t length_index;
	uint16_t new_push_index;

	type_index = l_receiver_buffer_block_start_index;
	length_index = type_index + 1;

	if (length_index >= drvESP8266_RECEIVER_BUFFER_LENGTH)
		length_index = 0;

	new_push_index = length_index + 1;
	if (new_push_index >= drvESP8266_RECEIVER_BUFFER_LENGTH)
		new_push_index = 0;

	// do nothing if there is not enough space in the buffer
	if (length_index == l_receiver_buffer_pop_index || new_push_index == l_receiver_buffer_pop_index)
		return;

	l_receiver_buffer[type_index] = drvESP8266_RBT_Unknown;
	l_receiver_buffer[length_index] = 0;

	l_receiver_buffer_push_index = new_push_index;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Updates received block header
/// @param in_type Type of the block
/// @param Length of the block
static void drvESP8266ReceivedBlockPushEnd(drvESP8266ResponseBlockType in_type)
{
	uint16_t type_index;
	uint16_t length_index;
	uint16_t block_length;
	
	// calculate block length
	if (l_receiver_buffer_push_index > l_receiver_buffer_block_start_index)
		block_length = l_receiver_buffer_push_index - l_receiver_buffer_block_start_index - drvESP8266_RECEIVER_BUFFER_BLOCK_HEADER_LENGTH;
	else
		block_length = drvESP8266_RECEIVER_BUFFER_LENGTH - l_receiver_buffer_block_start_index + l_receiver_buffer_push_index - drvESP8266_RECEIVER_BUFFER_BLOCK_HEADER_LENGTH;

	// determine header byte indices
	type_index = l_receiver_buffer_block_start_index;
	length_index = type_index + 1;

	if (length_index >= drvESP8266_RECEIVER_BUFFER_LENGTH)
		length_index = 0;

	// update block header
	l_receiver_buffer[length_index] = (uint8_t)block_length;
	l_receiver_buffer[type_index] = in_type;

	l_receiver_buffer_block_start_index = l_receiver_buffer_push_index;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Stores received character in the receiver queue
/// @param in_mode New receiver mode
static void drvESP8266ReceivedBlockPush(uint8_t in_char)
{
	uint16_t new_push_index;
	uint16_t block_length;

	// calculate block length
	block_length = drvESP8266ReceivedBlockGetLength();

	// if there is no header do not store data
	if (block_length < drvESP8266_RECEIVER_BUFFER_BLOCK_HEADER_LENGTH)
		return;

	// do not store block data part longer than 255 byte 
	if (block_length == drvESP8266_BLOCK_LENGTH_MAX + drvESP8266_RECEIVER_BUFFER_BLOCK_HEADER_LENGTH)
		return;

	// determine next push index
	new_push_index = l_receiver_buffer_push_index + 1;
	if (new_push_index >= drvESP8266_RECEIVER_BUFFER_LENGTH)
		new_push_index = 0;

	// check for buffer space
	if (new_push_index == l_receiver_buffer_pop_index)
		return;

	// store data
	l_receiver_buffer[l_receiver_buffer_push_index] = in_char;

/*
	if (l_receiver_mode == drvESP8266_RM_Data)
		printf("%02X ", in_char);
	else
		printf("%c", in_char);
*/
	l_receiver_buffer_push_index = new_push_index;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets current received block length
static uint16_t drvESP8266ReceivedBlockGetLength(void)
{
	uint16_t block_length;

	// calculate block length
	if (l_receiver_buffer_push_index >= l_receiver_buffer_block_start_index)
		block_length = l_receiver_buffer_push_index - l_receiver_buffer_block_start_index;
	else
		block_length = drvESP8266_RECEIVER_BUFFER_LENGTH - l_receiver_buffer_block_start_index + l_receiver_buffer_push_index;

	return block_length;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Send command string to the ESP8266
static void drvESP8266CommandSend(sysConstString in_command)
{
	sysCopyConstString(l_transmitter_command_buffer, drvESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, 0, in_command);

	// send command
	drvESP8266CommandFlush(drvESP8266_COMMAND_TIMEOUT);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Send command string to the ESP8266 with the specified timeout
static void drvESP8266CommandSendWithTimeout(sysConstString in_command, uint32_t in_timeout)
{
	sysCopyConstString(l_transmitter_command_buffer, drvESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, 0, in_command);

	// send command
	drvESP8266CommandFlush(in_timeout);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Flushed command string from the command buffer
static void drvESP8266CommandFlush(uint32_t in_timeout)
{
	sysStringLength pos;

	// terminate command
	pos = sysAppendConstString(l_transmitter_command_buffer, drvESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, "\r\n");
	l_transmitter_command_buffer_length = pos;

	// setup timeout
	l_timeout_value = in_timeout;
	l_timeout_timestamp = sysGetSystemTick();

	// send command
	halUARTSendBlock(l_uart_index, (uint8_t*)l_transmitter_command_buffer, pos);
}
/*
///////////////////////////////////////////////////////////////////////////////
/// @brief Waits for the specified time
/// @param in_delay Delay time (in ms)
static void drvESP8266StartTimeout(uint32_t in_delay)
{
	l_timeout_value = in_delay;
	l_timeout_timestamp = sysGetSystemTick();
}
*/
///////////////////////////////////////////////////////////////////////////////
/// @brief Changes communication stage to a new value and starts sending sequency of commands to the modem
/// @param in_new_communiation_state New communication stage. The stage step wil be set to zero.
static void drvESP8266CommandSequenceStart(drvESP8266CommunicationState in_new_communiation_state, drvESP8266CommandTableEntry* in_command_table, drvESP8266CommunicationState in_success_communication_state, drvESP8266CommunicationState in_timeout_communication_state)
{
	l_command_sequence_table = in_command_table;
	l_communication_state = in_new_communiation_state;
	l_communication_state_step = 0;
	l_communication_state_success = in_success_communication_state;
	l_communication_state_timeout = in_timeout_communication_state;

	// start with the first command
	drvESP8266SequenceCommandSend(in_command_table, 0);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sends one command from a sequence
/// @param in_command_table Pointer to the command sequence table
/// @param in_sequency_index Index within the command table
static void drvESP8266SequenceCommandSend(drvESP8266CommandTableEntry* in_command_table, uint8_t in_sequence_index)
{
	// prepare command
	if (l_command_sequence_table[in_sequence_index].Command[0] != '\0')
	{
		// if command specified then copy into the transmit buffer
		sysCopyConstString(l_transmitter_command_buffer, drvESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, 0, l_command_sequence_table[in_sequence_index].Command);
	}
	else
	{
		// clear command buffer
		l_transmitter_command_buffer[0] = '\0';

		// command is not specified -> call callback
		if (l_command_sequence_table[in_sequence_index].Callback != sysNULL)
			l_command_sequence_table[in_sequence_index].Callback(&l_communication_state_step, drvESP8266_CCR_CommandPrepare);
	}

	// send command
	drvESP8266CommandFlush(l_command_sequence_table[in_sequence_index].Timeout);
}


///////////////////////////////////////////////////////////////////////////////
/// @brief Appends IP address to the command string in the command buffer
/// @param in_pos Position where IP address will be written to
/// @param in_ip IP addres to write
/// @return New end of string position
sysStringLength drvESP8266AppendIPAddresToCommand(sysStringLength in_pos, uint32_t in_ip)
{
	sysStringLength pos = in_pos;

	pos = sysWordToStringPos(l_transmitter_command_buffer, drvESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos, (in_ip >> 24) & 0xff, 0, 0, 0);
	pos = sysCopyCharacter(l_transmitter_command_buffer, drvESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos, '.');
	pos = sysWordToStringPos(l_transmitter_command_buffer, drvESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos, (in_ip >> 16) & 0xff, 0, 0, 0);
	pos = sysCopyCharacter(l_transmitter_command_buffer, drvESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos, '.');
	pos = sysWordToStringPos(l_transmitter_command_buffer, drvESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos, (in_ip >> 8) & 0xff, 0, 0, 0);
	pos = sysCopyCharacter(l_transmitter_command_buffer, drvESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos, '.');
	pos = sysWordToStringPos(l_transmitter_command_buffer, drvESP8266_TRANSMITTER_COMMAND_BUFFER_LENGTH, pos, (in_ip) & 0xff, 0, 0, 0);

	return pos;
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Stops communication manager task
static void drvESP8266Deinit(void)
{
	l_stop_task = true;
	sysTaskNotifyGive(l_task_event);
}

// </editor-fold>
#pragma endregion

