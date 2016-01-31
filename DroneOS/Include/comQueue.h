/*****************************************************************************/
/* Ground Control Station Communication Module                               */
/*  Data paceket queue                                                       */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

#ifndef __comQueue_h
#define __comQueue_h

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <sysTypes.h>

typedef struct
{
	uint32_t TimeStamp;
	uint8_t PacketSize;
	uint8_t InterfaceNumber;
} comQueueEntryHeader;

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
void comQueueInit(void);



#endif