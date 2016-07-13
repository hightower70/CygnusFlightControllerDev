/*****************************************************************************/
/* HAL helper functions                                                      */
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
#include <stdio.h>

#pragma comment(lib,"Winmm.lib")

///////////////////////////////////////////////////////////////////////////////
// Constants
#define sysTASK_STOP_FUNCTION_COUNT 128

///////////////////////////////////////////////////////////////////////////////
// Module global variables
static sysTaskStopFunction l_task_stop_functions[sysTASK_STOP_FUNCTION_COUNT];
static uint8_t l_task_stop_function_count = 0;

///////////////////////////////////////////////////////////////////////////////
/// @brief Creates task under Win32
uint32_t sysWin32TaskCreate(sysTaskFunction in_task_code, const char* const in_task_name, uint16_t in_stack_size, void *in_parameters, uint8_t in_priority, sysTask* out_thread_handle, sysTaskStopFunction in_stop_function)
{
	DWORD thread_id;

	sysUNUSED(in_task_name);
	sysUNUSED(in_priority); // TODO: implement priority

	sysAddThreadStopFunction(in_stop_function);

	*out_thread_handle = CreateThread(0, in_stack_size, (LPTHREAD_START_ROUTINE)in_task_code, in_parameters, 0, &thread_id);

	return thread_id;
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

///////////////////////////////////////////////////////////////////////////////
/// @brief Shuts down system
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

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets program CPU usage
/// @return Returns CPU usage in percentage
uint8_t sysGetCPUUsage(void)
{
	static DWORD prev_timestamp = 0;
	DWORD timestamp = timeGetTime();
	FILETIME process_create, process_exit, process_user, process_kernel;
	uint8_t cpu_usage;
	DWORD diff_time;
	LONG kernel, user;
	static LONG prev_kernel = 0, prev_user = 0;
	SYSTEMTIME system_time;

	// get process time
	GetProcessTimes(GetCurrentProcess(), &process_create, &process_exit, &process_user, &process_kernel);

	// convert kernel time
	FileTimeToSystemTime(&process_kernel, &system_time);

	kernel = system_time.wSecond * 1000;
	kernel += system_time.wMilliseconds;

	// convert user time
	FileTimeToSystemTime(&process_user, &system_time);

	user = system_time.wSecond * 1000;
	user += system_time.wMilliseconds;

	diff_time = timestamp - prev_timestamp;

	if (diff_time > 0)
		cpu_usage = (uint8_t)(((((kernel - prev_kernel) + (user - prev_user)) * 100) / diff_time));
	else
		cpu_usage = 0;

	prev_timestamp = timestamp;
	prev_kernel = kernel;
	prev_user = user;

	return cpu_usage;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Debug printf function
/// @param fmt Format string
void sysDebugPrint(const char *fmt, ...) 
{
	char str[1024];

	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(str, sizeof(str), fmt, argptr);
	va_end(argptr);

	OutputDebugStringA(str);
}
