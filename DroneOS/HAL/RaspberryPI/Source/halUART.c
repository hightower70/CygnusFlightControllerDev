/*****************************************************************************/
/* UART HAL Driver (Linux)                                                   */
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
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <termios.h>
#include <unistd.h>
#include <sysRTOS.h>
#include <halUART.h>
#include <halIODefinitions.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define drvUART_RECEIVER_BUFFER_LENGTH 512


/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/

/// UART driver info
typedef struct
{
	int FileDescriptor;
	pthread_t ReceiverThread;
	pthread_t TransmitterThread;
	halUARTConfigInfo Config;
	sysTaskNotify TransmitNotification;
} halUARTDriverInfo;

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static char* l_uart_names[halUART_MAX_COUNT] = halUART_INIT_NAMES;
static halUARTDriverInfo l_uart_info[halUART_MAX_COUNT] = { 0 };
static bool l_task_stop;

/*****************************************************************************/
/* Local functions                                                           */
/*****************************************************************************/
static void halUARTShutdown(void);
static sysTaskRetval halReceiverThread(sysTaskParam in_param);
static sysTaskRetval halTransmitterThread(sysTaskParam in_param);
static bool halUARTInitOneUART(uint8_t in_uart_index);
static void halUARTSignalHandler(int in_signum);

/*****************************************************************************/
/* UART Functions                                                            */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes UART driver
void halUARTInit(void)
{
	uint8_t i;
	
	l_task_stop = false;
	
	for (i = 0; i < halUART_MAX_COUNT; i++)
	{
		halUARTInitOneUART(i);
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Initialize one UART
static bool halUARTInitOneUART(uint8_t in_uart_index)
{
	halUARTDriverInfo* uart_info;
	bool success = true;
	struct termios port_settings;

	// sanity check
	if (in_uart_index >= halUART_MAX_COUNT)
		return false;

	uart_info = &l_uart_info[in_uart_index];

	// open UART
	uart_info->FileDescriptor = open(l_uart_names[in_uart_index], O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (uart_info->FileDescriptor == -1)
		success = false;

	// setup port
	if (success)
		tcgetattr(uart_info->FileDescriptor, &port_settings);
	
	cfsetispeed(&port_settings, B9600);	// set baud rates
	cfsetospeed(&port_settings, B9600);
	
	port_settings.c_cflag |= (CLOCAL | CREAD);				// enable the receiver and set local mode
	port_settings.c_cflag &= ~(PARENB | CSTOPB);			// set no parity, stop bits, data bits
	port_settings.c_cflag = (port_settings.c_cflag & ~CSIZE) | CS8; // set data length
	port_settings.c_lflag &= ~(ICANON | ECHO | ISIG); // Raw input mode
	port_settings.c_oflag = ~OPOST;
	
	port_settings.c_cc[VMIN] = 1;											// read minimum one character
	port_settings.c_cc[VTIME] = 0;										// non-blocking mode already set when device is opened
	
	if(success)
		tcsetattr(uart_info->FileDescriptor, TCSANOW, &port_settings);	
	
	if (success)
	{
		sysTaskNotifyCreate(uart_info->TransmitNotification);
	}

	// create receiver  thread
	if (success)
	{
		sysTaskCreate(halReceiverThread, "halUARTReceiverThread", sysDEFAULT_STACK_SIZE, uart_info, 2, &uart_info->ReceiverThread, halUARTShutdown);

		if (uart_info->ReceiverThread == sysNULL)
			success = false;
	}

	// create transmitter thread
	if (success)
	{
		sysTaskCreate(halTransmitterThread, "halUARTTransmitterThread", sysDEFAULT_STACK_SIZE, uart_info, 2, &uart_info->TransmitterThread, halUARTShutdown);

		if (uart_info->ReceiverThread == sysNULL)
			success = false;
	}

	return success;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Changes UART configuration
void halUARTConfig(uint8_t in_uart_index, halUARTConfigInfo* in_config_info)
{
	l_uart_info[in_uart_index].Config = *in_config_info;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sends block of data over the UART
/// @param in_uart_index Index of the UART
/// @param in_buffer Buffer containing data to send
/// @param in_buffer_length Number of bytes to send
/// @return True if send operation was started
bool halUARTSendBlock(uint8_t in_uart_index, uint8_t* in_buffer, uint16_t in_buffer_length)
{
	halUARTDriverInfo* uart_info = &l_uart_info[in_uart_index];

	// send data
	write(uart_info->FileDescriptor, in_buffer, in_buffer_length);
	sysTaskNotifyGive(uart_info->TransmitNotification);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
// UART Transmitter thread function
static sysTaskRetval halTransmitterThread(sysTaskParam in_param)
{
	halUARTDriverInfo* uart_info = (halUARTDriverInfo*)in_param;
	
	while (!l_task_stop)
	{
		sysTaskNotifyTake(uart_info->TransmitNotification, sysINFINITE_TIMEOUT);
		
		if (l_task_stop)
			break;
		
		tcdrain(uart_info->FileDescriptor);

		// call callback
		if (uart_info->Config.TxEmptyCallback != sysNULL)
		{
			uart_info->Config.TxEmptyCallback(sysNULL);
		}
	}
	
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes drvUARTConfigInfo struct
void halUARTConfigInfoInit(halUARTConfigInfo* in_config_info)
{
	sysMemZero(in_config_info, sizeof(halUARTConfigInfo));
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sets UART baud rate
/// @param in_uart_index UART index
/// @param in_baud_rate Baud rate
bool halUARTSetBaudRate(uint8_t in_uart_index, uint32_t in_baud_rate)
{
	halUARTDriverInfo* uart_info = &l_uart_info[in_uart_index];
	struct termios port_settings;
	speed_t speed;
	
	switch (in_baud_rate)
	{
		case 50:
			speed = B50;
			break;

		case 75:
			speed = B75;
			break;
			
		case 110:
			speed = B110;
			break;
			
		case 134:
			speed = B134;
			break;
			
		case 150:
			speed = B150;
			break;
			
		case 200:
			speed = B200;
			break;
			
		case 300:
			speed = B300;
			break;
			
		case 600:
			speed = B600;
			break;
			
		case 1200:
			speed = B1200;
			break;
			
		case 1800:
			speed = B1800;
			break;
			
		case 2400:
			speed = B2400;
			break;
			
		case 4800:
			speed = B4800;
			break;
			
		case 9600:
			speed = B9600;
			break;
			
		case 19200:
			speed = B19200;
			break;
			
		case 38400:
			speed = B38400;
			break;
			
		case 57600:
			speed = B57600;
			break;
			
		case 115200:
			speed = B115200;
			break;
			
		case 230400:
			speed = B230400;
			break;
			
		case 460800:
			speed = B460800;
			break;
			
		case 500000:
			speed = B500000;
			break;
			
		case 576000:
			speed = B576000;
			break;
			
		case 921600:
			speed = B921600;
			break;
			
		case 1000000:
			speed = B1000000;
			break;
			
		case 1152000:
			speed = B1152000;
			break;
			
		case 1500000:
			speed = B1500000;
			break;
			
		case 2000000:
			speed = B2000000;
			break;
			
		case 2500000 :
			speed = B2500000;
			break;
			
		case 3000000:
			speed = B3000000;
			break;
			
		case 3500000:
			speed = B3500000;
			break;
			
		case 4000000:
			speed = B4000000;
			break;
		
		default:
			return false;
	}

	// get current attributes
	tcgetattr(uart_info->FileDescriptor, &port_settings);
	
	cfsetispeed(&port_settings, speed);	// set baud rates
	cfsetospeed(&port_settings, speed);
	
	tcsetattr(uart_info->FileDescriptor, TCSANOW, &port_settings);	
	
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// UART Receiver Thread function
static sysTaskRetval halReceiverThread(sysTaskParam in_param)
{
	fd_set read_set;
	struct timeval timeout;
	int select_result;
	int recvpos;
	int recvbytes;
	uint8_t receiver_buffer[drvUART_RECEIVER_BUFFER_LENGTH];
	halUARTDriverInfo* uart_info = (halUARTDriverInfo*)in_param;

	FD_ZERO(&read_set);
	FD_SET(uart_info->FileDescriptor, &read_set);
	
	while (!l_task_stop)
	{
		// Wait up to five seconds
		timeout.tv_sec = 20;
		timeout.tv_usec = 0;
		
		// wait for character received
		select_result = select(uart_info->FileDescriptor + 1, &read_set, NULL, NULL, &timeout);
		if ( select_result > 0)
		{
			if (FD_ISSET(uart_info->FileDescriptor, &read_set))
			{
				recvbytes = read(uart_info->FileDescriptor, receiver_buffer, sizeof(receiver_buffer));
				recvpos = 0;
				if (uart_info->Config.RxReceivedCallback != sysNULL)
				{
					while (recvpos < recvbytes)
					{
						uart_info->Config.RxReceivedCallback(receiver_buffer[recvpos++], sysNULL);
					}
				}
				FD_CLR(uart_info->FileDescriptor, &read_set);
			}
		}
		else
		{
			if (select_result == 0)
			{
				
				
			}
		}
	}
	
#if 0
  HANDLE handles[2];
  DWORD event_mask = 0;
  DWORD wait_res;
  DWORD bytes_read;
	BYTE buffer;
	BOOL result;
	
	// registersignal handler
	signal(SIGTERM, halUARTSignalHandler);

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
				if (GetCommMask(uart_info->UARTHandle, &event_mask))
				{
					if ((event_mask & EV_RXCHAR) != 0)
					{
						// process received characters
						do
						{
							// read pending character
							ReadFile(uart_info->UARTHandle, &buffer, sizeof(buffer), &bytes_read, &uart_info->Overlapped);

							// process character
							if (bytes_read > 0 && uart_info->Config.RxReceivedCallback != sysNULL)
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
#endif
}

///////////////////////////////////////////////////////////////////////////////
// Cleanup function
static void halUARTShutdown(void)
{
	int uart_index;
	int thread_index;
	void* retval;
	
	l_task_stop = true;

	// signal threads to stop
	for (uart_index = 0; uart_index < halUART_MAX_COUNT; uart_index++)
	{
		if (l_uart_info[uart_index].ReceiverThread != sysNULL)
		{
			sysTaskNotifyGive(l_uart_info[uart_index].TransmitNotification);
			pthread_kill(l_uart_info[uart_index].ReceiverThread, SIGUSR1);
			pthread_kill(l_uart_info[uart_index].TransmitterThread, SIGUSR1);
		}
	}

	// wait for threads to stop
	for (uart_index = 0; uart_index < halUART_MAX_COUNT; uart_index++)
	{
		if (l_uart_info[uart_index].ReceiverThread != sysNULL)
		{
			pthread_join(l_uart_info[uart_index].ReceiverThread, &retval);			
		}
		
		if (l_uart_info[uart_index].TransmitterThread != sysNULL)
		{
			pthread_join(l_uart_info[uart_index].ReceiverThread, &retval);			
		}

		l_uart_info[uart_index].ReceiverThread = sysNULL;
		l_uart_info[uart_index].TransmitterThread = sysNULL;
	}
	
	// close UARTs
	for (uart_index = 0; uart_index < halUART_MAX_COUNT; uart_index++)
	{
		if (l_uart_info[uart_index].FileDescriptor != -1)
		{
			close(l_uart_info[uart_index].FileDescriptor);
			l_uart_info[uart_index].FileDescriptor =  -1;
		}
	}
}
