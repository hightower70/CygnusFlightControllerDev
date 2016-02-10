/*****************************************************************************/
/* HMC5883 Magnetic compass sensor driver                                    */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Include files
#include <drvIMU.h>
#include <imuCommunication.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/

#define drvHMC5883_DATA_BUFFER_SIZE 16


// I2C address for drvHMC5883
#define drvHMC5883_I2C_ADDRESS 0x1e

// register addresses
#define drvHMC5883_CONFIG_REG_A         0x00
#define drvHMC5883_CONFIG_REG_B         0x01
#define drvHMC5883_MODE_REG             0x02
#define drvHMC5883_DATAOUT_XMSB_REG     0x03
#define drvHMC5883_DATAOUT_XLSB_REG     0x04
#define drvHMC5883_DATAOUT_ZMSB_REG     0x05
#define drvHMC5883_DATAOUT_ZLSB_REG     0x06
#define drvHMC5883_DATAOUT_YMSB_REG     0x07
#define drvHMC5883_DATAOUT_YLSB_REG     0x08
#define drvHMC5883_STATUS_REG 				  0x09
#define drvHMC5883_IDA_REG  				    0x0A
#define drvHMC5883_IDB_REG      				0x0B
#define drvHMC5883_IDC_REG      				0x0C

// CONFIG_REG_A bits

// Output Data Rate
#define drvHMC5883_ODR_MASK		0x1c
#define drvHMC5883_ODR_0_75		0x00
#define drvHMC5883_ODR_1_5    0x04
#define drvHMC5883_ODR_3      0x08
#define drvHMC5883_ODR_7_5    0x0C
#define drvHMC5883_ODR_15     0x10
#define drvHMC5883_ODR_30     0x14
#define drvHMC5883_ODR_75     0x18

// Measure configuration
#define drvHMC5883_MEASCONF_MASK				0x03
#define drvHMC5883_MEASCONF_NORMAL      0x00
#define drvHMC5883_MEASCONF_BIAS_POS    0x01
#define drvHMC5883_MEASCONF_BIAS_NEG    0x02

// CONFIG_REG_B bits
// Gain settings
#define drvHMC5883_GAIN_MASK		0xe0
#define drvHMC5883_GAIN_0_9    	0x00
#define drvHMC5883_GAIN_1_2     0x20
#define drvHMC5883_GAIN_1_9     0x40
#define drvHMC5883_GAIN_2_5     0x60
#define drvHMC5883_GAIN_4_0     0x80
#define drvHMC5883_GAIN_4_6     0xA0
#define drvHMC5883_GAIN_5_5     0xC0
#define drvHMC5883_GAIN_7_9     0xE0

// MODE_REG bits

// Modes
#define drvHMC5883_MODE_MASK				0x03
#define drvHMC5883_MODE_CONTINUOUS  0x00
#define drvHMC5883_MODE_SINGLE      0x01
#define drvHMC5883_MODE_IDLE        0x02
#define drvHMC5883_MODE_SLEEP       0x03

// STATUS_REG bits
#define drvHMC5883_STATUS_RDY				0x01
#define drvHMC5883_STATUS_LOCK			0x02
#define drvHMC5883_STATUS_REN				0x04

// Sensitivity Conversion Values */
#define drvHMC5883_Sensitivity_0_88Ga         1370    // LSB/Ga
#define drvHMC5883_Sensitivity_1_3Ga          1090    // LSB/Ga
#define drvHMC5883_Sensitivity_1_9Ga          820     // LSB/Ga
#define drvHMC5883_Sensitivity_2_5Ga          660     // LSB/Ga
#define drvHMC5883_Sensitivity_4_0Ga          440     // LSB/Ga
#define drvHMC5883_Sensitivity_4_7Ga          390     // LSB/Ga
#define drvHMC5883_Sensitivity_5_6Ga          330     // LSB/Ga
#define drvHMC5883_Sensitivity_8_1Ga          230     // LSB/Ga  --> NOT RECOMMENDED

#define drvHMC5883_IDA_VALUE 			0x48
#define drvHMC5883_IDB_VALUE 			0x34
#define drvHMC5883_IDC_VALUE 			0x33

/*****************************************************************************/
/* Local functions                                                           */
/*****************************************************************************/
static void drvHMC5883Detect(drvIMUDetectParameter* in_parameter);

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
 uint8_t l_data_buffer[drvHMC5883_DATA_BUFFER_SIZE ];

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Starts any sensor configuration function
/// @param in_imu_i2c I2C bus for IMU sensor
/// @param in_function Functon code to start
/// @param in_function_parameter Function parameter (if applicable)
void drvHMC5883Control(drvIMUControlFunction in_function, void* in_function_parameter)
{
	// start function
	switch(in_function)
	{
		// detect sensor function
		case drvIMU_CF_Detect:
			drvHMC5883Detect((drvIMUDetectParameter*)in_function_parameter);
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
static void drvHMC5883Detect(drvIMUDetectParameter* in_parameter)
{
	bool success = true;

	// try primary address first
	imuReadRegisterBlock(drvHMC5883_I2C_ADDRESS, drvHMC5883_IDA_REG, l_data_buffer, 3, &success);
	if(success && l_data_buffer[0] == drvHMC5883_IDA_VALUE && l_data_buffer[1] == drvHMC5883_IDB_VALUE && l_data_buffer[2] == drvHMC5883_IDC_VALUE)
	{
		// set result
		in_parameter->Success = true;
		in_parameter->Class = drvIMU_SC_MAGNETIC;
		in_parameter->Control = drvHMC5883Control;
	}

}
#if 0
static void drvHMC5883DetectionFinished(bool in_success)
{
	drvDetectFunctionParameter* detect = (drvDetectFunctionParameter*)l_current_function_parameter;

	if(in_success && l_data_buffer[0] == drvHMC5883_IDA_VALUE && l_data_buffer[1] == drvHMC5883_IDB_VALUE && l_data_buffer[2] == drvHMC5883_IDC_VALUE)
	{
		// sensor detected ->notify sensor task
		detect->Success = true;
		detect->Class = drvIMU_SC_Magnetic;
		detect->BeginRead = drvHMC5883BeginRead;

		drvIMUEndConfigFunction(l_current_function, l_current_function_parameter);
	}
	else
	{
			// no response from the sensor -> not detected
			detect->Success = false;
			drvIMUEndConfigFunction(l_current_function, l_current_function_parameter);
	}
}

void drvHMC5883BeginRead(void)
{

}
#endif

#if 0

///////////////////////////////////////////////////////////////////////////////
//! Initializes MCP324x device
void drvHMC5883Init(void)
{
   InitHMC5883I2C();
}

///////////////////////////////////////////////////////////////////////////////
//! Sets HMC5883 device configration byte
//! \param Configuration byte goes to Config A register
//! \param Configuration byte goes to Config B register
//! \return dosTrue if configuration bytes were successfully updated
dosBool drvHMC5883SetConfiguration(dosByte in_config_a, dosByte in_config_b)
{
	dosBool ack = HMC5883_I2C_Start(HMC5883_I2C, drvI2C_WRITE, drvHMC5883_BASE_ADDRESS);

  if( ack )
  {
  	HMC5883_I2C_WriteByte(HMC5883_I2C, drvHMC5883_CONFIG_REG_A);
		HMC5883_I2C_WriteByte(HMC5883_I2C, in_config_a);
		HMC5883_I2C_WriteByte(HMC5883_I2C, in_config_b);
	}      

	HMC5883_I2C_Stop(HMC5883_I2C);
    
	return ack;
}

///////////////////////////////////////////////////////////////////////////////
//! Sets HMC5883 device configration byte
//! \param Mode register value
//! \return dosTrue if mode byte were successfully updated
dosBool drvHMC5883SetMode(dosByte in_mode)
{
	dosBool ack = HMC5883_I2C_Start(HMC5883_I2C, drvI2C_WRITE, drvHMC5883_BASE_ADDRESS);

  if( ack )
  {
  	HMC5883_I2C_WriteByte(HMC5883_I2C, drvHMC5883_MODE_REG);
		HMC5883_I2C_WriteByte(HMC5883_I2C, in_mode);
	}      

	HMC5883_I2C_Stop(HMC5883_I2C);
    
	return ack;
}

///////////////////////////////////////////////////////////////////////////////
//! Reads HMC5883 device ID bytes
//! \param Magnetic field strength of X axis
//! \param Magnetic field strength of Y axis
//! \param Magnetic field strength of Z axis
//! \return dosTrue if values were red successfully
dosBool drvHMC5883ReadValues(dosWord* out_x, dosWord* out_y, dosWord* out_z)
{
	dosByte status;
	dosBool ack = dosFalse;
	dosByte retry = 3;
	
	do
	{   
		ack = HMC5883_I2C_Start(HMC5883_I2C, drvI2C_WRITE, drvHMC5883_BASE_ADDRESS);
	  if( ack )
	  {
			HMC5883_I2C_WriteByte(HMC5883_I2C, drvHMC5883_DATAOUT_XMSB_REG);
			HMC5883_I2C_Restart(HMC5883_I2C, drvI2C_READ, drvHMC5883_BASE_ADDRESS);
	   
			// read X value
			*out_x = HMC5883_I2C_ReadByte(HMC5883_I2C, drvI2C_ACK) << 8;
			*out_x |= HMC5883_I2C_ReadByte(HMC5883_I2C, drvI2C_ACK);
			
			// read Y value
			*out_y = HMC5883_I2C_ReadByte(HMC5883_I2C, drvI2C_ACK) << 8;
			*out_y |= HMC5883_I2C_ReadByte(HMC5883_I2C, drvI2C_ACK);
	
			// read Z value
			*out_z = HMC5883_I2C_ReadByte(HMC5883_I2C, drvI2C_ACK) << 8;
			*out_z |= HMC5883_I2C_ReadByte(HMC5883_I2C, drvI2C_ACK);
			
			// read status byte
			status = HMC5883_I2C_ReadByte(HMC5883_I2C, drvI2C_NACK);
		}
		
		HMC5883_I2C_Stop(HMC5883_I2C);
		
		retry--;
		
	}	while( retry > 0 && ack && (status & drvHMC5883_STATUS_RDY) == 0 );

	
	return ack;
}

///////////////////////////////////////////////////////////////////////////////
//! Reads HMC5883 device ID bytes
//! \return Device ID value
dosDWord drvHMC5883ReadID(void)
{
	dosWord i = 0;
	dosByte value;
	dosByte id_length = 3;
	dosDWord retval = 0;
   
	dosBool ack = HMC5883_I2C_Start(HMC5883_I2C, drvI2C_WRITE, drvHMC5883_BASE_ADDRESS);
  if( ack )
  {
		HMC5883_I2C_WriteByte(HMC5883_I2C, drvHMC5883_DATAOUT_IDA_REG);
		HMC5883_I2C_Restart(HMC5883_I2C, drvI2C_READ, drvHMC5883_BASE_ADDRESS);
   
		// read block  
		for(i = 0; i < id_length; i++)
		{
			value = HMC5883_I2C_ReadByte(HMC5883_I2C, (i==id_length-1)?drvI2C_NACK:drvI2C_ACK);
			
			retval |= (value << (i*8));
		}
	}

	HMC5883_I2C_Stop(HMC5883_I2C);
	
	return retval;
}

#endif
