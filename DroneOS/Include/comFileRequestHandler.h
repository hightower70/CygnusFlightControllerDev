/*****************************************************************************/
/* Handles File Request of Ground Control Station Communication              */
/*                                                                           */
/* Copyright (C) 2014 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/
#ifndef __gcscomFileRequestHandler_h
#define __gcscomFileRequestHandler_h

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <RTOSTypes.h>
#include <PacketDefinitions.h>

///////////////////////////////////////////////////////////////////////////////
// Constants
#define GCSCOM_INVALID_FILE_ID 0xff

///////////////////////////////////////////////////////////////////////////////
// Types
typedef struct
{
	rtosConstString Name;
	const uint8_t* Data;
	uint32_t Length;
} gcscomFileTableEntryType;

///////////////////////////////////////////////////////////////////////////////
// Global variables
extern gcscomFileTableEntryType g_gcscom_file_table[];

#endif
