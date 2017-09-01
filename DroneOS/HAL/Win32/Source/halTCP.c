/*****************************************************************************/
/* Ethernet Communciation Driver                                             */
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
#include <stdio.h>
#include <winsock2.h>
#include <sysTypes.h>
#include <sysRTOS.h>
#include <comSLIP.h>
#include <comInterfaces.h>
#include <comManager.h>
#include <sysCRC16.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define drvETH_INVALID_PACKET_BUFFER_INDEX 0xff
#define drvETH_RECEIVER_BUFFER_LENGTH 512
#define drvETH_PACKET_BUFFER_LENGTH 256
#define drvETH_PACKET_BUFFER_COUNT 4

/*****************************************************************************/
/* Module local variables                                                    */
/*****************************************************************************/
static HANDLE l_socket_thread_handle = NULL;
static SOCKET l_server_socket = INVALID_SOCKET;
static SOCKET l_socket = INVALID_SOCKET; 
static volatile bool l_task_stop = false;
static uint8_t l_receive_buffer[drvETH_RECEIVER_BUFFER_LENGTH];
static uint16_t l_receive_buffer_length;
static uint16_t l_receive_buffer_pos;
static uint8_t l_receive_packet_expected_length;
static uint16_t l_receive_packet_calculated_crc;
static uint8_t l_packet_buffer[drvETH_PACKET_BUFFER_COUNT][drvETH_PACKET_BUFFER_LENGTH];
static uint8_t l_packet_buffer_length[drvETH_PACKET_BUFFER_COUNT];
static uint8_t l_current_packet_buffer_index;
static sysBinarySemaphore l_buffer_released_event;
static slipDecoderState l_slip_decoder;

static uint8_t l_interface_index;

/*****************************************************************************/
/* Local functions                                                           */
/*****************************************************************************/
static void drvEthernetThread(void* lpParam);
static void drvEthernetShutdown(void);

/*****************************************************************************/
/* Functions implementation                                                  */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// @brief Initializes ethernet communication service on WIN32
void drvEthernetInit(void)
{
	WSADATA wsa;
	uint32_t task_id;
	comInterfaceDescription interface_description;
  
	// init socket
  if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
  {
      printf("Failed. Error Code : %d",WSAGetLastError());
      exit(1);
  }

	// add this interface to the list of communication interfaces
	l_interface_index = comAddInterface(&interface_description);

	// create thread for socket communication
	sysBinarySemaphoreCreate(l_buffer_released_event);
	sysBinarySemaphoreLock(l_buffer_released_event, 0);
	l_socket_thread_handle = sysWin32TaskCreate(drvEthernetThread, "drvEthernetThread", sysDEFAULT_STACK_SIZE, sysNULL, 2, &task_id, drvEthernetShutdown);
}

///////////////////////////////////////////////////////////////////////////////
// @brief Shots down ethernet communication service
void drvEthernetShutdown(void)
{
	SOCKET socket;

	// stops communication thread
	if(l_socket_thread_handle != NULL)
	{
		// signal thread to stop
		l_task_stop = true;

		// release lock in order to run the thread
		sysBinarySemaphoreUnlock(l_buffer_released_event);

		// shut down client socket
		if(l_socket != INVALID_SOCKET)
			shutdown(l_socket, SD_BOTH);

		// wait for thread exit
		if (WaitForSingleObject(l_socket_thread_handle, 100) != WAIT_OBJECT_0)
		{
			// close the socket to stop thread if its locked in accept function
			socket = l_server_socket;
			l_server_socket = INVALID_SOCKET;
			closesocket(socket);
		}

		// wait for thread exit
		if (WaitForSingleObject(l_socket_thread_handle, 100) != WAIT_OBJECT_0)
		{
			// force thread to terminate
			TerminateThread(l_socket_thread_handle, 0);
		}

		l_socket_thread_handle = NULL;
	}

	// closes socket if it is still opened
	if (l_server_socket != INVALID_SOCKET)
	{
		closesocket(l_server_socket);
		l_server_socket = INVALID_SOCKET;
	}

	if (l_socket != INVALID_SOCKET)
	{
		closesocket(l_socket);
		l_socket = INVALID_SOCKET;
	}

	// clean up sockets
	WSACleanup();
}

///////////////////////////////////////////////////////////////////////////////
// @brief Sends telemetry packet over socket
// @param in_buffer pointer to the byte buffer to send
// @param in_buffer_length Number of bytes to send
void drvEthernetSendData(uint8_t* in_buffer, uint16_t in_buffer_length)
{
	// check if socket is connected
	if(l_server_socket == INVALID_SOCKET)
		return;

	if( send(l_server_socket, (const char*)in_buffer, in_buffer_length, 0) == SOCKET_ERROR)
		l_server_socket = INVALID_SOCKET;
}

///////////////////////////////////////////////////////////////////////////////
// Telemetry socket thread functions
void drvEthernetThread(void* lpParam)
{
	int recv_result;
	struct sockaddr_in server, client;
	int size = sizeof(client);
	SOCKET sckt;
	uint8_t packet_buffer_index;
	
	// initialize
	l_current_packet_buffer_index = drvETH_INVALID_PACKET_BUFFER_INDEX;
	for (packet_buffer_index = 0; packet_buffer_index < drvETH_PACKET_BUFFER_COUNT; packet_buffer_index++)
	{
		l_packet_buffer_length[packet_buffer_index] = 0;
	}
	l_receive_buffer_length = 0;
	slipDecodeInitialize(&l_slip_decoder);

  //Create a socket
	if((l_server_socket = socket(AF_INET, SOCK_STREAM, 0 )) == INVALID_SOCKET)
  {
		printf("Communication: Could not create socket : %d\n", WSAGetLastError());
		return;
  }

	//Prepare the sockaddr_in structure
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(8888);
     
  //Bind
	if( bind(l_server_socket, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
  {
		printf("Communication: Bind failed with error code : %d\n", WSAGetLastError());
    return;
  }

	//Listen to incoming connections
	if(listen(l_server_socket, SOMAXCONN) == SOCKET_ERROR)
	{
		printf("Communication: Listen function failed with error: %d\n", WSAGetLastError());
    return;
	}

	// task loop
	while (!l_task_stop)
	{
		// accept incoming connection (only one client)
		if (l_socket == INVALID_SOCKET)
		{
			l_socket = accept(l_server_socket, (struct sockaddr*)&client, &size);

			l_receive_buffer_length = 0;
			l_receive_buffer_pos = 0;

			slipDecodeInitialize(&l_slip_decoder);
		}

		if (l_socket != INVALID_SOCKET)
		{
			// allocate buffer if there is no current buffer
			if (l_current_packet_buffer_index == drvETH_INVALID_PACKET_BUFFER_INDEX)
			{
				do
				{
					packet_buffer_index = 0;
					while (packet_buffer_index < drvETH_PACKET_BUFFER_COUNT && l_packet_buffer_length[packet_buffer_index] != 0)
					{
						packet_buffer_index++;
					}

					// if there is no buffer wait until it is released
					if (packet_buffer_index >= drvETH_PACKET_BUFFER_COUNT)
					{
						sysBinarySemaphoreLock(l_buffer_released_event, sysINFINITE_TIMEOUT);
					}
				} while (packet_buffer_index >= drvETH_PACKET_BUFFER_COUNT && !l_task_stop);

				// init buffer
				l_current_packet_buffer_index = packet_buffer_index;
				l_slip_decoder.TargetBuffer = &l_packet_buffer[l_current_packet_buffer_index][0];
				l_slip_decoder.TargetBufferPos = 0;
				l_slip_decoder.CyclicRedundancyCheck = crc16_INIT_VALUE;
				l_slip_decoder.Status = slip_ES_Idle;
			}

			// receive data
			if (l_receive_buffer_length == 0)
			{
				recv_result = recv(l_socket, l_receive_buffer, sizeof(l_receive_buffer), 0);
				if (recv_result > 0)
				{
					l_receive_buffer_length = recv_result;
					comManagerProcessReceivedData(l_interface_index, l_receive_buffer, l_receive_buffer_length);
				}
				else
				{
					sckt = l_socket;
					l_socket = INVALID_SOCKET;
					closesocket(sckt);
				}
			}
		}
	}

	// exit from thread
	ExitThread(0);
}



#if 0
// set it non blocking
if (l_server_socket != INVALID_SOCKET)
{
	u_long iMode = O_NONBLOCK;
	ioctlsocket(l_server_socket, FIONBIO, &iMode);
}

#define O_NONBLOCK 1



///////////////////////////////////////////////////////////////////////////////
// @brief Checks if telemetry is ready for sending new packet
// @return True is sender is ready
bool telemetryIsConnected(void)
{
	return l_server_socket != INVALID_SOCKET;
}

///////////////////////////////////////////////////////////////////////////////
// @brief Waits until telemety sender is ready to send new packet
// @return True is sender is ready
void telemetryWaitForSenderReady(void)
{
}


#endif