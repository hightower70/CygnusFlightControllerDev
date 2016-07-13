/*****************************************************************************/
/* UDP HAL driver (Win32)                                                    */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <sysTypes.h>
#include <sysRTOS.h>
#include <comSLIP.h>
#include <comInterfaces.h>
#include <comManager.h>
#include <crcCITT16.h>
#include <comUDP.h>
#include <cfgStorage.h>
#include "cfgConstants.h"

#pragma comment(lib,"ws2_32.lib") //Winsock Library

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define drvUDP_TRANSMITTER_BUFFER_LENGTH 256
#define drvUDP_RECEIVER_BUFFER_LENGTH 256
#define drvUDP_TASK_MAX_CYCLE_TIME 50

#define drvUDP_TRANSMITTER_BUFFER_RESEVED 0xffff

#define drvUDP_TASK_PRIORITY 2

/*****************************************************************************/
/* Module local variables                                                    */
/*****************************************************************************/

// task and socket variables
static volatile bool l_stop_task = false;
static WSAEVENT l_task_events[2];
static WSANETWORKEVENTS l_events;
static WSADATA l_wsa;
static SOCKET l_socket = INVALID_SOCKET;
struct sockaddr_in l_socket_address;

static sysTick l_periodic_timestamp;

// receiver variables
static uint8_t l_receive_buffer[drvUDP_RECEIVER_BUFFER_LENGTH];
static uint16_t l_receive_buffer_length;

// transmitter variables
static uint8_t l_transmitter_buffer[drvUDP_RECEIVER_BUFFER_LENGTH];
static volatile uint16_t l_transmitter_buffer_length;
static uint32_t l_transmitter_destination_address;

/*****************************************************************************/
/* Local functions                                                           */
/*****************************************************************************/
static void drvUDPTask(void* lpParam);
static void drvUDPDeinit(void);

/*****************************************************************************/
/* Functions implementation                                                  */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// @brief Initializes ethernet communication service on WIN32
void drvUDPInit(void)
{
	sysTask task_handle;

	// initialize communication tasks
	sysTaskCreate(drvUDPTask, "halUDP", sysDEFAULT_STACK_SIZE, sysNULL, drvUDP_TASK_PRIORITY, &task_handle, drvUDPDeinit);
}

///////////////////////////////////////////////////////////////////////////////
// @brief Shots down ethernet communication service
static void drvUDPDeinit(void)
{
	l_stop_task = true;
	sysTaskNotifyGive(l_task_events[0]);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Allocates and reserves transmitter buffer
/// @return Transmitter data buffer address or sysNULL if transmitter is not available
uint8_t* drvUDPAllocTransmitBuffer(void)
{
	uint8_t* buffer;

	sysCriticalSectionBegin();

	if (l_transmitter_buffer_length == 0)
	{
		l_transmitter_buffer_length = drvUDP_TRANSMITTER_BUFFER_RESEVED;
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
/// @brief Transmits data which is already placed in the transmitter buffer
/// @param in_data_length Number of bytes to send
/// @param in_destination_address IP address of the destination
void drvUDPTransmitData(uint16_t in_data_length, uint32_t in_destination_address)
{
	// sanity check
	if (l_transmitter_buffer_length != drvUDP_TRANSMITTER_BUFFER_RESEVED)
		return;

	// notify thread about the transmission request
	l_transmitter_destination_address = in_destination_address;
	l_transmitter_buffer_length = in_data_length;

	sysTaskNotifyGive(l_task_events[0]);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets local IP address
/// @return Local IP address
uint32_t drvUDPGetLocalIPAddress(void)
{
	char host_name[255];
	DWORD retval;
	struct addrinfo *result = NULL;
	struct addrinfo *ptr = NULL;

	if (gethostname(host_name, sizeof(host_name)) == 0)
	{
		retval = getaddrinfo(host_name, NULL, NULL, &result);
		if (retval == 0)
		{
			for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
			{
				switch (ptr->ai_family)
				{
					case AF_UNSPEC:
						break;
					case AF_INET:
						return htonl(((struct sockaddr_in *)ptr->ai_addr)->sin_addr.s_addr);
						break;
				}
			}
		}
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Telemetry socket thread functions
static void drvUDPTask(void* lpParam)
{
	DWORD wait_result;
	int one = 1;
	sysTick ellapsed_time;

	// create notofication
	sysTaskNotifyCreate(l_task_events[0]);

	// init socket
	if (WSAStartup(MAKEWORD(2, 2), &l_wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		ExitThread(1);
	}

	//Create a socket
	if ((l_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
	{
		printf("Communication: Could not create socket : %d\n", WSAGetLastError());
		ExitThread(1);
	}

	// enable reuse address
	setsockopt(l_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&one, sizeof(one));

	//Prepare the sockaddr_in structure
	l_socket_address.sin_family = AF_INET;
	l_socket_address.sin_addr.s_addr = htonl(INADDR_ANY);
	l_socket_address.sin_port = htons(cfgGetUInt16Value(cfgVAL_WIFI_LOCAL));

	//Bind
	if (bind(l_socket, (struct sockaddr*)&l_socket_address, sizeof(l_socket_address)) == SOCKET_ERROR)
	{
		printf("Communication: Bind failed with error code : %d\n", WSAGetLastError());
		ExitThread(0);
	}

	// create event
	l_task_events[1] = WSACreateEvent();
	WSAEventSelect(l_socket, l_task_events[1], FD_READ);

	// init
	l_transmitter_buffer_length = 0;
	l_periodic_timestamp = sysGetSystemTick();

	// task loop
	while (!l_stop_task)
	{
		wait_result = WSAWaitForMultipleEvents(2, (const HANDLE*)&l_task_events, FALSE, drvUDP_TASK_MAX_CYCLE_TIME, FALSE);

		if (l_stop_task)
			break;

		switch (wait_result)
		{
			// task event occured
			case WSA_WAIT_EVENT_0:
				break;

			// network event occured
			case WSA_WAIT_EVENT_0 + 1:
				if (WSAEnumNetworkEvents(l_socket, l_task_events[1], &l_events) != SOCKET_ERROR)
				{
					// process packet received events
					if ((l_events.lNetworkEvents & FD_READ) != 0)
					{
						l_receive_buffer_length = recv(l_socket, l_receive_buffer, drvUDP_RECEIVER_BUFFER_LENGTH, 0);

						if (l_receive_buffer_length > 0)
						{
							comUDPProcessReceivedPacket(l_receive_buffer, (uint8_t)l_receive_buffer_length);
						}
					}
				}
				else
				{

				}
				break;
		}

		// handle transmission request
		if (l_transmitter_buffer_length > 0 && l_transmitter_buffer_length < drvUDP_TRANSMITTER_BUFFER_LENGTH)
		{
			struct sockaddr_in dest;

			// send packet
			dest.sin_family = AF_INET;
			dest.sin_addr.s_addr = ntohl(l_transmitter_destination_address);
			dest.sin_port = htons(cfgGetUInt16Value(cfgVAL_WIFI_REMOTE));

			sendto(l_socket, (const char*)l_transmitter_buffer, l_transmitter_buffer_length, 0, (const struct sockaddr*)&dest, sizeof(dest));

			l_transmitter_buffer_length = 0;

			// Notify communication task about the available send buffer
			comManagerGenerateEvent();
		}

		// handle periodic callback
		ellapsed_time = sysGetSystemTickSince(l_periodic_timestamp);
		if ( ellapsed_time > comUDP_PERIODIC_CALLBACK_TIME)
		{
			l_periodic_timestamp += ellapsed_time;
			comUDPPeriodicCallback();
		}
	}

	// close socket
	closesocket(l_socket);
	l_socket = INVALID_SOCKET;

	// release events
	WSACloseEvent(l_task_events[1]);
	sysTaskNotifyDelete(l_task_events[0]);

	// clean up sockets
	WSACleanup();

	// exit from thread
	ExitThread(0);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Returns true when connected to the network media
bool drvUDPIsConnected(void)
{
	return true;
}
