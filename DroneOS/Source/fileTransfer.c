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
static void fileProcessFileInfoRequest(comPacketInfo* in_packet_info, comPacketFileInfoRequest* in_request);
static void fileProcessFileDataReadRequest(comPacketInfo * in_packet_info, comPacketFileDataReadRequest * in_request_packet);
static void fileProcessFileDataWriteRequest(comPacketInfo* in_packet_info, comPacketFileDataWriteRequestHeader* in_request_packet);
static void fileProcessFileOperationFinishedRequest(comPacketInfo* in_packet_info, comPacketFileOperationFinishedRequest* in_request_packet);

///////////////////////////////////////////////////////////////////////////////
/// @brief Processes file request command
/// @brief in_packet_info Local packet information (interface number, received timestamp, etc.)
/// @param in_packet Packet, containing file request (all other packet types are ingnored)
void fileProcessFileTransfer(comPacketInfo* in_packet_info, uint8_t* in_packet)
{
	comPacketHeader* packet_header = (comPacketHeader*)in_packet;

	switch(packet_header->PacketType)
	{
		case comPT_FILE_INFO_REQUEST:
			fileProcessFileInfoRequest(in_packet_info, (comPacketFileInfoRequest*)in_packet);
			return;

		case comPT_FILE_DATA_READ_REQUEST:
			fileProcessFileDataReadRequest(in_packet_info, (comPacketFileDataReadRequest*)in_packet);
			return;

		case comPT_FILE_DATA_WRITE_REQUEST:
			fileProcessFileDataWriteRequest(in_packet_info, (comPacketFileDataWriteRequestHeader*)in_packet);
			return;

		case comPT_FILE_OPERATION_FINISHED_REQUEST:
			fileProcessFileOperationFinishedRequest(in_packet_info, (comPacketFileOperationFinishedRequest*)in_packet);
			return;
	}
}

/*****************************************************************************/
/* Local functions                                                           */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Process file info request packet
/// @param in_packet Packet to process
static void fileProcessFileInfoRequest(comPacketInfo* in_packet_info, comPacketFileInfoRequest* in_request_packet)
{
	comPacketFileInfoResponse* response_packet;
	crcMD5State md5_state;
	uint16_t packet_index;

	response_packet = (comPacketFileInfoResponse*)comManagerTransmitPacketPushStart(sizeof(comPacketFileInfoResponse), in_packet_info->Interface, comPT_FILE_INFO_RESPONSE, &packet_index);
	if (response_packet != sysNULL)
	{
		// get file index (ID) file the file name
		response_packet->FileID = fileSystemFileGetIndex(in_request_packet->FileName);
		if (response_packet->FileID != fileINVALID_SYSTEM_FILE_ID)
		{
			if (g_system_files_info_table[response_packet->FileID].Callback == sysNULL)
			{
				// store length
				response_packet->FileLength = g_system_files_info_table[response_packet->FileID].Length;

				// calculate MD5 checksum
				crcMD5Open(&md5_state);
				crcMD5Update(&md5_state, g_system_files_info_table[response_packet->FileID].Content, g_system_files_info_table[response_packet->FileID].Length);
				crcMD5Close(&md5_state, (crcMD5Hash*)&response_packet->FileHash);
			}
			else
			{
				uint32_t length;

				// get length
				if (g_system_files_info_table[response_packet->FileID].Callback(fileCF_GetLength, &length, sizeof(length), 0))
				{
					response_packet->FileLength = length;
				}
				else
				{
					response_packet->FileLength = 0;
				}

				// get MD5
				g_system_files_info_table[response_packet->FileID].Callback(fileCF_GetMD5, response_packet->FileHash, sizeof(crcMD5Hash), 0);
			}

			// start packet transmission
			comManagerTransmitPacketPushEnd(packet_index);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Process file data request packet
static void fileProcessFileDataReadRequest(comPacketInfo* in_packet_info, comPacketFileDataReadRequest* in_request_packet)
{
	uint16_t packet_index;
	comFileDataReadResponseHeader* response_packet;
	uint8_t data_length = in_request_packet->Length;
	uint8_t* source_data_pointer;
	uint8_t* destination_data_pointer;
	uint32_t file_data_length;
	uint8_t system_file_count;

	// check file id validity
	system_file_count = fileSystemFileGetCount();
	if (in_request_packet->FileID >= system_file_count)
		return;

	// check file length
	if (in_request_packet->FilePos >= g_system_files_info_table[in_request_packet->FileID].Length)
		return;

	file_data_length = in_request_packet->Length;
	if (in_request_packet->FilePos + in_request_packet->Length >= g_system_files_info_table[in_request_packet->FileID].Length)
	{
		file_data_length = g_system_files_info_table[in_request_packet->FileID].Length - in_request_packet->FilePos;
	}

	// reserve packet storage
	response_packet = (comFileDataReadResponseHeader*)comManagerTransmitPacketPushStart(sizeof(comFileDataReadResponseHeader) + data_length, in_packet_info->Interface, comPT_FILE_DATA_READ_RESPONSE, &packet_index);
	if (response_packet != sysNULL)
	{
		// fill out response information
		response_packet->FileID = in_request_packet->FileID;
		response_packet->FilePos = in_request_packet->FilePos;

		destination_data_pointer = (uint8_t*)(comManagerGetTransmitPacketGetBuffer(packet_index) + sizeof(comFileDataReadResponseHeader));

		if (g_system_files_info_table[response_packet->FileID].Callback == sysNULL)
		{
			// copy response file data
			source_data_pointer = g_system_files_info_table[in_request_packet->FileID].Content + in_request_packet->FilePos;

			sysMemCopy(destination_data_pointer, source_data_pointer, file_data_length);
		}
		else
		{
			g_system_files_info_table[response_packet->FileID].Callback(fileCF_ReadBlock, destination_data_pointer, (uint16_t)file_data_length, (uint16_t)in_request_packet->FilePos);
		}

		// start packet transmission
		comManagerTransmitPacketPushEnd(packet_index);
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Process file data request packet
static void fileProcessFileDataWriteRequest(comPacketInfo* in_packet_info, comPacketFileDataWriteRequestHeader* in_request_packet)
{
	uint16_t packet_index;
	uint8_t system_file_count;
	comPacketFileDataWriteResponse* response_packet;
	uint8_t* source_data_pointer;
	uint8_t* destination_data_pointer;

	response_packet = (comPacketFileDataWriteResponse*)comManagerTransmitPacketPushStart(sizeof(comPacketFileDataWriteResponse), in_packet_info->Interface, comPT_FILE_DATA_WRITE_RESPONSE, &packet_index);
	if (response_packet != sysNULL)
	{
		response_packet->Error = comFRC_OK;
		response_packet->FileID = in_request_packet->FileID;

		// check file id validity
		system_file_count = fileSystemFileGetCount();
		if (response_packet->FileID >= system_file_count)
		{
			response_packet->FileID = fileINVALID_SYSTEM_FILE_ID;
			response_packet->Error = comFRC_NOT_FOUND;
		}
		else
		{
			// check file length
			if ((in_request_packet->FilePos + in_request_packet->DataLength) >= g_system_files_info_table[in_request_packet->FileID].Length)
			{
				response_packet->Error = comFRC_INVALID;
			}
			else
			{
				// check file RW flag
				if ((g_system_files_info_table[in_request_packet->FileID].Flags & fileSFF_READ_WRITE) == 0)
				{
					response_packet->Error = comFRC_READ_ONLY;
				}
				else
				{
					source_data_pointer = ((uint8_t*)in_request_packet) + sizeof(comPacketFileDataWriteRequestHeader);

					if (g_system_files_info_table[response_packet->FileID].Callback == sysNULL)
					{
						// copy data
						destination_data_pointer = g_system_files_info_table[in_request_packet->FileID].Content + in_request_packet->FilePos;

						sysMemCopy(destination_data_pointer, source_data_pointer, in_request_packet->DataLength);
					}
					else
					{
						g_system_files_info_table[response_packet->FileID].Callback(fileCF_WriteBlock, source_data_pointer, in_request_packet->DataLength, (uint16_t)in_request_packet->FilePos);
					}
				}
			}
		}

		// start packet transmission
		comManagerTransmitPacketPushEnd(packet_index);
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Process file operation finished request
static void fileProcessFileOperationFinishedRequest(comPacketInfo* in_packet_info, comPacketFileOperationFinishedRequest* in_request_packet)
{
	uint16_t packet_index;
	comPacketFileOperationFinishedResponse* response_packet;
	uint8_t system_file_count;

	response_packet = (comPacketFileOperationFinishedResponse*)comManagerTransmitPacketPushStart(sizeof(comPacketFileOperationFinishedResponse), in_packet_info->Interface, comPT_FILE_OPERATION_FINISHED_RESPONSE, &packet_index);
	if (response_packet != sysNULL)
	{
		response_packet->FileID = in_request_packet->FileID;
		response_packet->FinishMode = in_request_packet->FinishMode;

		// check file ID
		system_file_count = fileSystemFileGetCount();
		if (response_packet->FileID >= system_file_count)
		{
			response_packet->FileID = fileINVALID_SYSTEM_FILE_ID;
			response_packet->Error = comFRC_NOT_FOUND;
		}
		else
		{

			switch (in_request_packet->FinishMode)
			{
				case comFOFM_SUCCESS:
					if (g_system_files_info_table[response_packet->FileID].Callback != sysNULL)
					{
						g_system_files_info_table[response_packet->FileID].Callback(fileCF_FinishSuccess, &response_packet->Error, sizeof(uint8_t), 0);
					}
					break;

				case comFOFM_CANCEL:
					if (g_system_files_info_table[response_packet->FileID].Callback != sysNULL)
					{
						g_system_files_info_table[response_packet->FileID].Callback(fileCF_FinishCancel, &response_packet->Error, sizeof(uint8_t), 0);
					}
					break;
			}

			// start packet transmission
			comManagerTransmitPacketPushEnd(packet_index);
		}
	}
}
