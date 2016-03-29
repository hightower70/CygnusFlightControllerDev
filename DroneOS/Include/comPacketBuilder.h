/*****************************************************************************/
/* Communication packet builder functions                                    */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/
#ifndef __comPacketBuilder_h
#define __comPacketBuilder_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <comSystemPacketDefinitions.h>

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void comFillPacketHeader(comPacketHeader* in_header, uint8_t in_type, uint8_t in_length);

#endif
