/*****************************************************************************/
/* Linux wrapper                                                             */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Linux specific includes
#include <sysTypes.h>
#include <stdint.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>

///////////////////////////////////////////////////////////////////////////////
// Linux constants
#define sysDEFAULT_STACK_SIZE 0
#define sysINFINITE_TIMEOUT 0xffffffff

///////////////////////////////////////////////////////////////////////////////
// Types
typedef pthread_t sysTask;
typedef struct
{
	pthread_mutex_t mutex;
	pthread_cond_t cvar;
	int v;
} pthread_binsem_t;

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
typedef void (*sysTaskStopFunction)(void);
typedef void* sysTaskRetval;
typedef void* sysTaskParam;
typedef sysTaskRetval (*sysTaskFunction)(sysTaskParam);

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
//CreateThread(0, stacksize, (LPTHREAD_START_ROUTINE)taskfunc, param, 0, handle, stopfunction)
uint32_t sysLinuxTaskCreate(sysTaskFunction in_task_code, const char* const in_task_name, uint16_t in_stack_size, void *in_parameters, uint8_t in_priority, sysTask* out_thread_handle, sysTaskStopFunction in_stop_function);
void sysAddThreadStopFunction(sysTaskStopFunction in_stop_function);
static void sysMillisecToTimespec(unsigned long in_ms, struct timespec *out_ts);


///////////////////////////////////////////////////////////////////////////////
// Windows wrappers
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


#define sysTaskCreate(taskfunc, taskname, stacksize, param, priority, handle, stopfunction) 	sysLinuxTaskCreate(taskfunc, taskname, stacksize, param, priority, handle, stopfunction) 	

/////////
// Mutex 
typedef pthread_mutex_t sysMutex;

bool pthread_mutex_take(sysMutex* in_mutex, uint32_t in_timeout);

#define sysMutexCreate(x) pthread_mutex_init(&x, NULL)
#define sysMutexDelete(x) pthread_mutex_destroy(&x)
#define sysMutexTake(x,t) pthread_mutex_take(&x,t)
#define sysMutexGive(x) pthread_mutex_unlock(&x)

////////////////////
// Binary semaphore
typedef pthread_binsem_t sysBinarySemaphore;
void pthread_binsem_init(pthread_binsem_t* in_binary_semaphore, int in_init_value);
void pthread_binsem_destroy(pthread_binsem_t* in_binary_semaphore);
void pthread_binsem_lock(pthread_binsem_t* in_binary_semaphore, uint32_t in_timeout);
void pthread_binsem_unlock(pthread_binsem_t* in_binary_semaphore);


#define sysBinarySemaphoreCreate(x) pthread_binsem_init(&x, 1)
#define sysBinarySemaphoreDelete(x) pthread_binsem_destroy(&x)
#define sysBinarySemaphoreTake(x, t) pthread_binsem_lock(&x, t)
#define sysBinarySemaphoreGive(x) pthread_binsem_unlock(&x)
#define sysBinarySemaphoreGiveFromISR(x) { pthread_binsem_unlock(&x); (void*)(interrupt_param); }

///////////////
// Task Notify
typedef pthread_binsem_t sysTaskNotify;
#define sysTaskNotifyCreate(x) pthread_binsem_init(&x, 0)
#define sysTaskNotifyDelete(x) pthread_binsem_destroy(&x)
#define sysTaskNotifyTake(x,t) pthread_binsem_lock(&x ,t)
#define sysTaskNotifyGive(x) pthread_binsem_unlock(&x)
#define sysTaskNotifyGiveFromISR(x, interrupt_param) { pthread_binsem_unlock(&x); (void*)(interrupt_param); }

#define sysBeginInterruptRoutine() 
#define sysEndInterruptRoutine() 
#define sysInterruptParam() sysNULL

#define sysNOP() asm("nop")

