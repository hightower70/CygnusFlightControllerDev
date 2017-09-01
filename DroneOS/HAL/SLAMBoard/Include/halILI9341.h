/*****************************************************************************/
/* ILI9341 LCD driver on STM32F429 board                                     */
/*                                                                           */
/* Copyright (C) 2014 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __drvILI9341_h
#define __drvILI9341_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/

////////////////
//ILI9341 Timing
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
#define  drvILI9341_HSYNC            ((uint32_t)9)   /* Horizontal synchronization */
#define  drvILI9341_HBP              ((uint32_t)29)    /* Horizontal back porch      */
#define  drvILI9341_HFP              ((uint32_t)2)    /* Horizontal front porch     */
#define  drvILI9341_VSYNC            ((uint32_t)1)   /* Vertical synchronization   */
#define  drvILI9341_VBP              ((uint32_t)3)    /* Vertical back porch        */
#define  drvILI9341_VFP              ((uint32_t)2)    /* Vertical front porch       */


////////////////////
// ILI9341 Registers

// Level 1 Commands
#define drvILI9341_SWRESET             0x01   /* Software Reset */
#define drvILI9341_READ_DISPLAY_ID     0x04   /* Read display identification information */
#define drvILI9341_RDDST               0x09   /* Read Display Status */
#define drvILI9341_RDDPM               0x0A   /* Read Display Power Mode */
#define drvILI9341_RDDMADCTL           0x0B   /* Read Display MADCTL */
#define drvILI9341_RDDCOLMOD           0x0C   /* Read Display Pixel Format */
#define drvILI9341_RDDIM               0x0D   /* Read Display Image Format */
#define drvILI9341_RDDSM               0x0E   /* Read Display Signal Mode */
#define drvILI9341_RDDSDR              0x0F   /* Read Display Self-Diagnostic Result */
#define drvILI9341_SPLIN               0x10   /* Enter Sleep Mode */
#define drvILI9341_SLEEP_OUT           0x11   /* Sleep out register */
#define drvILI9341_PTLON               0x12   /* Partial Mode ON */
#define drvILI9341_NORMAL_MODE_ON      0x13   /* Normal Display Mode ON */
#define drvILI9341_DINVOFF             0x20   /* Display Inversion OFF */
#define drvILI9341_DINVON              0x21   /* Display Inversion ON */
#define drvILI9341_GAMMA               0x26   /* Gamma register */
#define drvILI9341_DISPLAY_OFF         0x28   /* Display off register */
#define drvILI9341_DISPLAY_ON          0x29   /* Display on register */
#define drvILI9341_COLUMN_ADDR         0x2A   /* Colomn address register */
#define drvILI9341_PAGE_ADDR           0x2B   /* Page address register */
#define drvILI9341_GRAM                0x2C   /* GRAM register */
#define drvILI9341_RGBSET              0x2D   /* Color SET */
#define drvILI9341_RAMRD               0x2E   /* Memory Read */
#define drvILI9341_PLTAR               0x30   /* Partial Area */
#define drvILI9341_VSCRDEF             0x33   /* Vertical Scrolling Definition */
#define drvILI9341_TEOFF               0x34   /* Tearing Effect Line OFF */
#define drvILI9341_TEON                0x35   /* Tearing Effect Line ON */
#define drvILI9341_MAC                 0x36   /* Memory Access Control register*/
#define drvILI9341_VSCRSADD            0x37   /* Vertical Scrolling Start Address */
#define drvILI9341_IDMOFF              0x38   /* Idle Mode OFF */
#define drvILI9341_IDMON               0x39   /* Idle Mode ON */
#define drvILI9341_PIXEL_FORMAT        0x3A   /* Pixel Format register */
#define drvILI9341_WRITE_MEM_CONTINUE  0x3C   /* Write Memory Continue */
#define drvILI9341_READ_MEM_CONTINUE   0x3E   /* Read Memory Continue */
#define drvILI9341_SET_TEAR_SCANLINE   0x44   /* Set Tear Scanline */
#define drvILI9341_GET_SCANLINE        0x45   /* Get Scanline */
#define drvILI9341_WDB                 0x51   /* Write Brightness Display register */
#define drvILI9341_RDDISBV             0x52   /* Read Display Brightness */
#define drvILI9341_WCD                 0x53   /* Write Control Display register*/
#define drvILI9341_RDCTRLD             0x54   /* Read CTRL Display */
#define drvILI9341_WRCABC              0x55   /* Write Content Adaptive Brightness Control */
#define drvILI9341_RDCABC              0x56   /* Read Content Adaptive Brightness Control */
#define drvILI9341_WRITE_CABC          0x5E   /* Write CABC Minimum Brightness */
#define drvILI9341_READ_CABC           0x5F   /* Read CABC Minimum Brightness */
#define drvILI9341_READ_ID1            0xDA   /* Read ID1 */
#define drvILI9341_READ_ID2            0xDB   /* Read ID2 */
#define drvILI9341_READ_ID3            0xDC   /* Read ID3 */

// Level 2 Commands
#define drvILI9341_RGB_INTERFACE       0xB0   /* RGB Interface Signal Control */
#define drvILI9341_FRMCTR1             0xB1   /* Frame Rate Control (In Normal Mode) */
#define drvILI9341_FRMCTR2             0xB2   /* Frame Rate Control (In Idle Mode) */
#define drvILI9341_FRMCTR3             0xB3   /* Frame Rate Control (In Partial Mode) */
#define drvILI9341_INVTR               0xB4   /* Display Inversion Control */
#define drvILI9341_BPC                 0xB5   /* Blanking Porch Control register*/
#define drvILI9341_DFC                 0xB6   /* Display Function Control register*/
#define drvILI9341_ETMOD               0xB7   /* Entry Mode Set */
#define drvILI9341_BACKLIGHT1          0xB8   /* Backlight Control 1 */
#define drvILI9341_BACKLIGHT2          0xB9   /* Backlight Control 2 */
#define drvILI9341_BACKLIGHT3          0xBA   /* Backlight Control 3 */
#define drvILI9341_BACKLIGHT4          0xBB   /* Backlight Control 4 */
#define drvILI9341_BACKLIGHT5          0xBC   /* Backlight Control 5 */
#define drvILI9341_BACKLIGHT7          0xBE   /* Backlight Control 7 */
#define drvILI9341_BACKLIGHT8          0xBF   /* Backlight Control 8 */
#define drvILI9341_POWER1              0xC0   /* Power Control 1 register */
#define drvILI9341_POWER2              0xC1   /* Power Control 2 register */
#define drvILI9341_VCOM1               0xC5   /* VCOM Control 1 register */
#define drvILI9341_VCOM2               0xC7   /* VCOM Control 2 register */
#define drvILI9341_NVMWR               0xD0   /* NV Memory Write */
#define drvILI9341_NVMPKEY             0xD1   /* NV Memory Protection Key */
#define drvILI9341_RDNVM               0xD2   /* NV Memory Status Read */
#define drvILI9341_READ_ID4            0xD3   /* Read ID4 */
#define drvILI9341_PGAMMA              0xE0   /* Positive Gamma Correction register*/
#define drvILI9341_NGAMMA              0xE1   /* Negative Gamma Correction register*/
#define drvILI9341_DGAMCTRL1           0xE2   /* Digital Gamma Control 1 */
#define drvILI9341_DGAMCTRL2           0xE3   /* Digital Gamma Control 2 */
#define drvILI9341_INTERFACE           0xF6   /* Interface control register */

// Extend register commands
#define drvILI9341_POWERA               0xCB   /* Power control A register */
#define drvILI9341_POWERB               0xCF   /* Power control B register */
#define drvILI9341_DTCA                 0xE8   /* Driver timing control A */
#define drvILI9341_DTCB                 0xEA   /* Driver timing control B */
#define drvILI9341_POWER_SEQ            0xED   /* Power on sequence register */
#define drvILI9341_3GAMMA_EN            0xF2   /* 3 Gamma enable register */
#define drvILI9341_PRC                  0xF7   /* Pump ratio control register */

// Size of read registers
#define drvILI9341_READ_ID4_SIZE        3      /* Size of Read ID4 */


/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void drvILI9341Initialize(void);


#endif
