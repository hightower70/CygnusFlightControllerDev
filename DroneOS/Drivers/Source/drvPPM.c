/*****************************************************************************/
/* Driver for Pulse Position Modulation (R/C receiver) interface             */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <halPPM.h>
#include <sysRTOS.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define drvPPM_TASK_EVENT_TIMEOUT 100
#define drvPPM_MAX_CHANNEL_COUNT 8

#define drvPPM_MIN_GAP_LENGTH 4000	// in us
#define drvPPM_MAX_FRAME_LENGTH 25000 // in us
#define drvPPM_MIN_ACCEPTED_PULSE_LEGTH 900 // in us
#define drvPPM_MAX_ACCEPTED_PULSE_LENGTH 2100 // in us
#define drvPPM_MIN_PULSE_LENGTH 1000 // in us
#define drvPPM_MAX_PULSE_LENGTH 2000 // in us

#define drvPPM_FAILSAFE_TIMEOUT 100 // in ms

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/
typedef enum
{
	drvPPM_ST_WaitForGap,
	drvPPM_ST_WaitForPulse,
	drvPPM_ST_Pulse
} drvPPMState;

/*****************************************************************************/
/* Local functions                                                           */
/*****************************************************************************/
static void drvPPMDeInit(void);
static void drvPPMThread(void* in_param);

/*****************************************************************************/
/* Module global variables                                                   */
/*****************************************************************************/

// thread variables
static sysTaskNotify l_task_event = sysNULL;
static bool l_stop_task = false;

// received buffer
static uint16_t l_received_input[drvPPM_MAX_CHANNEL_COUNT];
static volatile uint8_t l_received_channel_count;
static bool l_failsafe_active;
static sysTick l_last_received_frame_timestamp;

// PPM variables
static drvPPMState l_state;
static uint8_t l_current_channel;
static uint16_t l_pulse_pos;

/*****************************************************************************/
/* Public functions                                                          */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initialize PPM input driver
void drvPPMInit(void)
{
	sysTask task_id;

	sysTaskCreate(drvPPMThread, "drvPPM", sysDEFAULT_STACK_SIZE, sysNULL, 2, &task_id, drvPPMDeInit);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Stops PPM input driver
void drvPPMDeInit(void)
{
	l_stop_task = true;
	sysTaskNotifyGive(l_task_event);
}
/*****************************************************************************/
/* Thread function                                                           */
/*****************************************************************************/
static void drvPPMThread(void* in_param)
{
	sysUNUSED(in_param);

	// HAL init
	halPPMInit();

	// thread variable init
	l_state = drvPPM_ST_WaitForGap;
	halPPMStartDelay(drvPPM_MIN_GAP_LENGTH);
	l_received_channel_count = 0;
	l_last_received_frame_timestamp = 0;

	// task notification init
	sysTaskNotifyCreate(l_task_event);
	sysTaskNotifyGive(l_task_event);

	while (!l_stop_task)
	{
		// wait for event
		sysTaskNotifyTake(l_task_event, drvPPM_TASK_EVENT_TIMEOUT);

		// check if frame is received
		if(l_received_channel_count > 0)
		{
			l_received_channel_count = 0;
			l_last_received_frame_timestamp = sysGetSystemTick();
			l_failsafe_active = false;
		}

		// check if failsafe needs to be applied
		if(sysGetSystemTickSince(l_last_received_frame_timestamp) > drvPPM_FAILSAFE_TIMEOUT && !l_failsafe_active)
		{
			//TODO failsafe
			l_failsafe_active = true;
		}

		if (l_stop_task)
			break;
	}
}


/*****************************************************************************/
/* Callback functions                                                        */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief PPM Delay expired callback function
void halPPMDelayExpiredCallback(void* in_interrupt_param)
{
	switch(l_state)
	{
		// gap detected -> start frame receiveing
		case drvPPM_ST_WaitForGap:
			l_state = drvPPM_ST_WaitForPulse;
			halPPMStartDelay(drvPPM_MAX_FRAME_LENGTH);
			break;

		// still no pulse detected -> continue waiting
		case drvPPM_ST_WaitForPulse:
			halPPMStartDelay(drvPPM_MAX_FRAME_LENGTH);
			break;

		// less than MAX channel received together with the gap
		case drvPPM_ST_Pulse:
			// notify thread about the received frame
			l_received_channel_count = l_current_channel;
			sysTaskNotifyGiveFromISR(l_task_event, in_interrupt_param);

			l_state = drvPPM_ST_WaitForPulse;
			halPPMStartDelay(drvPPM_MIN_GAP_LENGTH);
			break;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// @brief PPM input pulse received callback
/// @param in_pulse_pos Time in us since the last received pulse edge
void halPPMPulseReceivedCallback(uint16_t in_pulse_pos, void* in_interrupt_param)
{
	uint16_t pulse_length;

	switch(l_state)
	{
		// first edge received
		case drvPPM_ST_WaitForPulse:
			l_state = drvPPM_ST_Pulse;
			l_current_channel = 0;
			l_pulse_pos = in_pulse_pos;
			halPPMStartDelay(drvPPM_MIN_GAP_LENGTH);
			break;

		// consecutive edges received
		case drvPPM_ST_Pulse:
			// check pulse length
			pulse_length = in_pulse_pos - l_pulse_pos;

			if( pulse_length >= drvPPM_MIN_ACCEPTED_PULSE_LEGTH && pulse_length <= drvPPM_MAX_ACCEPTED_PULSE_LENGTH)
			{
				// limit pulse length
				if(pulse_length < drvPPM_MIN_PULSE_LENGTH)
					pulse_length = drvPPM_MIN_PULSE_LENGTH;

				if(pulse_length > drvPPM_MAX_PULSE_LENGTH)
					pulse_length = drvPPM_MAX_PULSE_LENGTH;

				// store pulse
				l_received_input[l_current_channel] = pulse_length - drvPPM_MIN_PULSE_LENGTH;

				// prepare for next channel
				l_pulse_pos = in_pulse_pos;
				l_current_channel++;
				if(l_current_channel >= drvPPM_MAX_CHANNEL_COUNT)
				{
					// if all channels are received -> stop channel receiving
					l_state = drvPPM_ST_WaitForGap;
					l_received_channel_count = l_current_channel;

					sysTaskNotifyGiveFromISR(l_task_event, in_interrupt_param);
				}
				halPPMStartDelay(drvPPM_MIN_GAP_LENGTH);
			}
			else
			{
				// invalid pulse length detected -> restart framing
				l_state = drvPPM_ST_WaitForGap;
				halPPMStartDelay(drvPPM_MIN_GAP_LENGTH);
			}
			break;

		// should not happen -> do nothing
		default:
			break;
	}
}
