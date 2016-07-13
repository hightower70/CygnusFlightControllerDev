/*****************************************************************************/
/* Realtime Object Exchange Module                                           */
/*	 data storage class                                                      */
/*                                                                           */
/* Copyright (C) 2014 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysRTOS.h>

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

typedef uint16_t roxObjectAddress;
typedef uint16_t roxMemberAddress;

typedef void (*roxObjectChangedCallbackFunction)(roxObjectAddress in_object_adress);

typedef enum
{
	roxOWS_Unlocked,
	roxOWS_LockedForWrite,
	roxOWS_PendingAfterWrite
} roxObjectWriteState;

typedef struct
{
	roxObjectWriteState WriteState;
	uint8_t ReaderCount;
	uint16_t VersionNumber;
	uint16_t Address;
	uint8_t ReadStorageIndex;
	uint8_t WriteStorageIndex;
	roxObjectChangedCallbackFunction FirstCallbackFunction;
} roxObjectStorageInfo;

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define roxOBJECT_STORAGE_COUNT 2

#define roxTOTAL_STORAGE_SIZE 1024

#define roxTOTAL_OBJECT_COUNT 5
#define roxTOTAL_CALLACK_FUNCTION_COUNT 20

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static uint8_t l_realtime_object_value_storage[roxOBJECT_STORAGE_COUNT][roxTOTAL_STORAGE_SIZE];
static roxObjectStorageInfo l_object_storage_info[roxTOTAL_OBJECT_COUNT];
static roxObjectChangedCallbackFunction l_callback_functions[roxTOTAL_CALLACK_FUNCTION_COUNT];


/*****************************************************************************/
/* Callback function management                                              */
/*****************************************************************************/

bool roxCallbackFunctionAdd(roxObjectChangedCallbackFunction in_callback_function, roxObjectAddress in_object_address)
{
	return false;
}

void roxCallbackFunctionDelete(roxObjectChangedCallbackFunction in_callback_function, roxObjectAddress in_object_address)
{

}

/*****************************************************************************/
/* Object access functions                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Start object read operation
/// @param in_object_index Index of the object which will be read
void roxObjectReadBegin(roxObjectAddress in_object_index)
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
void roxObjectReadEnd(roxObjectAddress in_object_index)
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
			}
		}
	}

	sysCriticalSectionEnd();
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Lock object for write (reading of the actual values ais still possible)
/// @param in_object_index Index of the object to write
/// @return true if lock is accquired, false when object is already locked for write
bool roxObjectWriteBegin(roxObjectAddress in_object_index)
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
void roxObjectUpdateEnd(roxObjectAddress in_object_index)
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
	}
	else
	{
		// ther is active reader -> mark it as pending data
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
float roxGetFloat(roxObjectAddress in_object_address, roxMemberAddress in_member_address)
{
	roxObjectStorageInfo* object_storage_info;

	// sanity check
	sysASSERT(in_object_address < roxTOTAL_OBJECT_COUNT);

	// cache object info pointer
	object_storage_info = &l_object_storage_info[in_object_address];

	return *(float*)&l_realtime_object_value_storage[object_storage_info->ReadStorageIndex][object_storage_info->Address + in_member_address];
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sets meber value as a float number
/// @param in_object_index Index of the object to change
/// @param in_member_address Address of the member variable to change
/// @param in_value New value of the member
/// @param true if operation was success, false if member can't be modified (e.q. object is not in write state)
bool roxSetFloat(roxObjectAddress in_object_address, roxMemberAddress in_member_address, float in_value)
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


