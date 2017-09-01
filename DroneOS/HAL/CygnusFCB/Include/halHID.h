/*****************************************************************************/
/* USB HID Communication HAL routines                                        */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __halHID_h
#define __halHID_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <usbd_desc.h>
#include <usbd_customhid.h>
#include <sysTypes.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define halHID_TRANSMITER_PACKET_MAX_SIZE CUSTOM_HID_EPIN_SIZE
#define halHID_RECEIVER_PACKET_MAX_SIZE CUSTOM_HID_EPOUT_SIZE

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/
typedef void(*halHIDPacketReceivedCallback)(uint8_t* in_packet_buffer, void* in_interrupt_param);
typedef void(*halHIDTransmitterEmptyCallback)(void* in_interrupt_param);

typedef struct
{
	halHIDPacketReceivedCallback PacketReceivedCallback;
	halHIDTransmitterEmptyCallback TransmitterEmptyCallback;

} halHIDConfigInfo;

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void halHIDInit(void);

void halHIDConfig(halHIDConfigInfo* in_config_info);
void halHIDConfigInfoInit(halHIDConfigInfo* in_config_info);

bool halHIDIsConnected(void);
bool halHIDSendReport( uint8_t *in_report, uint16_t in_length);


#endif
