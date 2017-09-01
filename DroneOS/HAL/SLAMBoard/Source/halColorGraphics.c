/*****************************************************************************/
/* Graphics LCD HAL functions for STM32F429 Discovery board                  */
/*                                                                           */
/* Copyright (C) 2014 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <stm32f4xx_hal.h>
#include <halColorGraphics.h>
#include <halIODefinitions.h>
#include <halILI9341.h>

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/
void* g_gui_screen_pixels = guiLCD_FRAME_BUFFER;
int g_gui_screen_line_size = 2 * guiSCREEN_WIDTH;

/*****************************************************************************/
/* Local functions                                                           */
/*****************************************************************************/
static void halColorGraphicsLCDLayerDefaultInit(uint16_t in_layer_index, void* in_address);

/*****************************************************************************/
/* Module local variables                                                    */
/*****************************************************************************/
static LTDC_HandleTypeDef  l_ltdc;

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief  Configures the SPI interface for the LCD
/// @note This function is not dealing with GPIO Clock. It must be enabled in the general init section.
void halColorGraphicsInitialize(void)
{
	RCC_PeriphCLKInitTypeDef  periph_clk_init_struct;
	GPIO_InitTypeDef gpio_init_struct;

  /* GPIOs Configuration */
  /*
   +------------------------+-----------------------+----------------------------+
   +                       LCD pins assignment                                   +
   +------------------------+-----------------------+----------------------------+
   |  LCD_TFT R2 <-> PC.10  |  LCD_TFT G2 <-> PA.06 |  LCD_TFT B2 <-> PD.06      |
   |  LCD_TFT R3 <-> PB.00  |  LCD_TFT G3 <-> PG.10 |  LCD_TFT B3 <-> PG.11      |
   |  LCD_TFT R4 <-> PA.11  |  LCD_TFT G4 <-> PB.10 |  LCD_TFT B4 <-> PG.12      |
   |  LCD_TFT R5 <-> PA.12  |  LCD_TFT G5 <-> PB.11 |  LCD_TFT B5 <-> PA.03      |
   |  LCD_TFT R6 <-> PB.01  |  LCD_TFT G6 <-> PC.07 |  LCD_TFT B6 <-> PB.08      |
   |  LCD_TFT R7 <-> PG.06  |  LCD_TFT G7 <-> PD.03 |  LCD_TFT B7 <-> PB.09      |
   -------------------------------------------------------------------------------
            |  LCD_TFT HSYNC <-> PC.06  | LCDTFT VSYNC <->  PA.04 |
            |  LCD_TFT CLK   <-> PG.07  | LCD_TFT DE   <->  PF.10 |
             -----------------------------------------------------
  */

  /* GPIOA configuration */
  gpio_init_struct.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_6 |
                           GPIO_PIN_11 | GPIO_PIN_12;
  gpio_init_struct.Mode = GPIO_MODE_AF_PP;
  gpio_init_struct.Pull = GPIO_NOPULL;
  gpio_init_struct.Speed = GPIO_SPEED_FAST;
  gpio_init_struct.Alternate= GPIO_AF14_LTDC;
  HAL_GPIO_Init(GPIOA, &gpio_init_struct);

 /* GPIOB configuration */
  gpio_init_struct.Pin = GPIO_PIN_8 | \
                           GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11;
  HAL_GPIO_Init(GPIOB, &gpio_init_struct);

 /* GPIOC configuration */
  gpio_init_struct.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_10;
  HAL_GPIO_Init(GPIOC, &gpio_init_struct);

 /* GPIOD configuration */
  gpio_init_struct.Pin = GPIO_PIN_3 | GPIO_PIN_6;
  HAL_GPIO_Init(GPIOD, &gpio_init_struct);

 /* GPIOF configuration */
  gpio_init_struct.Pin = GPIO_PIN_10;
  HAL_GPIO_Init(GPIOF, &gpio_init_struct);

 /* GPIOG configuration */
  gpio_init_struct.Pin = GPIO_PIN_6 | GPIO_PIN_7 | \
                           GPIO_PIN_11;
  HAL_GPIO_Init(GPIOG, &gpio_init_struct);

  /* GPIOB configuration */
  gpio_init_struct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
  gpio_init_struct.Alternate= GPIO_AF9_LTDC;
  HAL_GPIO_Init(GPIOB, &gpio_init_struct);

  /* GPIOG configuration */
  gpio_init_struct.Pin = GPIO_PIN_10 | GPIO_PIN_12;
  HAL_GPIO_Init(GPIOG, &gpio_init_struct);

	// Enable LTDC clock
	__LTDC_CLK_ENABLE();

	// LTDC Configuration
	l_ltdc.Instance = LTDC;

	/* Timing configuration  (Typical configuration from ILI9341 datasheet)
				HSYNC=10 (9+1)
				HBP=20 (29-10+1)
				ActiveW=240 (269-20-10+1)
				HFP=10 (279-240-20-10+1)

				VSYNC=2 (1+1)
				VBP=2 (3-2+1)
				ActiveH=320 (323-2-2+1)
				VFP=4 (327-320-2-2+1)
		*/

	l_ltdc.Init.HorizontalSync	= drvILI9341_HSYNC;	// Configure horizontal synchronization width
	l_ltdc.Init.VerticalSync		= drvILI9341_VSYNC;		// Configure vertical synchronization height
	l_ltdc.Init.AccumulatedHBP	= drvILI9341_HBP;		// Configure accumulated horizontal back porch
	l_ltdc.Init.AccumulatedVBP 	= drvILI9341_VBP;		// Configure accumulated vertical back porch
	l_ltdc.Init.AccumulatedActiveW = 269;				// Configure accumulated active width
	l_ltdc.Init.AccumulatedActiveH = 323;				// Configure accumulated active height
	l_ltdc.Init.TotalWidth = 279;								// Configure total width
	l_ltdc.Init.TotalHeigh = 327;								// Configure total height

	// Configure R,G,B component values for LCD background color
	l_ltdc.Init.Backcolor.Red = 0;
	l_ltdc.Init.Backcolor.Blue = 0;
	l_ltdc.Init.Backcolor.Green = 0;

	/* LCD clock configuration */
	/* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 Mhz */
	/* PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAIN = 192 Mhz */
	/* PLLLCDCLK = PLLSAI_VCO Output/PLLSAIR = 192/4 = 48 Mhz */
	/* LTDC clock frequency = PLLLCDCLK / LTDC_PLLSAI_DIVR_8 = 48/4 = 6Mhz */
	periph_clk_init_struct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
	periph_clk_init_struct.PLLSAI.PLLSAIN = 192;
	periph_clk_init_struct.PLLSAI.PLLSAIR = 4;
	periph_clk_init_struct.PLLSAIDivR = RCC_PLLSAIDIVR_8;
	HAL_RCCEx_PeriphCLKConfig(&periph_clk_init_struct);

	// Polarity
	l_ltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
	l_ltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
	l_ltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
	l_ltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;

	HAL_LTDC_Init(&l_ltdc);

	// Layer2 Init
	halColorGraphicsLCDLayerDefaultInit(1, guiLCD_FRAME_BUFFER + g_gui_screen_line_size * guiSCREEN_HEIGHT);
	__HAL_LTDC_LAYER_DISABLE(&l_ltdc, 1);

	//Layer1 Init
	halColorGraphicsLCDLayerDefaultInit(0, guiLCD_FRAME_BUFFER);
	__HAL_LTDC_LAYER_ENABLE(&l_ltdc, 0);

	#if 0
  // Configure LCD SPI control bus
  132   BSP_LCD_SelectLayer(1);
  133   BSP_LCD_Clear(LCD_COLOR_WHITE);
  134   BSP_LCD_SetColorKeying(1, LCD_COLOR_WHITE);
  135   BSP_LCD_SetLayerVisible(1, DISABLE);
  136
  139   BSP_LCD_SelectLayer(0);
  140   BSP_LCD_DisplayOn();
  141   BSP_LCD_Clear(LCD_COLOR_BLACK);
 #endif

  // dithering activation
  HAL_LTDC_EnableDither(&l_ltdc);

  // reload config
	__HAL_LTDC_RELOAD_CONFIG(&l_ltdc);
}


void halColorGraphicsCleanup(void)
{
	// TODO
}

void drvColorGraphicsRefreshScreen(void)
{

}



static void halColorGraphicsLCDLayerDefaultInit(uint16_t in_layer_index, void* in_address)
{
	LTDC_LayerCfgTypeDef layer_cfg;

  // Layer Init
  layer_cfg.WindowX0 = 0;
  layer_cfg.WindowX1 = guiSCREEN_WIDTH;
  layer_cfg.WindowY0 = 0;
  layer_cfg.WindowY1 = guiSCREEN_HEIGHT;
  layer_cfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
  layer_cfg.FBStartAdress = (int32_t)in_address;
  layer_cfg.Alpha = 255;
  layer_cfg.Alpha0 = 0;
  layer_cfg.Backcolor.Blue = 0;
  layer_cfg.Backcolor.Green = 0;
  layer_cfg.Backcolor.Red = 0;
  layer_cfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
  layer_cfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
  layer_cfg.ImageWidth = guiSCREEN_WIDTH;
  layer_cfg.ImageHeight = guiSCREEN_HEIGHT;

  HAL_LTDC_ConfigLayer(&l_ltdc, &layer_cfg, in_layer_index);
}

