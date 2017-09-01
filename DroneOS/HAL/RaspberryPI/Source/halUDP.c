/*****************************************************************************/
/* UDP HAL driver (Linux)                                                    */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysRTOS.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <net/if.h>
#include <errno.h>
#include <comInterfaces.h>
#include <comManager.h>
#include <crcCITT16.h>
#include <comUDP.h>
#include <cfgStorage.h>
#include <signal.h>
#include "cfgConstants.h"
//#include <string.h>
//#include <arpa/inet.h>
#if 0
#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <sysTypes.h>
#include <comSLIP.h>
#include <comInterfaces.h>
#include <comManager.h>
#include <crcCITT16.h>
#include <comUDP.h>
#include <cfgStorage.h>
#include "cfgConstants.h"
#endif

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
static pthread_t l_udp_task;
static volatile bool l_stop_task = false;
static sysTick l_periodic_timestamp;
static int l_socket = -1;
struct sockaddr_in l_socket_address;

// receiver variables
static uint8_t l_receive_buffer[drvUDP_RECEIVER_BUFFER_LENGTH];
static uint16_t l_receive_buffer_length;

// transmitter variables
static uint8_t l_transmitter_buffer[drvUDP_RECEIVER_BUFFER_LENGTH];
static volatile uint16_t l_transmitter_buffer_length;
static uint32_t l_transmitter_destination_address;


#if 0
static WSAEVENT l_task_events[2];
static WSANETWORKEVENTS l_events;
static WSADATA l_wsa;
static SOCKET l_socket = INVALID_SOCKET;
struct sockaddr_in l_socket_address;



#endif
/*****************************************************************************/
/* Local functions                                                           */
/*****************************************************************************/
static sysTaskRetval drvUDPTask(sysTaskParam in_param);
static void drvUDPDeinit(void);
static void drvUDPNotifyTask(void);

/*****************************************************************************/
/* Functions implementation                                                  */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// @brief Initializes ethernet communication service on WIN32
void drvUDPInit(void)
{
	// initialize communication tasks
	sysTaskCreate(drvUDPTask, "halUDP", sysDEFAULT_STACK_SIZE, sysNULL, drvUDP_TASK_PRIORITY, &l_udp_task, drvUDPDeinit);
}

///////////////////////////////////////////////////////////////////////////////
// @brief Shots down ethernet communication service
static void drvUDPDeinit(void)
{
	l_stop_task = true;
	drvUDPNotifyTask();
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

	drvUDPNotifyTask();
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets local IP address
/// @return Local IP address
uint32_t drvUDPGetLocalIPAddress(void)
{
	struct ifaddrs * if_addresses = NULL;
	struct ifaddrs * ifa = NULL;
	uint32_t retval = 0;

	// get addresses
	getifaddrs(&if_addresses);

	// walk thru of all interfaces
	for (ifa = if_addresses; ifa != NULL; ifa = ifa->ifa_next) 
	{
		if (ifa->ifa_addr->sa_family == AF_INET) 
		{ 
			// do not use loppback
			if ((ifa->ifa_flags & IFF_LOOPBACK) == 0)
			{
		    // a valid IPv4 address
				retval = htonl(((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr);
				break;
			}
		}
		else
		{
			// Do not use IPv6 address
			if (ifa->ifa_addr->sa_family == AF_INET6) 
			{
			}
		}
	}
	
	// release address list
	if (if_addresses != NULL)
		freeifaddrs(if_addresses);
	
	return retval;
}

///////////////////////////////////////////////////////////////////////////////
// Telemetry socket thread functions
static sysTaskRetval drvUDPTask(sysTaskParam in_param)
{
	sysTick ellapsed_time;
	fd_set read_set, write_set;
	struct timeval timeout;
	int select_result;
	int one = 1;
	
	//DWORD wait_result;

	sysUNUSED(in_param);

	l_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (l_socket < 0) 
	{
		fprintf(stderr, "Communication: Error opening socket\n");
		exit(-1);
	}

	// enable reuse address
	setsockopt(l_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&one, sizeof(one));

	//Prepare the sockaddr_in structure
	bzero((char*) &l_socket_address, sizeof(l_socket_address));
	l_socket_address.sin_family = AF_INET;
	l_socket_address.sin_addr.s_addr = htonl(INADDR_ANY);
	l_socket_address.sin_port = htons(cfgGetUInt16Value(cfgVAL_WIFI_LOCAL));

	//Bind
	if (bind(l_socket, (struct sockaddr*)&l_socket_address, sizeof(l_socket_address)) < 0)
	{
		fprintf(stderr, "Communication: Error on binding\n");
		exit(-1);
	}

	// set socket options
	fcntl(l_socket, F_SETFL, O_NONBLOCK);
	setsockopt(l_socket, SOL_SOCKET, SO_BROADCAST, &one, sizeof(one));
	
	// init
	l_transmitter_buffer_length = 0;
	l_periodic_timestamp = sysGetSystemTick();
	
	FD_ZERO(&read_set);
	FD_SET(l_socket, &read_set);
	
	FD_ZERO(&write_set);
	FD_CLR(l_socket, &write_set);

	// task loop
	while (!l_stop_task)
	{
		// Wait up to five seconds
		timeout.tv_sec = 20;
		timeout.tv_usec = 0;//10000;
		
		// wait for character received
		select_result = select(l_socket + 1, &read_set, &write_set, NULL, &timeout);
		if (select_result > 0)
		{
			if (FD_ISSET(l_socket, &read_set))
			{
				FD_CLR(l_socket, &read_set);

				l_receive_buffer_length = recv(l_socket, l_receive_buffer, drvUDP_RECEIVER_BUFFER_LENGTH, 0);
				if (l_receive_buffer_length)
				{
					comUDPProcessReceivedPacket(l_receive_buffer, (uint8_t)l_receive_buffer_length);
				}	
				
			}
			if (FD_ISSET(l_socket, &write_set))
			{
				FD_CLR(l_socket, &write_set);
			}
		}
		else
		{
			if (select_result == 0)
			{
			}
			else
			{
				switch (errno)
				{
					case EINTR:
						break;
				}
			}	
		}
	
		// handle transmission request
		if (l_transmitter_buffer_length > 0 && l_transmitter_buffer_length < drvUDP_TRANSMITTER_BUFFER_LENGTH)
		{
			struct sockaddr_in dest;

			// send packet
			dest.sin_family = AF_INET;
			dest.sin_addr.s_addr = ntohl(l_transmitter_destination_address);
			dest.sin_port = htons(cfgGetUInt16Value(cfgVAL_WIFI_REMOTE));

			int a =sendto(l_socket, (const char*)l_transmitter_buffer, l_transmitter_buffer_length, 0, (const struct sockaddr*)&dest, sizeof(dest));

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
	close(l_socket);
	
	l_socket = -1;
}


static void drvUDPNotifyTask(void)
{
	pthread_kill(l_udp_task, SIGUSR1);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Returns true when connected to the network media
bool drvUDPIsConnected(void)
{
	return true;
}
