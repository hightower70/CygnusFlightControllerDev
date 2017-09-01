/*****************************************************************************/
/* Analog to digital converter interface                                     */
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
#include <halADC.h>
#include <sysRTOS.h>
#include <stm32f4xx_hal.h>
#include <stm32f4xx_hal_adc.h>
#include <stm32f4xx_hal_dma.h>
#include <halIODefinitions.h>

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static ADC_HandleTypeDef l_adc;
static DMA_HandleTypeDef l_adc_dma;

#define ADC_BUFFER_LENGTH 64

uint32_t g_ADCBuffer[ADC_BUFFER_LENGTH];

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
//static void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* AdcHandle);

/*****************************************************************************/
/* Public functions                                                          */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initialize ADC HAL layer
void halADCInit(void)
{
  ADC_ChannelConfTypeDef sConfig;
  GPIO_InitTypeDef GPIO_InitStruct;

  __ADC1_CLK_ENABLE();

  // ADC GPIO configuration
  GPIO_InitStruct.Pin = AIN3_Pin|AIN2_Pin|AIN1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = VBAT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(VBAT_GPIO_Port, &GPIO_InitStruct);

  // Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  l_adc.Instance = ADC1;
  l_adc.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV8;
  l_adc.Init.Resolution = ADC_RESOLUTION12b;
  l_adc.Init.ScanConvMode = ENABLE;
  l_adc.Init.ContinuousConvMode = ENABLE;
  l_adc.Init.DiscontinuousConvMode = DISABLE;
  l_adc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  l_adc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  l_adc.Init.NbrOfConversion = 4;
  l_adc.Init.DMAContinuousRequests = ENABLE;
  l_adc.Init.EOCSelection = DISABLE;
  HAL_ADC_Init(&l_adc);

  // Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
  HAL_ADC_ConfigChannel(&l_adc, &sConfig);

  // Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  sConfig.Channel = ADC_CHANNEL_7;
  sConfig.Rank = 2;
  HAL_ADC_ConfigChannel(&l_adc, &sConfig);

  // Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  sConfig.Channel = ADC_CHANNEL_6;
  sConfig.Rank = 3;
  HAL_ADC_ConfigChannel(&l_adc, &sConfig);

  // Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = 4;
  HAL_ADC_ConfigChannel(&l_adc, &sConfig);

  // DMA controller clock enable
  __DMA2_CLK_ENABLE();

  // Peripheral DMA init
  l_adc_dma.Instance = DMA2_Stream0;
  l_adc_dma.Init.Channel = DMA_CHANNEL_0;
  l_adc_dma.Init.Direction = DMA_PERIPH_TO_MEMORY;
  l_adc_dma.Init.PeriphInc = DMA_PINC_DISABLE;
  l_adc_dma.Init.MemInc = DMA_MINC_ENABLE;
  l_adc_dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  l_adc_dma.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
  l_adc_dma.Init.Mode = DMA_CIRCULAR;
  l_adc_dma.Init.Priority = DMA_PRIORITY_LOW;
  l_adc_dma.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  l_adc_dma.Init.MemBurst = DMA_MBURST_SINGLE;
  l_adc_dma.Init.PeriphBurst = DMA_PBURST_SINGLE;
  //l_adc_dma.XferCpltCallback = HAL_ADC_ConvCpltCallback;
  HAL_DMA_Init(&l_adc_dma);

  __HAL_LINKDMA(&l_adc, DMA_Handle, l_adc_dma);

  // DMA interrupt init
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

  HAL_ADC_Start_DMA(&l_adc, g_ADCBuffer, ADC_BUFFER_LENGTH);
}

void DMA2_Stream0_IRQHandler()
{
	HAL_DMA_IRQHandler(&l_adc_dma);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* AdcHandle)
{
	uint32_t offset = DMA2_Stream0->NDTR;

}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* AdcHandle)
{
	uint32_t offset = DMA2_Stream0->NDTR;
}
