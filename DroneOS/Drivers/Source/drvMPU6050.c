/*****************************************************************************/
/* MPU6050 Acceleration and Gyro sensor driver                               */
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
#include <sysHighresTimer.h>
#include <sysRTOS.h>
#include <drvIMU.h>
#include <imuCommunication.h>
#include <math.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define drvMPU6050_DATA_BUFFER_SIZE 16

// I2C address for the MPU-6050.
#define drvMPU6050_I2C_PRI_ADDRESS 0x68
#define drvMPU6050_I2C_SEC_ADDRESS 0x69

// Register addresses
#define drvMPU6050_RA_XG_OFFS_TC					0x00    //[7] PWR_MODE, [6:1] XG_OFFS_TC, [0] OTP_BNK_VLD
#define drvMPU6050_RA_YG_OFFS_TC					0x01    //[7] PWR_MODE, [6:1] YG_OFFS_TC, [0] OTP_BNK_VLD
#define drvMPU6050_RA_ZG_OFFS_TC					0x02    //[7] PWR_MODE, [6:1] ZG_OFFS_TC, [0] OTP_BNK_VLD
#define drvMPU6050_RA_X_FINE_GAIN					0x03    //[7:0] X_FINE_GAIN
#define drvMPU6050_RA_Y_FINE_GAIN					0x04    //[7:0] Y_FINE_GAIN
#define drvMPU6050_RA_Z_FINE_GAIN					0x05    //[7:0] Z_FINE_GAIN
#define drvMPU6050_RA_XA_OFFS_H						0x06    //[15:0] XA_OFFS
#define drvMPU6050_RA_XA_OFFS_L_TC				0x07
#define drvMPU6050_RA_YA_OFFS_H						0x08    //[15:0] YA_OFFS
#define drvMPU6050_RA_YA_OFFS_L_TC				0x09
#define drvMPU6050_RA_ZA_OFFS_H						0x0A    //[15:0] ZA_OFFS
#define drvMPU6050_RA_ZA_OFFS_L_TC				0x0B
#define drvMPU6050_RA_PRODUCT_ID					0x0C    // Product ID Register
#define drvMPU6050_RA_SELF_TEST_X					0x0D		// SELF_TEST_X register
#define drvMPU6050_RA_SELF_TEST_Y					0x0E		// SELF_TEST_Y register
#define drvMPU6050_RA_SELF_TEST_Z					0x0F		// SELF_TEST_Z register
#define drvMPU6050_RA_SELF_TEST_A					0x10		// SELF_TEST_A register
#define drvMPU6050_RA_XG_OFFS_USRH				0x13    //[15:0] XG_OFFS_USR
#define drvMPU6050_RA_XG_OFFS_USRL				0x14
#define drvMPU6050_RA_YG_OFFS_USRH				0x15    //[15:0] YG_OFFS_USR
#define drvMPU6050_RA_YG_OFFS_USRL				0x16
#define drvMPU6050_RA_ZG_OFFS_USRH				0x17    //[15:0] ZG_OFFS_USR
#define drvMPU6050_RA_ZG_OFFS_USRL				0x18
#define drvMPU6050_RA_SMPLRT_DIV					0x19   // R/W
#define drvMPU6050_RA_CONFIG							0x1A   // R/W
#define drvMPU6050_RA_GYRO_CONFIG					0x1B   // R/W
#define drvMPU6050_RA_ACCEL_CONFIG				0x1C   // R/W
#define drvMPU6050_RA_FF_THR							0x1D   // R/W
#define drvMPU6050_RA_FF_DUR							0x1E   // R/W
#define drvMPU6050_RA_MOT_THR							0x1F   // R/W
#define drvMPU6050_RA_MOT_DUR							0x20   // R/W
#define drvMPU6050_RA_ZRMOT_THR						0x21   // R/W
#define drvMPU6050_RA_ZRMOT_DUR						0x22   // R/W
#define drvMPU6050_RA_FIFO_EN							0x23   // R/W
#define drvMPU6050_RA_I2C_MST_CTRL				0x24   // R/W
#define drvMPU6050_RA_I2C_SLV0_ADDR				0x25   // R/W
#define drvMPU6050_RA_I2C_SLV0_REG				0x26   // R/W
#define drvMPU6050_RA_I2C_SLV0_CTRL				0x27   // R/W
#define drvMPU6050_RA_I2C_SLV1_ADDR				0x28   // R/W
#define drvMPU6050_RA_I2C_SLV1_REG				0x29   // R/W
#define drvMPU6050_RA_I2C_SLV1_CTRL				0x2A   // R/W
#define drvMPU6050_RA_I2C_SLV2_ADDR				0x2B   // R/W
#define drvMPU6050_RA_I2C_SLV2_REG				0x2C   // R/W
#define drvMPU6050_RA_I2C_SLV2_CTRL				0x2D   // R/W
#define drvMPU6050_RA_I2C_SLV3_ADDR				0x2E   // R/W
#define drvMPU6050_RA_I2C_SLV3_REG				0x2F   // R/W
#define drvMPU6050_RA_I2C_SLV3_CTRL				0x30   // R/W
#define drvMPU6050_RA_I2C_SLV4_ADDR				0x31   // R/W
#define drvMPU6050_RA_I2C_SLV4_REG				0x32   // R/W
#define drvMPU6050_RA_I2C_SLV4_DO					0x33   // R/W
#define drvMPU6050_RA_I2C_SLV4_CTRL				0x34   // R/W
#define drvMPU6050_RA_I2C_SLV4_DI					0x35   // R
#define drvMPU6050_RA_I2C_MST_STATUS			0x36   // R
#define drvMPU6050_RA_INT_PIN_CFG					0x37   // R/W
#define drvMPU6050_RA_INT_ENABLE					0x38   // R/W
#define drvMPU6050_RA_INT_STATUS					0x3A   // R
#define drvMPU6050_RA_ACCEL_XOUT_H				0x3B   // R
#define drvMPU6050_RA_ACCEL_XOUT_L				0x3C   // R
#define drvMPU6050_RA_ACCEL_YOUT_H				0x3D   // R
#define drvMPU6050_RA_ACCEL_YOUT_L				0x3E   // R
#define drvMPU6050_RA_ACCEL_ZOUT_H				0x3F   // R
#define drvMPU6050_RA_ACCEL_ZOUT_L				0x40   // R
#define drvMPU6050_RA_TEMP_OUT_H					0x41   // R
#define drvMPU6050_RA_TEMP_OUT_L					0x42   // R
#define drvMPU6050_RA_GYRO_XOUT_H					0x43   // R
#define drvMPU6050_RA_GYRO_XOUT_L					0x44   // R
#define drvMPU6050_RA_GYRO_YOUT_H					0x45   // R
#define drvMPU6050_RA_GYRO_YOUT_L					0x46   // R
#define drvMPU6050_RA_GYRO_ZOUT_H					0x47   // R
#define drvMPU6050_RA_GYRO_ZOUT_L					0x48   // R
#define drvMPU6050_RA_EXT_SENS_DATA_00		0x49   // R
#define drvMPU6050_RA_EXT_SENS_DATA_01		0x4A   // R
#define drvMPU6050_RA_EXT_SENS_DATA_02		0x4B   // R
#define drvMPU6050_RA_EXT_SENS_DATA_03		0x4C   // R
#define drvMPU6050_RA_EXT_SENS_DATA_04		0x4D   // R
#define drvMPU6050_RA_EXT_SENS_DATA_05		0x4E   // R
#define drvMPU6050_RA_EXT_SENS_DATA_06		0x4F   // R
#define drvMPU6050_RA_EXT_SENS_DATA_07		0x50   // R
#define drvMPU6050_RA_EXT_SENS_DATA_08		0x51   // R
#define drvMPU6050_RA_EXT_SENS_DATA_09		0x52   // R
#define drvMPU6050_RA_EXT_SENS_DATA_10		0x53   // R
#define drvMPU6050_RA_EXT_SENS_DATA_11		0x54   // R
#define drvMPU6050_RA_EXT_SENS_DATA_12		0x55   // R
#define drvMPU6050_RA_EXT_SENS_DATA_13		0x56   // R
#define drvMPU6050_RA_EXT_SENS_DATA_14		0x57   // R
#define drvMPU6050_RA_EXT_SENS_DATA_15		0x58   // R
#define drvMPU6050_RA_EXT_SENS_DATA_16		0x59   // R
#define drvMPU6050_RA_EXT_SENS_DATA_17		0x5A   // R
#define drvMPU6050_RA_EXT_SENS_DATA_18		0x5B   // R
#define drvMPU6050_RA_EXT_SENS_DATA_19		0x5C   // R
#define drvMPU6050_RA_EXT_SENS_DATA_20		0x5D   // R
#define drvMPU6050_RA_EXT_SENS_DATA_21		0x5E   // R
#define drvMPU6050_RA_EXT_SENS_DATA_22		0x5F   // R
#define drvMPU6050_RA_EXT_SENS_DATA_23		0x60   // R
#define drvMPU6050_RA_MOT_DETECT_STATUS		0x61   // R
#define drvMPU6050_RA_I2C_SLV0_DO					0x63   // R/W
#define drvMPU6050_RA_I2C_SLV1_DO					0x64   // R/W
#define drvMPU6050_RA_I2C_SLV2_DO					0x65   // R/W
#define drvMPU6050_RA_I2C_SLV3_DO					0x66   // R/W
#define drvMPU6050_RA_I2C_MST_DELAY_CTRL	0x67   // R/W
#define drvMPU6050_RA_SIGNAL_PATH_RESET		0x68   // R/W
#define drvMPU6050_RA_MOT_DETECT_CTRL			0x69   // R/W
#define drvMPU6050_RA_USER_CTRL						0x6A   // R/W
#define drvMPU6050_RA_PWR_MGMT_1					0x6B   // R/W
#define drvMPU6050_RA_PWR_MGMT_2					0x6C   // R/W
#define drvMPU6050_RA_FIFO_COUNTH					0x72   // R/W
#define drvMPU6050_RA_FIFO_COUNTL					0x73   // R/W
#define drvMPU6050_RA_FIFO_R_W						0x74   // R/W
#define drvMPU6050_RA_WHO_AM_I						0x75   // R


// AUX_VDDIO Register
//#define drvMPU6050_AUX_VDDIO (1<<7)  // I2C high: 1=VDD, 0=VLOGIC

// CONFIG Register
// DLPF is Digital Low Pass Filter for both gyro and accelerometers.
// These are the names for the bits.
// Use these only with the sysBV() macro.
#define drvMPU6050_DLPF_CFG0     (1<<0)
#define drvMPU6050_DLPF_CFG1     (1<<1)
#define drvMPU6050_DLPF_CFG2     (1<<2)
#define drvMPU6050_EXT_SYNC_SET0 (1<<3)
#define drvMPU6050_EXT_SYNC_SET1 (1<<4)
#define drvMPU6050_EXT_SYNC_SET2 (1<<5)

// Combined definitions for the EXT_SYNC_SET values
#define drvMPU6050_EXT_SYNC_SET_0 (0)
#define drvMPU6050_EXT_SYNC_SET_1 (sysBV(drvMPU6050_EXT_SYNC_SET0))
#define drvMPU6050_EXT_SYNC_SET_2 (sysBV(drvMPU6050_EXT_SYNC_SET1))
#define drvMPU6050_EXT_SYNC_SET_3 (sysBV(drvMPU6050_EXT_SYNC_SET1)|sysBV(drvMPU6050_EXT_SYNC_SET0))
#define drvMPU6050_EXT_SYNC_SET_4 (sysBV(drvMPU6050_EXT_SYNC_SET2))
#define drvMPU6050_EXT_SYNC_SET_5 (sysBV(drvMPU6050_EXT_SYNC_SET2)|sysBV(drvMPU6050_EXT_SYNC_SET0))
#define drvMPU6050_EXT_SYNC_SET_6 (sysBV(drvMPU6050_EXT_SYNC_SET2)|sysBV(drvMPU6050_EXT_SYNC_SET1))
#define drvMPU6050_EXT_SYNC_SET_7 (sysBV(drvMPU6050_EXT_SYNC_SET2)|sysBV(drvMPU6050_EXT_SYNC_SET1)|sysBV(drvMPU6050_EXT_SYNC_SET0))

// Alternative names for the combined definitions.
#define drvMPU6050_EXT_SYNC_DISABLED     drvMPU6050_EXT_SYNC_SET_0
#define drvMPU6050_EXT_SYNC_TEMP_OUT_L   drvMPU6050_EXT_SYNC_SET_1
#define drvMPU6050_EXT_SYNC_GYRO_XOUT_L  drvMPU6050_EXT_SYNC_SET_2
#define drvMPU6050_EXT_SYNC_GYRO_YOUT_L  drvMPU6050_EXT_SYNC_SET_3
#define drvMPU6050_EXT_SYNC_GYRO_ZOUT_L  drvMPU6050_EXT_SYNC_SET_4
#define drvMPU6050_EXT_SYNC_ACCEL_XOUT_L drvMPU6050_EXT_SYNC_SET_5
#define drvMPU6050_EXT_SYNC_ACCEL_YOUT_L drvMPU6050_EXT_SYNC_SET_6
#define drvMPU6050_EXT_SYNC_ACCEL_ZOUT_L drvMPU6050_EXT_SYNC_SET_7

// Combined definitions for the DLPF_CFG values
#define drvMPU6050_DLPF_CFG_0 (0)
#define drvMPU6050_DLPF_CFG_1 (sysBV(drvMPU6050_DLPF_CFG0))
#define drvMPU6050_DLPF_CFG_2 (sysBV(drvMPU6050_DLPF_CFG1))
#define drvMPU6050_DLPF_CFG_3 (sysBV(drvMPU6050_DLPF_CFG1)|sysBV(drvMPU6050_DLPF_CFG0))
#define drvMPU6050_DLPF_CFG_4 (sysBV(drvMPU6050_DLPF_CFG2))
#define drvMPU6050_DLPF_CFG_5 (sysBV(drvMPU6050_DLPF_CFG2)|sysBV(drvMPU6050_DLPF_CFG0))
#define drvMPU6050_DLPF_CFG_6 (sysBV(drvMPU6050_DLPF_CFG2)|sysBV(drvMPU6050_DLPF_CFG1))
#define drvMPU6050_DLPF_CFG_7 (sysBV(drvMPU6050_DLPF_CFG2)|sysBV(drvMPU6050_DLPF_CFG1)|sysBV(drvMPU6050_DLPF_CFG0))

// Alternative names for the combined definitions
// This name uses the bandwidth (Hz) for the accelometer,
// for the gyro the bandwidth is almost the same.
#define drvMPU6050_DLPF_260HZ    drvMPU6050_DLPF_CFG_0
#define drvMPU6050_DLPF_184HZ    drvMPU6050_DLPF_CFG_1
#define drvMPU6050_DLPF_94HZ     drvMPU6050_DLPF_CFG_2
#define drvMPU6050_DLPF_44HZ     drvMPU6050_DLPF_CFG_3
#define drvMPU6050_DLPF_21HZ     drvMPU6050_DLPF_CFG_4
#define drvMPU6050_DLPF_10HZ     drvMPU6050_DLPF_CFG_5
#define drvMPU6050_DLPF_5HZ      drvMPU6050_DLPF_CFG_6
#define drvMPU6050_DLPF_RESERVED drvMPU6050_DLPF_CFG_7

// GYRO_CONFIG Register
// The XG_ST, YG_ST, ZG_ST are bits for selftest.
// The FS_SEL sets the range for the gyro.
// These are the names for the bits.
#define drvMPU6050_FS_SEL0 (1<<3)
#define drvMPU6050_FS_SEL1 (1<<4)
#define drvMPU6050_GC_ST_Z   (1<<5)
#define drvMPU6050_GC_ST_Y   (1<<6)
#define drvMPU6050_GC_ST_X   (1<<7)

// Combined definitions for the FS_SEL values
#define drvMPU6050_FS_SEL_0 (0)
#define drvMPU6050_FS_SEL_1 (drvMPU6050_FS_SEL0)
#define drvMPU6050_FS_SEL_2 (drvMPU6050_FS_SEL1)
#define drvMPU6050_FS_SEL_3 (drvMPU6050_FS_SEL1|drvMPU6050_FS_SEL0)

// Alternative names for the combined definitions
// The name uses the range in degrees per second.
#define drvMPU6050_FS_SEL_250  drvMPU6050_FS_SEL_0
#define drvMPU6050_FS_SEL_500  drvMPU6050_FS_SEL_1
#define drvMPU6050_FS_SEL_1000 drvMPU6050_FS_SEL_2
#define drvMPU6050_FS_SEL_2000 drvMPU6050_FS_SEL_3

// ACCEL_CONFIG Register
// The XA_ST, YA_ST, ZA_ST are bits for selftest.
// The AFS_SEL sets the range for the accelerometer.
// These are the names for the bits.
#define drvMPU6050_ACCEL_HPF0			(1<<0)
#define drvMPU6050_ACCEL_HPF1			(1<<1)
#define drvMPU6050_ACCEL_HPF2			(1<<2)
#define drvMPU6050_AC_AFS_SEL0		(1<<3)
#define drvMPU6050_AC_AFS_SEL1		(1<<4)
#define drvMPU6050_AC_ST_Z				(1<<5)
#define drvMPU6050_AC_ST_Y				(1<<6)
#define drvMPU6050_AC_ST_X				(1<<7)

// Combined definitions for the ACCEL_HPF values
#define drvMPU6050_ACCEL_HPF_0 (0)
#define drvMPU6050_ACCEL_HPF_1 (drvMPU6050_ACCEL_HPF0)
#define drvMPU6050_ACCEL_HPF_2 (drvMPU6050_ACCEL_HPF1)
#define drvMPU6050_ACCEL_HPF_3 (drvMPU6050_ACCEL_HPF1|drvMPU6050_ACCEL_HPF0)
#define drvMPU6050_ACCEL_HPF_4 (drvMPU6050_ACCEL_HPF2)
#define drvMPU6050_ACCEL_HPF_7 (drvMPU6050_ACCEL_HPF2|drvMPU6050_ACCEL_HPF1|drvMPU6050_ACCEL_HPF0)

// Alternative names for the combined definitions
// The name uses the Cut-off frequency.
#define drvMPU6050_ACCEL_HPF_RESET  drvMPU6050_ACCEL_HPF_0
#define drvMPU6050_ACCEL_HPF_5HZ    drvMPU6050_ACCEL_HPF_1
#define drvMPU6050_ACCEL_HPF_2_5HZ  drvMPU6050_ACCEL_HPF_2
#define drvMPU6050_ACCEL_HPF_1_25HZ drvMPU6050_ACCEL_HPF_3
#define drvMPU6050_ACCEL_HPF_0_63HZ drvMPU6050_ACCEL_HPF_4
#define drvMPU6050_ACCEL_HPF_HOLD   drvMPU6050_ACCEL_HPF_7

// Combined definitions for the AFS_SEL values
#define drvMPU6050_AC_AFS_SEL_0 (0)
#define drvMPU6050_AC_AFS_SEL_1 (drvMPU6050_AC_AFS_SEL0)
#define drvMPU6050_AC_AFS_SEL_2 (drvMPU6050_AC_AFS_SEL1)
#define drvMPU6050_AC_AFS_SEL_3 (drvMPU6050_AC_AFS_SEL1|drvMPU6050_AC_AFS_SEL0)

// Alternative names for the combined definitions
// The name uses the full scale range for the accelerometer.
#define drvMPU6050_AC_AFS_SEL_2G  drvMPU6050_AC_AFS_SEL_0
#define drvMPU6050_AC_AFS_SEL_4G  drvMPU6050_AC_AFS_SEL_1
#define drvMPU6050_AC_AFS_SEL_8G  drvMPU6050_AC_AFS_SEL_2
#define drvMPU6050_AC_AFS_SEL_16G drvMPU6050_AC_AFS_SEL_3

// FIFO_EN Register
// These are the names for the bits.
// Use these only with the sysBV() macro.
#define drvMPU6050_SLV0_FIFO_EN  (1<<0)
#define drvMPU6050_SLV1_FIFO_EN  (1<<1)
#define drvMPU6050_SLV2_FIFO_EN  (1<<2)
#define drvMPU6050_ACCEL_FIFO_EN (1<<3)
#define drvMPU6050_ZG_FIFO_EN    (1<<4)
#define drvMPU6050_YG_FIFO_EN    (1<<5)
#define drvMPU6050_XG_FIFO_EN    (1<<6)
#define drvMPU6050_TEMP_FIFO_EN  (1<<7)

// I2C_MST_CTRL Register
// These are the names for the bits.
// Use these only with the sysBV() macro.
#define drvMPU6050_I2C_MST_CLK0  (1<<0)
#define drvMPU6050_I2C_MST_CLK1  (1<<1)
#define drvMPU6050_I2C_MST_CLK2  (1<<2)
#define drvMPU6050_I2C_MST_CLK3  (1<<3)
#define drvMPU6050_I2C_MST_P_NSR (1<<4)
#define drvMPU6050_SLV_3_FIFO_EN (1<<5)
#define drvMPU6050_WAIT_FOR_ES   (1<<6)
#define drvMPU6050_MULT_MST_EN   (1<<7)

// Combined definitions for the I2C_MST_CLK
#define drvMPU6050_I2C_MST_CLK_0 (0)
#define drvMPU6050_I2C_MST_CLK_1  (sysBV(drvMPU6050_I2C_MST_CLK0))
#define drvMPU6050_I2C_MST_CLK_2  (sysBV(drvMPU6050_I2C_MST_CLK1))
#define drvMPU6050_I2C_MST_CLK_3  (sysBV(drvMPU6050_I2C_MST_CLK1)|sysBV(drvMPU6050_I2C_MST_CLK0))
#define drvMPU6050_I2C_MST_CLK_4  (sysBV(drvMPU6050_I2C_MST_CLK2))
#define drvMPU6050_I2C_MST_CLK_5  (sysBV(drvMPU6050_I2C_MST_CLK2)|sysBV(drvMPU6050_I2C_MST_CLK0))
#define drvMPU6050_I2C_MST_CLK_6  (sysBV(drvMPU6050_I2C_MST_CLK2)|sysBV(drvMPU6050_I2C_MST_CLK1))
#define drvMPU6050_I2C_MST_CLK_7  (sysBV(drvMPU6050_I2C_MST_CLK2)|sysBV(drvMPU6050_I2C_MST_CLK1)|sysBV(drvMPU6050_I2C_MST_CLK0))
#define drvMPU6050_I2C_MST_CLK_8  (sysBV(drvMPU6050_I2C_MST_CLK3))
#define drvMPU6050_I2C_MST_CLK_9  (sysBV(drvMPU6050_I2C_MST_CLK3)|sysBV(drvMPU6050_I2C_MST_CLK0))
#define drvMPU6050_I2C_MST_CLK_10 (sysBV(drvMPU6050_I2C_MST_CLK3)|sysBV(drvMPU6050_I2C_MST_CLK1))
#define drvMPU6050_I2C_MST_CLK_11 (sysBV(drvMPU6050_I2C_MST_CLK3)|sysBV(drvMPU6050_I2C_MST_CLK1)|sysBV(drvMPU6050_I2C_MST_CLK0))
#define drvMPU6050_I2C_MST_CLK_12 (sysBV(drvMPU6050_I2C_MST_CLK3)|sysBV(drvMPU6050_I2C_MST_CLK2))
#define drvMPU6050_I2C_MST_CLK_13 (sysBV(drvMPU6050_I2C_MST_CLK3)|sysBV(drvMPU6050_I2C_MST_CLK2)|sysBV(drvMPU6050_I2C_MST_CLK0))
#define drvMPU6050_I2C_MST_CLK_14 (sysBV(drvMPU6050_I2C_MST_CLK3)|sysBV(drvMPU6050_I2C_MST_CLK2)|sysBV(drvMPU6050_I2C_MST_CLK1))
#define drvMPU6050_I2C_MST_CLK_15 (sysBV(drvMPU6050_I2C_MST_CLK3)|sysBV(drvMPU6050_I2C_MST_CLK2)|sysBV(drvMPU6050_I2C_MST_CLK1)|sysBV(drvMPU6050_I2C_MST_CLK0))

// Alternative names for the combined definitions
// The names uses I2C Master Clock Speed in kHz.
#define drvMPU6050_I2C_MST_CLK_348KHZ drvMPU6050_I2C_MST_CLK_0
#define drvMPU6050_I2C_MST_CLK_333KHZ drvMPU6050_I2C_MST_CLK_1
#define drvMPU6050_I2C_MST_CLK_320KHZ drvMPU6050_I2C_MST_CLK_2
#define drvMPU6050_I2C_MST_CLK_308KHZ drvMPU6050_I2C_MST_CLK_3
#define drvMPU6050_I2C_MST_CLK_296KHZ drvMPU6050_I2C_MST_CLK_4
#define drvMPU6050_I2C_MST_CLK_286KHZ drvMPU6050_I2C_MST_CLK_5
#define drvMPU6050_I2C_MST_CLK_276KHZ drvMPU6050_I2C_MST_CLK_6
#define drvMPU6050_I2C_MST_CLK_267KHZ drvMPU6050_I2C_MST_CLK_7
#define drvMPU6050_I2C_MST_CLK_258KHZ drvMPU6050_I2C_MST_CLK_8
#define drvMPU6050_I2C_MST_CLK_500KHZ drvMPU6050_I2C_MST_CLK_9
#define drvMPU6050_I2C_MST_CLK_471KHZ drvMPU6050_I2C_MST_CLK_10
#define drvMPU6050_I2C_MST_CLK_444KHZ drvMPU6050_I2C_MST_CLK_11
#define drvMPU6050_I2C_MST_CLK_421KHZ drvMPU6050_I2C_MST_CLK_12
#define drvMPU6050_I2C_MST_CLK_400KHZ drvMPU6050_I2C_MST_CLK_13
#define drvMPU6050_I2C_MST_CLK_381KHZ drvMPU6050_I2C_MST_CLK_14
#define drvMPU6050_I2C_MST_CLK_364KHZ drvMPU6050_I2C_MST_CLK_15

// I2C_SLV0_ADDR Register
// These are the names for the bits.
// Use these only with the sysBV() macro.
#define drvMPU6050_I2C_SLV0_RW (1<<7)

// I2C_SLV0_CTRL Register
// These are the names for the bits.
// Use these only with the sysBV() macro.
#define drvMPU6050_I2C_SLV0_LEN0    (1<<0)
#define drvMPU6050_I2C_SLV0_LEN1    (1<<1)
#define drvMPU6050_I2C_SLV0_LEN2    (1<<2)
#define drvMPU6050_I2C_SLV0_LEN3    (1<<3)
#define drvMPU6050_I2C_SLV0_GRP     (1<<4)
#define drvMPU6050_I2C_SLV0_REG_DIS (1<<5)
#define drvMPU6050_I2C_SLV0_BYTE_SW (1<<6)
#define drvMPU6050_I2C_SLV0_EN      (1<<7)

// A mask for the length
#define drvMPU6050_I2C_SLV0_LEN_MASK 0x0F

// I2C_SLV1_ADDR Register
// These are the names for the bits.
// Use these only with the sysBV() macro.
#define drvMPU6050_I2C_SLV1_RW (1<<7)

// I2C_SLV1_CTRL Register
// These are the names for the bits.
// Use these only with the sysBV() macro.
#define drvMPU6050_I2C_SLV1_LEN0    (1<<0)
#define drvMPU6050_I2C_SLV1_LEN1    (1<<1)
#define drvMPU6050_I2C_SLV1_LEN2    (1<<2)
#define drvMPU6050_I2C_SLV1_LEN3    (1<<3)
#define drvMPU6050_I2C_SLV1_GRP     (1<<4)
#define drvMPU6050_I2C_SLV1_REG_DIS (1<<5)
#define drvMPU6050_I2C_SLV1_BYTE_SW (1<<6)
#define drvMPU6050_I2C_SLV1_EN      (1<<7)

// A mask for the length
#define drvMPU6050_I2C_SLV1_LEN_MASK 0x0F

// I2C_SLV2_ADDR Register
// These are the names for the bits.
// Use these only with the sysBV() macro.
#define drvMPU6050_I2C_SLV2_RW (1<<7)

// I2C_SLV2_CTRL Register
// These are the names for the bits.
// Use these only with the sysBV() macro.
#define drvMPU6050_I2C_SLV2_LEN0    (1<<0)
#define drvMPU6050_I2C_SLV2_LEN1    (1<<1)
#define drvMPU6050_I2C_SLV2_LEN2    (1<<2)
#define drvMPU6050_I2C_SLV2_LEN3    (1<<3)
#define drvMPU6050_I2C_SLV2_GRP     (1<<4)
#define drvMPU6050_I2C_SLV2_REG_DIS (1<<5)
#define drvMPU6050_I2C_SLV2_BYTE_SW (1<<6)
#define drvMPU6050_I2C_SLV2_EN      (1<<7)

// A mask for the length
#define drvMPU6050_I2C_SLV2_LEN_MASK 0x0F

// I2C_SLV3_ADDR Register
// These are the names for the bits.
// Use these only with the sysBV() macro.
#define drvMPU6050_I2C_SLV3_RW (1<<7)

// I2C_SLV3_CTRL Register
// These are the names for the bits.
// Use these only with the sysBV() macro.
#define drvMPU6050_I2C_SLV3_LEN0    (1<<0)
#define drvMPU6050_I2C_SLV3_LEN1    (1<<1)
#define drvMPU6050_I2C_SLV3_LEN2    (1<<2)
#define drvMPU6050_I2C_SLV3_LEN3    (1<<3)
#define drvMPU6050_I2C_SLV3_GRP     (1<<4)
#define drvMPU6050_I2C_SLV3_REG_DIS (1<<5)
#define drvMPU6050_I2C_SLV3_BYTE_SW (1<<6)
#define drvMPU6050_I2C_SLV3_EN      (1<<7)

// A mask for the length
#define drvMPU6050_I2C_SLV3_LEN_MASK 0x0F

// I2C_SLV4_ADDR Register
// These are the names for the bits.
// Use these only with the sysBV() macro.
#define drvMPU6050_I2C_SLV4_RW (1<<7)

// I2C_SLV4_CTRL Register
// These are the names for the bits.
// Use these only with the sysBV() macro.
#define drvMPU6050_I2C_MST_DLY0     (1<<0)
#define drvMPU6050_I2C_MST_DLY1     (1<<1)
#define drvMPU6050_I2C_MST_DLY2     (1<<2)
#define drvMPU6050_I2C_MST_DLY3     (1<<3)
#define drvMPU6050_I2C_MST_DLY4     (1<<4)
#define drvMPU6050_I2C_SLV4_REG_DIS (1<<5)
#define drvMPU6050_I2C_SLV4_INT_EN  (1<<6)
#define drvMPU6050_I2C_SLV4_EN      (1<<7)

// A mask for the delay
#define drvMPU6050_I2C_MST_DLY_MASK 0x1F

// I2C_MST_STATUS Register
// These are the names for the bits.
// Use these only with the sysBV() macro.
#define drvMPU6050_I2C_SLV0_NACK (1<<0)
#define drvMPU6050_I2C_SLV1_NACK (1<<1)
#define drvMPU6050_I2C_SLV2_NACK (1<<2)
#define drvMPU6050_I2C_SLV3_NACK (1<<3)
#define drvMPU6050_I2C_SLV4_NACK (1<<4)
#define drvMPU6050_I2C_LOST_ARB  (1<<5)
#define drvMPU6050_I2C_SLV4_DONE (1<<6)
#define drvMPU6050_PASS_THROUGH  (1<<7)

// I2C_PIN_CFG Register
// These are the names for the bits.
// Use these only with the sysBV() macro.
#define drvMPU6050_IPC_CLKOUT_EN       (1<<0)
#define drvMPU6050_IPC_I2C_BYPASS_EN   (1<<1)
#define drvMPU6050_IPC_FSYNC_INT_EN    (1<<2)
#define drvMPU6050_IPC_FSYNC_INT_LEVEL (1<<3)
#define drvMPU6050_IPC_INT_RD_CLEAR    (1<<4)
#define drvMPU6050_IPC_LATCH_INT_EN    (1<<5)
#define drvMPU6050_IPC_INT_OPEN        (1<<6)
#define drvMPU6050_IPC_INT_LEVEL       (1<<7)

// INT_ENABLE Register
// These are the names for the bits.
// Use these only with the sysBV() macro.
#define drvMPU6050_DATA_RDY_EN    (1<<0)
#define drvMPU6050_I2C_MST_INT_EN (1<<3)
#define drvMPU6050_FIFO_OFLOW_EN  (1<<4)
#define drvMPU6050_ZMOT_EN        (1<<5)
#define drvMPU6050_MOT_EN         (1<<6)
#define drvMPU6050_FF_EN          (1<<7)

// INT_STATUS Register
// These are the names for the bits.
// Use these only with the sysBV() macro.
#define drvMPU6050_DATA_RDY_INT   (1<<0)
#define drvMPU6050_I2C_MST_INT    (1<<3)
#define drvMPU6050_FIFO_OFLOW_INT (1<<4)
#define drvMPU6050_ZMOT_INT       (1<<5)
#define drvMPU6050_MOT_INT        (1<<6)
#define drvMPU6050_FF_INT         (1<<7)

// MOT_DETECT_STATUS Register
// These are the names for the bits.
// Use these only with the sysBV() macro.
#define drvMPU6050_MOT_ZRMOT (1<<0)
#define drvMPU6050_MOT_ZPOS  (1<<2)
#define drvMPU6050_MOT_ZNEG  (1<<3)
#define drvMPU6050_MOT_YPOS  (1<<4)
#define drvMPU6050_MOT_YNEG  (1<<5)
#define drvMPU6050_MOT_XPOS  (1<<6)
#define drvMPU6050_MOT_XNEG  (1<<7)

// IC2_MST_DELAY_CTRL Register
// These are the names for the bits.
// Use these only with the sysBV() macro.
#define drvMPU6050_I2C_SLV0_DLY_EN (1<<0)
#define drvMPU6050_I2C_SLV1_DLY_EN (1<<1)
#define drvMPU6050_I2C_SLV2_DLY_EN (1<<2)
#define drvMPU6050_I2C_SLV3_DLY_EN (1<<3)
#define drvMPU6050_I2C_SLV4_DLY_EN (1<<4)
#define drvMPU6050_DELAY_ES_SHADOW (1<<7)

// SIGNAL_PATH_RESET Register
// These are the names for the bits.
// Use these only with the sysBV() macro.
#define drvMPU6050_TEMP_RESET  (1<<0)
#define drvMPU6050_ACCEL_RESET (1<<1)
#define drvMPU6050_GYRO_RESET  (1<<2)

// MOT_DETECT_CTRL Register
// These are the names for the bits.
// Use these only with the sysBV() macro.
#define drvMPU6050_MOT_COUNT0      (1<<0)
#define drvMPU6050_MOT_COUNT1      (1<<1)
#define drvMPU6050_FF_COUNT0       (1<<2)
#define drvMPU6050_FF_COUNT1       (1<<3)
#define drvMPU6050_ACCEL_ON_DELAY0 (1<<4)
#define drvMPU6050_ACCEL_ON_DELAY1 (1<<5)

// Combined definitions for the MOT_COUNT
#define drvMPU6050_MOT_COUNT_0 (0)
#define drvMPU6050_MOT_COUNT_1 (sysBV(drvMPU6050_MOT_COUNT0))
#define drvMPU6050_MOT_COUNT_2 (sysBV(drvMPU6050_MOT_COUNT1))
#define drvMPU6050_MOT_COUNT_3 (sysBV(drvMPU6050_MOT_COUNT1)|sysBV(drvMPU6050_MOT_COUNT0))

// Alternative names for the combined definitions
#define drvMPU6050_MOT_COUNT_RESET drvMPU6050_MOT_COUNT_0

// Combined definitions for the FF_COUNT
#define drvMPU6050_FF_COUNT_0 (0)
#define drvMPU6050_FF_COUNT_1 (sysBV(drvMPU6050_FF_COUNT0))
#define drvMPU6050_FF_COUNT_2 (sysBV(drvMPU6050_FF_COUNT1))
#define drvMPU6050_FF_COUNT_3 (sysBV(drvMPU6050_FF_COUNT1)|sysBV(drvMPU6050_FF_COUNT0))

// Alternative names for the combined definitions
#define drvMPU6050_FF_COUNT_RESET drvMPU6050_FF_COUNT_0

// Combined definitions for the ACCEL_ON_DELAY
#define drvMPU6050_ACCEL_ON_DELAY_0 (0)
#define drvMPU6050_ACCEL_ON_DELAY_1 (sysBV(drvMPU6050_ACCEL_ON_DELAY0))
#define drvMPU6050_ACCEL_ON_DELAY_2 (sysBV(drvMPU6050_ACCEL_ON_DELAY1))
#define drvMPU6050_ACCEL_ON_DELAY_3 (sysBV(drvMPU6050_ACCEL_ON_DELAY1)|sysBV(drvMPU6050_ACCEL_ON_DELAY0))

// Alternative names for the ACCEL_ON_DELAY
#define drvMPU6050_ACCEL_ON_DELAY_0MS drvMPU6050_ACCEL_ON_DELAY_0
#define drvMPU6050_ACCEL_ON_DELAY_1MS drvMPU6050_ACCEL_ON_DELAY_1
#define drvMPU6050_ACCEL_ON_DELAY_2MS drvMPU6050_ACCEL_ON_DELAY_2
#define drvMPU6050_ACCEL_ON_DELAY_3MS drvMPU6050_ACCEL_ON_DELAY_3

// USER_CTRL Register
// These are the names for the bits.
// Use these only with the sysBV() macro.
#define drvMPU6050_UC_SIG_COND_RESET (1<<0)
#define drvMPU6050_UC_I2C_MST_RESET  (1<<1)
#define drvMPU6050_UC_FIFO_RESET     (1<<2)
#define drvMPU6050_UC_I2C_IF_DIS     (1<<4)   // must be 0 for MPU-6050
#define drvMPU6050_UC_I2C_MST_EN     (1<<5)
#define drvMPU6050_UC_FIFO_EN        (1<<6)

// PWR_MGMT_1 Register
// These are the names for the bits.
// Use these only with the sysBV() macro.
#define drvMPU6050_PM1_CLKSEL0      (1<<0)
#define drvMPU6050_PM1_CLKSEL1      (1<<1)
#define drvMPU6050_PM1_CLKSEL2      (1<<2)
#define drvMPU6050_PM1_TEMP_DIS     (1<<3)    // 1: disable temperature sensor
#define drvMPU6050_PM1_CYCLE        (1<<5)    // 1: sample and sleep
#define drvMPU6050_PM1_SLEEP        (1<<6)    // 1: sleep mode
#define drvMPU6050_PM1_DEVICE_RESET (1<<7)    // 1: reset to default values

// Combined definitions for the CLKSEL
#define drvMPU6050_CLKSEL_0 (0)
#define drvMPU6050_CLKSEL_1 (sysBV(drvMPU6050_CLKSEL0))
#define drvMPU6050_CLKSEL_2 (sysBV(drvMPU6050_CLKSEL1))
#define drvMPU6050_CLKSEL_3 (sysBV(drvMPU6050_CLKSEL1)|sysBV(drvMPU6050_CLKSEL0))
#define drvMPU6050_CLKSEL_4 (sysBV(drvMPU6050_CLKSEL2))
#define drvMPU6050_CLKSEL_5 (sysBV(drvMPU6050_CLKSEL2)|sysBV(drvMPU6050_CLKSEL0))
#define drvMPU6050_CLKSEL_6 (sysBV(drvMPU6050_CLKSEL2)|sysBV(drvMPU6050_CLKSEL1))
#define drvMPU6050_CLKSEL_7 (sysBV(drvMPU6050_CLKSEL2)|sysBV(drvMPU6050_CLKSEL1)|sysBV(drvMPU6050_CLKSEL0))

// Alternative names for the combined definitions
#define drvMPU6050_CLKSEL_INTERNAL    drvMPU6050_CLKSEL_0
#define drvMPU6050_CLKSEL_X           drvMPU6050_CLKSEL_1
#define drvMPU6050_CLKSEL_Y           drvMPU6050_CLKSEL_2
#define drvMPU6050_CLKSEL_Z           drvMPU6050_CLKSEL_3
#define drvMPU6050_CLKSEL_EXT_32KHZ   drvMPU6050_CLKSEL_4
#define drvMPU6050_CLKSEL_EXT_19_2MHZ drvMPU6050_CLKSEL_5
#define drvMPU6050_CLKSEL_RESERVED    drvMPU6050_CLKSEL_6
#define drvMPU6050_CLKSEL_STOP        drvMPU6050_CLKSEL_7

// PWR_MGMT_2 Register
// These are the names for the bits.
// Use these only with the sysBV() macro.
#define drvMPU6050_STBY_ZG       (1<<0)
#define drvMPU6050_STBY_YG       (1<<1)
#define drvMPU6050_STBY_XG       (1<<2)
#define drvMPU6050_STBY_ZA       (1<<3)
#define drvMPU6050_STBY_YA       (1<<4)
#define drvMPU6050_STBY_XA       (1<<5)
#define drvMPU6050_LP_WAKE_CTRL0 (1<<6)
#define drvMPU6050_LP_WAKE_CTRL1 (1<<7)

// Combined definitions for the LP_WAKE_CTRL
#define drvMPU6050_LP_WAKE_CTRL_0 (0)
#define drvMPU6050_LP_WAKE_CTRL_1 (sysBV(drvMPU6050_LP_WAKE_CTRL0))
#define drvMPU6050_LP_WAKE_CTRL_2 (sysBV(drvMPU6050_LP_WAKE_CTRL1))
#define drvMPU6050_LP_WAKE_CTRL_3 (sysBV(drvMPU6050_LP_WAKE_CTRL1)|sysBV(drvMPU6050_LP_WAKE_CTRL0))

// Alternative names for the combined definitions
// The names uses the Wake-up Frequency.
#define drvMPU6050_LP_WAKE_1_25HZ drvMPU6050_LP_WAKE_CTRL_0
#define drvMPU6050_LP_WAKE_2_5HZ  drvMPU6050_LP_WAKE_CTRL_1
#define drvMPU6050_LP_WAKE_5HZ    drvMPU6050_LP_WAKE_CTRL_2
#define drvMPU6050_LP_WAKE_10HZ   drvMPU6050_LP_WAKE_CTRL_3

// WHO_AM_I register value
#define drvMPU6050_WHO_AM_I_MASK 0x7E
#define drvMPU6050_WHO_AM_I_VALUE 0x68


// sensor resolution constants
#define drvMPU6050_HALF_RESOLUTION 0
#define drvMPU6050_FULL_RESOLUTION (!drvMPU6050_HALF_RESOLUTION)

#define drvMPU6050_SELF_TEST_MAX_DEVIATION 14
#define drvMPU6050_SELF_TEST_MEASUREMENT_CYCLE_COUNT 100


#define GET_INT16_FROM_DATA_BUFER(x) ((int16_t)(((int16_t)l_data_buffer[x] << 8) + (int16_t)l_data_buffer[x+1]))

/*****************************************************************************/
/* Default settings                                                          */
/*****************************************************************************/
#define drvMPU6050_DEFAULT_FILTER_FREQUENCY 44

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static uint8_t l_i2c_address;
static uint8_t l_sensor_resolution;
static uint8_t l_data_buffer[drvMPU6050_DATA_BUFFER_SIZE ];

/*****************************************************************************/
/* Local functions                                                           */
/*****************************************************************************/
static void drvMPU6050Detect(drvIMUDetectParameter* in_parameter);
static void drvMPU6050SelfTest(drvIMUSelfTestParameter* in_parameter);
static void drvMPU6050FindRevision(bool* inout_success);
static void drvMPU6050SetSampleRateAndLowPassFilter(uint16_t in_sample_rate_hz, uint16_t in_filter_frequency_hz, bool* inout_success);


/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Starts any sensor control (configuration) function
/// @param in_imu_i2c I2C bus for IMU sensor
/// @param in_function Functon code to start
/// @param in_function_parameter Function parameter (if applicable)
void drvMPU6050Control(drvIMUControlFunction in_function, void* in_function_parameter)
{
	// start function
	switch(in_function)
	{
		// detect sensor function
		case drvIMU_CF_Detect:
			drvMPU6050Detect((drvIMUDetectParameter*)in_function_parameter);
			break;

		// activate self test
		case drvIMU_CF_SelfTest:
			drvMPU6050SelfTest((drvIMUSelfTestParameter*)in_function_parameter);
			break;

		case drvIMU_CF_Unknown:
		default:
			// TODO: error
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sensor detection function
/// @param in_parameter Detection function parameter
static void drvMPU6050Detect(drvIMUDetectParameter* in_parameter)
{
	bool success = true;
	uint8_t id_value;

	// try primary address first
	l_i2c_address = drvMPU6050_I2C_PRI_ADDRESS;
	imuReadByteRegister(l_i2c_address, drvMPU6050_RA_WHO_AM_I, &id_value, &success);
	if(!success)
	{
		// if not found try secondary address
		l_i2c_address = drvMPU6050_I2C_SEC_ADDRESS;
		imuReadByteRegister(l_i2c_address, drvMPU6050_RA_WHO_AM_I, &id_value, &success);
	}

	// check if found
	if(success && ((id_value & drvMPU6050_WHO_AM_I_MASK) == drvMPU6050_WHO_AM_I_VALUE))
	{
		// sensor found

		// enable I2C bypass
		imuWriteByteRegister(l_i2c_address, drvMPU6050_RA_USER_CTRL, 0, &success); // Disable I2C master
		imuWriteByteRegister(l_i2c_address, drvMPU6050_RA_INT_PIN_CFG, drvMPU6050_IPC_I2C_BYPASS_EN, &success); // Enable I2C bypass
		imuWriteByteRegister(l_i2c_address, drvMPU6050_RA_PWR_MGMT_1, 0, &success); // No sleep

		// set result
		in_parameter->Success = true;
		in_parameter->Class = drvIMU_SC_ACCELERATION | drvIMU_SC_GYRO;
		in_parameter->Control = drvMPU6050Control;

	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Runs sensor self test
/// @param in_parameter Self test function parameter
static void drvMPU6050SelfTest(drvIMUSelfTestParameter* in_parameter)
{
	bool success = true;
	uint8_t i;
	uint8_t accel_int_trim[3];
	uint8_t gyro_int_trim[3];
	float accel_float_trim[3];
	float gyro_float_trim[3];
	float accel_base[3];
	float gyro_base[3];
	float accel[3];
	float gyro[3];
	float diff;
	float error;

	// find revision number of the chip
	drvMPU6050FindRevision(&success);

	// set sample rate and low pass filter
	drvMPU6050SetSampleRateAndLowPassFilter(1000, drvMPU6050_DEFAULT_FILTER_FREQUENCY, &success);

	// set gyro sensitivity to 250DPS for self-test as per datasheet
	imuWriteByteRegister(l_i2c_address, drvMPU6050_RA_GYRO_CONFIG, drvMPU6050_FS_SEL_250, &success);

	// set accel range to 8g
	imuWriteByteRegister(l_i2c_address, drvMPU6050_RA_ACCEL_CONFIG, drvMPU6050_AC_AFS_SEL_8G, &success);

	// initialize
	for(i=0; i<3; i++)
	{
		accel_base[i]	= 0;
		gyro_base[i]	= 0;
		accel[i]			= 0;
		gyro[i]				= 0;
	}

	// wait for sensor settings stabilize
	sysDelay(20);


	// read data without self test excitation
	for (i = 0; i < drvMPU6050_SELF_TEST_MEASUREMENT_CYCLE_COUNT && success; i++)
	{
		sysHighresTimerDelay(1000);

		// read sensor values
		imuReadRegisterBlock(l_i2c_address, drvMPU6050_RA_INT_STATUS, l_data_buffer, 15, &success);

		accel_base[0] += GET_INT16_FROM_DATA_BUFER(1);
		accel_base[1] += GET_INT16_FROM_DATA_BUFER(3);
		accel_base[2] += GET_INT16_FROM_DATA_BUFER(5);
		gyro_base[0] += GET_INT16_FROM_DATA_BUFER(9);
		gyro_base[1] += GET_INT16_FROM_DATA_BUFER(11);
		gyro_base[2] += GET_INT16_FROM_DATA_BUFER(13);
	}

	// enable self test
	imuWriteByteRegister(l_i2c_address, drvMPU6050_RA_GYRO_CONFIG, drvMPU6050_FS_SEL_250 | drvMPU6050_GC_ST_X | drvMPU6050_GC_ST_Y | drvMPU6050_GC_ST_Z, &success );

	// accel 8g, self-test enabled all axes
	imuWriteByteRegister(l_i2c_address, drvMPU6050_RA_ACCEL_CONFIG, drvMPU6050_AC_AFS_SEL_8G | drvMPU6050_AC_ST_X | drvMPU6050_AC_ST_Y | drvMPU6050_AC_ST_Z, &success);

	sysDelay(20);

	// read data with the self test excitation
	for (i = 0; i < drvMPU6050_SELF_TEST_MEASUREMENT_CYCLE_COUNT && success; i++)
	{
		sysHighresTimerDelay(1000);

		// read sensor value
		imuReadRegisterBlock(l_i2c_address, drvMPU6050_RA_INT_STATUS, l_data_buffer, 15, &success);

		accel[0] += GET_INT16_FROM_DATA_BUFER(1);
		accel[1] += GET_INT16_FROM_DATA_BUFER(3);
		accel[2] += GET_INT16_FROM_DATA_BUFER(5);
		gyro[0] += GET_INT16_FROM_DATA_BUFER(9);
		gyro[1] += GET_INT16_FROM_DATA_BUFER(11);
		gyro[2] += GET_INT16_FROM_DATA_BUFER(13);
	}

	// calculate average
	for (i = 0; i < 3; i++) 
	{
		accel_base[i]	/= drvMPU6050_SELF_TEST_MEASUREMENT_CYCLE_COUNT;
		gyro_base[i]	/= drvMPU6050_SELF_TEST_MEASUREMENT_CYCLE_COUNT;
		accel[i]			/= drvMPU6050_SELF_TEST_MEASUREMENT_CYCLE_COUNT;
		gyro[i]				/= drvMPU6050_SELF_TEST_MEASUREMENT_CYCLE_COUNT;
	}

	// load factory trim values
	imuReadRegisterBlock(l_i2c_address, drvMPU6050_RA_SELF_TEST_X, l_data_buffer, 4, &success);

	if(success)
	{
		accel_int_trim[0] = ((l_data_buffer[0] >> 3) & 0x1C) | ((l_data_buffer[3] >> 4) & 0x03);
		accel_int_trim[1] = ((l_data_buffer[1] >> 3) & 0x1C) | ((l_data_buffer[3] >> 2) & 0x03);
		accel_int_trim[2] = ((l_data_buffer[2] >> 3) & 0x1C) | ((l_data_buffer[3] >> 0) & 0x03);
		gyro_int_trim[0] = l_data_buffer[0] & 0x1F;
		gyro_int_trim[1] = l_data_buffer[1] & 0x1F;
		gyro_int_trim[2] = l_data_buffer[2] & 0x1F;

		// convert factory trims to right units
		for(i = 0; i < 3; i++)
		{
			accel_float_trim[i] = 4096 * 0.34f * powf(0.92f / 0.34f, (accel_int_trim[i] - 1) / 30.0f);
			gyro_float_trim[i] = 25 * 131.0f * powf(1.046f, gyro_int_trim[i] - 1);
		}

		// Y gyro trim is negative
		gyro_float_trim[1] *= -1;

		// calculate and check errors for accel
		for (i = 0; i < 3; i++) 
		{
			diff = accel[i] - accel_base[i];
			error = 100 * (diff - accel_float_trim[i]) / accel_float_trim[i];

			if (fabsf(error) > drvMPU6050_SELF_TEST_MAX_DEVIATION)
			{
				success = false;
			}
		}

		// calculate and check error for gyro
		for (i = 0; i < 3; i++)
		{
			diff = gyro[i] - gyro_base[i];
			error = 100 * (diff - gyro_float_trim[i]) / gyro_float_trim[i];

			if (fabsf(error) > drvMPU6050_SELF_TEST_MAX_DEVIATION)
			{
				sucess = false;
			}
		}

		// turn off self test
		imuWriteByteRegister(l_i2c_address, drvMPU6050_RA_GYRO_CONFIG, drvMPU6050_FS_SEL_250, &success );
		imuWriteByteRegister(l_i2c_address, drvMPU6050_RA_ACCEL_CONFIG, drvMPU6050_AC_AFS_SEL_8G, &success);
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sets sample rate of the sensor
static void drvMPU6050SetSampleRateAndLowPassFilter(uint16_t in_sample_rate_hz, uint16_t in_filter_frequency_hz, bool* inout_success)
{
	uint8_t filter_value;
	uint16_t gyro_sample_rate = 1000;
	uint8_t divisor;

	// choose next highest filter frequency available
	if (in_filter_frequency_hz == 0)
	{
		filter_value = drvMPU6050_DLPF_260HZ;
		gyro_sample_rate = 8000;
	}
	else
	{
		if (in_filter_frequency_hz <= 5)
		{
			filter_value = drvMPU6050_DLPF_5HZ;
		}
		else
		{
			if (in_filter_frequency_hz <= 10) 
			{
				filter_value = drvMPU6050_DLPF_10HZ;
			}
			else
			{
				if (in_filter_frequency_hz <= 21) 
				{
					filter_value = drvMPU6050_DLPF_21HZ;
				}
				else
				{
					if (in_filter_frequency_hz <= 44) 
					{
						filter_value = drvMPU6050_DLPF_44HZ;
					} 
					else
					{
						if (in_filter_frequency_hz <= 94) 
						{

							filter_value = drvMPU6050_DLPF_94HZ;
						}
						else
						{
							if (in_filter_frequency_hz <= 184) 
							{
								filter_value = drvMPU6050_DLPF_184HZ;
							} 
							else
							{
								filter_value = drvMPU6050_DLPF_260HZ;
								gyro_sample_rate = 8000;
							}
						}
					}
				}
			}
		}
	}

	// set filter value
	imuWriteByteRegister(l_i2c_address, drvMPU6050_RA_CONFIG, filter_value, inout_success);

	// calculate sample rate
	if(in_sample_rate_hz == 0)
		in_sample_rate_hz = 1000;

	divisor = gyro_sample_rate / in_sample_rate_hz;

	if (divisor > 200)
	{
		divisor = 200;
	}
	if (divisor < 1)
	{
		divisor = 1;
	}

	imuWriteByteRegister(l_i2c_address, drvMPU6050_RA_SMPLRT_DIV, divisor - 1, inout_success);

	sysHighresTimerDelay(1000);
}


///////////////////////////////////////////////////////////////////////////////
/// @brief Finds revision number of the sensor chip and sets the resolution based on the version
/// @return True if operation was success
static void drvMPU6050FindRevision(bool* inout_success)
{
	bool success = *inout_success;
	uint8_t revision;
	uint8_t product_id;

	// read product id
	imuReadByteRegister(l_i2c_address, drvMPU6050_RA_PRODUCT_ID, &product_id, &success);
	imuReadRegisterBlock(l_i2c_address, drvMPU6050_RA_XA_OFFS_H, l_data_buffer, 6, &success);

	// Find revision number
	if( success)
	{
		revision = ((l_data_buffer[5] & 0x01) << 2) | ((l_data_buffer[3] & 0x01) << 1) | (l_data_buffer[1] & 0x01);
		if (revision)
		{
			if (revision == 1)
			{
				l_sensor_resolution = drvMPU6050_HALF_RESOLUTION;
			}
			else
			{
				if (revision == 2)
				{
					l_sensor_resolution = drvMPU6050_FULL_RESOLUTION;
				}
				else
				{
					if ((revision == 3) || (revision == 7))
					{
						l_sensor_resolution = drvMPU6050_FULL_RESOLUTION;
					}
					else
					{
						//TODO: failureMode(FAILURE_ACC_INCOMPATIBLE);
						success = false;
					}
				}
			}
	  }
		else
		{
			revision = product_id & 0x0F;
			if (!revision)
			{
				//TODO: failureMode(FAILURE_ACC_INCOMPATIBLE);
				success = false;
			}
			else
			{
				if (revision == 4)
				{
					l_sensor_resolution = drvMPU6050_HALF_RESOLUTION;
				}
				else
				{
					l_sensor_resolution = drvMPU6050_FULL_RESOLUTION;
				}
			}
		}
	}
	else
	{
		//TODO: error
		success = false;
	}

	*inout_success = success;
}
