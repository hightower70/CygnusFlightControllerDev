/*****************************************************************************/
/* Hardware Abstraction Layer Functions (general, system wide functions)     */
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
#include <stm32f4xx_hal.h>
#include <drvHAL.h>
#include <usb_device.h>
#include <sysRTOS.h>
#include <imuTask.h>


/*****************************************************************************/
/* Local function prototypes                                                 */
/*****************************************************************************/
static void drvHALSystemClockConfig(void);

/*****************************************************************************/
/* External functions                                                        */
/*****************************************************************************/
extern void xPortSysTickHandler(void);
extern void drvServoInit(void);
extern void drvStatLEDInit(void);
extern void cfgCreateTasks(void);
extern void rtosSystickHandler(void);
extern void sysHighresTimerInit(void);
extern void drvIMUInit(void);

/*****************************************************************************/
/* External variables                                                        */
/*****************************************************************************/
extern PCD_HandleTypeDef hpcd_USB_OTG_FS;


/*****************************************************************************/
/* Function implementations                                                  */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Main entry function
int main(void)
{
  HAL_Init();

  // systme clock setup
  drvHALSystemClockConfig();

  // GPIO Ports Clock Enable
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  // Init USB
  MX_USB_DEVICE_Init();

  // Init drivers
  drvServoInit();
  drvStatLEDInit();
  imuInitialize();
  sysHighresTimerInit();

  // Create tasks
  cfgCreateTasks();

  // start
  sysStartSheduler();
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets clock frequency of the given timer
/// @param in_timer_id Timer id [1..14]
// Get the frequency (in Hz) of the source clock for the given timer.
// On STM32F405/407/415/417 there are 2 cases for how the clock freq is set.
// If the APB prescaler is 1, then the timer clock is equal to its respective
// APB clock.  Otherwise (APB prescaler > 1) the timer clock is twice its
// respective APB clock.  See DM00031020 Rev 4, page 115.
uint32_t drvHALTimerGetSourceFrequency(uint32_t in_timer_id)
{
	uint32_t source;

	if (in_timer_id == 1 || (8 <= in_timer_id && in_timer_id <= 11))
	{
		// TIM{1,8,9,10,11} are on APB2
		source = HAL_RCC_GetPCLK2Freq();
		if ((uint32_t)((RCC->CFGR & RCC_CFGR_PPRE2) >> 3) != RCC_HCLK_DIV1)
		{
			source *= 2;
		}
	}
	else
	{
		// TIM{2,3,4,5,6,7,12,13,14} are on APB1
		source = HAL_RCC_GetPCLK1Freq();
		if ((uint32_t)(RCC->CFGR & RCC_CFGR_PPRE1) != RCC_HCLK_DIV1)
		{
			source *= 2;
		}
	}

	return source;
}


///////////////////////////////////////////////////////////////////////////////
/// @brief System Clock Configuration
static void drvHALSystemClockConfig(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);


  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1
                              |RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

}

/******************************************************************************/
/* General interrupt handlers                                                 */
/******************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief System tick handler
void SysTick_Handler(void)
{
  HAL_IncTick();
  rtosSystickHandler();
}


////////////////////////////////////////////////////////////////////////////////
/// @brief USB interrupt handler
void OTG_FS_IRQHandler(void)
{
  HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
}
