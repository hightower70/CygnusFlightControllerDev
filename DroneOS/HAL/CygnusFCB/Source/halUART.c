/*****************************************************************************/
/* UART HAL Driver                                                           */
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
#include <sysRTOS.h>
#include <stm32f4xx_hal.h>
#include <stm32f4xx_hal_uart.h>
#include <stm32f4xx_hal_dma.h>
#include <halIODefinitions.h>
#include <halUART.h>

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/

/// UART driver info
typedef struct
{
	UART_HandleTypeDef UARTHandle;
	DMA_HandleTypeDef UARTTxDMA;
	halUARTConfigInfo Config;
} halUARTDriverInfo;

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static halUARTDriverInfo l_uart_info[halUART_MAX_COUNT];

/*****************************************************************************/
/* Local function prototypes                                                 */
/*****************************************************************************/
static void halUARTInitDMA(uint8_t in_uart_index, DMA_Stream_TypeDef* in_dma, uint32_t in_channel_index);
static void halUARTInitHandle(uint8_t in_uart_index, USART_TypeDef* in_uart);
static void UARTIRQHandler(uint8_t in_uart_index);


/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes UART driver
void halUARTInit(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	// just for sure zero all members
	sysMemZero(l_uart_info, sizeof(l_uart_info));

	/***************/
	/* UART 1 init */
	/***************/
	__USART1_CLK_ENABLE();

  /* UART1 GPIO Configuration
  PB6     ------> USART1_TX
  PB7     ------> USART1_RX
  */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* UART1 HAL config */
  halUARTInitHandle(0, USART1);

  /* UART1 Peripheral DMA init */
  halUARTInitDMA(0, DMA2_Stream7, DMA_CHANNEL_4);

  /* UART1 interrupt init */
  HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);

  HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);

  __HAL_UART_ENABLE_IT(&l_uart_info[0].UARTHandle, UART_IT_RXNE);

  __HAL_USART_ENABLE(&l_uart_info[0].UARTHandle);

	/***************/
	/* UART 6 init */
	/***************/
  __USART6_CLK_ENABLE();

  /**USART6 GPIO Configuration
  PC6     ------> USART6_TX
  PC7     ------> USART6_RX
  */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* UART6 HAL config */
  halUARTInitHandle(1, USART6);

  /* Peripheral DMA init*/
  halUARTInitDMA(1, DMA2_Stream6, DMA_CHANNEL_5);

  /* UART6 interrupt init*/
  HAL_NVIC_SetPriority(USART6_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(USART6_IRQn);

  HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);

  __HAL_UART_ENABLE_IT(&l_uart_info[1].UARTHandle, UART_IT_RXNE);

  __HAL_USART_ENABLE(&l_uart_info[1].UARTHandle);

	/***************/
	/* UART 2 init */
	/***************/
  __USART2_CLK_ENABLE();

  /**USART2 GPIO Configuration
  PA2     ------> USART2_TX
  PA3     ------> USART2_RX
  */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* UART2 HAL config */
  halUARTInitHandle(2, USART2);

  /* Peripheral DMA init*/
  halUARTInitDMA(2, DMA1_Stream6, DMA_CHANNEL_4 );

  /* UART2 interrupt init*/
  HAL_NVIC_SetPriority(USART2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(USART2_IRQn);

  HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);

  __HAL_UART_ENABLE_IT(&l_uart_info[2].UARTHandle, UART_IT_RXNE);

  __HAL_USART_ENABLE(&l_uart_info[2].UARTHandle);

	/***************/
	/* UART 4 init */
	/***************/
  __UART4_CLK_ENABLE();

  /**UART4 GPIO Configuration
  PA0-WKUP     ------> UART4_TX
  PA1     ------> UART4_RX
  */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* UART4 HAL config */
  halUARTInitHandle(3, UART4);

  /* Peripheral DMA init*/
  halUARTInitDMA(3, DMA1_Stream4, DMA_CHANNEL_4);

  /* Peripheral interrupt init*/
  HAL_NVIC_SetPriority(UART4_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(UART4_IRQn);

  HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);

  __HAL_UART_ENABLE_IT(&l_uart_info[3].UARTHandle, UART_IT_RXNE);

  __HAL_USART_ENABLE(&l_uart_info[3].UARTHandle);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes HAL UART data structures
static void halUARTInitHandle(uint8_t in_uart_index, USART_TypeDef* in_uart)
{
	UART_HandleTypeDef* uart_handle = &l_uart_info[in_uart_index].UARTHandle;

	uart_handle->Instance = in_uart;
	uart_handle->Init.BaudRate = 115200;
	uart_handle->Init.WordLength = UART_WORDLENGTH_8B;
	uart_handle->Init.StopBits = UART_STOPBITS_1;
	uart_handle->Init.Parity = UART_PARITY_NONE;
	uart_handle->Init.Mode = UART_MODE_TX_RX;
	uart_handle->Init.HwFlowCtl = UART_HWCONTROL_NONE;
	uart_handle->Init.OverSampling = UART_OVERSAMPLING_16;

  HAL_UART_Init(uart_handle);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes UART DMA config structures
static void halUARTInitDMA(uint8_t in_uart_index, DMA_Stream_TypeDef* in_dma, uint32_t in_channel_index)
{
	DMA_HandleTypeDef* uart_dma = &l_uart_info[in_uart_index].UARTTxDMA;

	uart_dma->Instance = in_dma;
	uart_dma->Init.Channel = in_channel_index;
	uart_dma->Init.Direction = DMA_MEMORY_TO_PERIPH;
	uart_dma->Init.PeriphInc = DMA_PINC_DISABLE;
	uart_dma->Init.MemInc = DMA_MINC_ENABLE;
	uart_dma->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	uart_dma->Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	uart_dma->Init.Mode = DMA_NORMAL;
	uart_dma->Init.Priority = DMA_PRIORITY_LOW;
	uart_dma->Init.FIFOMode = DMA_FIFOMODE_DISABLE;

  HAL_DMA_Init(uart_dma);

  __HAL_LINKDMA(&l_uart_info[in_uart_index].UARTHandle, hdmatx, l_uart_info[in_uart_index].UARTTxDMA);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Changes UART configuration
void halUARTConfig(uint8_t in_uart_index, halUARTConfigInfo* in_config_info)
{
	l_uart_info[in_uart_index].Config = *in_config_info;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes drvUARTConfigInfo struct
void halUARTConfigInfoInit(halUARTConfigInfo* in_config_info)
{
	sysMemZero(in_config_info, sizeof(halUARTConfigInfo));
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sets UART baud rate
/// @param in_uart_index UART index
/// @param in_baud_rate Baud rate
bool halUARTSetBaudRate(uint8_t in_uart_index, uint32_t in_baud_rate)
{
	UART_HandleTypeDef* uart_handle = &l_uart_info[in_uart_index].UARTHandle;

	__HAL_USART_DISABLE(uart_handle);

	uart_handle->Init.BaudRate = 115200;
	HAL_UART_Init(uart_handle);

	__HAL_USART_ENABLE(uart_handle);

	return true;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Sends block of data over the UART
/// @param in_uart_index Index of the UART
/// @param in_buffer Buffer containing data to send
/// @param in_buffer_length Number of bytes to send
/// @return True if send operation was started
bool halUARTSendBlock(uint8_t in_uart_index, uint8_t* in_buffer, uint16_t in_buffer_length)
{
	UART_HandleTypeDef* uart_handle = &l_uart_info[in_uart_index].UARTHandle;

	return (HAL_UART_Transmit_DMA(uart_handle, in_buffer, in_buffer_length) == HAL_OK);
}

/*****************************************************************************/
/* Interrupt handler                                                         */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief General IRQ handler
static void UARTIRQHandler(uint8_t in_uart_index)
{
	uint32_t tmp1 = 0;
	uint32_t tmp2 = 0;
	uint8_t received_data;
	halUARTDriverInfo* uart_info = &l_uart_info[in_uart_index];

	sysBeginInterruptRoutine();

	// Override HAL original interrupt handler and handle incoming characters individually
	// All other interrupt sources will be handled by the HAL interrupt handler function
	// Check for received character interrupt
  tmp1 = __HAL_UART_GET_FLAG(&uart_info->UARTHandle, UART_FLAG_RXNE);
  tmp2 = __HAL_UART_GET_IT_SOURCE(&uart_info->UARTHandle, UART_IT_RXNE);
  if((tmp1 != RESET) && (tmp2 != RESET))
  {
  	// get received byte
  	received_data = (uint8_t)(uart_info->UARTHandle.Instance->DR & (uint8_t)0x00FF);

		// process received byte
		if (uart_info->Config.RxReceivedCallback != sysNULL)
			uart_info->Config.RxReceivedCallback(received_data, sysInterruptParam());
  }

  // HAL interrupt handler
 	HAL_UART_IRQHandler(&uart_info->UARTHandle);

 	//TODO: Error handling

	sysEndInterruptRoutine();
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	uint8_t uart_index = 0;
	halUARTDriverInfo* uart_info;

	sysBeginInterruptRoutine();

	// search for uart index
	while(uart_index < halUART_MAX_COUNT && &l_uart_info[uart_index].UARTHandle != huart)
		uart_index++;

	// uart found
	if(uart_index < halUART_MAX_COUNT)
	{
		uart_info = &l_uart_info[uart_index];

		if (uart_info->Config.TxEmptyCallback != sysNULL)
			uart_info->Config.TxEmptyCallback(sysInterruptParam());
	}

	sysEndInterruptRoutine();
}

///////////////////////////////////////////////////////////////////////////////
/// @brief UART1 interrupt handler
void USART1_IRQHandler(void)
{
	UARTIRQHandler(0);
}

///////////////////////////////////////////////////////////////////////////////
// @brief This function handles DMA2 stream7 global interrupt.
void DMA2_Stream7_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&l_uart_info[0].UARTTxDMA);
}

///////////////////////////////////////////////////////////////////////////////
// @brief This function handles USART6 global interrupt.
void USART6_IRQHandler(void)
{
	UARTIRQHandler(1);
}

///////////////////////////////////////////////////////////////////////////////
// @brief This function handles DMA2 stream6 global interrupt.
void DMA2_Stream6_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&l_uart_info[1].UARTTxDMA);
}

///////////////////////////////////////////////////////////////////////////////
// @brief This function handles USART2 global interrupt.
void USART2_IRQHandler(void)
{
	UARTIRQHandler(2);
}

///////////////////////////////////////////////////////////////////////////////
// @brief This function handles DMA1 stream6 global interrupt.
void DMA1_Stream6_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&l_uart_info[2].UARTTxDMA);
}

///////////////////////////////////////////////////////////////////////////////
// @brief This function handles USART4 global interrupt.
void UART4_IRQHandler(void)
{
	UARTIRQHandler(3);
}

///////////////////////////////////////////////////////////////////////////////
// @brief This function handles DMA1 stream4 global interrupt.
void DMA1_Stream4_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&l_uart_info[3].UARTTxDMA);
}

