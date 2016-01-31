/*****************************************************************************/
/* Main Entry Function for Win32 application                                 */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <sysRTOS.h>

///////////////////////////////////////////////////////////////////////////////
// Constants
#define sysTASK_STOP_FUNCTION_COUNT 128

///////////////////////////////////////////////////////////////////////////////
// Module global variables
static sysTaskStopFunction l_task_stop_functions[sysTASK_STOP_FUNCTION_COUNT];
static uint8_t l_task_stop_function_count = 0;

///////////////////////////////////////////////////////////////////////////////
/// @brief Creates task under Win32
HANDLE sysWin32TaskCreate(sysTaskFunction in_task_code, const char* const in_task_name, uint16_t in_stack_size, void *in_parameters, uint8_t in_priority, uint32_t* out_thread_id, sysTaskStopFunction in_stop_function)
{
	sysAddThreadStopFunction(in_stop_function);

	return CreateThread(0, in_stack_size, (LPTHREAD_START_ROUTINE)in_task_code, in_parameters, 0, out_thread_id);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Store thread stop functions for system shutdown
void sysAddThreadStopFunction(sysTaskStopFunction in_stop_function)
{
	// store stop function
	if (in_stop_function != sysNULL && l_task_stop_function_count < sysTASK_STOP_FUNCTION_COUNT)
	{
		l_task_stop_functions[l_task_stop_function_count++] = in_stop_function;
	}

}

void sysShutdown(void)
{
	uint8_t i;

	// stop system
	for (i = 0; i < l_task_stop_function_count; i++)
	{
		if (l_task_stop_functions[i] != 0)
			(l_task_stop_functions[i])();
	}
}


uint8_t sysGetCPUUsage(void)
{
	FILETIME idle;
	FILETIME kernel;
	FILETIME user;
	BOOL val = GetSystemTimes(&idle, &kernel, &user);
	ULONGLONG lkernel = (((ULONGLONG)kernel.dwHighDateTime) << 32) + kernel.dwLowDateTime;
	ULONGLONG luser = (((ULONGLONG)user.dwHighDateTime) << 32) + user.dwLowDateTime;
	ULONGLONG lidle = (((ULONGLONG)idle.dwHighDateTime) << 32) + idle.dwLowDateTime;
	ULONGLONG lsys = lkernel + luser;
	uint8_t cpu = (uint8_t)((lsys - lidle) * 100 / lsys);

	return cpu;
}