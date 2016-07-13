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
#include <sysRTOS.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/

#define drvHMC5883_DATA_BUFFER_SIZE 16


// I2C address for drvHMC5883
#define drvHMC5883_I2C_ADDRESS 0x1e

// register addresses
#define drvHMC5883_RA_CONFIG_A				0x00
#define drvHMC5883_RA_CONFIG_B				0x01
#define drvHMC5883_RA_MODE						0x02
#define drvHMC5883_RA_DATAOUT_XMSB		0x03
#define drvHMC5883_RA_DATAOUT_XLSB		0x04
#define drvHMC5883_RA_DATAOUT_ZMSB		0x05
#define drvHMC5883_RA_DATAOUT_ZLSB		0x06
#define drvHMC5883_RA_DATAOUT_YMSB		0x07
#define drvHMC5883_RA_DATAOUT_YLSB		0x08
#define drvHMC5883_RA_STATUS					0x09
#define drvHMC5883_IDA								0x0A
#define drvHMC5883_IDB								0x0B
#define drvHMC5883_IDC								0x0C

// CONFIG_REG_A bits

// Output Data Rate
#define drvHMC5883_CRA_ODR_MASK		0x1c
#define drvHMC5883_CRA_ODR_0_75		0x00
#define drvHMC5883_CRA_ODR_1_5    0x04
#define drvHMC5883_CRA_ODR_3      0x08
#define drvHMC5883_CRA_ODR_7_5    0x0C
#define drvHMC5883_CRA_ODR_15     0x10
#define drvHMC5883_CRA_ODR_30     0x14
#define drvHMC5883_CRA_ODR_75     0x18

// Measure configuration
#define drvHMC5883_CRA_MEASCONF_MASK				0x03
#define drvHMC5883_CRA_MEASCONF_NORMAL      0x00
#define drvHMC5883_CRA_MEASCONF_BIAS_POS    0x01
#define drvHMC5883_CRA_MEASCONF_BIAS_NEG    0x02

// sample averraging
#define drvHMC5883_CRA_MEAS_AVG     0x60	// sample average mask (2 bits)
#define drvHMC5883_CRA_MEAS_AVG_1   0x00	// output = 1 sample (no avg.)
#define drvHMC5883_CRA_MEAS_AVG_2   0x20  // output = 2 samples averaged
#define drvHMC5883_CRA_MEAS_AVG_4   0x40  // output = 4 samples averaged
#define drvHMC5883_CRA_MEAS_AVG_8   0x60  // output = 8 samples averaged

// CONFIG_REG_B bits
// Gain settings
#define drvHMC5883_CRB_GAIN_MASK		0xe0
#define drvHMC5883_CRB_GAIN_0_9    	0x00
#define drvHMC5883_CRB_GAIN_1_2     0x20
#define drvHMC5883_CRB_GAIN_1_9     0x40
#define drvHMC5883_CRB_GAIN_2_5     0x60
#define drvHMC5883_CRB_GAIN_4_0     0x80
#define drvHMC5883_CRB_GAIN_4_6     0xA0
#define drvHMC5883_CRB_GAIN_5_5     0xC0
#define drvHMC5883_CRB_GAIN_7_9     0xE0

// MODE_REG bits

// Modes
#define drvHMC5883_MODE_MASK				0x03
#define drvHMC5883_MODE_CONTINUOUS  0x00
#define drvHMC5883_MODE_SINGLE      0x01
#define drvHMC5883_MODE_IDLE        0x02
#define drvHMC5883_MODE_SLEEP       0x03

// STATUS_REG bits
#define drvHMC5883_ST_RDY				0x01
#define drvHMC5883_ST_LOCK			0x02
#define drvHMC5883_ST_REN				0x04

// Sensitivity Conversion Values */
#define drvHMC5883_Sensitivity_0_88Ga         1370    // LSB/Ga
#define drvHMC5883_Sensitivity_1_3Ga          1090    // LSB/Ga
#define drvHMC5883_Sensitivity_1_9Ga          820     // LSB/Ga
#define drvHMC5883_Sensitivity_2_5Ga          660     // LSB/Ga
#define drvHMC5883_Sensitivity_4_0Ga          440     // LSB/Ga
#define drvHMC5883_Sensitivity_4_7Ga          390     // LSB/Ga
#define drvHMC5883_Sensitivity_5_6Ga          330     // LSB/Ga
#define drvHMC5883_Sensitivity_8_1Ga          230     // LSB/Ga  --> NOT RECOMMENDED

#define drvHMC5883_TEST_GAIN      drvHMC5883_CRB_GAIN_2_5  /* gain value during self-test */
#define drvHMC5883_TEST_X_MIN     550         /* min X */
#define drvHMC5883_TEST_X_NORM    766         /* normal X */
#define drvHMC5883_TEST_X_MAX     850         /* max X */
#define drvHMC5883_TEST_Y_MIN     550         /* min Y */
#define drvHMC5883_TEST_Y_NORM    766         /* normal Y */
#define drvHMC5883_TEST_Y_MAX     850         /* max Y */
#define drvHMC5883_TEST_Z_MIN     550         /* min Z */
#define drvHMC5883_TEST_Z_NORM    713         /* normal Z */
#define drvHMC5883_TEST_Z_MAX     850         /* max Z */

#define drvHMC5883_IDA_VALUE 			0x48
#define drvHMC5883_IDB_VALUE 			0x34
#define drvHMC5883_IDC_VALUE 			0x33

#define drvHMC5883_SELF_TEST_DELAY 250
#define drvHMC5883_SELF_TEST_MEASUREMENT_DELAY 80

/*****************************************************************************/
/* Local functions                                                           */
/*****************************************************************************/
static void drvHMC5883Detect(drvIMUDetectParameter* in_parameter);
static void drvHMC5883SelfTest(drvIMUSelfTestParameter* in_parameter);

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

		// activate self test
		case drvIMU_CF_SelfTest:
			drvHMC5883SelfTest((drvIMUSelfTestParameter*)in_function_parameter);
			break;

		// invalid function
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
	imuReadRegisterBlock(drvHMC5883_I2C_ADDRESS, drvHMC5883_IDA, l_data_buffer, 3, &success);
	if(success && l_data_buffer[0] == drvHMC5883_IDA_VALUE && l_data_buffer[1] == drvHMC5883_IDB_VALUE && l_data_buffer[2] == drvHMC5883_IDC_VALUE)
	{
		// set result
		in_parameter->Success = true;
		in_parameter->Class = drvIMU_SC_MAGNETIC;
		in_parameter->Control = drvHMC5883Control;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Runs sensor self test
/// @param in_parameter Self test function parameter
static void drvHMC5883SelfTest(drvIMUSelfTestParameter* in_parameter)
{
	uint8_t test_phase;
	bool success = true;
	uint8_t meas_mode;

	// set range for self test
	imuWriteByteRegister(drvHMC5883_I2C_ADDRESS, drvHMC5883_RA_CONFIG_B, drvHMC5883_TEST_GAIN, &success);

	// do all tests
	for(test_phase = 0; test_phase < 3; test_phase++)
	{
		switch(meas_mode)
		{
			case 0:
				meas_mode = drvHMC5883_CRA_MEASCONF_NORMAL;
				break;

			case 1:
				meas_mode = drvHMC5883_CRA_MEASCONF_BIAS_POS;
				break;

			case 2:
				meas_mode = drvHMC5883_CRA_MEASCONF_BIAS_NEG;
				break;
		}

		// set test mode
		imuWriteByteRegister(drvHMC5883_I2C_ADDRESS, drvHMC5883_RA_CONFIG_A,	(drvHMC5883_CRA_ODR_15 | drvHMC5883_CRA_MEAS_AVG_1 | meas_mode), &success);

		sysDelay(drvHMC5883_SELF_TEST_DELAY);

		// Start test measurement
		imuWriteByteRegister(drvHMC5883_I2C_ADDRESS, drvHMC5883_RA_MODE, drvHMC5883_MODE_SINGLE, &success);

		sysDelay(drvHMC5883_SELF_TEST_MEASUREMENT_DELAY);

#if 0

	int count;
	uint8_t meas_mode;          /* test measurement mode */
	vector3_t data;
	bool status = true;
	sensor_hal_t *const hal = sensor->hal;

	struct {
		uint8_t config_reg_a;
		uint8_t config_reg_b;
		uint8_t mode_reg;
	}
	reg_set;

	/* Initialize result code */
	*test_code = SENSOR_TEST_ERR_NONE;

	/* Save register values */
	count = sensor_bus_read(hal, HMC5883L_CONFIG_REG_A, (uint8_t *)&reg_set,
			sizeof(reg_set));

	if (count != sizeof(reg_set)) {
		*test_code = SENSOR_TEST_ERR_READ;
		return false;
	}


	/* Set test mode */
	switch (*test_code) {          /* which test was specified */
	case SENSOR_TEST_DEFAULT:
	case SENSOR_TEST_BIAS_POS:
		meas_mode = MEAS_MODE_POS;  /* positive bias measurement mode */
		break;

	case SENSOR_TEST_BIAS_NEG:
		meas_mode = MEAS_MODE_NEG;  /* negative bias measurement mode */
		break;

	default:
		/* bad test code specified */
		sensor_bus_put(hal, HMC5883L_CONFIG_REG_B,
				reg_set.config_reg_b);
		/* restore reg */
		*test_code = SENSOR_TEST_ERR_FUNCTION;
		return false;
	}


	delay_ms(SELF_TEST_DELAY_MSEC);

	/* Perform test measurement & check results */
	sensor_bus_put(hal, HMC5883L_MODE_REG, MODE_SINGLE); /* single meas mode
	                                                      **/

	if (hmc5883l_get_data(hal, &data) != true) {
		*test_code = SENSOR_TEST_ERR_READ;
		status = false;    /* failed to read data registers */
	} else {
		if (arg != NULL) {
			((sensor_data_t *)arg)->scaled = false; /* only raw values */
			((sensor_data_t *)arg)->timestamp = 0;  /* no timestamp */

			((sensor_data_t *)arg)->axis.x = (int32_t)data.x; /* copy values */
			((sensor_data_t *)arg)->axis.y = (int32_t)data.y;
			((sensor_data_t *)arg)->axis.z = (int32_t)data.z;
		}

		/* Check range of readings */
		if ((HMC5883L_TEST_X_MIN > data.x) ||
				(data.x > HMC5883L_TEST_X_MAX) ||
				(HMC5883L_TEST_Y_MIN > data.y) ||
				(data.y > HMC5883L_TEST_Y_MAX) ||
				(HMC5883L_TEST_Z_MIN > data.z) ||
				(data.z > HMC5883L_TEST_Z_MAX)) {
			*test_code = SENSOR_TEST_ERR_RANGE;
			status = false; /* value out of range */
		}
	}

	/* Restore registers */
	count = sensor_bus_write(hal, HMC5883L_CONFIG_REG_A,
			(uint8_t *)&reg_set, sizeof(reg_set));

	if (count != sizeof(reg_set)) {
		*test_code = SENSOR_TEST_ERR_WRITE;
		status = false;
#endif
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
  	HMC5883_I2C_WriteByte(HMC5883_I2C, drvHMC5883_RA_CONFIG_A);
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
  	HMC5883_I2C_WriteByte(HMC5883_I2C, drvHMC5883_RA_MODE);
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
			HMC5883_I2C_WriteByte(HMC5883_I2C, drvHMC5883_RA_DATAOUT_XMSB);
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
