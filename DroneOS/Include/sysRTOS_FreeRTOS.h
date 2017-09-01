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
typedef void(*TaskFunction_t)(void*);
typedef void(*sysTaskStopFunction)(void);

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


