/*****************************************************************************/
/* Handles file transfer communication                                       */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <fileTransfer.h>
#include <fileSystemFiles.h>
#include <strString.h>
#include <comSystemPacketDefinitions.h>
#include <crcMD5.h>

///////////////////////////////////////////////////////////////////////////////
// Module local functions
static void fileProcessFileInfoRequestPacket(comPacketInfo* in_packet_info, comPacketFileInfoRequest* in_request);
static void gcscomProcessGetFileDataPacket(uint8_t* in_packet);
static uint8_t fileGetSystemFileIndex(sysString in_file_name);


///////////////////////////////////////////////////////////////////////////////
/// @brief Processes file request command
/// @param in_packet Packet containing file request (all other packet types are ingnored)
void fileProcessFileTransfer(comPacketInfo* in_packet_info, uint8_t* in_packet)
{
	comPacketHeader* packet_header = (comPacketHeader*)in_packet;

	switch(packet_header->PacketType)
	{
		case comPT_FILE_INFO_REQUEST:
			fileProcessFileInfoRequestPacket(in_packet_info, (comPacketFileInfoRequest*)in_packet);
			return;

		case comPT_FILE_BLOCK_REQUEST:
			return;
	}
}

/*****************************************************************************/
/* Local functions                                                           */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Process file info request packet
/// @param in_packet Packet to process
static void fileProcessFileInfoRequestPacket(comPacketInfo* in_packet_info, comPacketFileInfoRequest* in_request_packet)
{
	comPacketFileInfoResponse* response_packet;
	crcMD5State md5_state;
	uint16_t packet_index;

	response_packet = (comPacketFileInfoResponse*)comManagerTransmitPacketPushStart(sizeof(comPacketFileInfoResponse), in_packet_info->Interface, comPT_FILE_INFO_RESPONSE, packet_index);
	if (response_packet != sysNULL)
	{
		// get file index (ID) file the file name
		response_packet->FileID = fileGetSystemFileIndex(in_request_packet->FileName);
		if (response_packet->FileID != fileINVALID_SYSTEM_FILE_ID)
		{
			// store length
			response_packet->FileLength = g_system_files_info_table[response_packet->FileID].Length;

			// calculate MD5 checksum
			crcMD5Open(&md5_state);
			crcMD5Update(&md5_state, g_system_files_info_table[response_packet->FileID].Content, g_system_files_info_table[response_packet->FileID].Length);
			crcMD5Close(&md5_state, response_packet->FileHash);
		}

		// start packet transmission
		comManagerTransmitPacketPushEnd(packet_index);
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Process file data request packet
static void gcscomProcessGetFileDataPacket(comPacketInfo* in_packet_info, comPacketFileDataRequest* in_request_packet)
{
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Convert file name to file index
/// @param in_file_name Filename to convert
/// @return File index (0xff if file name is invalid)
static uint8_t fileGetSystemFileIndex(sysString in_file_name)
{
	uint8_t index;

	index = 0;
	while(g_system_files_info_table[index].Name != sysNULL)
	{
		if(strCompareConstStringNoCase(in_file_name, g_system_files_info_table[index].Name) == 0)
		{
			return index;
		}
		else
		{
			index++;
		}
	}

	return fileINVALID_SYSTEM_FILE_ID;
}
