/*****************************************************************************/
/* Packet Definitions for Ground COntrol Station Communication               */
/*                                                                           */
/* Copyright (C) 2014 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __PacketDefinitions_h
#define __PacketDefinitions_h

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <sysTypes.h>
#include <sysTimer.h>
#include <crcMD5.h>


#define roxTOTAL_OBJECT_COUNT 5

///////////////////////////////////////////////////////////////////////////////
// Constants
#define comFILENAME_LENGTH 32
#define comDEVICE_NAME_LENGTH 16

// Starts packed struct
#include <sysPackedStructStart.h>

///////////////////////////////////////////////////////////////////////////////
// Constants

// packet types bits: 
// bit 7 system message
// bit 6 ------------
// bit 5 Packet class
// bit 4 ------------
// bit 3 - Request flag
// bit 2 ---------
// bit 1 Packet id
// bit 0 ---------
#define comPT_SYSTEM_FLAG		(1<<7)		// flag of the packet (1 - system message, 0 - telemetry message)
#define comPT_REQUEST_FLAG	(1<<3)		// flag of the message (1 - request, 0 - response)

#define comPT_CLASS_COMM		(1<<4)		// packet class: Communication
#define comPT_CLASS_FILE		(2<<4)		// packet class: File management
#define comPT_CLASS_CONFIG	(3<<4)		// packet class: System configuration

#define comPT_GET_CLASS(x) (x & (7<<4))

// Communication class packet types
#define comPT_DEVICE_HEARTBEAT			(comPT_SYSTEM_FLAG | comPT_CLASS_COMM | 1)
#define comPT_HOST_HEARTBEAT				(comPT_SYSTEM_FLAG | comPT_CLASS_COMM | 2)
#define comPT_DEVICE_ANNOUNCE				(comPT_SYSTEM_FLAG | comPT_CLASS_COMM | 3)
#define comPT_HOST_ANNOUNCE					(comPT_SYSTEM_FLAG | comPT_CLASS_COMM | 4)
#define comPT_DEVICE_NAME_REQUEST		(comPT_SYSTEM_FLAG | comPT_CLASS_COMM | comPT_REQUEST_FLAG | 5)
#define comPT_DEVICE_NAME_RESPONSE	(comPT_SYSTEM_FLAG | comPT_CLASS_COMM | 5)

// File transfer class packet types
#define comPT_FILE_INFO						(comPT_SYSTEM_FLAG | comPT_CLASS_FILE | 1)
#define comPT_FILE_INFO_REQUEST		(comPT_FILE_INFO | comPT_REQUEST_FLAG)
#define comPT_FILE_INFO_RESPONSE	(comPT_FILE_INFO)

#define comPT_FILE_DATA_READ						(comPT_SYSTEM_FLAG | comPT_CLASS_FILE | 2)
#define comPT_FILE_DATA_READ_REQUEST		(comPT_FILE_DATA_READ | comPT_REQUEST_FLAG)
#define comPT_FILE_DATA_READ_RESPONSE		(comPT_FILE_DATA_READ)

#define comPT_FILE_DATA_WRITE						(comPT_SYSTEM_FLAG | comPT_CLASS_FILE | 3)
#define comPT_FILE_DATA_WRITE_REQUEST		(comPT_FILE_DATA_WRITE | comPT_REQUEST_FLAG)
#define comPT_FILE_DATA_WRITE_RESPONSE	(comPT_FILE_DATA_WRITE)

#define comPT_FILE_OPERATION_FINISHED						(comPT_SYSTEM_FLAG | comPT_CLASS_FILE | 4)
#define comPT_FILE_OPERATION_FINISHED_REQUEST		(comPT_FILE_OPERATION_FINISHED | comPT_REQUEST_FLAG)
#define comPT_FILE_OPERATION_FINISHED_RESPONSE	(comPT_FILE_OPERATION_FINISHED)

// File result codes
#define comFRC_OK					0
#define comFRC_NOT_FOUND	1
#define comFRC_INVALID		2
#define comFRC_READ_ONLY	3
#define comFRC_FALED			4

// File operation finished modes
#define comFOFM_SUCCESS 1
#define comFOFM_CANCEL	2

// packet definitions

/////////////////////
// Packet header
typedef struct
{
	uint8_t PacketLength;
	uint8_t PacketType;
	uint8_t PacketCounter;

} comPacketHeader;

/*****************************************************************************/
/* Communication class packets                                               */
/*****************************************************************************/

////////////////////////
// Device Announce 
typedef struct
{
	comPacketHeader Header;

	sysChar Name[comDEVICE_NAME_LENGTH];
	uint32_t UniqueID;

	uint32_t Address;

} comPacketDeviceAnnounce;

////////////////////////
// Host Info 
typedef struct
{
	comPacketHeader Header;

	sysChar Name[comDEVICE_NAME_LENGTH];
	uint32_t Address;

} comPacketHostAnnounce;


/////////////////
// Host heartbeat
typedef struct
{
	comPacketHeader Header;

	uint16_t Year;
	uint8_t Month;
	uint8_t Day;

	uint8_t Hour;
	uint8_t Minute;
	uint8_t Second;

} comPacketHostHeartbeat;

///////////////////
// Device Heartbeat 
typedef struct
{
	comPacketHeader Header;

	uint32_t UniqueID;	// Unique device ID
	uint8_t CPULoad;		// Current CPU load in percentage

} comPacketDeviceHeartbeat;

//////////////////////
// Device Name Request
typedef struct
{
	comPacketHeader Header;
} comPacketDeviceNameRequest;

////////////////////////
// Device Name Response
typedef struct
{
	comPacketHeader Header;

	uint32_t UniqueID;										// Unique device ID
	sysChar Name[comDEVICE_NAME_LENGTH];	// Device name

} comPacketDeviceNameResponse;

/*****************************************************************************/
/* File transfer class packets                                               */
/*****************************************************************************/

//////////////////////////
// Header for file packets
typedef struct
{
	comPacketHeader Header;
	uint8_t ID;
} comPacketFileHeader;

////////////////////
// File info request
typedef struct
{
	comPacketFileHeader Header;

	sysChar FileName[comFILENAME_LENGTH];

} comPacketFileInfoRequest;

/////////////////////
// File info response
typedef struct
{
	comPacketFileHeader Header;

	sysChar FileName[comFILENAME_LENGTH];
	uint32_t Length;
	uint8_t Hash[crcMD5_HASH_SIZE];

} comPacketFileInfoResponse;

////////////////////
// File data request
typedef struct
{
	comPacketFileHeader Header;

	uint32_t Pos;
	uint16_t Length;

} comPacketFileDataReadRequest;

///////////////////////////////////
// File data response (header only)
typedef struct
{
	comPacketFileHeader Header;

	uint32_t Pos;
	uint16_t Length;

} comFileDataReadResponseHeader;

////////////////////
// File data request
typedef struct
{
	comPacketFileHeader Header;

	uint32_t Pos;
	uint16_t Length;

} comPacketFileDataWriteRequestHeader;

///////////////////////////////////
// File data response (header only)
typedef struct
{
	comPacketFileHeader Header;

	uint8_t Error;

} comPacketFileDataWriteResponse;

///////////////////////////////////
// File operation finished request
typedef struct
{
	comPacketFileHeader Header;

	uint8_t FinishMode;

} comPacketFileOperationFinishedRequest;

////////////////////////////////////
// File operation: Finished response
typedef struct
{
	comPacketFileHeader Header;

	uint8_t FinishMode;
	uint8_t Error;

} comPacketFileOperationFinishedResponse;


////////////////////////////////////
// File operation: Get changes request
typedef struct
{
	comPacketFileHeader Header;

	sysTick Timestamp;
	uint32_t Pos;
	uint32_t Length;

} comPacketFileOperationGetChangesRequest;

////////////////////////////////////
// File operation: Get changes response
typedef struct
{
	comPacketFileHeader Header;

	sysTick Timestamp;
	uint32_t FilePos;
	uint32_t Lenth;

} comPacketFileOperationGetChangesResponse;

// Ends packed struct
#include <sysPackedStructEnd.h>




#endif
