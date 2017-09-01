/*****************************************************************************/
/* Real time object storage telemetry functions                              */
/*                                                                           */
/* Copyright (C) 2017 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __roxStorage_h
#define __roxStorage_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define roxSTORAGE_INVALID_OBJECT_INDEX 0xffff

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/
typedef uint16_t roxObjectIndex;
typedef uint16_t roxMemberAddress; 
typedef void(*roxObjectChangedCallbackFunction)(roxObjectIndex in_object_index);

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void roxStorageInitialize(void);
void roxCallbackFunctionAdd(roxObjectIndex in_object_index, roxObjectChangedCallbackFunction in_callback_function);
void roxCallbackFunctionDelete(roxObjectChangedCallbackFunction in_callback_function, roxObjectIndex in_object_index);

void roxObjectReadBegin(roxObjectIndex in_object_index);
void roxObjectReadEnd(roxObjectIndex in_object_index);

bool roxObjectWriteBegin(roxObjectIndex in_object_index);
void roxObjectWriteEnd(roxObjectIndex in_object_index);

float roxGetFloat(roxObjectIndex in_object_index, roxMemberAddress in_member_address);
bool roxSetFloat(roxObjectIndex in_object_address, roxMemberAddress in_member_address, float in_value);






#endif
