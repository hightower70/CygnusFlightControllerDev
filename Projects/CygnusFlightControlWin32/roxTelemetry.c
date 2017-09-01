#include <sysRTOS.h>
//#include <sysTimer.h>
#include <comSystemPacketDefinitions.h>
#include <roxTelemetry.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define roxTELEMETRY_INVALID_OBJECT_INDEX 0xff
#define roxTELEMETRY_TASK_PRIORITY 2
#define comTELEMETRY_TASK_MAX_CYCLE_TIME 100

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/

// object information for terlemetry transmission
typedef struct
{
	bool Changed;
	uint16_t MinimumPeriod;
	sysTick LastTransmissionTimestamp;
	uint8_t NextChangedObject;
} roxObjectTransmissionInfo;

/*****************************************************************************/
/* Local function prototypes                                                 */
/*****************************************************************************/
static sysTaskRetval roxTelemetryTask(sysTaskParam in_param);
static void roxTelemetrySendObject(roxObjectIndex in_object_index);
static void roxTelemetryRemoveObjectChanged(uint8_t in_object_index, uint8_t in_previous_object_index);


/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/

// object list variables
static roxObjectTransmissionInfo g_telemetry_object_info[roxTOTAL_OBJECT_COUNT];
static roxObjectIndex g_first_changed_object_index;
static roxObjectIndex g_last_changed_object_index;
static sysMutex g_list_lock;

// task variables
static bool l_stop_task = false;
static sysTaskNotify l_task_event;

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initialize telemetry functionality
void roxTelemetryInitialize(void)
{
	int i;
	sysTask task_handle;

	// initialize list
	g_first_changed_object_index = roxTELEMETRY_INVALID_OBJECT_INDEX;
	g_last_changed_object_index = roxTELEMETRY_INVALID_OBJECT_INDEX;

	// intialize all object
	for (i = 0; i < roxTOTAL_OBJECT_COUNT; i++)
	{
		g_telemetry_object_info[i].Changed = false;
	}

	// mutex for list locking
	sysMutexCreate(g_list_lock);

	// initialize telemetry tasks
	sysTaskCreate(roxTelemetryTask, "roxTelemetry", sysDEFAULT_STACK_SIZE, sysNULL, roxTELEMETRY_TASK_PRIORITY, &task_handle, roxTelemetryTaskStop);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sets object changed flag of the object and appends object to the changed object list
/// @param in_object_index Index of the changed object 
void roxTelemetrySetObjectChanged(roxObjectIndex in_object_index)
{
	// update list of changed object in a critical section
	sysMutexTake(g_list_lock, sysINFINITE_TIMEOUT);

	if (g_last_changed_object_index == roxTELEMETRY_INVALID_OBJECT_INDEX)
	{
		// if list is empty
		g_first_changed_object_index = g_last_changed_object_index = in_object_index;
		g_telemetry_object_info[in_object_index].NextChangedObject = roxTELEMETRY_INVALID_OBJECT_INDEX;
		g_telemetry_object_info[in_object_index].Changed = true;
	}
	else
	{
		// only add to the list if it is not included in it
		if (!g_telemetry_object_info[in_object_index].Changed)
		{
			// append this object to the end of the list
			g_telemetry_object_info[g_last_changed_object_index].NextChangedObject = in_object_index;
			g_telemetry_object_info[in_object_index].NextChangedObject = roxTELEMETRY_INVALID_OBJECT_INDEX;
			g_telemetry_object_info[in_object_index].Changed = true;
			g_last_changed_object_index = in_object_index;
		}
	}

	// release lock
	sysMutexGive(g_list_lock);
}


///////////////////////////////////////////////////////////////////////////////
/// @brief Stops telemetry task
void roxTelemetryTaskStop(void)
{
	l_stop_task = true;
	sysTaskNotifyGive(l_task_event);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief telemetry task function
static sysTaskRetval roxTelemetryTask(sysTaskParam in_param)
{
	uint32_t delay_time;
	uint32_t remaining_time;
	roxObjectIndex object_index_to_send;
	roxObjectIndex prev_object_index;

	sysUNUSED(in_param);

	// task loop
	while (!l_stop_task)
	{
		// wait for event
		sysTaskNotifyTake(l_task_event, delay_time);
		delay_time = comTELEMETRY_TASK_MAX_CYCLE_TIME;

		// stop task is requested
		if (l_stop_task)
			break;

		// check list of object to send
		object_index_to_send = g_first_changed_object_index;
		prev_object_index = roxTELEMETRY_INVALID_OBJECT_INDEX;
		while (object_index_to_send != roxTELEMETRY_INVALID_OBJECT_INDEX)
		{
			// if minimum period is not defined -> send immediatelly
			if (g_telemetry_object_info[object_index_to_send].MinimumPeriod == 0)
			{
				roxTelemetrySendObject(object_index_to_send);
				roxTelemetryRemoveObjectChanged(object_index_to_send, prev_object_index);
			}
			else
			{
				// minimum period is defined -> check if it is expired
				remaining_time = sysGetSystemTickSince(g_telemetry_object_info[object_index_to_send].LastTransmissionTimestamp);
				if (remaining_time > g_telemetry_object_info[object_index_to_send].MinimumPeriod)
				{
					// minimum period expired -> send packet
					roxTelemetrySendObject(object_index_to_send);
					roxTelemetryRemoveObjectChanged(object_index_to_send, prev_object_index);
				}
				else
				{
					// minimum period is not expired -> update task delay time for the shortest not expired remaining time
					if (delay_time == comTELEMETRY_TASK_MAX_CYCLE_TIME)
						delay_time = remaining_time;
					else
					{
						if (delay_time > remaining_time)
						{
							delay_time = remaining_time;
						}
					}
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Remove object from the changed list
/// @param in_object_index Index of the object to remove
/// @param in_previous_object_index Previous object index
static void roxTelemetryRemoveObjectChanged(roxObjectIndex in_object_index, roxObjectIndex in_previous_object_index)
{
	roxObjectIndex ret_object_index;
	roxObjectTransmissionInfo* current_object;

	current_object = &g_telemetry_object_info[in_object_index];

	// update list of changed object in a critical section
	sysMutexTake(g_list_lock, sysINFINITE_TIMEOUT);

	if (in_previous_object_index == roxTELEMETRY_INVALID_OBJECT_INDEX)
	{
		// first object in the list to remove
		 g_first_changed_object_index = current_object->NextChangedObject;
	}
	else
	{
		// remove object from the list
		g_telemetry_object_info[in_previous_object_index].NextChangedObject = current_object->NextChangedObject;

		// update tail pointer if last object was removed
		if (in_object_index == g_last_changed_object_index)
			g_last_changed_object_index = in_previous_object_index;
	}

	// update other object properties
	current_object->NextChangedObject = roxTELEMETRY_INVALID_OBJECT_INDEX;
	current_object->Changed = false;

	// release lock
	sysMutexGive(g_list_lock);
}

///////////////////////////////////////////////////////////////////////////////
/// @param Sends object over the telemetery channel
/// @param in_object_index Object to send
static void roxTelemetrySendObject(roxObjectIndex in_object_index)
{
	// update object
	g_telemetry_object_info[in_object_index].LastTransmissionTimestamp = sysGetSystemTick();
}