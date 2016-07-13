/*****************************************************************************/
/* System initialization functinos                                           */
/*                                                                           */
/* Copyright (C) 2014 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <stm32f4xx_hal.h>
#include <usb_device.h>
#include <comManager.h>
#include <comInterfaces.h>
#include <halHelpers.h>
#include <sysHighresTimer.h>
#include <drvServo.h>
#include <cfgStorage.h>
#include <halUART.h>
#include <comUART.h>
#include <halRTC.h>
#include <comUDP.h>
#include <comHID.h>
#include <drvPPM.h>

//#include <drvUART.h>
//#include <halIODefinitions.h>


//#include <imuTask.h>


/*****************************************************************************/
/* Local function prototypes                                                 */
/*****************************************************************************/


/*****************************************************************************/
/* Task function prototypes                                                  */
/*****************************************************************************/
extern void tskHeartbeat(void* in_argument);


/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes all system components
void sysInitialize(void)
{
  HAL_Init();

  // system clock setup
  halSystemClockConfig();

  // GPIO Ports Clock Enable
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();

  __DMA1_CLK_ENABLE();
  __DMA2_CLK_ENABLE();

  // Init drivers
  halRTCInit();
  sysHighresTimerInit();
  drvServoInit();
  drvPPMInit();
  //imuInitialize();


	// load configuration
	cfgStorageInit();
	cfgLoadDefaultConfiguration();
	cfgLoadConfiguration();

	// init uarts
	halUARTInit();

	// Init communication interfaces
	comManagerInit();
	//comUARTInit();
  //comUDPInit();
  //comHIDInit();


	//cfgSaveConfiguration();
	// init commuication
	//comManagerInit();
	//comUARTInit();

	/*

	uint8_t i;

	// load configuration
	cfgStorageInit();
	cfgLoadDefaultConfiguration();
	cfgLoadConfiguration();

	// init uarts
	for (i = 0; i < drvUART_MAX_COUNT; i++)
	{
		drvUARTInit(i);
	}

	// init commuication
	comManagerInit();
	comUDPInit();
	comUARTInit();


	//comESP8266Init();


	//drvEthernetInit();
	//comManagerInit();
	
	//UARTInit(0);
	//telemetryInit();

	 */

}

///////////////////////////////////////////////////////////////////////////////
/// @brief Creates all tasks needed for system
void sysCreateTasks(void)
{
  sysTaskCreate(tskHeartbeat, "HeartBeat", configMINIMAL_STACK_SIZE, sysNULL, 1, sysNULL, sysNULL );
}
