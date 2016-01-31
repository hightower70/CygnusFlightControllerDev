/*****************************************************************************/
/* Wrapper functions for freeRTOS                                            */
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


/********************** NOTES **********************************************
To use CPU usage measurement module, the following steps should be followed :

1- in the _OS_Config.h file (ex. FreeRTOSConfig.h) enable the following macros :
      - #define configUSE_IDLE_HOOK        1
      - #define configUSE_TICK_HOOK        1

2- in the _OS_Config.h define the following macros :
      - #define traceTASK_SWITCHED_IN()  extern void StartIdleMonitor(void); \
                                         StartIdleMonitor()
      - #define traceTASK_SWITCHED_OUT() extern void EndIdleMonitor(void); \
                                         EndIdleMonitor()
*******************************************************************************/


///////////////////////////////////////////////////////////////////////////////
// Constants
#define CPU_USAGE_CALCULATION_PERIOD    1000

///////////////////////////////////////////////////////////////////////////////
// Module global variables
static xTaskHandle    l_idle_task_handle = NULL;
volatile uint32_t     l_cpu_usage = 0;
static uint32_t       l_cpu_idle_start_time = 0;
static uint32_t       l_cpu_idle_spent_time = 0;
static uint32_t       l_cpu_total_idle_time = 0;


uint32_t rtosCreateTask(TaskFunction_t pvTaskCode, const char * const pcName, uint16_t usStackDepth, void *pvParameters, UBaseType_t uxPriority, TaskHandle_t *pvCreatedTask )
{
	TaskHandle_t handle;

	if( xTaskCreate(pvTaskCode, pcName, usStackDepth, pvParameters, uxPriority, &handle) != pdPASS)
		handle = NULL;

	if(pvCreatedTask != NULL)
	{
		*pvCreatedTask = handle;
	}

	return (uint32_t)handle;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets CPU usage in percentage
/// @retval Usage in percentage
uint16_t rtosGetCPUUsage(void)
{
  return (uint16_t)l_cpu_usage;
}

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Application Idle Hook
  * @param  None
  * @retval None
  */
void vApplicationIdleHook(void)
{
  if( l_idle_task_handle == NULL )
  {
    /* Store the handle to the idle task. */
    l_idle_task_handle = xTaskGetCurrentTaskHandle();
  }
}

/**
  * @brief  Application Idle Hook
  * @param  None
  * @retval None
  */
void vApplicationTickHook (void)
{
  static int tick = 0;

  if(tick ++ > CPU_USAGE_CALCULATION_PERIOD)
  {
    tick = 0;

    if(l_cpu_total_idle_time > 1000)
    {
      l_cpu_total_idle_time = 1000;
    }
    l_cpu_usage = (100 - (l_cpu_total_idle_time * 100) / CPU_USAGE_CALCULATION_PERIOD);
    l_cpu_total_idle_time = 0;
  }
}

/**
  * @brief  Start Idle monitor
  * @param  None
  * @retval None
  */
void StartIdleMonitor (void)
{
  if( xTaskGetCurrentTaskHandle() == l_idle_task_handle )
  {
    l_cpu_idle_start_time = xTaskGetTickCountFromISR();
  }
}

/**
  * @brief  Stop Idle monitor
  * @param  None
  * @retval None
  */
void EndIdleMonitor (void)
{
  if( xTaskGetCurrentTaskHandle() == l_idle_task_handle )
  {
    /* Store the handle to the idle task. */
    l_cpu_idle_spent_time = xTaskGetTickCountFromISR() - l_cpu_idle_start_time;
    l_cpu_total_idle_time += l_cpu_idle_spent_time;
  }
}

