/*****************************************************************************/
/* HAL helper functions (Linux)                                              */
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
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/resource.h> 

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define sysTASK_STOP_FUNCTION_COUNT 128

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static pthread_mutex_t l_critical_section_mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP; // This is the critical section object
static sysTaskStopFunction l_task_stop_functions[sysTASK_STOP_FUNCTION_COUNT];
static uint8_t l_task_stop_function_count = 0;
static struct timeval l_prev_user_time;
static struct timeval l_prev_system_time;


/*****************************************************************************/
/* Local function declaration                                                */
/*****************************************************************************/
static void halUser1SignalHandler(int in_signum);


/*****************************************************************************/
/* Function implementations                                                  */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes HAL layer
void halInitialize(void)
{
	timerclear(&l_prev_user_time);
	timerclear(&l_prev_system_time);
	
	signal(SIGUSR1, halUser1SignalHandler);	
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Deinitializes HAL layer (releases all resources)
void halDeinitialize(void)
{
	
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Enters int ocritical section
void sysCriticalSectionBegin(void)
{
	// Enter the critical section -- other threads are locked out
	pthread_mutex_lock(&l_critical_section_mutex);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Leavs critical section
void sysCriticalSectionEnd(void)
{
  // Leave the critical section -- other threads can now pthread_mutex_lock()
	pthread_mutex_unlock(&l_critical_section_mutex);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Creates task under Win32
uint32_t sysLinuxTaskCreate(sysTaskFunction in_task_code, const char* const in_task_name, uint16_t in_stack_size, void *in_parameters, uint8_t in_priority, sysTask* out_thread_handle, sysTaskStopFunction in_stop_function)
{
	sysUNUSED(in_task_name);
	sysUNUSED(in_priority); // TODO: implement priority

	sysAddThreadStopFunction(in_stop_function);
	
	pthread_create(out_thread_handle, NULL, in_task_code, in_parameters);

	return 0;
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
	static sysTick prev_timestamp = 0;
	sysTick timestamp = sysGetSystemTick();
	uint8_t cpu_usage;
	sysTick diff_time;
	struct rusage usage;
	struct timeval user_time;
	struct timeval system_time;
	uint32_t user_time_in_ms;
	uint32_t system_time_in_ms;
		
	// get process time
	if (getrusage(RUSAGE_SELF, &usage) == 0)
	{
		if (timerisset(&l_prev_user_time) && timerisset(&l_prev_system_time))
		{
			timersub(&usage.ru_utime, &l_prev_user_time, &user_time);		
			timersub(&usage.ru_stime, &l_prev_system_time, &system_time);	
			
			user_time_in_ms = (user_time.tv_sec * 1000) + (user_time.tv_usec / 1000);
			system_time_in_ms = (system_time.tv_sec * 1000) + (system_time.tv_usec / 1000);

			diff_time = timestamp - prev_timestamp;
			
			if (diff_time == 0)
				cpu_usage = 0;
			else
				cpu_usage = (uint8_t)((user_time_in_ms + system_time_in_ms) * 100 / diff_time);
		}
	}
	else
	{
		cpu_usage = 0;
	}
	
	l_prev_system_time = usage.ru_stime;
	l_prev_user_time = usage.ru_utime;
	prev_timestamp =  timestamp;
	
	return cpu_usage;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets system timer current value in ms (current timestamp)
/// @return System timer value in ms
sysTick sysGetSystemTick(void)
{
	struct timeval te; 
	sysTick milliseconds;
	
	gettimeofday(&te, NULL); // get current time
	milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000; // caculate milliseconds

	return milliseconds;
}	

///////////////////////////////////////////////////////////////////////////////
/// @brief Waits for the given time in ms
/// @return Delay time in ms
void sysDelay(uint32_t in_delay_in_ms)
{
	usleep(in_delay_in_ms * 1000);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Converts millisec to timespec struct
/// @param in_ms time in millisec to convert
/// @param out_ts timespec to receive the converted time
static void sysMillisecToTimespec(unsigned long in_ms, struct timespec *out_ts)
{
	out_ts->tv_sec = in_ms / 1000;
	out_ts->tv_nsec = (in_ms % 1000) * 1000000;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Takes (locks) mutex with given timeout
/// @param in_mutex Pointer to the mutex object
/// @param in_timeout Timeout on ms
/// @return True if mutex was successfully taken or false if timeout occured without locking mutex
bool pthread_mutex_take(pthread_mutex_t* in_mutex, uint32_t in_timeout)
{
	struct timespec tv;
	
	// convert itmeout from ms to timespec
	sysMillisecToTimespec(in_timeout, &tv);
	
	// lock mutex
	if (in_timeout == sysINFINITE_TIMEOUT)
		return pthread_mutex_lock(in_mutex);
	else
		return pthread_mutex_timedlock(in_mutex, &tv) == 0;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes binary semaphore
/// @param in_binary_semaphore Semaphore to initialize
/// @param in_init_value Init value (0 or 1)
void pthread_binsem_init(pthread_binsem_t* in_binary_semaphore, int in_init_value)
{
	pthread_mutex_init(&in_binary_semaphore->mutex, NULL);
	pthread_cond_init(&in_binary_semaphore->cvar, NULL);
	
	in_binary_semaphore->v = in_init_value;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Deletes binary semaphore
/// @param in_binary_semaphore Semaphore to delete
void pthread_binsem_destroy(pthread_binsem_t* in_binary_semaphore)
{
	pthread_cond_destroy(&in_binary_semaphore->cvar);
	pthread_mutex_destroy(&in_binary_semaphore->mutex);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Acquires lock on the given binary semaphore
/// @param in_binary_semapahore Semaphore to lock
/// @param in_timeout TImeout until the lockmust be acquired otherwise timeout will be occured
void pthread_binsem_lock(pthread_binsem_t* in_binary_semaphore, uint32_t in_timeout)
{
	struct timespec tv;
	int retval = 0;
	
	// convert itmeout from ms to timespec
	sysMillisecToTimespec(in_timeout, &tv);

	pthread_mutex_lock(&in_binary_semaphore->mutex);
	
	while (in_binary_semaphore->v == 0 && retval == 0)
	{
		if (in_timeout == sysINFINITE_TIMEOUT)
		{
			pthread_cond_wait(&in_binary_semaphore->cvar, &in_binary_semaphore->mutex);
			in_binary_semaphore->v = 0;
		}
		else
		{
			retval = pthread_cond_timedwait(&in_binary_semaphore->cvar, &in_binary_semaphore->mutex, &tv);
			switch (retval)
			{
				case ETIMEDOUT:
					break;
					
				default:
					in_binary_semaphore->v = 0;
					break;
			}
				break;
		}
	}
	
	pthread_mutex_unlock(&in_binary_semaphore->mutex);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Unlock (releases) binary semaphore
/// @param in_binary_semaphore Semaphore to unlock
void pthread_binsem_unlock(pthread_binsem_t* in_binary_semaphore)
{
	pthread_mutex_lock(&in_binary_semaphore->mutex);
	in_binary_semaphore->v = 1;
	pthread_cond_signal(&in_binary_semaphore->cvar);
	pthread_mutex_unlock(&in_binary_semaphore->mutex);	
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

	fprintf(stderr, "%s", str);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Empty signal handler
static void halUser1SignalHandler(int in_signum)
{
	sysUNUSED(in_signum);
}
