/*****************************************************************************/
/* Main Entry Function for STM32 application                                 */
/*                                                                           */
/* Copyright (C) 2015 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysRTOS.h>
#include <sysInitialize.h>

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/


///////////////////////////////////////////////////////////////////////////////
/// @brief Main entry function
int main(void)
{
	// initialize system
	sysInitialize();

	// Create tasks
	sysCreateTasks();

  // start
  sysStartSheduler();

  return 0;
}
