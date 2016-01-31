/*****************************************************************************/
/* Main Entry Function for Win32 application                                 */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <sysRTOS.h>
#include <sysInitialize.h>
#include <stdio.h>
#include <conio.h>

///////////////////////////////////////////////////////////////////////////////
// Global variables



///////////////////////////////////////////////////////////////////////////////
// Main entry function
int main(void)
{
	// initialize system
	sysInitialize();
	sysCreateTasks();

	printf("Cygnus system is initialized.\n");

	while (_getch() != 27)
	{
	}


	// wait for threads to exit
	Sleep(100);

	// shutdown system
	sysShutdown();

	return 0;
}

