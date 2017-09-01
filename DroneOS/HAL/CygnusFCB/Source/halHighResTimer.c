/*****************************************************************************/
/* High resolution (1us resolution) timer driver                             */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <halIODefinitions.h>
#include <stm32f4xx_hal.h>
#include <halHelpers.h>
#include <sysHighresTimer.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define drvHIGHRESTIMER_CLOCK 1000000

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static TIM_HandleTypeDef l_high_res_timer;
static volatile uint16_t l_timer_high = 0;

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes high resolution timer
void halHighresTimerInit(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig;

    // Clock Init
  __TIM9_CLK_ENABLE();

  l_high_res_timer.Instance = TIM9;
  l_high_res_timer.Init.Prescaler = halTimerGetSourceFrequency(9) / drvHIGHRESTIMER_CLOCK - 1;
  l_high_res_timer.Init.CounterMode = TIM_COUNTERMODE_UP;
  l_high_res_timer.Init.Period = 0xffff;
  l_high_res_timer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  l_high_res_timer.Init.RepetitionCounter = 0;
  HAL_TIM_Base_Init(&l_high_res_timer);

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource(&l_high_res_timer, &sClockSourceConfig);

  // Interrupt enable
  HAL_NVIC_SetPriority(TIM1_BRK_TIM9_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(TIM1_BRK_TIM9_IRQn);

  HAL_TIM_Base_Start_IT(&l_high_res_timer);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets high resolution time timestamp
sysHighresTimestamp sysHighresTimerGetTimestamp(void)
{
	uint32_t timestamp;
	uint16_t upper_word;

	do
	{
		upper_word = l_timer_high;
		timestamp =  __HAL_TIM_GetCounter(&l_high_res_timer);
	} while(upper_word != l_timer_high);

  return (((uint32_t)upper_word)<<16) | timestamp;
}
/*****************************************************************************/
/* Interrupt handler                                                         */
/*****************************************************************************/
void TIM1_BRK_TIM9_IRQHandler(void)
{
  if (__HAL_TIM_GET_FLAG(&l_high_res_timer, TIM_FLAG_UPDATE) != RESET)      //In case other interrupts are also running
  {
  	if (__HAL_TIM_GET_ITSTATUS(&l_high_res_timer, TIM_IT_UPDATE) != RESET)
  	{
  		l_timer_high++;
  		__HAL_TIM_CLEAR_FLAG(&l_high_res_timer, TIM_FLAG_UPDATE);
  	}
  }
}

