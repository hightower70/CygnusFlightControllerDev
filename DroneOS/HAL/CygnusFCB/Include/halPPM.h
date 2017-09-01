/*****************************************************************************/
/* HAL layer for Pulse Position Modulation (R/C receiver) interface          */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __halPPM_h
#define __halPPM_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void halPPMInit(void);

void halPPMStartDelay(uint16_t in_delay_in_us);
extern void halPPMDelayExpiredCallback(void* in_interrupt_param);
extern void halPPMPulseReceivedCallback(uint16_t in_pulse_pos, void* in_interrupt_param);

#endif
