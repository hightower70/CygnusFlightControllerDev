/*****************************************************************************/
/* Realtime Object Exchange Module                                           */
/*	 data storage class                                                      */
/*                                                                           */
/* Copyright (C) 2014 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysRTOS.h>
#include <roxTelemetry.h>
#include <comSystemPacketDefinitions.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define roxOBJECT_STORAGE_COUNT 2
#define roxTOTAL_STORAGE_SIZE 1024
#define roxTOTAL_CALLBACK_FUNCTION_COUNT 20
#define roxSTORAGE_INVALID_CALLBACK_INDEX 0xffff

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/
typedef struct
{
	float AccelX;
	float AccelY;
	float AccelZ;

	float GyroX;
	float GyroY;
	float GyroZ;

	float MagnetoX;
	float MagenetoY;
	float MagnetoZ;
} roxIMURawData;

typedef enum
{
	roxOWS_Unlocked,
	roxOWS_LockedForWrite,
	roxOWS_PendingAfterWrite
} roxObjectWriteState;

typedef struct
{
	roxObjectChangedCallbackFunction Callback;
	uint16_t NextCallbackFunctionIndex;
} roxCallbackFunctionColectionEntry;

typedef struct
{
	roxObjectWriteState WriteState;
	uint8_t ReaderCount;
	uint16_t VersionNumber;
	uint16_t Address;
	uint8_t ReadStorageIndex;
	uint8_t WriteStorageIndex;
	uint16_t FirstCallbackFunctionIndex;
} roxObjectStorageInfo;

typedef uint16_t roxCallbackIndex;

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static uint8_t l_realtime_object_value_storage[roxOBJECT_STORAGE_COUNT][roxTOTAL_STORAGE_SIZE];
static roxObjectStorageInfo l_object_storage_info[roxTOTAL_OBJECT_COUNT];

// callback variables
static roxCallbackFunctionColectionEntry l_callback_collection[roxTOTAL_CALLBACK_FUNCTION_COUNT];
static sysMutex l_callback_collection_lock;

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes realtime object storage
void roxStorageInitialize(void)
{
	uint16_t i;

	for (i = 0; i < roxTOTAL_CALLBACK_FUNCTION_COUNT; i++)
	{
		l_callback_collection[i].Callback = sysNULL;
		l_callback_collection[i].NextCallbackFunctionIndex = roxSTORAGE_INVALID_CALLBACK_INDEX;
	}

	sysMutexCreate(l_callback_collection_lock);
}

/*****************************************************************************/
/* Callback function management                                              */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Adds change callback function to a given object
/// @param in_object_index Index of the object to receive a new change callback
/// @param in_callback_function Callback function pointer
void roxCallbackFunctionAdd(roxObjectIndex in_object_index, roxObjectChangedCallbackFunction in_callback_function)
{
	roxCallbackIndex callback_to_append_index;
	roxObjectIndex prev_callback_index;
	roxObjectIndex callback_index;

	// update list of changed object in a critical section
	sysMutexTake(l_callback_collection_lock, sysINFINITE_TIMEOUT);

	// find free storage in the callback list
	callback_to_append_index = 0;
	while (callback_to_append_index < roxTOTAL_CALLBACK_FUNCTION_COUNT)
	{
		if (l_callback_collection[callback_to_append_index].Callback == sysNULL)
		{
			l_callback_collection[callback_to_append_index].Callback = in_callback_function;
			l_callback_collection[callback_to_append_index].NextCallbackFunctionIndex = roxSTORAGE_INVALID_CALLBACK_INDEX;

			break;
		}

		callback_to_append_index++;
	}

	// check free space
	sysASSERT(callback_to_append_index < roxTOTAL_CALLBACK_FUNCTION_COUNT);

	if (callback_to_append_index < roxTOTAL_CALLBACK_FUNCTION_COUNT)
	{
		if (l_object_storage_info[in_object_index].FirstCallbackFunctionIndex == roxSTORAGE_INVALID_CALLBACK_INDEX)
		{
			// if it is the first callback for the object
			l_object_storage_info[in_object_index].FirstCallbackFunctionIndex = callback_to_append_index;
		}
		else
		{
			// append callback to the end of the list
			prev_callback_index = roxSTORAGE_INVALID_CALLBACK_INDEX;
			callback_index = l_object_storage_info[in_object_index].FirstCallbackFunctionIndex;
			while (callback_index != roxSTORAGE_INVALID_CALLBACK_INDEX)
			{
				prev_callback_index = callback_index;
				callback_index = l_callback_collection[callback_index].NextCallbackFunctionIndex;
			}

			l_callback_collection[prev_callback_index].NextCallbackFunctionIndex = callback_to_append_index;
		}
	}

	// release lock
	sysMutexGive(l_callback_collection_lock);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Removes change callback function from a given object
/// @param in_object_index Index of the object from remove the change callback
/// @param in_callback_function Callback function pointer
void roxCallbackFunctionDelete(roxObjectChangedCallbackFunction in_callback_function, roxObjectIndex in_object_index)
{
	roxCallbackIndex prev_callback_index;
	roxCallbackIndex callback_index;

	// update list of changed object in a critical section
	sysMutexTake(l_callback_collection_lock, sysINFINITE_TIMEOUT);

	// find callback
	prev_callback_index = roxSTORAGE_INVALID_CALLBACK_INDEX;
	callback_index = l_object_storage_info[in_object_index].FirstCallbackFunctionIndex;
	while (callback_index != roxSTORAGE_INVALID_CALLBACK_INDEX && l_callback_collection[callback_index].Callback != in_callback_function)
	{
		prev_callback_index = callback_index;
		callback_index = l_callback_collection[callback_index].NextCallbackFunctionIndex;
	}

	// remove callback from the list
	if (prev_callback_index == roxSTORAGE_INVALID_CALLBACK_INDEX)
	{
		l_object_storage_info[in_object_index].FirstCallbackFunctionIndex = roxSTORAGE_INVALID_CALLBACK_INDEX;
	}
	else
	{
		l_callback_collection[prev_callback_index].NextCallbackFunctionIndex = l_callback_collection[callback_index].NextCallbackFunctionIndex;
	}

	// delete callback
	l_callback_collection[callback_index].Callback = sysNULL;
	l_callback_collection[callback_index].NextCallbackFunctionIndex = roxSTORAGE_INVALID_CALLBACK_INDEX;

	// release lock
	sysMutexGive(l_callback_collection_lock);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Call all callback function of the given object
/// @param in_object_index Object index to call
static void roxStorageCallbackExecute(roxObjectIndex in_object_index)
{
	roxCallbackIndex callback_index;

	sysASSERT(in_object_index < roxTOTAL_OBJECT_COUNT);

	callback_index = l_object_storage_info[in_object_index].FirstCallbackFunctionIndex;
	while (callback_index != roxSTORAGE_INVALID_CALLBACK_INDEX)
	{
		if (l_callback_collection[callback_index].Callback != sysNULL)
			(l_callback_collection[callback_index].Callback)(in_object_index);

		callback_index = l_callback_collection[callback_index].NextCallbackFunctionIndex;
	}
}

/*****************************************************************************/
/* Object access functions                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Start object read operation
/// @param in_object_index Index of the object which will be read
void roxObjectReadBegin(roxObjectIndex in_object_index)
{
	roxObjectStorageInfo* object_storage_info;

	// sanity check
	sysASSERT(in_object_index < roxTOTAL_OBJECT_COUNT);

	// cache object info pointer
	object_storage_info = &l_object_storage_info[in_object_index];

	// increment read counter
	sysCriticalSectionBegin();
	object_storage_info->ReaderCount++;
	sysCriticalSectionEnd();
}

///////////////////////////////////////////////////////////////////////////////
/// @param End of object read operation. If there is no more read access and object values are pending after a write operation then the values for read will be updated.
/// @param in_object_index Index of the object which reading will be ended
void roxObjectReadEnd(roxObjectIndex in_object_index)
{
	roxObjectStorageInfo* object_storage_info;

	// sanity check
	sysASSERT(in_object_index < roxTOTAL_OBJECT_COUNT);

	// cache object info pointer
	object_storage_info = &l_object_storage_info[in_object_index];

	// update states in a critical section
	sysCriticalSectionBegin();

	if (object_storage_info->ReaderCount > 0)
	{
		// decrement read state counter
		object_storage_info->ReaderCount--;

		if (object_storage_info->ReaderCount == 0)
		{
			// if data pending after write then update read data
			if (object_storage_info->WriteState == roxOWS_PendingAfterWrite)
			{
				// set read data to the most current data
				l_object_storage_info->ReadStorageIndex = l_object_storage_info->WriteStorageIndex;
				l_object_storage_info->WriteState = roxOWS_Unlocked;

				// call callbacks
				roxStorageCallbackExecute(in_object_index);

				// update telemetry object list
				roxTelemetrySetObjectChanged(in_object_index);
			}
		}
	}

	sysCriticalSectionEnd();
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Lock object for write (reading of the actual values ais still possible)
/// @param in_object_index Index of the object to write
/// @return true if lock is accquired, false when object is already locked for write
bool roxObjectWriteBegin(roxObjectIndex in_object_index)
{
	roxObjectStorageInfo* object_storage_info;

	// sanity check
	sysASSERT(in_object_index < roxTOTAL_OBJECT_COUNT);

	// cache object info pointer
	object_storage_info = &l_object_storage_info[in_object_index];

	// check for object lock
	sysCriticalSectionBegin();
	if (l_object_storage_info->WriteState == roxOWS_LockedForWrite)
	{
		// object is locked -> exit with error
		sysCriticalSectionEnd();
		return false;
	}
	else
	{
		// object unlocked -> lock it
		l_object_storage_info->WriteState = roxOWS_LockedForWrite;
		sysCriticalSectionEnd();
	}

	// set write storage index to the unused (not used for read) storage
	l_object_storage_info->WriteStorageIndex = 1 - l_object_storage_info->ReadStorageIndex;

	// object lock is accuired and ready for write
	return true;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Releases object write lock
/// @param Objec index to release write lock
void roxObjectWriteEnd(roxObjectIndex in_object_index)
{
	roxObjectStorageInfo* object_storage_info;

	// sanity check
	sysASSERT(in_object_index < roxTOTAL_OBJECT_COUNT);

	// cache object info pointer
	object_storage_info = &l_object_storage_info[in_object_index];

	// update state and version in a critical sections
	sysCriticalSectionBegin();
	if (object_storage_info->ReaderCount == 0)
	{
		// there is no active reader -> switch to the newly refreshed buffer
		object_storage_info->ReadStorageIndex = object_storage_info->WriteStorageIndex;
		object_storage_info->WriteState = roxOWS_Unlocked;

		// call callbacks
		roxStorageCallbackExecute(in_object_index);

		// update telemetry object list
		roxTelemetrySetObjectChanged(in_object_index);
	}
	else
	{
		// there is active reader -> mark it as pending data
		object_storage_info->WriteState = roxOWS_PendingAfterWrite;
	}

	// increment version number
	object_storage_info->VersionNumber++;

	sysCriticalSectionEnd();
}

/*****************************************************************************/
/* Member access functions                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets member value as float
/// @param in_object_address Object address to get value
/// @param in_member_address Member address to get value
/// @return Member value as float number
float roxGetFloat(roxObjectIndex in_object_index, roxMemberAddress in_member_address)
{
	roxObjectStorageInfo* object_storage_info;

	// sanity check
	sysASSERT(in_object_index < roxTOTAL_OBJECT_COUNT);

	// cache object info pointer
	object_storage_info = &l_object_storage_info[in_object_index];

	return *(float*)&l_realtime_object_value_storage[object_storage_info->ReadStorageIndex][object_storage_info->Address + in_member_address];
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sets meber value as a float number
/// @param in_object_index Index of the object to change
/// @param in_member_address Address of the member variable to change
/// @param in_value New value of the member
/// @param true if operation was success, false if member can't be modified (e.q. object is not in write state)
bool roxSetFloat(roxObjectIndex in_object_address, roxMemberAddress in_member_address, float in_value)
{
	roxObjectStorageInfo* object_storage_info;

	// sanity check
	sysASSERT(in_object_address < roxTOTAL_OBJECT_COUNT);

	// cache object info pointer
	object_storage_info = &l_object_storage_info[in_object_address];

	// the object must be locked at this pont
	if (object_storage_info->WriteState == roxOWS_LockedForWrite)
		return false;

	// change object value
	*(float*)&l_realtime_object_value_storage[object_storage_info->WriteStorageIndex][object_storage_info->Address + in_member_address] = in_value;

	return true;

}


