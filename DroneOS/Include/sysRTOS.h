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

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>
#include <string.h>

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/
typedef uint32_t sysTick;

/*****************************************************************************/
/* RTOS independent function prototypes                                      */
/*****************************************************************************/
uint32_t sysGetSystemTickSince(sysTick in_start_tick);
uint8_t sysGetCPUUsage(void);

#define sysMemZero(dst, siz)  memset(dst, 0, siz)
#define sysMemCopy(dst, src, siz) memcpy(dst, src, siz)

/*****************************************************************************/
/* RTOS dependent function prototypes                                        */
/*****************************************************************************/

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
// Types
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

void sysCriticalSectionBegin(void);
void sysCriticalSectionEnd(void);

#define sysASSERT(x)

#define sysTaskCreate(taskfunc, taskname, stacksize, param, priority, handle, stopfunction) 	sysWin32TaskCreate(taskfunc, taskname, stacksize, param, priority, handle, stopfunction) 	


#define sysINFINITE_TIMEOUT INFINITE

// Mutex related definitions
typedef HANDLE sysMutex;
#define sysMutexCreate(x) x = CreateMutex( NULL, FALSE, NULL)
#define sysMutexDelete(x) CloseHandle(x)
#define sysMutexTake(x,t) ((WaitForSingleObject(x, t) == WAIT_OBJECT_0) ? true : false)
#define sysMutexGive(x) ReleaseMutex(x)

typedef HANDLE sysBinarySemaphore;
#define sysBinarySemaphoreCreate(x) x = CreateSemaphore( NULL, 1, 1, NULL)
#define sysBinarySemaphoreDelete(x) CloseHandle(x)
#define sysBinarySemaphoreLock(x,t) ((WaitForSingleObject(x, t) == WAIT_OBJECT_0) ? true : false)
#define sysBinarySemaphoreUnlock(x) ReleaseSemaphore( x, 1, NULL)
#define sysBinarySemaphoreUnlockFromISR(x) ReleaseSemaphore( x, 1, NULL)

typedef HANDLE sysTaskNotify;
#define sysTaskNotifyCreate(x) x = CreateSemaphore( NULL, 0, 1, NULL)
#define sysTaskNotifyDelete(x) CloseHandle(x)
#define sysTaskNotifyTake(x,t) (WaitForSingleObject(x, t) == WAIT_OBJECT_0)
#define sysTaskNotifyGive(x) ReleaseSemaphore( x, 1, NULL)
#define sysTaskNotifyGiveFromISR(x, interrupt_param) ReleaseSemaphore( x, 1, NULL)



#define sysNOP() asm("nop")


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
#define sys_DEFAULT_STACK_SIZE configMINIMAL_STACK_SIZE

///////////////////////////////////////////////////////////////////////////////
// Types
typedef TaskHandle_t sysTask;

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
typedef void (*TaskFunction_t)(void*);

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
uint32_t sysTaskCreate(TaskFunction_t pvTaskCode, const char * const pcName, uint16_t usStackDepth, void *pvParameters, UBaseType_t uxPriority, sysTask* pvCreatedTask );
uint32_t sysGetSystemTick(void);

#define sysCriticalSectionBegin() vTaskEnterCritical()
#define sysCriticalSectionEnd() vTaskExitCritical()

///////////////////////////////////////////////////////////////////////////////
// FreeRTOS wrappers
#define sysStartSheduler() vTaskStartScheduler()
#define sysDelay(x) vTaskDelay((portTickType) (x) / portTICK_RATE_MS)
#define sysStartScheduler() vTaskStartScheduler()
#define sysShutdown()
#define sysTaskLoop() 1

#define sysBeginInterruptRoutine() BaseType_t xHigherPriorityTaskWoken = pdFALSE
#define sysEndInterruptRoutine() portYIELD_FROM_ISR( xHigherPriorityTaskWoken )
#define sysInterruptParam() &xHigherPriorityTaskWoken

// Semaphore related definitions
/*
typedef SemaphoreHandle_t rtosSemaphoreHandle_t;
#define rtosCreateBinarySemaphore(x) vSemaphoreCreateBinary(x)


typedef SemaphoreHandle_t sysBinarySemaphore;
#define sysBinarySemaphoreCreate(x) x = CreateSemaphore( NULL, 1, 1, NULL)
#define sysBinarySemaphoreDelete(x) CloseHandle(x)
#define sysBinarySemaphoreLock(x,t) ((WaitForSingleObject(x, t) == WAIT_OBJECT_0) ? true : false)
#define sysBinarySemaphoreUnlock(x) ReleaseSemaphore( x, 1, NULL)
#define sysBinarySemaphoreUnlockFromISR(x) ReleaseSemaphore( x, 1, NULL)
*/


typedef TaskHandle_t sysTaskNotify;
#define sysTaskNotifyCreate(x) x = xTaskGetCurrentTaskHandle()
#define sysTaskNotifyDelete(x)
#define sysTaskNotifyTake(x,t) ulTaskNotifyTake( pdTRUE, t )
#define sysTaskNotifyGive(x) xTaskNotifyGive( x )
#define sysTaskNotifyGiveFromISR(x, interrupt_param) vTaskNotifyGiveFromISR( x, interrupt_param )



#define sysNOP() asm("nop")

#endif

#endif
