/*****************************************************************************/
/* IMU Communication function                                                */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __imuCommunication_h
#define __imuCommunication_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/


/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void imuCommunicationInit(void);
void imuReadByteRegister(uint8_t in_i2caddress, uint8_t in_register_address, uint8_t* out_register_value, bool* inout_success);
void imuWriteByteRegister(uint8_t in_i2c_address, uint8_t in_register_address, uint8_t in_register_value, bool* inout_success);
void imuReadRegisterBlock(uint8_t in_i2c_address, uint8_t in_register_address, uint8_t* out_register_block, uint8_t in_register_block_length, bool* inout_success);
void imuWriteRegisterBlock(uint8_t in_i2c_address, uint8_t in_register_address, uint8_t* in_register_block, uint8_t in_register_block_length, bool* inout_success);

#endif
