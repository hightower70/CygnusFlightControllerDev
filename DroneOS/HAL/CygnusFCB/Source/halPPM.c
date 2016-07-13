/*****************************************************************************/
/* HAL layer for Pulse Position Modulation (R/C receiver) interface          */
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
#include <sysRTOS.h>
#include <halIODefinitions.h>
#include <stm32f4xx_hal.h>
#include <halHelpers.h>
#include <halPPM.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define halPPM_TIMER_CLOCK 1000000 // it gives 1us resolution

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static TIM_HandleTypeDef l_ppm_timer;

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes PPM HAL
void halPPMInit(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_IC_InitTypeDef sConfigIC;
  TIM_OC_InitTypeDef sConfigOC;

  __HAL_DBGMCU_FREEZE_TIM1(); // TODO: remove

  // PPM input pin init
  GPIO_InitStruct.Pin = PPM_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
  HAL_GPIO_Init(PPM_GPIO_Port, &GPIO_InitStruct);

  // Clock Init
  __TIM1_CLK_ENABLE();

  // Timer init
  l_ppm_timer.Instance = TIM1;
  l_ppm_timer.Init.Prescaler = halTimerGetSourceFrequency(1) / halPPM_TIMER_CLOCK - 1;
  l_ppm_timer.Init.CounterMode = TIM_COUNTERMODE_UP;
  l_ppm_timer.Init.Period = 0xffff;
  l_ppm_timer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  l_ppm_timer.Init.RepetitionCounter = 0;
  HAL_TIM_OC_Init(&l_ppm_timer);

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource(&l_ppm_timer, &sClockSourceConfig);

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  HAL_TIMEx_MasterConfigSynchronization(&l_ppm_timer, &sMasterConfig);

  // Input capture init
  HAL_TIM_IC_Init(&l_ppm_timer);

  // Input capture config
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  HAL_TIM_IC_ConfigChannel(&l_ppm_timer, &sConfigIC, TIM_CHANNEL_1);

  // config output compare for frame interrupt
  sConfigOC.OCMode = TIM_OCMODE_TOGGLE;
  sConfigOC.Pulse = 1000;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  HAL_TIM_OC_ConfigChannel(&l_ppm_timer, &sConfigOC, TIM_CHANNEL_2);

  // Interrupt enable
  HAL_NVIC_SetPriority(TIM1_CC_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(TIM1_CC_IRQn);

  // Start timer
  HAL_TIM_OC_Start_IT(&l_ppm_timer, TIM_CHANNEL_2);
  HAL_TIM_IC_Start_IT(&l_ppm_timer, TIM_CHANNEL_1);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Start delay for PPM input. When delay expired the callback function will be executed.
/// @param in_delay_in_us Delay time in microseconds
void halPPMStartDelay(uint16_t in_delay_in_us)
{
	uint16_t counter;

	counter = __HAL_TIM_GET_COUNTER(&l_ppm_timer);

	counter += in_delay_in_us;

	__HAL_TIM_SetCompare(&l_ppm_timer, TIM_CHANNEL_2, counter);
}


///////////////////////////////////////////////////////////////////////////////
/// @brief Capture/compare interrupt handler
void TIM1_CC_IRQHandler(void)
{
	uint16_t captured_value;
	sysBeginInterruptRoutine();

	// Input capture
	if(__HAL_TIM_GET_FLAG(&l_ppm_timer, TIM_FLAG_CC1) != RESET)
  {
    if(__HAL_TIM_GET_IT_SOURCE(&l_ppm_timer, TIM_IT_CC1) !=RESET)
    {
      __HAL_TIM_CLEAR_IT(&l_ppm_timer, TIM_IT_CC1);

      captured_value = HAL_TIM_ReadCapturedValue(&l_ppm_timer, TIM_CHANNEL_1);

      halPPMPulseReceivedCallback(captured_value, sysInterruptParam());
    }
  }

	// delay time
	if(__HAL_TIM_GET_FLAG(&l_ppm_timer, TIM_FLAG_CC2) != RESET)
  {
    if(__HAL_TIM_GET_IT_SOURCE(&l_ppm_timer, TIM_IT_CC2) !=RESET)
    {
    	// clear IT flag
      __HAL_TIM_CLEAR_IT(&l_ppm_timer, TIM_IT_CC2);

      halPPMDelayExpiredCallback(sysInterruptParam());
    }
  }

	sysEndInterruptRoutine();
}

