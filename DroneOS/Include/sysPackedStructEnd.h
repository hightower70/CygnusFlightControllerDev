/*****************************************************************************/
/* Turn-off byte packed structs                                              */
/*                                                                           */
/* Copyright (C) 2014 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

// Microsoft specific
#if defined(_MSC_VER)
#pragma warning( disable : 4103 )
#pragma pack()

// GNU specific
#elif defined(__GNUC__)
#pragma pack(pop)

// unknown compiler
#else
#error Unknown compiler for packed struct.
#endif
