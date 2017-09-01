/*****************************************************************************/
/* ILI9341 LCD controller driver for VSYNC/HSYNC interface                   */
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
#include <halLCDSPI.h>
#include <halILI9341.h>

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/
static bool l_initialized = false;

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes ILI9341 LCD driver chip
void halGraphicsDisplayInitialize(void)
{
	if (!l_initialized)
	{
		// Initialize ILI9341 SPI bus
		halLCDSPIinitialize();

		// Configure LCD
		halLCDSPIWriteReg(0xCA);
		halLCDSPIWriteData(0xC3);
		halLCDSPIWriteData(0x08);
		halLCDSPIWriteData(0x50);
		halLCDSPIWriteReg(drvILI9341_POWERB);
		halLCDSPIWriteData(0x00);
		halLCDSPIWriteData(0xC1);
		halLCDSPIWriteData(0x30);
		halLCDSPIWriteReg(drvILI9341_POWER_SEQ);
		halLCDSPIWriteData(0x64);
		halLCDSPIWriteData(0x03);
		halLCDSPIWriteData(0x12);
		halLCDSPIWriteData(0x81);
		halLCDSPIWriteReg(drvILI9341_DTCA);
		halLCDSPIWriteData(0x85);
		halLCDSPIWriteData(0x00);
		halLCDSPIWriteData(0x78);
		halLCDSPIWriteReg(drvILI9341_POWERA);
		halLCDSPIWriteData(0x39);
		halLCDSPIWriteData(0x2C);
		halLCDSPIWriteData(0x00);
		halLCDSPIWriteData(0x34);
		halLCDSPIWriteData(0x02);
		halLCDSPIWriteReg(drvILI9341_PRC);
		halLCDSPIWriteData(0x20);
		halLCDSPIWriteReg(drvILI9341_DTCB);
		halLCDSPIWriteData(0x00);
		halLCDSPIWriteData(0x00);
		halLCDSPIWriteReg(drvILI9341_FRMCTR1);
		halLCDSPIWriteData(0x00);
		halLCDSPIWriteData(0x1B);
		halLCDSPIWriteReg(drvILI9341_DFC);
		halLCDSPIWriteData(0x0A);
		halLCDSPIWriteData(0xA2);
		halLCDSPIWriteReg(drvILI9341_POWER1);
		halLCDSPIWriteData(0x10);
		halLCDSPIWriteReg(drvILI9341_POWER2);
		halLCDSPIWriteData(0x10);
		halLCDSPIWriteReg(drvILI9341_VCOM1);
		halLCDSPIWriteData(0x45);
		halLCDSPIWriteData(0x15);
		halLCDSPIWriteReg(drvILI9341_VCOM2);
		halLCDSPIWriteData(0x90);
		halLCDSPIWriteReg(drvILI9341_MAC);
		halLCDSPIWriteData(0xC8);
		halLCDSPIWriteReg(drvILI9341_3GAMMA_EN);
		halLCDSPIWriteData(0x00);
		halLCDSPIWriteReg(drvILI9341_RGB_INTERFACE);
		halLCDSPIWriteData(0xC2);
		halLCDSPIWriteReg(drvILI9341_DFC);
		halLCDSPIWriteData(0x0A);
		halLCDSPIWriteData(0xA7);
		halLCDSPIWriteData(0x27);
		halLCDSPIWriteData(0x04);

		// Column address set
		halLCDSPIWriteReg(drvILI9341_COLUMN_ADDR);
		halLCDSPIWriteData(0x00);
		halLCDSPIWriteData(0x00);
		halLCDSPIWriteData(0x00);
		halLCDSPIWriteData(0xEF);

		// Page Address Set
		halLCDSPIWriteReg(drvILI9341_PAGE_ADDR);
		halLCDSPIWriteData(0x00);
		halLCDSPIWriteData(0x00);
		halLCDSPIWriteData(0x01);
		halLCDSPIWriteData(0x3F);
		halLCDSPIWriteReg(drvILI9341_INTERFACE);
		halLCDSPIWriteData(0x01);
		halLCDSPIWriteData(0x00);
		halLCDSPIWriteData(0x06);

		halLCDSPIWriteReg(drvILI9341_GRAM);
		halLCDSPIDelay(200);

		halLCDSPIWriteReg(drvILI9341_GAMMA);
		halLCDSPIWriteData(0x01);

		halLCDSPIWriteReg(drvILI9341_PGAMMA);
		halLCDSPIWriteData(0x0F);
		halLCDSPIWriteData(0x29);
		halLCDSPIWriteData(0x24);
		halLCDSPIWriteData(0x0C);
		halLCDSPIWriteData(0x0E);
		halLCDSPIWriteData(0x09);
		halLCDSPIWriteData(0x4E);
		halLCDSPIWriteData(0x78);
		halLCDSPIWriteData(0x3C);
		halLCDSPIWriteData(0x09);
		halLCDSPIWriteData(0x13);
		halLCDSPIWriteData(0x05);
		halLCDSPIWriteData(0x17);
		halLCDSPIWriteData(0x11);
		halLCDSPIWriteData(0x00);
		halLCDSPIWriteReg(drvILI9341_NGAMMA);
		halLCDSPIWriteData(0x00);
		halLCDSPIWriteData(0x16);
		halLCDSPIWriteData(0x1B);
		halLCDSPIWriteData(0x04);
		halLCDSPIWriteData(0x11);
		halLCDSPIWriteData(0x07);
		halLCDSPIWriteData(0x31);
		halLCDSPIWriteData(0x33);
		halLCDSPIWriteData(0x42);
		halLCDSPIWriteData(0x05);
		halLCDSPIWriteData(0x0C);
		halLCDSPIWriteData(0x0A);
		halLCDSPIWriteData(0x28);
		halLCDSPIWriteData(0x2F);
		halLCDSPIWriteData(0x0F);

		halLCDSPIWriteReg(drvILI9341_SLEEP_OUT);
		halLCDSPIDelay(200);
		halLCDSPIWriteReg(drvILI9341_DISPLAY_ON);

		// GRAM start writing
		halLCDSPIWriteReg(drvILI9341_GRAM);

		l_initialized = true;
	}
}
