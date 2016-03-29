/*****************************************************************************/
/* Handles file transfer communication                                       */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

#ifndef __fileTransfer_h
#define __fileTransfer_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>
#include <comManager.h>

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void fileProcessFileTransfer(comPacketInfo* in_packet_info, uint8_t* in_packet);


#endif