/*****************************************************************************/
/* Configuration file storage                                                */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

#ifndef __cfgStorage_h
#define __cfgStorage_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>
#include <fileSystemFiles.h>
#include "cfgConstants.h"

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
extern uint8_t g_configuration_data[];

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void cfgStorageInit(void);
void cfgLoadDefaultConfiguration(void);
void cfgLoadConfiguration(void);
void cfgSaveConfiguration(void);
bool cfgValueDataFileHandler(fileCallbackRequest in_request, void* in_buffer, uint16_t in_buffer_length, uint16_t in_start_pos);


sysString cfgGetStringValue(uint16_t in_value_index);
uint16_t cfgGetUInt16Value(uint16_t in_value_index);
uint32_t cfgGetUInt32Value(uint16_t in_value_index);
uint8_t cfgGetEnumValue(uint16_t in_value_index);
float cfgGetFloatValue(uint16_t in_value_index);



#endif
