/*****************************************************************************/
/* RTOS Independent Type Definitions                                         */
/*                                                                           */
/* Copyright (C) 2014 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/
#ifndef __RTOSTypes_h
#define __RTOSTypes_h

///////////////////////////////////////////////////////////////////////////////
// Type definitions
#include <stdint.h>

#ifdef _WIN32
#pragma once

#define false   0
#define true    1

#define bool int
#else
#include <stdbool.h>
#endif

/* string types */
typedef uint16_t sysStringLength;
typedef char sysChar;
typedef sysChar*	sysString;
typedef const sysChar* sysConstString;

///////////////////////////////////////////////////////////////////////////////
// Macros
#define sysLOW(x) ((x)&0xff)
#define sysHIGH(x) ((x)>>8)
#define sysBV(x) (1<<(x))

#define sysSTRINGIZE(x) ___sysSTRINGIZE(x)
#define ___sysSTRINGIZE(x) #x

#define sysNULL 0

#define sysUNUSED(x) (void)(x)

#endif
