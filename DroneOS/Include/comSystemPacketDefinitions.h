/*****************************************************************************/
/* Packet Definitions for Ground COntrol Station Communication               */
/*                                                                           */
/* Copyright (C) 2014 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

#ifndef __PacketDefinitions_h
#define __PacketDefinitions_h

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <sysTypes.h>
#include <crcMD5.h>

///////////////////////////////////////////////////////////////////////////////
// Constants
#define comFILENAME_LENGTH 32
#define comDEVICE_NAME_LENGTH 16

// Starts packed struct
#include <sysPackedStructStart.h>

///////////////////////////////////////////////////////////////////////////////
// Constants

// packet types
#define comPT_SYSTEM_FLAG		(1<<7)		// flag of the packet (1 - system message, 0 - telemetry message)
#define comPT_REQUEST_FLAG	(1<<3)		// flag of the message (1 - request, 0 - response)

#define comPT_CLASS_COMM		(1<<4)		// packet class: Communication
#define comPT_CLASS_FILE		(2<<4)		// packet class: File management
#define comPT_CLASS_CONFIG	(3<<4)		// packet class: System configuration

#define comPT_GET_CLASS(x) (x & (7<<4))

// Communication class packet types
#define comPT_DEVICE_HEARTBEAT	(comPT_SYSTEM_FLAG | comPT_CLASS_COMM | 1)
#define comPT_HOST_HEARTBEAT		(comPT_SYSTEM_FLAG | comPT_CLASS_COMM | 2)
#define comPT_DEVICE_INFO				(comPT_SYSTEM_FLAG | comPT_CLASS_COMM | 3)
#define comPT_HOST_INFO					(comPT_SYSTEM_FLAG | comPT_CLASS_COMM | 4)

// File transfer class packet types
#define comPT_FILE_INFO						(comPT_SYSTEM_FLAG | comPT_CLASS_FILE | 1)
#define comPT_FILE_INFO_REQUEST		(comPT_FILE_INFO | comPT_REQUEST_FLAG)
#define comPT_FILE_INFO_RESPONSE	(comPT_FILE_INFO)

#define comPT_FILE_DATA						(comPT_SYSTEM_FLAG | comPT_CLASS_FILE | 2)
#define comPT_FILE_DATA_REQUEST		(comPT_FILE_DATA | comPT_REQUEST_FLAG)
#define comPT_FILE_DATA_RESPONSE	(comPT_FILE_DATA)

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
// Device Info 
typedef struct
{
	comPacketHeader Header;

	sysChar Name[comDEVICE_NAME_LENGTH];
	uint32_t UniqueID;

	uint32_t Address;

} comPacketDeviceInformation;

////////////////////////
// Host Info 
typedef struct
{
	comPacketHeader Header;

	sysChar Name[comDEVICE_NAME_LENGTH];
	uint32_t Address;

} comPacketHostInformation;


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

	// used telemetry bandwidth
	uint8_t CPULoad;

} comPacketDeviceHeartbeat;


/*****************************************************************************/
/* File transfer class packets                                               */
/*****************************************************************************/

////////////////////
// File info request
typedef struct
{
	comPacketHeader Header;

	sysChar FileName[comFILENAME_LENGTH];
} comPacketFileInfoRequest;

/////////////////////
// File info response
typedef struct
{
	comPacketHeader Header;

	uint8_t FileID;
	uint32_t FileLength;
	uint8_t FileHash[crcMD5_CHECKSUM_SIZE];
} comPacketFileInfoResponse;

////////////////////
// File data request
typedef struct
{
	comPacketHeader Header;

	uint8_t FileID;
	uint32_t FilePos;
	uint8_t Length;
} comPacketFileDataRequest;

///////////////////////////////////
// File data response (header only)
typedef struct
{
	comPacketHeader Header;

	uint8_t FileID;
	uint32_t FilePos;
} comFileDataResponseHeader;

// Ends packed struct
#include <sysPackedStructEnd.h>

#endif
