/*****************************************************************************/
/* Windows wrapper                                                           */
/*****************************************************************************/

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
typedef void* sysTaskRetval;
typedef void* sysTaskParam;

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
typedef void(*sysTaskStopFunction)(void);
typedef sysTaskRetval(*sysTaskFunction)(sysTaskParam);

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
//CreateThread(0, stacksize, (LPTHREAD_START_ROUTINE)taskfunc, param, 0, handle, stopfunction)
uint32_t sysWin32TaskCreate(sysTaskFunction in_task_code, const char* const in_task_name, uint16_t in_stack_size, void *in_parameters, uint8_t in_priority, sysTask* out_thread_handle, sysTaskStopFunction in_stop_function);
void sysAddThreadStopFunction(sysTaskStopFunction in_stop_function);


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


