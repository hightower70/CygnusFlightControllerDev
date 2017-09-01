/*****************************************************************************/
/* System (flash) files handler functions                                    */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <fileSystemFiles.h>
#include <sysString.h>

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Convert file name to file index
/// @param in_file_name Filename to convert
/// @return File index (0xff if file name is invalid)
uint8_t fileSystemFileGetIndex(sysString in_file_name)
{
	uint8_t index;

	index = 0;
	while (g_system_files_info_table[index].Name != sysNULL)
	{
		if (sysCompareConstStringNoCase(in_file_name, g_system_files_info_table[index].Name) == 0)
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

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets system file count
/// @return Number of system files
uint8_t fileSystemFileGetCount(void)
{
	uint8_t index;

	// file entries
	index = 0;
	while (g_system_files_info_table[index].Name != sysNULL)
	{
		index++;
	}

	return index;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets length of a system file
/// @param in_file_index System file index
/// @return Length of the file
uint32_t fileSystemFileGetLength(uint8_t in_file_index)
{
	uint8_t file_count;

	file_count = fileSystemFileGetCount();

	if (in_file_index < file_count)
	{
		if (g_system_files_info_table[in_file_index].Callback == sysNULL)
		{
			return g_system_files_info_table[in_file_index].Length;
		}
		else
		{
			uint32_t length;

			if (g_system_files_info_table[in_file_index].Callback(fileCF_GetLength, &length, sizeof(length), 0))
				return length;
			else
				return 0;
		}
	}
	else
	{
		return 0;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets system file flag
/// @param in_file_index System file index
/// @return File flags (see fileSFF_ values for possible flag values)
uint8_t fileSystemFileGetFlag(uint8_t in_file_index)
{
	uint8_t file_count;

	file_count = fileSystemFileGetCount();

	if (in_file_index < file_count)
		return g_system_files_info_table[in_file_index].Flags;
	else
		return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets system file storage buffer pointer
/// @param in_file_index System file index
/// @return Pointer to the binary buffer
const uint8_t* fileSystemFileGetContent(uint8_t in_file_index)
{
	uint8_t file_count;

	file_count = fileSystemFileGetCount();

	if (in_file_index < file_count)
	{
		if (g_system_files_info_table[in_file_index].Callback == sysNULL)
		{
			return (uint8_t*)g_system_files_info_table[in_file_index].Content;
		}
		else
		{
			uint8_t* content;

			if (g_system_files_info_table[in_file_index].Callback(fileCF_GetContent, &content, sizeof(content), 0))
				return content;
			else
				return sysNULL;
		}
	}
	else
	{
		return sysNULL;
	}
}
