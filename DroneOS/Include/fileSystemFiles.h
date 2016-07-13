/*****************************************************************************/
/* System (flash) files storage and handling                                 */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/
#ifndef __fileSystemFiles_h
#define __fileSystemFiles_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define fileINVALID_SYSTEM_FILE_ID 0xff

#define fileSFF_READ_WRITE (1<<0)
#define fileSFF_READ_ONLY (0)

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/

/// File handler callback request reasons
typedef enum
{
	fileCF_GetLength,
	fileCF_GetMD5,
	fileCF_GetContent,
	fileCF_ReadBlock,
	fileCF_WriteBlock,
	fileCF_FinishSuccess,
	fileCF_FinishCancel
} fileCallbackRequest;

/// File request callback function
typedef bool(*fileSystemFileHandlerCallback)(fileCallbackRequest in_function, void* in_buffer, uint16_t in_buffer_length, uint16_t in_start_pos);

/// Entry (file information) for internal file table
typedef struct
{
	sysConstString Name;
	uint8_t* Content;
	fileSystemFileHandlerCallback Callback;
	uint32_t Length;
	uint8_t Flags;
} fileInternalFileTableEntry;

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/
extern fileInternalFileTableEntry g_system_files_info_table[];

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
uint8_t fileSystemFileGetIndex(sysString in_file_name);
uint8_t fileSystemFileGetCount(void);
uint32_t fileSystemFileGetLength(uint8_t in_file_index);
const uint8_t* fileSystemFileGetContent(uint8_t in_file_index);
uint8_t fileSystemFileGetFlag(uint8_t in_file_index);

#endif
