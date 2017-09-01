/*****************************************************************************/
/* Configuration file storage                                                */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysRTOS.h>
#include <crcMD5.h>
#include <drvEEPROM.h>
#include <fileSystemFiles.h>
#include <cfgStorage.h>
#include <comSystemPacketDefinitions.h>
#include "cfgConstants.h"

/*****************************************************************************/
/* Const                                                                     */
/*****************************************************************************/
#define cfg_STORAGE_COUNT 2

// configuration value types
#define cfg_VT_UINT8 1
#define cfg_VT_INT8 2
#define cfg_VT_UINT16 3
#define cfg_VT_INT16 4
#define cfg_VT_UINT32 5
#define cfg_VT_INT32 6

#define cfg_VT_UINT8_FIXED 7
#define cfg_VT_INT8_FIXED 8
#define cfg_VT_UINT16_FIXED 9
#define cfg_VT_INT16_FIXED 10

#define cfg_VT_ENUM 11

#define cfg_VT_FLOAT 12

#define cfg_VT_STRING 13

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/
#include <sysPackedStructStart.h>
typedef struct
{
	uint16_t Length;
	uint8_t Hash[crcMD5_HASH_SIZE];
} cfgConfigurationDataHeader;

// Value information struct. Used in 'ConfigurationValueInfo' file.
typedef struct
{
	uint16_t Offset;	// value position in the value data file
	uint8_t Size;			// binary size of the value
	uint8_t Type;			// type of the value
} cfgConfigurationValueInfo;

#include <sysPackedStructEnd.h>

/*****************************************************************************/
/* Module local variables                                                    */
/*****************************************************************************/
static uint8_t l_configuration_data_primary[cfg_VALUE_DATA_FILE_LENGTH];
static uint8_t l_configuration_data_secondary[cfg_VALUE_DATA_FILE_LENGTH];
static uint8_t l_config_storage_index[cfg_VALUE_COUNT];
static cfgConfigurationValueInfo* l_configuration_value_info;

/*****************************************************************************/
/* Module local functions                                                    */
/*****************************************************************************/
static void cfgActualizePrimaryBuffer(void);
static uint16_t cfgGetValueIndexFromPos(uint16_t in_pos);

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @param Initializes configuration storage
void cfgStorageInit(void)
{
	drvEEPROMInit();
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Loads configration from external storage (EEPROM)
void cfgLoadConfiguration(void)
{
	cfgConfigurationDataHeader settings_header;
	uint8_t xml_file_id;
	uint32_t xml_file_length;
	crcMD5Hash xml_file_hash;
	crcMD5State md5_state;
	uint16_t value_index;

	// move config settings to the primary buffer
	cfgActualizePrimaryBuffer();
		
	// get xml file information
	xml_file_id = fileSystemFileGetIndex("ConfigurationXML");
	xml_file_length = fileSystemFileGetLength(xml_file_id);
	crcMD5Open(&md5_state);
	crcMD5Update(&md5_state, fileSystemFileGetContent(xml_file_id), xml_file_length);
	crcMD5Close(&md5_state, &xml_file_hash);

	// load settings header
	drvEEPROMReadBlock(0, (uint8_t*)&settings_header, sizeof(cfgConfigurationDataHeader));

	// check settings validity
	if (settings_header.Length == cfg_VALUE_DATA_FILE_LENGTH && crcMD5IsEqual(&xml_file_hash, (crcMD5Hash*)&settings_header.Hash))
	{
		// settings seems to be ok -> load settings
		drvEEPROMReadBlock(sizeof(cfgConfigurationDataHeader), (uint8_t*)&l_configuration_data_secondary, cfg_VALUE_DATA_FILE_LENGTH);

		// actualize secondary buffer
		for (value_index = 0; value_index < cfg_VALUE_COUNT; value_index++)
		{
			l_config_storage_index[value_index] = 1;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Loads default configuration
void cfgLoadDefaultConfiguration(void)
{
	uint8_t file_id;

	// get default settings file
	file_id = fileSystemFileGetIndex("DefaultConfigurationData");
	sysMemCopy(l_configuration_data_primary, fileSystemFileGetContent(file_id), cfg_VALUE_DATA_FILE_LENGTH);
	sysMemZero(l_config_storage_index, sizeof(l_config_storage_index));

	// get value info file
	file_id = fileSystemFileGetIndex("ConfigurationValueInfo");
	l_configuration_value_info = (cfgConfigurationValueInfo*)fileSystemFileGetContent(file_id);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Copies all configuration data into the primary (first) storage buffer of the configuration data
static void cfgActualizePrimaryBuffer(void)
{
	uint16_t value_index;

	for (value_index = 0; value_index < cfg_VALUE_COUNT; value_index++)
	{
		if (l_config_storage_index[value_index] != 0)
		{
			// copy config value from the secondary buffer to the primary
			sysMemCopy(&l_configuration_data_primary[l_configuration_value_info[value_index].Offset], &l_configuration_data_secondary[l_configuration_value_info[value_index].Offset], l_configuration_value_info[value_index].Size);
			l_config_storage_index[value_index] = 0; // actualize primary buffer
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Saves current configuration into external storage (EEPROM)
void cfgSaveConfiguration(void)
{
	cfgConfigurationDataHeader settings_header;
	uint8_t xml_file_id;
	uint32_t xml_file_length;
	crcMD5State md5_state;

	// move config data to the primary buffer
	cfgActualizePrimaryBuffer();

	// prepare configuration header
	xml_file_id = fileSystemFileGetIndex("ConfigurationXML");
	xml_file_length = fileSystemFileGetLength(xml_file_id);
	crcMD5Open(&md5_state);
	crcMD5Update(&md5_state, fileSystemFileGetContent(xml_file_id), xml_file_length);
	crcMD5Close(&md5_state,(crcMD5Hash*)&settings_header.Hash);

	settings_header.Length = cfg_VALUE_DATA_FILE_LENGTH;

	// write header
	drvEEPROMWriteBlock(0, (uint8_t*)&settings_header, sizeof(cfgConfigurationDataHeader));

	// write data
	drvEEPROMWriteBlock(sizeof(cfgConfigurationDataHeader), l_configuration_data_primary, cfg_VALUE_DATA_FILE_LENGTH);
}

/*****************************************************************************/
/* File handling functions                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Handles file requests from the system file manager. The configuration value data file has double buffer therefore it needs a special access.
/// @param in_request File operation request type
/// @param in_buffer Buffer used for data transfer
/// @param in_buffer_length Length of the buffer in bytes used for data transfer
/// @param in_start_position The position within the file where operation must be started
/// @return True if file operation was success
bool cfgValueDataFileHandler(fileCallbackRequest in_request, void* in_buffer, uint16_t in_buffer_length, uint16_t in_start_pos)
{
	switch (in_request)
	{
		// Get length of the file
		case fileCF_GetLength:
			*((uint32_t*)in_buffer) = cfg_VALUE_DATA_FILE_LENGTH;
			return true;
			
		// Get MD5 checksum
		case fileCF_GetMD5:
		{
			crcMD5State md5_state;

			cfgActualizePrimaryBuffer();

			crcMD5Open(&md5_state);
			crcMD5Update(&md5_state, l_configuration_data_primary, cfg_VALUE_DATA_FILE_LENGTH);
			crcMD5Close(&md5_state, (crcMD5Hash*)in_buffer);
		}
		return true;
			
		// gets content
		case fileCF_GetContent:
			cfgActualizePrimaryBuffer();

			*((uint8_t**)in_buffer) = l_configuration_data_primary;

			return true;

		// reads data block
		case fileCF_ReadBlock:
		{
			uint16_t value_index;
			uint16_t source_pos;
			uint16_t length;
			uint8_t* source_data;
			uint8_t* destination_data;

			destination_data = (uint8_t*)in_buffer;
			value_index = cfgGetValueIndexFromPos(in_start_pos);
			source_pos = in_start_pos;
			if (l_config_storage_index[value_index] == 0)
				source_data = &l_configuration_data_primary[source_pos];
			else
				source_data = &l_configuration_data_secondary[source_pos];

			for (length = 0; length < in_buffer_length; length++)
			{
				*destination_data++ = *source_data++;
				source_pos++;

				// switch to the next value when end of the current value of reached
				if (l_configuration_value_info[value_index].Offset + l_configuration_value_info[value_index].Size < source_pos)
				{
					value_index++;

					if (l_config_storage_index[value_index] == 0)
						source_data = &l_configuration_data_primary[source_pos];
					else
						source_data = &l_configuration_data_secondary[source_pos];
				}
			}
		}
		return true;

		// write data block
		case fileCF_WriteBlock:
		{
			uint16_t value_index;
			uint16_t destination_pos;
			uint16_t length;
			uint8_t* source_data;
			uint8_t* destination_data;

			source_data = (uint8_t*)in_buffer;
			value_index = cfgGetValueIndexFromPos(in_start_pos);
			destination_pos = in_start_pos;

			if (l_config_storage_index[value_index] == 0)
				destination_data = &l_configuration_data_secondary[destination_pos];
			else
				destination_data = &l_configuration_data_primary[destination_pos];

			for (length = 0; length < in_buffer_length; length++)
			{
				*destination_data++ = *source_data++;
				destination_pos++;

				// switch to the next value when end of the current value of reached
				if (l_configuration_value_info[value_index].Offset + l_configuration_value_info[value_index].Size <= destination_pos)
				{
					// switch to the newly written storage
					l_config_storage_index[value_index] = 1 - l_config_storage_index[value_index];

					value_index++;

					if (l_config_storage_index[value_index] == 0)
						destination_data = &l_configuration_data_secondary[destination_pos];
					else
						destination_data = &l_configuration_data_primary[destination_pos];
				}
			}
		}
		return true;

		case fileCF_FinishSuccess:
			cfgSaveConfiguration();
			*(uint8_t*)in_buffer = comFRC_OK;
			return true;

		case fileCF_FinishCancel:
			cfgLoadConfiguration();
			*(uint8_t*)in_buffer = comFRC_OK;
			return true;

		default:
			return false;
	} 
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets value index from value data file position
/// @param in_pos value data file position
/// @return Value index
static uint16_t cfgGetValueIndexFromPos(uint16_t in_pos)
{
	uint16_t first_index;
	uint16_t last_index;
	uint16_t middle_index;

	first_index = 0;
	last_index = cfg_VALUE_COUNT - 1;
	middle_index = (first_index + last_index) / 2;

	while (first_index <= last_index) 
	{
		middle_index = (first_index + last_index) / 2;

		if (l_configuration_value_info[middle_index].Offset > in_pos)
		{
			last_index = middle_index - 1;
		}
		else
		{
			if (l_configuration_value_info[middle_index].Offset < in_pos)
				first_index = middle_index + 1;
			else
				return middle_index;
		}
	}

	if (middle_index > 0)
		return middle_index - 1;
	else
		return 0;
}

/*****************************************************************************/
/* Value access functions                                                    */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets configuration string value
/// @param in_value_index Index of the value to get
/// @return Configuration string
sysString cfgGetStringValue(uint16_t in_value_index)
{
	sysASSERT(in_value_index < cfg_VALUE_COUNT);
	sysASSERT(l_configuration_value_info[in_value_index].Type == cfg_VT_STRING);

	if(l_config_storage_index[in_value_index] == 0)
		return (sysString)&l_configuration_data_primary[l_configuration_value_info[in_value_index].Offset];
	else
		return (sysString)&l_configuration_data_secondary[l_configuration_value_info[in_value_index].Offset];
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets configuration uint16 value
/// @param in_value_index Index of the value to get
/// @return Configuration value
uint16_t cfgGetUInt16Value(uint16_t in_value_index)
{
	sysASSERT(in_value_index < cfg_VALUE_COUNT);
	sysASSERT(l_configuration_value_info[in_value_index].Type == cfg_VT_INT32);

	if (l_config_storage_index[in_value_index] == 0)
		return *(uint16_t*)&l_configuration_data_primary[l_configuration_value_info[in_value_index].Offset];
	else
		return *(uint16_t*)&l_configuration_data_secondary[l_configuration_value_info[in_value_index].Offset];
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets configuration uint32 value
/// @param in_value_index Index of the value to get
/// @return Configuration value
uint32_t cfgGetUInt32Value(uint16_t in_value_index)
{
	sysASSERT(in_value_index < cfg_VALUE_COUNT);
	sysASSERT(l_configuration_value_info[in_value_index].Type == cfg_VT_INT32);

	if (l_config_storage_index[in_value_index] == 0)
		return *(uint32_t*)&l_configuration_data_primary[l_configuration_value_info[in_value_index].Offset];
	else
		return *(uint32_t*)&l_configuration_data_secondary[l_configuration_value_info[in_value_index].Offset];
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets configuration Enum value
/// @param in_value_index Index of the value to get
/// @return Configuration value
uint8_t cfgGetEnumValue(uint16_t in_value_index)
{
	sysASSERT(in_value_index < cfg_VALUE_COUNT);
	sysASSERT(l_configuration_value_info[in_value_index].Type == cfg_VT_ENUM);

	if (l_config_storage_index[in_value_index] == 0)
		return l_configuration_data_primary[l_configuration_value_info[in_value_index].Offset];
	else
		return l_configuration_data_secondary[l_configuration_value_info[in_value_index].Offset];
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets configuration float value
/// @param in_value_index Index of the value to get
/// @return Configuration value
float cfgGetFloatValue(uint16_t in_value_index)
{
	sysASSERT(in_value_index < cfg_VALUE_COUNT);
	sysASSERT(l_configuration_value_info[in_value_index].Type == cfg_VT_INT32);

	if (l_config_storage_index[in_value_index] == 0)
		return *(float*)&l_configuration_data_primary[l_configuration_value_info[in_value_index].Offset];
	else
		return *(float*)&l_configuration_data_secondary[l_configuration_value_info[in_value_index].Offset];
}
