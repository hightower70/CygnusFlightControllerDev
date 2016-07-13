/*****************************************************************************/
/* I2C Master driver (Interrupt driven)                                      */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <halIODefinitions.h>
#include <drvI2CMaster.h>
#include <sysRTOS.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define I2C_TIMEOUT_BUSY_FLAG 10

/*****************************************************************************/
/* Local functions                                                           */
/*****************************************************************************/
static void drvI2CMasterStart(drvI2CMasterModule* in_module);
static HAL_StatusTypeDef I2C_WaitOnFlagUntilTimeout(I2C_HandleTypeDef *hi2c, uint32_t Flag, FlagStatus Status, uint32_t Timeout);
static void drvI2CMasterStopped(drvI2CMasterModule* in_module, void* in_interrupt_param);

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Starts writing block of bytes
/// @param in_i2c_module Module state
/// @param in_address I2C address [0h..7fh]
/// @param in_buffer Data buffer address
/// @param in_buffer_length Number of bytes to write
void drvI2CMasterStartWriteBlock(drvI2CMasterModule* in_i2c_module, uint8_t in_address, uint8_t* in_buffer, uint8_t in_buffer_length)
{
	// store communication info
	in_i2c_module->Address							= in_address << 1;
	in_i2c_module->WriteBuffer1					= in_buffer;
	in_i2c_module->WriteBufferLength1		= in_buffer_length;
	in_i2c_module->WriteBuffer2					= NULL;
	in_i2c_module->WriteBufferLength2		= 0;
	in_i2c_module->WriteBufferPos				= 0;
	in_i2c_module->ReadBuffer						= NULL;
	in_i2c_module->ReadBufferLength			= 0;
	in_i2c_module->ReadBufferPos				= 0;

	// start communication
	in_i2c_module->Status								= drvI2CM_ST_WRITE_START;

	drvI2CMasterStart(in_i2c_module);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Starts block write and read
/// @param in_i2c_module Module state
/// @param in_address I2C address [0h..7fh]
/// @param in_buffer Data buffer address
/// @param in_buffer_length Number of bytes to write
void drvI2CMasterStartWriteAndReadBlock(drvI2CMasterModule* in_i2c_module, uint8_t in_address, uint8_t* in_write_buffer, uint8_t in_write_buffer_length, uint8_t* in_read_buffer, uint8_t in_read_buffer_length)
{
	// store communication info
	in_i2c_module->Address						= in_address << 1;
	in_i2c_module->WriteBuffer1				= in_write_buffer;
	in_i2c_module->WriteBufferLength1	= in_write_buffer_length;
	in_i2c_module->WriteBuffer2				= NULL;
	in_i2c_module->WriteBufferLength2	= 0;
	in_i2c_module->WriteBufferPos			= 0;
	in_i2c_module->ReadBuffer					= in_read_buffer;
	in_i2c_module->ReadBufferLength		= in_read_buffer_length;
	in_i2c_module->ReadBufferPos			= 0;

	// start communication
	in_i2c_module->Status							= drvI2CM_ST_WRITE_START;

	drvI2CMasterStart(in_i2c_module);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Starts block write and read
/// @param in_i2c_module Module state
/// @param in_address I2C address [0h..7fh]
/// @param in_buffer Data buffer address
/// @param in_buffer_length Number of bytes to write
void drvI2CMasterStartWriteAndWriteBlock(drvI2CMasterModule* in_i2c_module, uint8_t in_address, uint8_t* in_write_buffer1, uint8_t in_write_buffer_length1, uint8_t* in_write_buffer2, uint8_t in_write_buffer_length2)
{
	// store communication info
	in_i2c_module->Address						= in_address << 1;
	in_i2c_module->WriteBuffer1				= in_write_buffer1;
	in_i2c_module->WriteBufferLength1	= in_write_buffer_length1;
	in_i2c_module->WriteBuffer2				= in_write_buffer2;
	in_i2c_module->WriteBufferLength2	= in_write_buffer_length2;
	in_i2c_module->WriteBufferPos			= 0;
	in_i2c_module->ReadBuffer					= NULL;
	in_i2c_module->ReadBufferLength		= 0;
	in_i2c_module->ReadBufferPos			= 0;

	// start communication
	in_i2c_module->Status							= drvI2CM_ST_WRITE_START;

	drvI2CMasterStart(in_i2c_module);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Starts master mode communication
/// @param Module to start communication
static void drvI2CMasterStart(drvI2CMasterModule* in_module)
{
  /* Wait until BUSY flag is reset */
  if(I2C_WaitOnFlagUntilTimeout(&in_module->I2CPort, I2C_FLAG_BUSY, SET, I2C_TIMEOUT_BUSY_FLAG) != HAL_OK)
  {
  	return;
  }

	// Enable EVT, BUF and ERR interrupt
	__HAL_I2C_ENABLE_IT(&in_module->I2CPort, I2C_IT_EVT | I2C_IT_BUF | I2C_IT_ERR);

  // Disable Pos
	in_module->I2CPort.Instance->CR1 &= ~I2C_CR1_POS;

	// Start communication
	in_module->I2CPort.Instance->CR1 |= I2C_CR1_START;
}

/*****************************************************************************/
/* Interrupt handler                                                         */
/*****************************************************************************/
void drvI2CMasterEventInterruptHandler(drvI2CMasterModule* in_module)
{
	uint16_t SR1;
	//uint16_t SR2;

	sysBeginInterruptRoutine();

	// read status register
	SR1 = in_module->I2CPort.Instance->SR1;
	//SR2 = in_module->I2CPort.Instance->SR2;

	switch((in_module->Status ) & ~drvI2CM_ST_ERROR)
	{
		// send address after start condition
		case drvI2CM_ST_WRITE_START:
			// check flags
			if( (SR1 & (I2C_FLAG_SB & I2C_FLAG_MASK) ) != 0)
			{
				// start bit sent -> write address
				in_module->Status = drvI2CM_ST_WRITE_ADDRESS;
				in_module->I2CPort.Instance->DR = in_module->Address & 0xFE;
			}
			break;

		// send first data byte of write buffer 1
		case drvI2CM_ST_WRITE_ADDRESS:
			if( (SR1 & (I2C_FLAG_ADDR & I2C_FLAG_MASK) ) != 0 )
			{
				// clear address flag
				__HAL_I2C_CLEAR_ADDRFLAG(&in_module->I2CPort);
			}

			if( (SR1 & (I2C_FLAG_TXE & I2C_FLAG_MASK) ) != 0 )
			{
				// send data
				in_module->Status = drvI2CM_ST_WRITE_DATA1;
				in_module->WriteBufferPos = 1;
				in_module->I2CPort.Instance->DR = in_module->WriteBuffer1[0];
			}
			break;

		// send data bytes of write buffer 1
		case drvI2CM_ST_WRITE_DATA1:
      if( (SR1 & (I2C_FLAG_TXE & I2C_FLAG_MASK) ) != 0 )
      {
				if(in_module->WriteBufferPos < in_module->WriteBufferLength1)
				{
					// write data from write buffer 1
					in_module->I2CPort.Instance->DR = in_module->WriteBuffer1[in_module->WriteBufferPos++];
				}
				else
				{
					if(in_module->WriteBufferLength2 > 0)
					{
						// start sending data from write buffer 2
						in_module->Status = drvI2CM_ST_WRITE_DATA2;
						in_module->WriteBufferPos = 1;
						in_module->I2CPort.Instance->DR = in_module->WriteBuffer2[0];
					}
					else
					{
						if(in_module->ReadBufferLength > 0)
						{
							// read block
							in_module->Status = drvI2CM_ST_RESTART;
							in_module->I2CPort.Instance->CR1 |= I2C_CR1_START;
						}
						else
						{
							// send stop condition
							in_module->Status = drvI2CM_ST_STOP;
							in_module->I2CPort.Instance->CR1 |= I2C_CR1_STOP;
						}
					}
				}
      }
			break;

		// sends data byte of write buffer 2
		case drvI2CM_ST_WRITE_DATA2:
      if( (SR1 & (I2C_FLAG_TXE & I2C_FLAG_MASK) ) == 0)
      {
      	// error occured
      	in_module->Status = drvI2CM_ST_ERROR;
        in_module->I2CPort.Instance->CR1 |= I2C_CR1_STOP;
      	drvI2CMasterStopped(in_module, sysInterruptParam());
      	break;
      }

      if(in_module->WriteBufferPos < in_module->WriteBufferLength2)
      {
        // write data
        in_module->I2CPort.Instance->DR = in_module->WriteBuffer2[in_module->WriteBufferPos++];
      }
      else
      {
        if(in_module->ReadBufferLength > 0)
        {
          // restart communication with reading a block
          in_module->Status = drvI2CM_ST_RESTART;
          in_module->I2CPort.Instance->CR1 |= I2C_CR1_START;
        }
        else
        {
          // send stop condition
          in_module->Status = drvI2CM_ST_STOP;
          in_module->I2CPort.Instance->CR1 |= I2C_CR1_STOP;
          drvI2CMasterStopped(in_module, sysInterruptParam());
        }
      }
			break;

		// start read
		case drvI2CM_ST_RESTART:
      // check flags
      if((SR1 & ( I2C_FLAG_SB & I2C_FLAG_MASK)) != 0)
      {
      	// prepare ACK for data read
      	if(in_module->ReadBufferLength == 1)
      	{
      		in_module->I2CPort.Instance->CR1 &= ~I2C_CR1_ACK;
      	}

				// send address and read mode
				in_module->Status = drvI2CM_ST_READ_ADDRESS;
				in_module->I2CPort.Instance->DR = in_module->Address | 0x01;
      }
			break;

		// start data read
		case drvI2CM_ST_READ_ADDRESS:
      if((SR1 & ( I2C_FLAG_ADDR & I2C_FLAG_MASK)) != 0)
      {
				// start reading data
				if(in_module->ReadBufferLength == 1)
				{
					// Clear ADDR flag
					__HAL_I2C_CLEAR_ADDRFLAG(&in_module->I2CPort);

					// stop communication after receiving current byte
					in_module->I2CPort.Instance->CR1 |= I2C_CR1_STOP;

					// start data reception
					in_module->Status = drvI2CM_ST_READ_DATA;
				}
				else
				{
					if(in_module->ReadBufferLength == 2)
					{
						 // Enable Pos
						 in_module->I2CPort.Instance->CR1 |= I2C_CR1_POS;

						 // Disable Acknowledge
						 in_module->I2CPort.Instance->CR1 &= ~I2C_CR1_ACK;

						 // Clear ADDR flag
						 __HAL_I2C_CLEAR_ADDRFLAG(&in_module->I2CPort);

						 // start data reception
						 in_module->Status = drvI2CM_ST_READ_DATA;
					}
					else
					{
						// Enable Acknowledge
						in_module->I2CPort.Instance->CR1 |= I2C_CR1_ACK;

						// Clear ADDR flag
						__HAL_I2C_CLEAR_ADDRFLAG(&in_module->I2CPort);

						 // start data reception
						 in_module->Status = drvI2CM_ST_READ_DATA;
					}
				}
      }
			break;

		// read data
		case drvI2CM_ST_READ_DATA:
			// set NAK before receiving the last byte
    	if(in_module->ReadBufferPos + 2 == in_module->ReadBufferLength)
    	{
    		in_module->I2CPort.Instance->CR1 &= ~I2C_CR1_ACK;
    	}

			// Read data from DR
			in_module->ReadBuffer[in_module->ReadBufferPos] = in_module->I2CPort.Instance->DR;
			in_module->ReadBufferPos++;

			// if last byte received
			if(in_module->ReadBufferPos == in_module->ReadBufferLength)
			{
				// send stop
				in_module->I2CPort.Instance->CR1 |= I2C_CR1_STOP;
				in_module->Status = drvI2CM_ST_READ_STOP;

				drvI2CMasterStopped(in_module, sysInterruptParam());
			}
			break;
	}

	sysEndInterruptRoutine();
}

static void drvI2CMasterStopped(drvI2CMasterModule* in_module, void* in_interrupt_param)
{
	// keep error flag and set bus to idle
	in_module->Status = (in_module->Status & drvI2CM_ST_ERROR) | drvI2CM_ST_IDLE;

	// Disable EVT, BUF and ERR interrupt
	__HAL_I2C_DISABLE_IT(&in_module->I2CPort, I2C_IT_EVT | I2C_IT_BUF | I2C_IT_ERR);

	// call callback
	if(in_module->CallbackFunction != sysNULL)
		in_module->CallbackFunction((in_module->Status & drvI2CM_ST_ERROR) == 0, in_interrupt_param);
}

void drvI2CMasterErrorInterruptHandler(drvI2CMasterModule* in_module)
{
	//uint16_t SR1;
	//uint16_t SR2;

	sysBeginInterruptRoutine();

	//SR1 = in_module->I2CPort.Instance->SR1;
	//SR2 = in_module->I2CPort.Instance->SR2;

	__HAL_I2C_CLEAR_FLAG(&in_module->I2CPort, I2C_FLAG_AF);

	in_module->Status = drvI2CM_ST_ERROR;

	// stop communication
	in_module->I2CPort.Instance->CR1 |= I2C_CR1_STOP;
	drvI2CMasterStopped(in_module, sysInterruptParam());

	sysEndInterruptRoutine();
}


/**
  * @brief  This function handles I2C Communication Timeout.
  * @param  hi2c: pointer to a I2C_HandleTypeDef structure that contains
  *         the configuration information for I2C module
  * @param  Flag: specifies the I2C flag to check.
  * @param  Status: The new Flag status (SET or RESET).
  * @param  Timeout: Timeout duration
  * @retval HAL status
  */
static HAL_StatusTypeDef I2C_WaitOnFlagUntilTimeout(I2C_HandleTypeDef *hi2c, uint32_t Flag, FlagStatus Status, uint32_t Timeout)
{
  uint32_t tickstart = 0;

  /* Get tick */
  tickstart = HAL_GetTick();

  /* Wait until flag is set */
  if(Status == RESET)
  {
    while(__HAL_I2C_GET_FLAG(hi2c, Flag) == RESET)
    {
      /* Check for the Timeout */
      if(Timeout != HAL_MAX_DELAY)
      {
        if((Timeout == 0)||((HAL_GetTick() - tickstart ) > Timeout))
        {
          hi2c->State= HAL_I2C_STATE_READY;

          /* Process Unlocked */
          __HAL_UNLOCK(hi2c);

          return HAL_TIMEOUT;
        }
      }
    }
  }
  else
  {
    while(__HAL_I2C_GET_FLAG(hi2c, Flag) != RESET)
    {
      /* Check for the Timeout */
      if(Timeout != HAL_MAX_DELAY)
      {
        if((Timeout == 0)||((HAL_GetTick() - tickstart ) > Timeout))
        {
          hi2c->State= HAL_I2C_STATE_READY;

          /* Process Unlocked */
          __HAL_UNLOCK(hi2c);

          return HAL_TIMEOUT;
        }
      }
    }
  }
  return HAL_OK;
}

