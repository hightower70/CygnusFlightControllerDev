/*****************************************************************************/
/* I2C Master driver                                                         */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

#ifndef __drvI2CMaster_h
#define __drvI2CMaster_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>
#include <stm32f4xx_hal.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/


/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/
typedef struct _drvI2CMasterModule drvI2CMasterModule;

typedef void (*drvI2CMasterCallbackFunction)(bool in_success, void* in_interrupt_param);

/// @brief Bus Status codes
typedef enum
{
	drvI2CM_ST_IDLE,
	drvI2CM_ST_WRITE_START,
	drvI2CM_ST_WRITE_ADDRESS,
	drvI2CM_ST_WRITE_DATA1,
	drvI2CM_ST_WRITE_DATA2,
	drvI2CM_ST_STOP,
	drvI2CM_ST_RESTART,
	drvI2CM_ST_READ_ADDRESS,
	drvI2CM_ST_READ_DATA,
	drvI2CM_ST_READ_STOP,

	drvI2CM_ST_ERROR = 0x80

} drvI2CMasterStatus;

/// @brief Bus status information
typedef struct _drvI2CMasterModule
{
	drvI2CMasterStatus Status;
  uint8_t Address;

  uint8_t* WriteBuffer1;
  uint8_t WriteBufferLength1;

  uint8_t* WriteBuffer2;
  uint8_t WriteBufferLength2;

	uint8_t WriteBufferPos;

	uint8_t* ReadBuffer;
	uint8_t ReadBufferLength;
  uint8_t ReadBufferPos;

  I2C_HandleTypeDef I2CPort;

  drvI2CMasterCallbackFunction CallbackFunction;

} drvI2CMasterModule;


/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void drvI2CMasterEventInterruptHandler(drvI2CMasterModule* in_i2c_module);
void drvI2CMasterErrorInterruptHandler(drvI2CMasterModule* in_i2c_module);
void drvI2CMasterStartWriteBlock(drvI2CMasterModule* in_i2c_module, uint8_t in_address, uint8_t* in_buffer, uint8_t in_buffer_length);
void drvI2CMasterStartWriteAndReadBlock(drvI2CMasterModule* in_i2c_module, uint8_t in_address, uint8_t* in_write_buffer, uint8_t in_write_buffer_length, uint8_t* in_read_buffer, uint8_t in_read_buffer_length);
void drvI2CMasterStartWriteAndWriteBlock(drvI2CMasterModule* in_i2c_module, uint8_t in_address, uint8_t* in_write_buffer1, uint8_t in_write_buffer_length1, uint8_t* in_write_buffer2, uint8_t in_write_buffer_length2);

#endif
