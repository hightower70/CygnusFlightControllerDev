/*****************************************************************************/
/* EEPROM driver (Linux implementation using file)                           */
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
#include <drvEEPROM.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <limits.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define halEEPROM_SIZE 2048

/*****************************************************************************/
/* Module local functions                                                    */
/*****************************************************************************/
static uint32_t halEEPROMGetFileLength(void);

/*****************************************************************************/
/* Module global variable                                                    */
/*****************************************************************************/
static char l_file_path[PATH_MAX];
extern int g_argc;
extern char** g_argv;

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initialize EEPROM driver
void drvEEPROMInit(void)
{
	FILE* config_file;
	char* extension;
	long file_length;
	uint8_t buffer;

	// generate file name
	strncpy(l_file_path, g_argv[0], PATH_MAX);

	extension = strchr(l_file_path, '.');
	if (extension != NULL)
	{
		extension++;
		strcpy(extension, "eep");
	}

	// check if file exists and length is ok
	file_length = halEEPROMGetFileLength();

	// generate empty EEPROM file
	if (file_length != halEEPROM_SIZE)
	{
		config_file = fopen(l_file_path, "w");
		if (config_file != NULL)
		{
			buffer = 0xff;
			for (file_length = 0; file_length < halEEPROM_SIZE; file_length++)
				fwrite(&buffer, sizeof(uint8_t), 1, config_file);
			fclose(config_file);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Reads block from EEPROM
/// @param in_address Read from address
/// @param out_buffer Buffer will receive data
/// @param in_length Number of bytes to read
bool drvEEPROMReadBlock(uint16_t in_address, uint8_t* out_buffer, uint16_t in_length)
{
	FILE* config_file;

	// read block of file
	config_file = fopen(l_file_path, "r");
	if (config_file != NULL)
	{
		fseek(config_file, in_address, SEEK_SET);
		fread(out_buffer, sizeof(uint8_t), in_length, config_file);
		fclose(config_file);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Writes block to the EEPROM
/// @param in_address Write to address
/// @param in_buffer Buffer containing data to write
/// @param in_length Number of bytes to write
bool drvEEPROMWriteBlock(uint16_t in_address, uint8_t* in_buffer, uint16_t in_length)
{
	FILE* config_file;

	config_file = fopen(l_file_path, "r+");
	if (config_file != NULL)
	{
		fseek(config_file, in_address, SEEK_SET);
		fwrite(in_buffer, sizeof(uint8_t), in_length, config_file);
		fclose(config_file);
	}

	return true;
}

/*****************************************************************************/
/* Local function implementation                                             */
/*****************************************************************************/


///////////////////////////////////////////////////////////////////////////////
/// @brief Gets EEPROM file actual length
/// @return Length of the file
static uint32_t halEEPROMGetFileLength(void)
{
	struct stat stat_info;

	if (stat(l_file_path, &stat_info) == 0)
		return (uint32_t)stat_info.st_size;
	else
		return 0;
}
