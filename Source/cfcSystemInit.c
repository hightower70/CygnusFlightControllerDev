/*****************************************************************************/
/* System initialization functinos                                           */
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
#include <comManager.h>
#include <comInterfaces.h>
#include <drvUART.h>
#include <halIODefinitions.h>
#include <comUDP.h>

/*****************************************************************************/
/* External functions                                                        */
/*****************************************************************************/
extern void drvEthernetInit(uint8_t in_uart_index);
extern void drvESP8266Init(void);

///////////////////////////////////////////////////////////////////////////////
// Initializes all system components
void sysInitialize(void)
{
	uint8_t i;

	for (i = 0; i < drvUART_MAX_COUNT; i++)
	{
		drvUARTInit(i);
	}

	comManagerInit();

	comUDPInit();


	//comESP8266Init();


	//drvEthernetInit();
	//comManagerInit();
	
	//UARTInit(0);
	//telemetryInit();
}

///////////////////////////////////////////////////////////////////////////////
// Creates all tasks needed for system 
void sysCreateTasks(void)
{
	//rtosCreateTask(telemetryTask, "Telemetry", rtos_DEFAULT_STACK_SIZE, rtosNULL, 1, rtosNULL);

	//rtosCreateTask(vHeartBeatTask, "HeartBeat", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
	//rtosCreateTask(ESP8266Thread, "ESP8266Thread", rtos_DEFAULT_STACK_SIZE, rtosNULL, 1, rtosNULL);

	//rtosStartScheduler();
}
