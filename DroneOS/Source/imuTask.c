/*****************************************************************************/
/* Inertial Measurement Unit handler task                                    */
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
#include <drvIMU.h>
#include <imuCommunication.h>
#include <sysRTOS.h>

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/

typedef struct
{
	bool IsValid;
	uint8_t Class;
	drvIMUSensorControlFunction Control;
	drvIMUSensorReadFunction Read;
} tskIMUSensorInfo;


/*****************************************************************************/
/* External function prototypes                                              */
/*****************************************************************************/
extern void drvADXL345Control(drvIMUControlFunction in_function, void* in_function_parameter);
extern void drvHMC5883Control(drvIMUControlFunction in_function, void* in_function_parameter);
extern void drvMPU6050Control(drvIMUControlFunction in_function, void* in_function_parameter);

/*****************************************************************************/
/* Local functions                                                           */
/*****************************************************************************/
static void tskIMU(void* in_argument);

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
sysTask l_imu_task;
tskIMUSensorInfo l_sensor_info[drvIMU_MaxSensorCount];

static drvIMUSensorControlFunction l_imu_sensor_config_functions[] =
{
	drvMPU6050Control,
		//drvADXL345Control,
	//drvHMC5883Control,

	sysNULL
};


///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes IMU operation
void imuInitialize(void)
{
  sysTaskCreate(tskIMU, "IMU", configMINIMAL_STACK_SIZE, NULL, 4, &l_imu_task );
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Main IMU data processing task
static void tskIMU(void* in_argument)
{
	uint8_t sensor_index;
	uint8_t driver_index;
	drvIMUDetectParameter param;
	drvIMUSelfTestParameter self_test_param;

	// initialize
	for(sensor_index = 0; sensor_index < drvIMU_MaxSensorCount; sensor_index++)
		l_sensor_info[sensor_index].IsValid = false;

	// init communication
  drvIMUInit();
  imuCommunicationInit();

  // wait for sensor power-up
  sysDelay(100);

	// detect available sensors
	sensor_index = 0;
	driver_index = 0;
	while( l_imu_sensor_config_functions[driver_index] != sysNULL)
	{
		param.Success = false;
		l_imu_sensor_config_functions[driver_index](drvIMU_CF_Detect, &param);

		if(param.Success)
		{
			l_sensor_info[sensor_index].IsValid = true;
			l_sensor_info[sensor_index].Class = param.Class;
			l_sensor_info[sensor_index].Control = param.Control;
			l_sensor_info[sensor_index].Read = param.Read;

			sensor_index++;
		}

		driver_index++;
	}

	// self test sensors
	for(sensor_index=0; sensor_index<drvIMU_MaxSensorCount; sensor_index++)
	{
		if(l_sensor_info[sensor_index].IsValid)
		{
			l_sensor_info[sensor_index].Control(drvIMU_CF_SelfTest, &self_test_param);
		}
	}



	while(1)
	{
			sysDelay(1000);
	}
}

