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

/*****************************************************************************/
/* External functions                                                        */
/*****************************************************************************/
extern void drvEthernetInit(void);


///////////////////////////////////////////////////////////////////////////////
// Initializes all system components
void sysInitialize(void)
{
	drvEthernetInit();
	comManagerInit();
	
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
