/*****************************************************************************/
/* Main Entry Function for Win32 console application                         */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <sysRTOS.h>
#include <sysInitialize.h>
#include <halHelpers.h>
#include <stdio.h>
#include <conio.h>

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/
int g_argc;
char** g_argv;

/*****************************************************************************/
/* Module local variables                                                    */
/*****************************************************************************/


///////////////////////////////////////////////////////////////////////////////
/// @brief  Win32 console emulation main entry function
int main(int argc, char* argv[])
{
	g_argc = argc;
	g_argv = argv;

	// initialize system
	halInitialize();
	sysInitialize();
	sysCreateTasks();

	printf("Cygnus system is initialized.\n");

	while (_getch() != 27)
	{
	}

	// shutdown system
	sysShutdown();

	// wait for threads to exit
	Sleep(100);

	halDeinitialize();

	return 0;
}
