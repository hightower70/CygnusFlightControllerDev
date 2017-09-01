/*****************************************************************************/
/* USB HID Communication routines                                            */
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
#include <comHID.h>
#include <sysRTOS.h>
#include <comInterfaces.h>
#include <comManager.h>
#include <comSystemPacketDefinitions.h>
#include <crcCITT16.h>
#include <halHID.h>
#include <sysString.h>
#include <comPacketBuilder.h>
#include <cfgStorage.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define comHID_TASK_PRIORITY 2
#define comHID_TASK_MAX_CYCLE_TIME 100

#define comHID_TRANSMITTER_BUFFER_SIZE halHID_TRANSMITER_PACKET_MAX_SIZE
#define comHID_RECEIVER_BUFFER_SIZE halHID_RECEIVER_PACKET_MAX_SIZE
#define comHID_REPORT_PAYLOAD_SIZE (halHID_TRANSMITER_PACKET_MAX_SIZE - comHID_PACKET_HEADER_SIZE)
#define comHID_REPORT_BUFFER_COUNT ((comMAX_PACKET_SIZE + comHID_REPORT_PAYLOAD_SIZE - 1) / (comHID_REPORT_PAYLOAD_SIZE))


/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
// task variables
static sysTaskNotify l_task_event = sysNULL;
static bool l_stop_task = false;
static uint8_t l_interface_index;

// transmitter variables
static uint8_t l_transmitter_buffer[comHID_REPORT_BUFFER_COUNT][comHID_TRANSMITTER_BUFFER_SIZE];
static uint16_t l_transmitter_buffer_index;
static bool l_transmitter_buffer_reserved;

// receiver variables
static uint8_t l_receiver_buffer[comMAX_PACKET_SIZE];
static uint16_t l_receiver_buffer_pos;
static bool l_receiver_buffer_full;

/*****************************************************************************/
/* Local functions prototypes                                                */
/*****************************************************************************/
static bool comHIDSendPacket(uint8_t* in_packet, uint16_t in_packet_length);
static void comHIDThread(void* in_param);
static void comHIDTransmitReadyCallback(void* in_interrupt_param);
static void comHIDProcessReceivedPacket(uint8_t* in_packet_buffer, void* in_interrupt_param);
void comHIDStartPacketSending(void);

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes USB HID communication module
void comHIDInit(void)
{
  sysTask task_id;
  comInterfaceDescription interface_description;
  halHIDConfigInfo hid_config_info;

  // init HAL HID
  halHIDConfigInfoInit(&hid_config_info);
  hid_config_info.TransmitterEmptyCallback = comHIDTransmitReadyCallback;
  hid_config_info.PacketReceivedCallback = comHIDProcessReceivedPacket;
  halHIDConfig(&hid_config_info);

  // init COM interface
  interface_description.PacketSendFunction = comHIDSendPacket;

  // add this interface to the list of communication interfaces
  l_interface_index = comAddInterface(&interface_description);

  sysTaskCreate(comHIDThread, "comHID", sysDEFAULT_STACK_SIZE, sysNULL, comHID_TASK_PRIORITY, &task_id, comHIDDeinit);
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Stops USB HID communication task
void comHIDDeinit(void)
{
  l_stop_task = true;
  sysTaskNotifyGive(l_task_event);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Main USB HID communication task
static void comHIDThread(void* in_param)
{
  sysUNUSED(in_param);

  // init task
  sysTaskNotifyCreate(l_task_event);

  // init variables
  l_transmitter_buffer_reserved = false;
  l_receiver_buffer_pos = 0;
  l_receiver_buffer_full = false;

  // init HAL
  halHIDInit();

  // task loop
  while (!l_stop_task)
  {
    // wait for event
    sysTaskNotifyTake(l_task_event, comHID_TASK_MAX_CYCLE_TIME);

    // stop task is requested
    if (l_stop_task)
      break;

    // handle transmitter
    if(l_transmitter_buffer_reserved)
    {
      comHIDStartPacketSending();
    }

    // handle receiver
    if(l_receiver_buffer_full)
    {
  		// further process other packets
  		comManagerStoreReceivedPacket(l_interface_index, l_receiver_buffer, l_receiver_buffer_pos);
  		l_receiver_buffer_pos = 0;
  		l_receiver_buffer_full = false;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Processes received HID packets
/// @param in_packet Pointer to packet buffer
/// @param in_packet_size Length of the packet in bytes
static void comHIDProcessReceivedPacket(uint8_t* in_packet_buffer, void* in_interrupt_param)
{
  // get real data length
	uint8_t report_remaining_byte_count = in_packet_buffer[0];
	int bytes_to_copy = report_remaining_byte_count;

	// drop packet if buffer is full
	if(l_receiver_buffer_full)
		return;

	// determine number of bytes to copy
	if (bytes_to_copy > comHID_REPORT_PAYLOAD_SIZE)
		bytes_to_copy = comHID_REPORT_PAYLOAD_SIZE;

	// reset buffer before it overruns
	if (l_receiver_buffer_pos + bytes_to_copy > comMAX_PACKET_SIZE)
	{
		l_receiver_buffer_pos = 0;
	}

	// append/store packet in the buffer
	sysMemCopy(&l_receiver_buffer[l_receiver_buffer_pos], &in_packet_buffer[comHID_PACKET_HEADER_SIZE], bytes_to_copy);
	l_receiver_buffer_pos += bytes_to_copy;

	// check if complete packet has been received
	if (report_remaining_byte_count <= comHID_REPORT_PAYLOAD_SIZE)
	{
		// the complete packet received -> notify thread about the received packet
		l_receiver_buffer_full = true;
	  sysTaskNotifyGiveFromISR(l_task_event, in_interrupt_param);
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sends packet over UDP communication line
/// @param in_packet Pointer to the packet
/// @param in_packet_length Length of the packet in bytes (total length, including CRC and header)
/// @return True if packet sending was started, false
static bool comHIDSendPacket(uint8_t* in_packet, uint16_t in_packet_length)
{
  bool success = true;
  uint16_t buffer_pos;
  uint16_t buffer_index;
  uint16_t bytes_to_copy;

  sysASSERT(in_packet_length <= comMAX_PACKET_SIZE);
  sysASSERT(in_packet != sysNULL);

  // check if connected to the host
  if (!halHIDIsConnected())
    return false;

  // reserve transmit buffer
  sysCriticalSectionBegin();

  if (!l_transmitter_buffer_reserved)
  {
    l_transmitter_buffer_reserved = true;
  }
  else
  {
    success = false;
  }

  sysCriticalSectionEnd();

  if(!success)
    return success;

  // store packet in 64 byte chunks
  buffer_pos = 0;
  buffer_index = 0;
  while(buffer_pos < in_packet_length)
  {
    // store packet header (remaining byte count)
    l_transmitter_buffer[buffer_index][0] = (uint8_t)(in_packet_length-buffer_pos);

    // store packet
    bytes_to_copy = in_packet_length - buffer_pos;
    if(bytes_to_copy > comHID_REPORT_PAYLOAD_SIZE)
      bytes_to_copy = comHID_REPORT_PAYLOAD_SIZE;

    sysMemCopy(&l_transmitter_buffer[buffer_index][comHID_PACKET_HEADER_SIZE], in_packet + buffer_pos, bytes_to_copy);

    buffer_pos += bytes_to_copy;
    buffer_index++;
  }

  // invalidate other buffers
  while(buffer_index < comHID_REPORT_BUFFER_COUNT)
  {
    l_transmitter_buffer[buffer_index][0] = 0;
    buffer_index++;
  }

  // send packet
  l_transmitter_buffer_index = 0;
  comHIDStartPacketSending();

  return success;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Starts sending packet from the transmitter buffer
void comHIDStartPacketSending(void)
{
  // send buffer
  if(l_transmitter_buffer_index < comHID_REPORT_BUFFER_COUNT && l_transmitter_buffer[l_transmitter_buffer_index][0] > 0)
  {
    halHIDSendReport(&l_transmitter_buffer[l_transmitter_buffer_index][0], comHID_TRANSMITTER_BUFFER_SIZE);
  }
  else
  {
    l_transmitter_buffer_index = 0; // shouldn't be happened
    l_transmitter_buffer_reserved = false;
  }

  // increment buffer index
  l_transmitter_buffer_index++;
  if(l_transmitter_buffer_index >= comHID_REPORT_BUFFER_COUNT || l_transmitter_buffer[l_transmitter_buffer_index][0] == 0)
  {
    l_transmitter_buffer_index = 0;
    l_transmitter_buffer_reserved = false;
  }
}

///////////////////////////////////////////////////////////////////////////////
/// @brief HID Transmitter Ready Callback
static void comHIDTransmitReadyCallback(void* in_interrupt_param)
{
  // notify thread about the transmitter ready condition
  sysTaskNotifyGiveFromISR(l_task_event, in_interrupt_param);
}

