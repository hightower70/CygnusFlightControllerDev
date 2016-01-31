/*****************************************************************************/
/* Communication driver and GCS communication manager                        */
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
#include <comManager.h>
#include <sysRTOS.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define comManager_MAX_INTERFACE_NUMBER 4

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static bool l_task_stop = false;
static sysBinarySemaphore l_task_event;
static comInterfaceDescription g_com_interfaces[comManager_MAX_INTERFACE_NUMBER] = { 0 };


/*****************************************************************************/
/* Local function prototypes                                                 */
/*****************************************************************************/
void comManagerTask(void* in_param);
void comManagerTaskStop(void);

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initialize communication manager
void comManagerInit(void)
{
	uint32_t task_id;
	
	// initialize communication tasks
	sysBinarySemaphoreCreate(l_task_event);
	sysTaskCreate( comManagerTask, "comManager", sysDEFAULT_STACK_SIZE, sysNULL, 2, &task_id, comManagerTaskStop);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief  Adds interface to the list of active interfaces
/// @param in_interface Pointer to the interface description struct
void comAddInterface(comInterfaceDescription* in_interface)
{
	uint8_t interface_index;

	// find empty slot
	interface_index = 0;
	while (g_com_interfaces[interface_index].BufferRelease != sysNULL)
		interface_index++;

	if (interface_index < comManager_MAX_INTERFACE_NUMBER)
	{
		g_com_interfaces[interface_index] = *in_interface;
	}
	else
	{
		// TODO: Error handling
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Stops communication manager task
void comManagerTaskStop(void)
{
	l_task_stop = true;
	sysBinarySemaphoreUnlock(l_task_event);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Communicatio nmanager task function
void comManagerTask(void* in_param)
{
	sysBinarySemaphoreLock(l_task_event, sysINFINITE_TIMEOUT);

	// task loop
	while(sysBinarySemaphoreLock(l_task_event, sysINFINITE_TIMEOUT))
	{
		// stop task is requested
		if(l_task_stop)
			break;

	}
}


void comSendPacket(void)
{
}
