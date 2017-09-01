/*****************************************************************************/
/* SPI EEPROM HAL                                                            */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

/*****************************************************************************/
/* Include files                                                             */
/*****************************************************************************/
#include <stm32f4xx_hal.h>
#include <halIODefinitions.h>
#include <halHelpers.h>
#include <halEEPROM.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define halEEPROM_SPI_TIMEOUT 10

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/
//typedef enum
//{
//	drvSPI_CS_Idle,
//
//	// write-write operation
//	drvSPI_CS_WriteBlock1,
//	drvSPI_CS_WriteBlock2,
//
//	// write-read operation
//	drvSPI_CS_WriteBlock,
//	drvSPI_CS_ReadBlock
//
//} drvSPICommunicationState;

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static SPI_HandleTypeDef l_spi;
//static drvSPICommunicationState l_communication_state = drvSPI_CS_Idle;
//static DMA_HandleTypeDef l_hdma_spi1_rx;
//static DMA_HandleTypeDef l_hdma_spi1_tx;
//static uint8_t* l_block2_data;
//static uint16_t l_block2_length;

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/


///////////////////////////////////////////////////////////////////////////////
/// @brief Initialize EEPROM SPI HAL layer
void halEEPROMInit(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  // Enable DMA clock
  //__DMA2_CLK_ENABLE();

  // setup DMA priority
  //HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 5, 0);
  //HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);

  //HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 5, 0);
  //HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);

  // Peripheral clock enable
  __SPI1_CLK_ENABLE();

  // setup SPI
  l_spi.Instance = SPI1;
  l_spi.Init.Mode = SPI_MODE_MASTER;
  l_spi.Init.Direction = SPI_DIRECTION_2LINES;
  l_spi.Init.DataSize = SPI_DATASIZE_8BIT;
  l_spi.Init.CLKPolarity = SPI_POLARITY_LOW;
  l_spi.Init.CLKPhase = SPI_PHASE_1EDGE;
  l_spi.Init.NSS = SPI_NSS_SOFT;
  l_spi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
  l_spi.Init.FirstBit = SPI_FIRSTBIT_MSB;
  l_spi.Init.TIMode = SPI_TIMODE_DISABLED;
  l_spi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
  l_spi.Init.CRCPolynomial = 10;
  HAL_SPI_Init(&l_spi);

  /**SPI1 GPIO Configuration
  PB3     ------> SPI1_SCK
  PB4     ------> SPI1_MISO
  PB5     ------> SPI1_MOSI
  */
  GPIO_InitStruct.Pin = SPI_SCK_Pin|SPI_MISO_Pin|SPI_MOSI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  // Configure CS Pin
  GPIO_InitStruct.Pin = SPI_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  drvHAL_SetPinHigh(SPI_CS_GPIO_Port, SPI_CS_Pin);

  // Peripheral DMA init
	//l_hdma_spi1_rx.Instance = DMA2_Stream2;
	//l_hdma_spi1_rx.Init.Channel = DMA_CHANNEL_3;
	//l_hdma_spi1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
	//l_hdma_spi1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
	//l_hdma_spi1_rx.Init.MemInc = DMA_MINC_ENABLE;
	//l_hdma_spi1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	//l_hdma_spi1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	//l_hdma_spi1_rx.Init.Mode = DMA_NORMAL;
	//l_hdma_spi1_rx.Init.Priority = DMA_PRIORITY_LOW;
	//l_hdma_spi1_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	//HAL_DMA_Init(&l_hdma_spi1_rx);

	//__HAL_LINKDMA(&l_spi,hdmarx,l_hdma_spi1_rx);

	//l_hdma_spi1_tx.Instance = DMA2_Stream3;
	//l_hdma_spi1_tx.Init.Channel = DMA_CHANNEL_3;
	//l_hdma_spi1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
	//l_hdma_spi1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
  //l_hdma_spi1_tx.Init.MemInc = DMA_MINC_ENABLE;
  //l_hdma_spi1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  //l_hdma_spi1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  //l_hdma_spi1_tx.Init.Mode = DMA_NORMAL;
  //l_hdma_spi1_tx.Init.Priority = DMA_PRIORITY_LOW;
  //l_hdma_spi1_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  //HAL_DMA_Init(&l_hdma_spi1_tx);

  //  //__HAL_LINKDMA(&l_spi,hdmatx,l_hdma_spi1_tx);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Writes then reads block of data over the SPI bus
/// @param in_write_block Pointer to data block to write
/// @param in_write_block_length Number of bytes to write
/// @param out_read_block Pointer to data block to read data
/// @param in_write_block_length Number of bytes to read
/// @return True if operation is success
bool halEEPROMWriteAndReadBlock(uint8_t* in_write_block, uint16_t in_write_block_length, uint8_t* out_read_block, uint16_t in_read_block_length)
{
	drvHAL_SetPinLow(SPI_CS_GPIO_Port, SPI_CS_Pin);

	//l_communication_state = drvSPI_CS_WriteBlock;
	//l_block2_data = out_read_block;
	//l_block2_length = in_read_block_length;

	//HAL_SPI_Transmit_DMA(&l_spi, in_write_block, in_write_block_length);
	HAL_SPI_Transmit(&l_spi, in_write_block, in_write_block_length, halEEPROM_SPI_TIMEOUT);
	HAL_SPI_Receive(&l_spi, out_read_block, in_read_block_length, halEEPROM_SPI_TIMEOUT);

	drvHAL_SetPinHigh(SPI_CS_GPIO_Port, SPI_CS_Pin);

	//TODO: Error handling
	return true;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Writes blocks of data then writes another block again
/// @param in_write_block1 Pointer to first data block to write
/// @param in_write_block1_length Number of bytes to write as first data block
/// @param in_write_block2 Pointer to second data block to write
/// @param in_write_block2_length Number of bytes to write as second data block (can be zero length)
/// @return True if operation is success
bool halEEPROMWriteAndWriteBlock(uint8_t* in_write_block1, uint16_t in_write_block1_length, uint8_t* in_write_block2, uint16_t in_write_block2_length)
{
	drvHAL_SetPinLow(SPI_CS_GPIO_Port, SPI_CS_Pin);
	HAL_SPI_Transmit(&l_spi, in_write_block1, in_write_block1_length, halEEPROM_SPI_TIMEOUT);

	if(in_write_block2_length > 0 && in_write_block2 != sysNULL)
		HAL_SPI_Transmit(&l_spi, in_write_block2, in_write_block2_length, halEEPROM_SPI_TIMEOUT);

	drvHAL_SetPinHigh(SPI_CS_GPIO_Port, SPI_CS_Pin);

	//TODO: Error handling
	return true;
}

/*****************************************************************************/
/* Callbacks                                                                 */
/*****************************************************************************/
//void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
//{
//	switch(l_communication_state)
//	{
//		case drvSPI_CS_WriteBlock:
//			l_communication_state = drvSPI_CS_ReadBlock;
//			HAL_SPI_Receive_DMA(&l_spi, l_block2_data, l_block2_length);
//			break;
//
//		case drvSPI_CS_ReadBlock:
//			l_communication_state = drvSPI_CS_Idle;
//			drvHAL_SetPinHigh(SPI_CS_GPIO_Port, SPI_CS_Pin);
//			break;
//	}
//}
//
//void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
//{
//	switch(l_communication_state)
//	{
//		case drvSPI_CS_ReadBlock:
//			l_communication_state = drvSPI_CS_Idle;
//			drvHAL_SetPinHigh(SPI_CS_GPIO_Port, SPI_CS_Pin);
//			break;
//	}
//}

/*****************************************************************************/
/* Interrupt handlers                                                        */
/*****************************************************************************/

/////////////////////////////////////////////////////////////////////////////////
///// @brief This function handles DMA2 stream2 global interrupt.
//void DMA2_Stream2_IRQHandler(void)
//{
//  HAL_DMA_IRQHandler(&l_hdma_spi1_rx);
//}
//
/////////////////////////////////////////////////////////////////////////////////
///// @brief This function handles DMA2 stream3 global interrupt.
//void DMA2_Stream3_IRQHandler(void)
//{
//  HAL_DMA_IRQHandler(&l_hdma_spi1_tx);
//}
