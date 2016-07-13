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

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/
int g_argc;
char** g_argv;

/*****************************************************************************/
/* Module local variables                                                    */
/*****************************************************************************/
static CRITICAL_SECTION l_critical_section;

///////////////////////////////////////////////////////////////////////////////
// Main entry function
int main(int argc, char* argv[])
{
	g_argc = argc;
	g_argv = argv;

	InitializeCriticalSection(&l_critical_section);

	// initialize system
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

	DeleteCriticalSection(&l_critical_section);

	return 0;
}

void sysCriticalSectionBegin(void)
{
	EnterCriticalSection(&l_critical_section);
}

void sysCriticalSectionEnd(void)
{
	LeaveCriticalSection(&l_critical_section);
}