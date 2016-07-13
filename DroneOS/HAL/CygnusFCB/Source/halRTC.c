/*****************************************************************************/
/* Real-time clock HAL interface                                             */
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
#include <halRTC.h>
#include <stm32f4xx_hal.h>
#include <halHelpers.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/


/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static sysDateTime l_datetime;
static volatile bool l_datetime_access_locked = false;
static TIM_HandleTypeDef l_rtc_timer;

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes real time clock
void halRTCInit(void)
{
	TIM_ClockConfigTypeDef sClockSourceConfig;

  __TIM13_CLK_ENABLE();

	l_rtc_timer.Instance = TIM13;
	l_rtc_timer.Init.Prescaler = halTimerGetSourceFrequency(13) / 4 / 1000 - 1;
	l_rtc_timer.Init.CounterMode = TIM_COUNTERMODE_UP;
	l_rtc_timer.Init.Period = 1000 - 1;
	l_rtc_timer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV4;
  HAL_TIM_Base_Init(&l_rtc_timer);

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource(&l_rtc_timer, &sClockSourceConfig);

  HAL_NVIC_SetPriority(TIM8_UP_TIM13_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn);

  HAL_TIM_Base_Start_IT(&l_rtc_timer);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets current Date & Time
/// @param out_datetime Current datetime
void sysDateTimeGet(sysDateTime* out_datetime)
{
	*out_datetime = l_datetime ;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sets Date & Time
/// @param in_datetime Datetime to set
void sysDateTimeSet(sysDateTime* in_datetime)
{
	l_datetime_access_locked = true;

	l_datetime = *in_datetime;

	l_datetime_access_locked = false;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Interrupt handler
void TIM8_UP_TIM13_IRQHandler(void)
{
  /* TIM13 Update event */
  if(__HAL_TIM_GET_FLAG(&l_rtc_timer, TIM_FLAG_UPDATE) != RESET)
  {
    if(__HAL_TIM_GET_IT_SOURCE(&l_rtc_timer, TIM_IT_UPDATE) !=RESET)
    {
      __HAL_TIM_CLEAR_IT(&l_rtc_timer, TIM_IT_UPDATE);

      if(!l_datetime_access_locked)
      {
      	sysDateTimeAddOneSecond(&l_datetime);
      }
    }
  }

   //HAL_TIM_IRQHandler(&htim8);
   //HAL_TIM_IRQHandler(&l_rtc_timer);
 }
