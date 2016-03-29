/*****************************************************************************/
/* UART Driver                                                               */
/*                                                                           */
/* Copyright (C) 2014 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <windows.h>
#include <tchar.h>
#include <sysRTOS.h>
#include <drvUART.h>
#include <halIODefinitions.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/


/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/

/// UARt driver info
typedef struct
{
	HANDLE UARTHandle;
	HANDLE UARTThread;
	HANDLE UARTEvent;
	OVERLAPPED Overlapped;
	drvUARTConfigInfo Config;
} drvUARTDriverInfo;

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static TCHAR* l_uart_names[drvUART_MAX_COUNT] = drvUART_INIT_NAMES;
static volatile bool l_task_stop = false;
static HANDLE l_system_is_running_event = NULL;

static drvUARTDriverInfo l_uart_info[drvUART_MAX_COUNT] = { 0 };

/*****************************************************************************/
/* Local functions                                                           */
/*****************************************************************************/
static void drvUARTShutdown(void);
static void drvUARTThread(void* in_param);

/*****************************************************************************/
/* UART Functions                                                            */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes UART driver
void drvUARTInit(uint8_t in_uart_index)
{
	BOOL success = TRUE;
	DCB comm_state;
	COMMTIMEOUTS time;
	drvUARTDriverInfo* uart_info;

	if (in_uart_index >= drvUART_MAX_COUNT)
		return;


	if (l_system_is_running_event == NULL)
	{
		l_system_is_running_event = CreateEvent(NULL, TRUE, FALSE, NULL);
	}

	uart_info = &l_uart_info[in_uart_index];

	uart_info->UARTHandle = CreateFile(l_uart_names[in_uart_index],
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_OVERLAPPED,
		0);

	if (uart_info->UARTHandle == INVALID_HANDLE_VALUE)
		success = FALSE;

	// setup port
	if (success)
		success = GetCommState(uart_info->UARTHandle, &comm_state);

	comm_state.BaudRate = 9600;
	comm_state.ByteSize = 8;
	comm_state.Parity = NOPARITY;
	comm_state.StopBits = ONESTOPBIT;

	if (success)
		success = SetCommState(uart_info->UARTHandle, &comm_state);

	// set timeouts
	if (success)
		success = GetCommTimeouts(uart_info->UARTHandle, &time);

	time.ReadIntervalTimeout = MAXDWORD;
	time.ReadTotalTimeoutMultiplier = 0;
	time.ReadTotalTimeoutConstant = 0;
	time.WriteTotalTimeoutConstant = 0;
	time.WriteTotalTimeoutMultiplier = 0;

	if (success)
		success = SetCommTimeouts(uart_info->UARTHandle, &time);

	// setup queue
	if (success)
		success = SetupComm(uart_info->UARTHandle, 1024, 1024);

	// set event mask
	if (success)
		SetCommMask(uart_info->UARTHandle, EV_RXCHAR | EV_TXEMPTY);

	// create UART thread
	if (success)
	{
		uart_info->UARTThread = sysWin32TaskCreate(drvUARTThread, "drvUARTThread", sysDEFAULT_STACK_SIZE, uart_info, 2, NULL, drvUARTShutdown);

		if (uart_info->UARTThread == NULL)
			success = FALSE;
	}

	if (success)
	{
		// UART event
		uart_info->UARTEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

		// create read overlapped struct
		memset(&uart_info->Overlapped, 0, sizeof(&uart_info->Overlapped));
		uart_info->Overlapped.hEvent = uart_info->UARTEvent;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Changes UART configuration
void drvUARTConfig(uint8_t in_uart_index, drvUARTConfigInfo* in_config_info)
{
	l_uart_info[in_uart_index].Config = *in_config_info;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes drvUARTConfigInfo struct
void drvUARTConfigInfoInit(drvUARTConfigInfo* in_config_info)
{
	sysMemZero(in_config_info, sizeof(drvUARTConfigInfo));
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sets UART baud rate
/// @param in_uart_index UART index
/// @param in_baud_rate Baud rate
bool drvUARTSetBaudRate(uint8_t in_uart_index, uint32_t in_baud_rate)
{
	drvUARTDriverInfo* uart_info = &l_uart_info[in_uart_index];
  BOOL success = TRUE;
  DCB comm_state;

	// setup port
  if( success )
		success = GetCommState( uart_info->UARTHandle, &comm_state );

	comm_state.BaudRate = in_baud_rate;

  if( success )
    success = SetCommState( uart_info->UARTHandle, &comm_state );

	return success;
}

///////////////////////////////////////////////////////////////////////////////
// Send byte on UART
/*
void UART1SendCharacter( dosChar in_byte )
{
  DWORD bytes_written;

  WriteFile( l_uart1_handle, &in_byte, sizeof(in_byte), &bytes_written, &l_uart1_overlapped);
}
	*/

///////////////////////////////////////////////////////////////////////////////
/// @brief Sends block of data over the UART
/// @param in_uart_index Index of the UART
/// @param in_buffer Buffer containing data to send
/// @param in_buffer_length Number of bytes to send
void drvUARTSendBlock(uint8_t in_uart_index, uint8_t* in_buffer, uint16_t in_buffer_length)
{
	drvUARTDriverInfo* uart_info = &l_uart_info[in_uart_index];
	DWORD bytes_written;

	// send data
	WriteFile( uart_info->UARTHandle, in_buffer, in_buffer_length, &bytes_written, &uart_info->Overlapped);
}

///////////////////////////////////////////////////////////////////////////////
// UART1 Thread function
static void drvUARTThread(void* in_param)
{
  HANDLE handles[2];
  DWORD event_mask = 0;
  DWORD wait_res;
  DWORD bytes_read;
	BYTE buffer;
	BOOL result;
	drvUARTDriverInfo* uart_info = (drvUARTDriverInfo*)in_param;

  // store handles
  handles[0] = l_system_is_running_event;
	handles[1] = uart_info->UARTEvent;


	while(!l_task_stop)
	{
    // start overlapped comm event watching
		result = WaitCommEvent( uart_info->UARTHandle, &event_mask, &uart_info->Overlapped);

    // wait for eighter thread stop or communication event
    wait_res = WaitForMultipleObjects(2, handles, FALSE, INFINITE);

		switch(wait_res)
		{
			// thread exit event
			case WAIT_OBJECT_0:
				// do nothing
				break;

			// character received event
			case WAIT_OBJECT_0+1:
				//if (GetCommMask(uart_info->UARTHandle, &event_mask))
				{
					if ((event_mask & EV_RXCHAR) != 0)
					{
						// process received characters
						do
						{
							// read pending character
							ReadFile(uart_info->UARTHandle, &buffer, sizeof(buffer), &bytes_read, &uart_info->Overlapped);

							// process character
							if (bytes_read > 0 && uart_info->Config.RxReceivedCallback != NULL)
								uart_info->Config.RxReceivedCallback(buffer, sysNULL);
						} while (bytes_read > 0);
					}

					if ((event_mask & EV_TXEMPTY) != 0)
					{
						if (uart_info->Config.TxEmptyCallback != sysNULL)
						{
							uart_info->Config.TxEmptyCallback(sysNULL);
						}
					}
				}
				break;
		}
  }

	// cleanup
	uart_info->UARTThread = NULL;

	CloseHandle(uart_info->UARTEvent);
	uart_info->UARTEvent = NULL;

	CloseHandle(uart_info->UARTHandle);
	uart_info->UARTHandle = NULL;

  // exit thread
  ExitThread( 0 );
}

///////////////////////////////////////////////////////////////////////////////
// Cleanup function
static void drvUARTShutdown(void)
{
  HANDLE threads[drvUART_MAX_COUNT];
  int uart_index;
	int thread_index;
	DWORD result;

	// create thread list
	thread_index = 0;
	for (uart_index = 0; uart_index < drvUART_MAX_COUNT; uart_index++)
	{
		if (l_uart_info[uart_index].UARTThread != NULL)
			threads[thread_index++] = l_uart_info[uart_index].UARTThread;
	}

	if (thread_index == 0)
		return;

	// stop threads
	l_task_stop = true;
	SetEvent(l_system_is_running_event);

  // wait for thread stop
	result = WaitForMultipleObjects(thread_index, threads, TRUE, 1000);
  
  // force releasing resources
	for(uart_index = 0; uart_index < drvUART_MAX_COUNT; uart_index++)
	{
		// close thread
		if(l_uart_info[uart_index].UARTThread != NULL)
		{
			CloseHandle(l_uart_info[uart_index].UARTThread );
			l_uart_info[uart_index].UARTThread = NULL;
		}

		// destroy event 
		if(l_uart_info[uart_index].UARTEvent != NULL)
		{
			CloseHandle(l_uart_info[uart_index].UARTEvent);
			l_uart_info[uart_index].UARTEvent = NULL;
		}

	  // close UART
		if(l_uart_info[uart_index].UARTHandle != NULL)
		{
			CloseHandle(l_uart_info[uart_index].UARTHandle);
			l_uart_info[uart_index].UARTHandle = NULL;
		}
	}
}
