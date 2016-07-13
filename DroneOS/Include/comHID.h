/*****************************************************************************/
/* USB HID Communication routines                                            */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

#ifndef __comHID_h
#define __comHID_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define comHID_PACKET_HEADER_SIZE 1

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void comHIDInit(void);
void comHIDDeinit(void);

#endif
