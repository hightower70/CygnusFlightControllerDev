/*****************************************************************************/
/* Main Entry Function for Win32 application                                 */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <windows.h>
#include <sysRTOS.h>
#include <sysInitialize.h>
#include <halHelpers.h>
#include <guiColorGraphics.h>


/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/

/*****************************************************************************/
/* Module local variables                                                    */
/*****************************************************************************/

extern void guiGraphicsDisplayInit(void);
extern void sysMainTask(void);


///////////////////////////////////////////////////////////////////////////////
/// @brief Win32 windowed application main entry function
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, INT nCmdShow)
{
	MSG msg;
	BOOL cont;

	// initialize system
	guiColorGraphicsInitialize();
	halInitialize();
	sysInitialize();
	sysCreateTasks();

	// Main message loop:
	cont = TRUE;
	do
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			cont = GetMessage(&msg, NULL, 0, 0);
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		sysMainTask();
		Sleep(0);
	} while (cont);

	// shutdown system
	sysShutdown();

	// cleanup
	guiColorGraphicsCleanup();
	halDeinitialize();

	return 0;
}
