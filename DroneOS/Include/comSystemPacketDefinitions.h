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
#include <sysypes.h>

///////////////////////////////////////////////////////////////////////////////
// Constants
#define comFILENAME_LENGTH 32


// Starts packed struct
#include <sysPackedStructStart.h>

///////////////////////////////////////////////////////////////////////////////
// Constants

// packet types
#define PT_HEART_BEAT 1
#define PT_STATUS 2

#define comPT_SYSTEM_FLAG 0x80
#define comPT_REQUEST_FLAG 0x40
#define comPT_FILE_INFO (comPT_SYSTEM_FLAG  | 1)
#define comPT_FILE_BLOCK (comPT_SYSTEM_FLAG | 2)

// packet definitions

/////////////////////
// Packet header
typedef struct
{
	uint8_t PacketLength;
	uint8_t PacketType;
	uint8_t PacketCounter;

} PacketHeaderType;

/////////////////////
// Heart Beat packet
typedef struct
{
	// system status
	uint32_t Status;

	// used telemetry bandwidth
	uint16_t UsedBandWidth;

} HeartBeatPacketType;



/////////////////////
// Packet header
typedef struct
{
	uint8_t PacketLength;
	uint8_t PacketType;
	uint8_t PacketID;
} gcscomRequestResponsePacketHeaderType;

typedef struct
{
	rtosChar FileName[GCSCOM_FILENAME_LENGTH];
} gcscomFileInfoQueryType;

typedef struct
{
	uint8_t FileID;
	uint32_t FileLength;
	uint8_t FileHash[16];
} gcscomFileInfoResponseType;

typedef struct
{
	uint8_t FileID;
	uint32_t FilePos;
	uint8_t Length;
} gcscomFileDataQueryType;

typedef struct
{
	uint8_t FileID;
	uint32_t FilePos;
} gcscomFileDataResponseHeaderType;

// Ends packed struct
#include <sysPackedStructEnd.h>

#endif
