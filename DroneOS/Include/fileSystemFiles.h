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

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/
typedef struct
{
	sysConstString Name;
	const uint8_t* Content;
	uint32_t Length;
} fileInternalFileTableEntry;

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/
extern fileInternalFileTableEntry g_system_files_info_table[];

#endif
