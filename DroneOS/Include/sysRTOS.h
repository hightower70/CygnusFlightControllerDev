/*****************************************************************************/
/* RTOS Wrapper functions                                                    */
/*                                                                           */
/* Copyright (C) 2014-2015 Laszlo Arvai                                      */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/
#ifndef __sysRTOS_h
#define __sysRTOS_h

///////////////////////////////////////////////////////////////////////////////
// RTOS Independent includes
#include <sysTypes.h>

///////////////////////////////////////////////////////////////////////////////
// RTOS independent functions
uint32_t rtosGetSystemTickSince(uint32_t in_start_tick);
uint16_t rtosGetCPUUsage(void);

///////////////////////////////////////////////////////////////////////////////
// RTOS Dependent includes

/*****************************************************************************/
/* Windows wrapper                                                           */
/*****************************************************************************/
#ifdef _WIN32

///////////////////////////////////////////////////////////////////////////////
// Windows specific includes
#include <Windows.h>

///////////////////////////////////////////////////////////////////////////////
// Windows constants
#define sysDEFAULT_STACK_SIZE 0

///////////////////////////////////////////////////////////////////////////////
// Tyoes
typedef void (*sysTaskStopFunction)(void);
typedef void (*sysTaskFunction)(void*);

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
//CreateThread(0, stacksize, (LPTHREAD_START_ROUTINE)taskfunc, param, 0, handle, stopfunction)
HANDLE sysWin32TaskCreate(sysTaskFunction in_task_code, const char* const in_task_name, uint16_t in_stack_size, void *in_parameters, uint8_t in_priority, uint32_t* out_thread_id, sysTaskStopFunction in_stop_function);
void sysShutdown(void);
void sysAddThreadStopFunction(sysTaskStopFunction in_stop_function);

///////////////////////////////////////////////////////////////////////////////
// Windows wrappers
#define sysGetSystemTick() GetTickCount()
#define sysDelay(x) Sleep(x)
#define sysStartScheduler()

#define sysTaskCreate(taskfunc, taskname, stacksize, param, priority, handle, stopfunction) 	sysWin32TaskCreate(taskfunc, taskname, stacksize, param, priority, handle, stopfunction) 	

// Mutex related definitions
typedef HANDLE rtosMutexHandle_t;

#define sysINFINITE_TIMEOUT INFINITE

#define rtosCreateMutex(x) x = CreateMutex( NULL, FALSE, NULL)
#define rtosDeleteMutex(x) CloseHandle(x)
#define rtosRequestMutex(x,t) ((WaitForSingleObject(x, t) == WAIT_OBJECT_0) ? true : false)
#define rtosReleaseMutex(x) ReleaseMutex(x)
#define rtosGiveMutexFromISR(x) ReleaseMutex(x)

typedef HANDLE sysBinarySemaphore;
#define sysBinarySemaphoreCreate(x) x = CreateSemaphore( NULL, 1, 1, NULL)
#define sysBinarySemaphoreDelete(x) CloseHandle(x)
#define sysBinarySemaphoreLock(x,t) ((WaitForSingleObject(x, t) == WAIT_OBJECT_0) ? true : false)
#define sysBinarySemaphoreUnlock(x) ReleaseSemaphore( x, 1, NULL)
#define sysBinarySemaphoreUnlockFromISR(x) ReleaseSemaphore( x, 1, NULL)


#else

/*****************************************************************************/
/* FreeRTOS wrapper                                                          */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// FreeRTOS specific includes
#include <FreeRTOS.h>
#include <portmacro.h>
#include <semphr.h>
#include <task.h>

///////////////////////////////////////////////////////////////////////////////
// Constants
#define rtos_DEFAULT_STACK_SIZE configMINIMAL_STACK_SIZE

///////////////////////////////////////////////////////////////////////////////
// Types
typedef TaskHandle_t rtosTaskHandle_t;

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
typedef void (*TaskFunction_t)(void*);

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
uint32_t rtosCreateTask(TaskFunction_t pvTaskCode, const char * const pcName, uint16_t usStackDepth, void *pvParameters, UBaseType_t uxPriority, TaskHandle_t *pvCreatedTask );

///////////////////////////////////////////////////////////////////////////////
// FreeRTOS wrappers
#define rtosDelay(x) vTaskDelay((portTickType) (x) / portTICK_RATE_MS)
#define rtosStartScheduler() vTaskStartScheduler()
#define rtosShutdown()
#define rtosTaskLoop() 1

#define rtosGetSystemTick() xTaskGetTickCount()

// Semaphore related definitions
typedef SemaphoreHandle_t rtosSemaphoreHandle_t;
#define rtosCreateBinarySemaphore(x) vSemaphoreCreateBinary(x)

#endif

#endif
