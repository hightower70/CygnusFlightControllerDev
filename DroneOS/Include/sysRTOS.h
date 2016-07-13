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
#include <assert.h>

///////////////////////////////////////////////////////////////////////////////
// Windows constants
#define sysDEFAULT_STACK_SIZE 0
#define sysINFINITE_TIMEOUT INFINITE

///////////////////////////////////////////////////////////////////////////////
// Types
typedef HANDLE sysTask;

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
typedef void (*sysTaskStopFunction)(void);
typedef void (*sysTaskFunction)(void*);

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
//CreateThread(0, stacksize, (LPTHREAD_START_ROUTINE)taskfunc, param, 0, handle, stopfunction)
uint32_t sysWin32TaskCreate(sysTaskFunction in_task_code, const char* const in_task_name, uint16_t in_stack_size, void *in_parameters, uint8_t in_priority, sysTask* out_thread_handle, sysTaskStopFunction in_stop_function);
void sysAddThreadStopFunction(sysTaskStopFunction in_stop_function);


///////////////////////////////////////////////////////////////////////////////
// Windows wrappers
#define sysGetSystemTick() GetTickCount()
#define sysDelay(x) Sleep(x)
#define sysStartScheduler()

#define sysBeginInterruptRoutine()
#define sysEndInterruptRoutine()
#define sysInterruptParam() sysNULL


void sysCriticalSectionBegin(void);
void sysCriticalSectionEnd(void);

#define sysASSERT(x) assert(x)

void sysDebugPrint(const char *fmt, ...);

#ifdef _DEBUG
#define sysDBGPRINT(fmt,...) sysDebugPrint(fmt, ##__VA_ARGS__)
#else
#define sysDBGPRINT(fmt,...) ((void)0) 
#endif 


#define sysTaskCreate(taskfunc, taskname, stacksize, param, priority, handle, stopfunction) 	sysWin32TaskCreate(taskfunc, taskname, stacksize, param, priority, handle, stopfunction) 	

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
#define sysTaskNotifyGiveFromISR(x, interrupt_param) { ReleaseSemaphore( x, 1, NULL); (void*)(interrupt_param); }

#define sysBeginInterruptRoutine() 
#define sysEndInterruptRoutine() 
#define sysInterruptParam() sysNULL


#define sysNOP() asm("nop")


#else

/*****************************************************************************/
/* FreeRTOS wrapper                                                          */
/*****************************************************************************/

#pragma GCC diagnostic ignored "-Wunknown-pragmas"

///////////////////////////////////////////////////////////////////////////////
// FreeRTOS specific includes
#include <FreeRTOS.h>
#include <portmacro.h>
#include <semphr.h>
#include <task.h>

///////////////////////////////////////////////////////////////////////////////
// Constants
#define sysDEFAULT_STACK_SIZE configMINIMAL_STACK_SIZE
#define sysINFINITE_TIMEOUT  portMAX_DELAY

///////////////////////////////////////////////////////////////////////////////
// Types
typedef TaskHandle_t sysTask;

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
typedef void (*TaskFunction_t)(void*);
typedef void (*sysTaskStopFunction)(void);

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
uint32_t sysTaskCreate(TaskFunction_t pvTaskCode, const char * const pcName, uint16_t usStackDepth, void *pvParameters, UBaseType_t uxPriority, sysTask* pvCreatedTask, sysTaskStopFunction in_stop_function);
uint32_t sysGetSystemTick(void);

#define sysCriticalSectionBegin() vPortEnterCritical()
#define sysCriticalSectionEnd() vPortExitCritical()

#define sysASSERT(x) configASSERT(x)

///////////////////////////////////////////////////////////////////////////////
// FreeRTOS wrappers
#define sysStartSheduler() vTaskStartScheduler()
#define sysDelay(x) vTaskDelay((portTickType) (x) / portTICK_RATE_MS)
#define sysStartScheduler() vTaskStartScheduler()
#define sysTaskLoop() 1

#define sysBeginInterruptRoutine() BaseType_t xHigherPriorityTaskWoken = pdFALSE
#define sysEndInterruptRoutine() portYIELD_FROM_ISR( xHigherPriorityTaskWoken )
#define sysInterruptParam() &xHigherPriorityTaskWoken

// Mutex related definitions
typedef SemaphoreHandle_t sysMutex;
#define sysMutexCreate(x) x = xSemaphoreCreateMutex()
#define sysMutexDelete(x) vSemaphoreDelete(x)
#define sysMutexTake(x,t) xSemaphoreTake(x, (t==portMAX_DELAY)?(portMAX_DELAY):(t / portTICK_RATE_MS))
#define sysMutexGive(x) xSemaphoreGive(x)
#define sysMutexGiveFromISR(x, interrupt_param) xSemaphoreGiveFromISR( x, interrupt_param )

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
