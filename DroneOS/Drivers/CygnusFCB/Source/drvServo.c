/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <drvIODefinitions.h>
#include <stm32f4xx_hal.h>
#include <drvHAL.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define drvSERVO_CHANNEL_COUNT 8
#define drvSERVO_CHANNEL_PERIOD 2500
#define drvSERVO_TIME_CLOCK 1000000

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static TIM_HandleTypeDef l_servo_timer;
static uint8_t l_current_channel = 0;

void drvServoInit(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;
  GPIO_InitTypeDef GPIO_InitStruct;

  // GPIO Configuration
  // PB1     ------> TIM3_CH4
  GPIO_InitStruct.Pin = SERVO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
  HAL_GPIO_Init(SERVO_GPIO_Port, &GPIO_InitStruct);

  // configure servo select pins
  GPIO_InitStruct.Pin = SERVO_SEL_A_Pin|SERVO_SEL_B_Pin|SERVO_SEL_C_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(SERVO_SEL_A_GPIO_Port, &GPIO_InitStruct);

  drvHAL_SetPinLow(SERVO_SEL_A_GPIO_Port, SERVO_SEL_A_Pin);
  drvHAL_SetPinLow(SERVO_SEL_B_GPIO_Port, SERVO_SEL_B_Pin);
  drvHAL_SetPinLow(SERVO_SEL_C_GPIO_Port, SERVO_SEL_C_Pin);

  // Clock Init
  __TIM3_CLK_ENABLE();

  l_servo_timer.Instance = TIM3;
  l_servo_timer.Init.Prescaler = drvHALTimerGetSourceFrequency(3) / drvSERVO_TIME_CLOCK - 1;
  l_servo_timer.Init.CounterMode = TIM_COUNTERMODE_UP;
  l_servo_timer.Init.Period = drvSERVO_CHANNEL_PERIOD - 1;
  l_servo_timer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_Base_Init(&l_servo_timer);

  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  HAL_TIM_ConfigClockSource(&l_servo_timer, &sClockSourceConfig);

  HAL_TIM_PWM_Init(&l_servo_timer);

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  HAL_TIMEx_MasterConfigSynchronization(&l_servo_timer, &sMasterConfig);

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = drvSERVO_CHANNEL_PERIOD-1500;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  HAL_TIM_PWM_ConfigChannel(&l_servo_timer, &sConfigOC, TIM_CHANNEL_4);

  // Interrupt enable
  HAL_NVIC_SetPriority(TIM3_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(TIM3_IRQn);

  HAL_TIM_PWM_Start(&l_servo_timer, TIM_CHANNEL_4);

  HAL_TIM_Base_Start_IT(&l_servo_timer);
}

void TIM3_IRQHandler(void)
{
  if (__HAL_TIM_GET_FLAG(&l_servo_timer, TIM_FLAG_UPDATE) != RESET)      //In case other interrupts are also running
  {
      if (__HAL_TIM_GET_ITSTATUS(&l_servo_timer, TIM_IT_UPDATE) != RESET)
      {
          __HAL_TIM_CLEAR_FLAG(&l_servo_timer, TIM_FLAG_UPDATE);

          l_current_channel++;

          if(l_current_channel >= drvSERVO_CHANNEL_COUNT)
          	l_current_channel = 0;

          drvHAL_SetPinValue(SERVO_SEL_A_GPIO_Port, SERVO_SEL_A_Pin, l_current_channel & 0x01);
          drvHAL_SetPinValue(SERVO_SEL_B_GPIO_Port, SERVO_SEL_B_Pin, l_current_channel & 0x02);
          drvHAL_SetPinValue(SERVO_SEL_C_GPIO_Port, SERVO_SEL_C_Pin, l_current_channel & 0x04);
          /*put your code here */
      }
  }
}

#if 0

	  TIM_ClockConfigTypeDef sClockSourceConfig;
	  TIM_MasterConfigTypeDef sMasterConfig;
	  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;
	  TIM_IC_InitTypeDef sConfigIC;
	  TIM_OC_InitTypeDef sConfigOC;
	  GPIO_InitTypeDef GPIO_InitStruct;

	  // Clock Init
	    __TIM1_CLK_ENABLE();

	  // GPIO Configuration
	  // PA10     ------> TIM1_CH3
	  GPIO_InitStruct.Pin = GPIO_PIN_10;
	  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	  GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	  htim1.Instance = TIM1;
	  htim1.Init.Prescaler = 2;
	  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	  htim1.Init.Period = 65535;
	  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	  htim1.Init.RepetitionCounter = 0;
	  HAL_TIM_Base_Init(&htim1);

	  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	  HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig);

	  HAL_TIM_IC_Init(&htim1);

	  HAL_TIM_OC_Init(&htim1);

	  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	  HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig);

	  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
	  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
	  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
	  sBreakDeadTimeConfig.DeadTime = 0;
	  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
	  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
	  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
	  HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig);

	  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
	  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
	  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
	  sConfigIC.ICFilter = 0;
	  HAL_TIM_IC_ConfigChannel(&htim1, &sConfigIC, TIM_CHANNEL_1);

	  sConfigOC.OCMode = TIM_OCMODE_PWM1;
	  sConfigOC.Pulse = 1000;
	  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
	  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	  HAL_TIM_OC_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3);

	  /* Peripheral interrupt init*/
	  HAL_NVIC_SetPriority(TIM1_UP_TIM10_IRQn, 5, 0);
	  HAL_NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);

	  HAL_TIM_OC_Start(&htim1, TIM_CHANNEL_3);

	  HAL_TIM_Base_Start_IT(&htim1);
}

/**
* @brief This function handles TIM1 update interrupt and TIM10 global interrupt.
*/
void TIM1_UP_TIM10_IRQHandler(void)
{
    if (__HAL_TIM_GET_FLAG(&htim1, TIM_FLAG_UPDATE) != RESET)      //In case other interrupts are also running
    {
        if (__HAL_TIM_GET_ITSTATUS(&htim1, TIM_IT_UPDATE) != RESET)
        {
            __HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_UPDATE);
            /*put your code here */
        }
    }
}

#endif
