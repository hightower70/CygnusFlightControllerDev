/*****************************************************************************/
/* Communication manager functions                                           */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

#ifndef __comInterfaces_h
#define __comInterfaces_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/
typedef void(*comInterfaceBufferReleaseFunction)(uint8_t in_buffer_index);

typedef struct
{
	comInterfaceBufferReleaseFunction BufferRelease;

} comInterfaceDescription;

#endif
