/*****************************************************************************/
/* HAL helper functions                                                      */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/
#ifndef __sysInitialize_h
#define __sysInitialize_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>

///////////////////////////////////////////////////////////////////////////////
// Global variables
#ifdef _WIN32
#include <Windows.h>

extern bool g_system_is_running;
extern HANDLE g_system_is_running_event;
#endif

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
void sysCreateTasks(void);
void sysInitialize(void);
void sysShutdown(void);

#endif
