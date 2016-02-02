/*****************************************************************************/
/* ADXL345 Acceleration sensor driver                                        */
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
#include <drvADXL345.h>


/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define drvADXL345_REGISTER_ADRRESS_SIZE 1
#define drvADXL345_DATA_BUFFER_SIZE 16

#define drvADXL345_PRI_ADDRESS    0x53    // Device primary I2C address
#define drvADXL345_SEC_ADDRESS    0x1d    // Secondary address


//drvADXL345_ Registers Start
#define drvADXL345_DEVID           0x00    // R  // Device ID.
#define drvADXL345_THRESH_TAP      0x1D    // RW // Tap Threshold 62.5mg/LSB (0xFF = +16g)
#define drvADXL345_OFSX            0x1E    // RW // X-axis Offset 15.6mg/LSB
#define drvADXL345_OFSY            0x1F    // RW // Y-axis Offset 15.6mg/LSB
#define drvADXL345_OFSZ            0x20    // RW // Z-axis Offset 15.6mg/LSB
#define drvADXL345_DUR             0x21    // RW // Tap Duration 625us/LSB
#define drvADXL345_LATENT          0x22    // RW // Tap Latency 1.25ms/LSB
#define drvADXL345_WINDOW          0x23    // RW // Tap Window 1.25ms/LSB
#define drvADXL345_THRESH_ACT      0x24    // RW // Activity threshold 62.5mg/LSB
#define drvADXL345_THRESH_INACT    0x25    // RW // Inactivity Threshold 62.5mg/LSB
#define drvADXL345_TIME_INACT      0x26    // RW // Inactivity Time. 1s/LSB
#define drvADXL345_ACT_INACT_CTL   0x27    // RW // xis enable control for activity and inactivity detection.
#define drvADXL345_THRESH_FF       0x28    // RW // Free-fall threshold. 62.5mg/LSB
#define drvADXL345_TIME_FF         0x29    // RW // Free-fall Time 5ms/LSB (values 0x14 to 0x46 are recommended)
#define drvADXL345_TAP_AXES        0x2A    // RW // Axis control for tap/double tap
#define drvADXL345_ACT_TAP_STATUS  0x2B    // R  // Source of tap/double tap
#define drvADXL345_BW_RATE         0x2C    // RW // Data rate and power control mode (default 0xA)
#define drvADXL345_POWER_CTL       0x2D    // RW // Power saving features control
#define drvADXL345_INT_ENABLE      0x2E    // RW // Interrupt enable control
#define drvADXL345_INT_MAP         0x2F    // RW // Interrupt mapping control
#define drvADXL345_INT_SOURCE      0x30    // R  // Source of interrupts
#define drvADXL345_DATAFORMAT      0x31    // RW // Data format control
#define drvADXL345_DATAX0          0x32    // R  // X-Axis
#define drvADXL345_DATAY0          0x34    // R  // Y-Axis
#define drvADXL345_DATAZ0          0x36    // R  // Z-Axis
#define drvADXL345_FIFO_CTL        0x38    // RW // FIFO control
#define drvADXL345_FIFO_STATE      0x39    // R  // FIFO status
//ADXL Registers End

#define drvADXL345_MEASURE_MODE    0x08
#define drvADXL345_STANDBY_MODE    0xF7
#define drvADXL345_SLEEP_MODE      0x04

// DATAFORMAT register bits
#define drvADXL345_DF_RANGE_2G    0
#define drvADXL345_DF_RANGE_4G    1
#define drvADXL345_DF_RANGE_8G    2
#define drvADXL345_DF_RANGE_16G   3
#define drvADXL345_DF_JUSTIFY     (1<<2)

#define drvADXL345_DF_LOWRES			0
#define drvADXL345_DF_FULLRES			(1<<3)
#define drvADXL345_DF_INT_INVERT  (1<<5)
#define drvADXL345_DF_SPI         (1<<6)
#define drvADXL345_DF_SELF_TEST   (1<<7)

// BW rate register bits
#define drvADXL345_BW_1600 0xF // 1111
#define drvADXL345_BW_800  0xE // 1110
#define drvADXL345_BW_400  0xD // 1101
#define drvADXL345_BW_200  0xC // 1100
#define drvADXL345_BW_100  0xB // 1011
#define drvADXL345_BW_50   0xA // 1010
#define drvADXL345_BW_25   0x9 // 1001
#define drvADXL345_BW_12   0x8 // 1000
#define drvADXL345_BW_6    0x7 // 0111
#define drvADXL345_BW_3    0x6 // 0110

//Power Control Register Bits
#define drvADXL345_PC_WU_0      (1<<0)   //Wake Up Mode - Bit 0
#define drvADXL345_PC_WU_1      (1<<1)   //Wake Up mode - Bit 1
#define drvADXL345_PC_SLEEP     (1<<2)   //Sleep Mode
#define drvADXL345_PC_MEASURE   (1<<3)   //Measurement Mode
#define drvADXL345_PC_AUTO_SLP  (1<<4)   //Auto Sleep Mode bit
#define drvADXL345_PC_LINK      (1<<5)   //Link bit


/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
uint8_t l_register_address_buffer[drvADXL345_REGISTER_ADRRESS_SIZE];
uint8_t l_data_buffer[drvADXL345_DATA_BUFFER_SIZE ];


/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initialize ADXL345 driver and detect chip
/// @param in_imu_i2c I2C bus state variable
void drvADXL345Init(drvI2CMasterModule* in_imu_i2c)
{
	l_register_address_buffer[0] = drvADXL345_DEVID;


	/* Write command to device */
  //HAL_I2C_Mem_Write(&in_imu_i2c->I2CPort, drvADXL345_PRI_ADDRESS << 1, l_register_address_buffer, I2C_MEMADD_SIZE_8BIT, l_register_address_buffer, 1, 1000);
  //HAL_I2C_Mem_Read(&in_imu_i2c->I2CPort, drvADXL345_PRI_ADDRESS << 1, 0, I2C_MEMADD_SIZE_8BIT, l_data_buffer, 1, 1000);

	drvI2CMasterStartWriteAndReadBlock(in_imu_i2c, drvADXL345_PRI_ADDRESS, l_register_address_buffer, drvADXL345_REGISTER_ADRRESS_SIZE, l_data_buffer, 1 );

}

