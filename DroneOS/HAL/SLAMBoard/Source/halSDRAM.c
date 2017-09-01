/*****************************************************************************/
/* Hardware Abstraction Layer for SDRAM interface on STM32F429-DISCO         */
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
#include <halSDRAM.h>
#include <halIODefinitions.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define halSDRAM_COMMAND_TARGET_BANK   FMC_SDRAM_CMD_TARGET_BANK2
#define halSDRAM_TIMEOUT               ((uint32_t)0xFFFF)
#define halSDRAM_REFRESH_COUNT         ((uint32_t)1386)			// (15.62 us x Freq) - 20

 /// @brief  FMC SDRAM Mode definition register defines
#define halSDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define halSDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define halSDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define halSDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define halSDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define halSDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define halSDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define halSDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define halSDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define halSDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define halSDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)


/*****************************************************************************/
/* Module local functions                                                    */
/*****************************************************************************/
static void halSDRAMGPIOInit(void);

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static bool l_sdram_initialized = false;

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes SDRAM interface
/// @return True if successfully initialized
bool halSDRAMInitialize(void)
{
	SDRAM_HandleTypeDef sdram;
	FMC_SDRAM_TimingTypeDef timing;
	FMC_SDRAM_CommandTypeDef command;
	uint32_t int_buffer;

	// check if already initialized
	if (l_sdram_initialized)
	{
		return true;
	}

  // Perform the SDRAM memory initialization sequence
  sdram.Instance = FMC_SDRAM_DEVICE;

  // Init GPIO pins
  halSDRAMGPIOInit();

  // init SDRAM struct members
  sdram.Init.SDBank							= FMC_SDRAM_BANK2;
  sdram.Init.ColumnBitsNumber		= FMC_SDRAM_COLUMN_BITS_NUM_8;	// Row addressing: [7:0]
  sdram.Init.RowBitsNumber			= FMC_SDRAM_ROW_BITS_NUM_12;		// Column addressing: [11:0]
  sdram.Init.MemoryDataWidth		= FMC_SDRAM_MEM_BUS_WIDTH_16;
  sdram.Init.InternalBankNumber	= FMC_SDRAM_INTERN_BANKS_NUM_4;
  sdram.Init.CASLatency					= FMC_SDRAM_CAS_LATENCY_3;
  sdram.Init.WriteProtection		= FMC_SDRAM_WRITE_PROTECTION_DISABLE;
  sdram.Init.SDClockPeriod			= FMC_SDRAM_CLOCK_PERIOD_2;
  sdram.Init.ReadBurst					= FMC_SDRAM_RBURST_DISABLE;
  sdram.Init.ReadPipeDelay			= FMC_SDRAM_RPIPE_DELAY_0;

  // SDRAM Timing
  // Timing configuration for 90 Mhz of SD clock frequency (180Mhz/2)
  timing.LoadToActiveDelay    = 2;	// TMRD: 2 Clock cycles
  timing.ExitSelfRefreshDelay = 7;	// TXSR: min=70ns (7x11.11ns)
  timing.SelfRefreshTime      = 4;	// TRAS: min=42ns (4x11.11ns) max=120k (ns)
  timing.RowCycleDelay        = 7;	// TRC:  min=70 (7x11.11ns)
  timing.WriteRecoveryTime    = 2;	// TWR:  min=1+ 7ns (1+1x11.11ns)
  timing.RPDelay              = 2;	// TRP:  20ns => 2x11.11ns
  timing.RCDDelay             = 2;	// TRCD: 20ns => 2x11.11ns

  HAL_SDRAM_Init(&sdram, &timing);

	// SDRAM Init sequence

  // Step 1:  Configure a clock configuration enable command
	command.CommandMode							= FMC_SDRAM_CMD_CLK_ENABLE;
	command.CommandTarget 					= halSDRAM_COMMAND_TARGET_BANK;
	command.AutoRefreshNumber 			= 1;
	command.ModeRegisterDefinition 	= 0;

	HAL_SDRAM_SendCommand(&sdram, &command, halSDRAM_TIMEOUT);

  // Step 2: Insert 100 us minimum delay
  // Inserted delay is equal to 1 ms due to systick time base unit (ms)
  HAL_Delay(1);

  // Step 3: Configure a PALL (precharge all) command
	command.CommandMode          	= FMC_SDRAM_CMD_PALL;
	command.CommandTarget          	= halSDRAM_COMMAND_TARGET_BANK;
	command.AutoRefreshNumber      	= 1;
	command.ModeRegisterDefinition 	= 0;

	HAL_SDRAM_SendCommand(&sdram, &command, halSDRAM_TIMEOUT);

	// Step 4: Configure an Auto Refresh command
	command.CommandMode            	= FMC_SDRAM_CMD_AUTOREFRESH_MODE;
	command.CommandTarget          	= halSDRAM_COMMAND_TARGET_BANK;
	command.AutoRefreshNumber      	= 8;
	command.ModeRegisterDefinition 	= 0;

	HAL_SDRAM_SendCommand(&sdram, &command, halSDRAM_TIMEOUT);

	// Step 5: Program the external memory mode register

	int_buffer =  (uint32_t)halSDRAM_MODEREG_BURST_LENGTH_2          |
      										halSDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |
      										halSDRAM_MODEREG_CAS_LATENCY_3           |
      										halSDRAM_MODEREG_OPERATING_MODE_STANDARD |
      										halSDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

	command.CommandMode            	= FMC_SDRAM_CMD_LOAD_MODE;
	command.CommandTarget          	= halSDRAM_COMMAND_TARGET_BANK;
	command.AutoRefreshNumber      	= 1;
	command.ModeRegisterDefinition 	= int_buffer;

	HAL_SDRAM_SendCommand(&sdram, &command, halSDRAM_TIMEOUT);

	// Set the refresh rate
	HAL_SDRAM_ProgramRefreshRate(&sdram, halSDRAM_REFRESH_COUNT);

  // Short delay
  HAL_Delay(1);

	// brief RAM check
	// write value
	halSDRAMWriteByte(0xbeef, 0xA5);

	// read back the previous value
	int_buffer = halSDRAMReadByte(0xbeef);
	if (halSDRAMReadByte(0xbeef) == 0xA5)
	{
		// initialized
		l_sdram_initialized = true;
	}
	else
	{
		// not initialized
		l_sdram_initialized = false;
	}

	// return status
	return l_sdram_initialized;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes GPIO pins for SDRAM interface
/// @note GPIO clock for the used port must be enabled separately. This function will not deal with the GPIO clock.
static void halSDRAMGPIOInit(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;

  /* Peripheral clock enable */
  __FMC_CLK_ENABLE();

  /** FMC GPIO Configuration
  PF0   ------> FMC_A0
  PF1   ------> FMC_A1
  PF2   ------> FMC_A2
  PF3   ------> FMC_A3
  PF4   ------> FMC_A4
  PF5   ------> FMC_A5
  PC0   ------> FMC_SDNWE
  PF11   ------> FMC_SDNRAS
  PF12   ------> FMC_A6
  PF13   ------> FMC_A7
  PF14   ------> FMC_A8
  PF15   ------> FMC_A9
  PG0   ------> FMC_A10
  PG1   ------> FMC_A11
  PE7   ------> FMC_D4
  PE8   ------> FMC_D5
  PE9   ------> FMC_D6
  PE10   ------> FMC_D7
  PE11   ------> FMC_D8
  PE12   ------> FMC_D9
  PE13   ------> FMC_D10
  PE14   ------> FMC_D11
  PE15   ------> FMC_D12
  PD8   ------> FMC_D13
  PD9   ------> FMC_D14
  PD10   ------> FMC_D15
  PD14   ------> FMC_D0
  PD15   ------> FMC_D1
  PG4   ------> FMC_BA0
  PG5   ------> FMC_BA1
  PG8   ------> FMC_SDCLK
  PD0   ------> FMC_D2
  PD1   ------> FMC_D3
  PG15   ------> FMC_SDNCAS
  PB5   ------> FMC_SDCKE1
  PB6   ------> FMC_SDNE1
  PE0   ------> FMC_NBL0
  PE1   ------> FMC_NBL1
  */
  GPIO_InitStruct.Pin = A0_PIN|A1_PIN|A2_PIN|A3_PIN
                          |A4_PIN|A5_PIN|SDNRAS_PIN|A6_PIN
                          |A7_PIN|A8_PIN|A9_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = SDNWE_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(SDNWE_GPIO_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = A10_PIN|A11_PIN|BA0_PIN|BA1_PIN
                          |SDCLK_PIN|SDNCAS_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = D4_PIN|D5_PIN|D6_PIN|D7_PIN
                          |D8_PIN|D9_PIN|D10_PIN|D11_PIN
                          |D12_PIN|NBL0_PIN|NBL1_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = D13_PIN|D14_PIN|D15_PIN|D0_PIN
                          |D1_PIN|D2_PIN|D3_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = SDCKE1_PIN|SDNE1_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

