/*****************************************************************************/
/* External I2C bus driver                                                   */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

#ifndef __drvEXT_h
#define __drvEXT_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/
typedef void (*drvEXTCallbackFunction)(bool in_success, void* in_interupt_param);

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void drvEXTInit(void);
void drvEXTStartWriteAndReadBlock(uint8_t in_address, uint8_t* in_write_buffer, uint8_t in_write_buffer_length, uint8_t* in_read_buffer, uint8_t in_read_buffer_length, drvEXTCallbackFunction in_callback_function);
void drvEXTStartWriteAndWriteBlock(uint8_t in_address, uint8_t* in_buffer1, uint8_t in_buffer1_length, uint8_t* in_buffer2, uint8_t in_buffer2_length, drvEXTCallbackFunction in_callback_function);

#endif
