/*****************************************************************************/
/* Status LED driver (dimming) functions                                     */
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
#include <stm32f4xx_hal.h>
#include <halHelpers.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define drvLED_CLOCK 100000
#define drvLED_PERIOD 100

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static TIM_HandleTypeDef l_led_timer;

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Status LED driver initialization function
void drvStatLEDInit(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_OC_InitTypeDef sConfigOC;
  GPIO_InitTypeDef GPIO_InitStruct;

  // Init GPIO
  // PB15     ------> TIM12_CH2
  GPIO_InitStruct.Pin = STAT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF9_TIM12;
  HAL_GPIO_Init(STAT_GPIO_Port, &GPIO_InitStruct);

  // clock init
  __TIM12_CLK_ENABLE();

  // configure timer
  l_led_timer.Instance = TIM12;
  l_led_timer.Init.Prescaler = halTimerGetSourceFrequency(12) / drvLED_CLOCK - 1;
  l_led_timer.Init.CounterMode = TIM_COUNTERMODE_UP;
  l_led_timer.Init.Period = drvLED_PERIOD - 1;
  l_led_timer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_Base_Init(&l_led_timer);

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource(&l_led_timer, &sClockSourceConfig);

  HAL_TIM_PWM_Init(&l_led_timer);

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  HAL_TIM_PWM_ConfigChannel(&l_led_timer, &sConfigOC, TIM_CHANNEL_2);

  HAL_TIM_PWM_Start(&l_led_timer, TIM_CHANNEL_2);

  HAL_TIM_Base_Start(&l_led_timer);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Set status LED current dimming value
/// @param in_dim Dimming value [0..100]
void drvStatLEDSetDim(uint8_t in_dim)
{
	__HAL_TIM_SET_COMPARE(&l_led_timer, TIM_CHANNEL_2, in_dim);
}
