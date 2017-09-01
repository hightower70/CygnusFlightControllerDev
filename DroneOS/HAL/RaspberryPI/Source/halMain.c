/*****************************************************************************/
/* Main Entry Function for Linux console application                         */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// Includes
#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sysRTOS.h>
#include <sysInitialize.h>
#include <halHelpers.h>

/*****************************************************************************/
/* Global variables                                                          */
/*****************************************************************************/
int g_argc;
char** g_argv;

/*****************************************************************************/
/* Module local variables                                                    */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Waits for key press
int mygetch(void) 
{
	int ch;
	struct termios oldt, newt;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

	return ch;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Linux console emulation main entry function
int main(int argc, char* argv[])
{
	g_argc = argc;
	g_argv = argv;

	// initialize system
	halInitialize();
	sysInitialize();
	//sysCreateTasks();

	printf("Cygnus system is initialized.\n");
	printf("  (Press ESC to exit)\n");

	//while (mygetch() != 27)
	//{
	//	
	//}
	getchar();
	
	// shutdown system
	sysShutdown();
	halDeinitialize();

	return 0;
}
