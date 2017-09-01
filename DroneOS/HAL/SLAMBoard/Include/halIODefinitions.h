/*****************************************************************************/
/* I/O definitions                                                           */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

/*****************************************************************************/
/* Used system resources:                                                    */
/*                                                                           */
/*****************************************************************************/

#ifndef __halIODefinitions_h
#define __halIODefinitions_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/

//////////////
// UART config
#define halUART_MAX_COUNT 2

///////////////
// SDRAM config
#define halSDRAM_START_ADDRESS     ((void*)0xD0000000)
#define halSDRAM_MEMORY_SIZE       (uint32_t)0x800000


//////////////////////
// Graphics resolution
#define guiSCREEN_WIDTH 240
#define guiSCREEN_HEIGHT 320

#define guiCOLOR_DEPTH 24

///////////////////////
// Graphics display RAM
#define  guiLCD_FRAME_BUFFER   halSDRAM_START_ADDRESS    // Start of the SDRAM


///////////////////
// resource address
typedef int sysResourceAddress;

/*****************************************************************************/
/* Pin definitions                                                           */
/*****************************************************************************/

//////////////////////
// LCD pin definitions
#define halLCD_RDX_PIN							GPIO_PIN_12
#define halLCD_RDX_PORT							GPIOD
#define halLCD_WRX_PIN              GPIO_PIN_13
#define halLCD_WRX_PORT          		GPIOD
#define halLCD_NCS_PIN              GPIO_PIN_2
#define halLCD_NCS_PORT          		GPIOC

#define halLCD_SPI                  SPI5
#define halLCD_SPI_GPIO_CLK_ENABLE()	__SPI5_CLK_ENABLE()
#define halLCD_SPI_AF								GPIO_AF5_SPI5
#define halLCD_SPI_PORT							GPIOF

#define halLCD_SPI_SCK_PIN					GPIO_PIN_7				// SCK
#define halLCD_SPI_MISO_PIN         GPIO_PIN_8				// MISO
#define halLCD_SPI_MOSI_PIN         GPIO_PIN_9				// MOSI


// LED pins
#define halLD3_PIN									GPIO_PIN_13
#define halLD3_PORT									GPIOG
#define halLD4_PIN									GPIO_PIN_14
#define halLD4_PORT									GPIOG

// SDRAM pins
#define A0_PIN GPIO_PIN_0
#define A0_GPIO_PORT GPIOF
#define A1_PIN GPIO_PIN_1
#define A1_GPIO_PORT GPIOF
#define A2_PIN GPIO_PIN_2
#define A2_GPIO_PORT GPIOF
#define A3_PIN GPIO_PIN_3
#define A3_GPIO_PORT GPIOF
#define A4_PIN GPIO_PIN_4
#define A4_GPIO_PORT GPIOF
#define A5_PIN GPIO_PIN_5
#define A5_GPIO_PORT GPIOF
#define A6_PIN GPIO_PIN_12
#define A6_GPIO_PORT GPIOF
#define A7_PIN GPIO_PIN_13
#define A7_GPIO_PORT GPIOF
#define A8_PIN GPIO_PIN_14
#define A8_GPIO_PORT GPIOF
#define A9_PIN GPIO_PIN_15
#define A9_GPIO_PORT GPIOF
#define A10_PIN GPIO_PIN_0
#define A10_GPIO_PORT GPIOG
#define A11_PIN GPIO_PIN_1
#define A11_GPIO_PORT GPIOG

#define D0_PIN GPIO_PIN_14
#define D0_GPIO_PORT GPIOD
#define D1_PIN GPIO_PIN_15
#define D1_GPIO_PORT GPIOD
#define D2_PIN GPIO_PIN_0
#define D2_GPIO_PORT GPIOD
#define D3_PIN GPIO_PIN_1
#define D3_GPIO_PORT GPIOD
#define D4_PIN GPIO_PIN_7
#define D4_GPIO_PORT GPIOE
#define D5_PIN GPIO_PIN_8
#define D5_GPIO_PORT GPIOE
#define D6_PIN GPIO_PIN_9
#define D6_GPIO_PORT GPIOE
#define D7_PIN GPIO_PIN_10
#define D7_GPIO_PORT GPIOE
#define D8_PIN GPIO_PIN_11
#define D8_GPIO_PORT GPIOE
#define D9_PIN GPIO_PIN_12
#define D9_GPIO_PORT GPIOE
#define D10_PIN GPIO_PIN_13
#define D10_GPIO_PORT GPIOE
#define D11_PIN GPIO_PIN_14
#define D11_GPIO_PORT GPIOE
#define D12_PIN GPIO_PIN_15
#define D12_GPIO_PORT GPIOE
#define D13_PIN GPIO_PIN_8
#define D13_GPIO_PORT GPIOD
#define D14_PIN GPIO_PIN_9
#define D14_GPIO_PORT GPIOD
#define D15_PIN GPIO_PIN_10
#define D15_GPIO_PORT GPIOD

#define SDNRAS_PIN GPIO_PIN_11
#define SDNRAS_GPIO_PORT GPIOF

#define SDNWE_PIN GPIO_PIN_0
#define SDNWE_GPIO_PORT GPIOC

#define SDNCAS_PIN GPIO_PIN_15
#define SDNCAS_GPIO_PORT GPIOG

#define SDCLK_PIN GPIO_PIN_8
#define SDCLK_GPIO_PORT GPIOG

#define SDCKE1_PIN GPIO_PIN_5
#define SDCKE1_GPIO_PORT GPIOB

#define SDNE1_PIN GPIO_PIN_6
#define SDNE1_GPIO_PORT GPIOB

#define BA0_PIN GPIO_PIN_4
#define BA0_GPIO_PORT GPIOG
#define BA1_PIN GPIO_PIN_5
#define BA1_GPIO_PORT GPIOG

#define NBL0_PIN GPIO_PIN_0
#define NBL0_GPIO_PORT GPIOE
#define NBL1_PIN GPIO_PIN_1
#define NBL1_GPIO_PORT GPIOE

// other pin definitions
#if 0
#define PC14_OSC32_IN_PIN GPIO_PIN_14
#define PC14_OSC32_IN_GPIO_PORT GPIOC
#define PC15_OSC32_OUT_PIN GPIO_PIN_15
#define PC15_OSC32_OUT_GPIO_PORT GPIOC
#define ENABLE_PIN GPIO_PIN_10
#define ENABLE_GPIO_PORT GPIOF
#define PH0_OSC_IN_PIN GPIO_PIN_0
#define PH0_OSC_IN_GPIO_PORT GPIOH
#define PH1_OSC_OUT_PIN GPIO_PIN_1
#define PH1_OSC_OUT_GPIO_PORT GPIOH
#define NCS_MEMS_SPI_PIN GPIO_PIN_1
#define NCS_MEMS_SPI_GPIO_PORT GPIOC
#define CSX_PIN GPIO_PIN_2
#define CSX_GPIO_PORT GPIOC
#define B1_PIN GPIO_PIN_0
#define B1_GPIO_PORT GPIOA
#define MEMS_INT1_PIN GPIO_PIN_1
#define MEMS_INT1_GPIO_PORT GPIOA
#define MEMS_INT2_PIN GPIO_PIN_2
#define MEMS_INT2_GPIO_PORT GPIOA
#define B5_PIN GPIO_PIN_3
#define B5_GPIO_PORT GPIOA
#define VSYNC_PIN GPIO_PIN_4
#define VSYNC_GPIO_PORT GPIOA
#define G2_PIN GPIO_PIN_6
#define G2_GPIO_PORT GPIOA
#define ACP_RST_PIN GPIO_PIN_7
#define ACP_RST_GPIO_PORT GPIOA
#define OTG_FS_PSO_PIN GPIO_PIN_4
#define OTG_FS_PSO_GPIO_PORT GPIOC
#define OTG_FS_OC_PIN GPIO_PIN_5
#define OTG_FS_OC_GPIO_PORT GPIOC
#define R3_PIN GPIO_PIN_0
#define R3_GPIO_PORT GPIOB
#define R6_PIN GPIO_PIN_1
#define R6_GPIO_PORT GPIOB
#define BOOT1_PIN GPIO_PIN_2
#define BOOT1_GPIO_PORT GPIOB
#define G4_PIN GPIO_PIN_10
#define G4_GPIO_PORT GPIOB
#define G5_PIN GPIO_PIN_11
#define G5_GPIO_PORT GPIOB
#define OTG_FS_ID_PIN GPIO_PIN_12
#define OTG_FS_ID_GPIO_PORT GPIOB
#define VBUS_FS_PIN GPIO_PIN_13
#define VBUS_FS_GPIO_PORT GPIOB
#define OTG_FS_DM_PIN GPIO_PIN_14
#define OTG_FS_DM_GPIO_PORT GPIOB
#define OTG_FS_DP_PIN GPIO_PIN_15
#define OTG_FS_DP_GPIO_PORT GPIOB
#define TE_PIN GPIO_PIN_11
#define TE_GPIO_PORT GPIOD
#define RDX_PIN GPIO_PIN_12
#define RDX_GPIO_PORT GPIOD
#define WRX_DCX_PIN GPIO_PIN_13
#define WRX_DCX_GPIO_PORT GPIOD
#define R7_PIN GPIO_PIN_6
#define R7_GPIO_PORT GPIOG
#define DOTCLK_PIN GPIO_PIN_7
#define DOTCLK_GPIO_PORT GPIOG
#define HSYNC_PIN GPIO_PIN_6
#define HSYNC_GPIO_PORT GPIOC
#define G6_PIN GPIO_PIN_7
#define G6_GPIO_PORT GPIOC
#define R4_PIN GPIO_PIN_11
#define R4_GPIO_PORT GPIOA
#define R5_PIN GPIO_PIN_12
#define R5_GPIO_PORT GPIOA
#define SWDIO_PIN GPIO_PIN_13
#define SWDIO_GPIO_PORT GPIOA
#define SWCLK_PIN GPIO_PIN_14
#define SWCLK_GPIO_PORT GPIOA
#define TP_INT1_PIN GPIO_PIN_15
#define TP_INT1_GPIO_PORT GPIOA
#define R2_PIN GPIO_PIN_10
#define R2_GPIO_PORT GPIOC
#define G7_PIN GPIO_PIN_3
#define G7_GPIO_PORT GPIOD
#define B2_PIN GPIO_PIN_6
#define B2_GPIO_PORT GPIOD
#define G3_PIN GPIO_PIN_10
#define G3_GPIO_PORT GPIOG
#define B3_PIN GPIO_PIN_11
#define B3_GPIO_PORT GPIOG
#define B4_PIN GPIO_PIN_12
#define B4_GPIO_PORT GPIOG
#define B6_PIN GPIO_PIN_8
#define B6_GPIO_PORT GPIOB
#define B7_PIN GPIO_PIN_9
#define B7_GPIO_PORT GPIOB
#endif

#endif
