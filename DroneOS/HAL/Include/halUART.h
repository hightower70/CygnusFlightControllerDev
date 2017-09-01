/*****************************************************************************/
/* UART HAL Driver                                                           */
/*                                                                           */
/* Copyright (C) 2014 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/
#ifndef __halUART_h
#define __halUART_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/
typedef void(*drvUARTRxReceivedCallback)(uint8_t in_byte, void* in_interrupt_param);
typedef void(*drvUARTTxEmptyCallback)(void* in_interrupt_param);

typedef struct
{
	drvUARTRxReceivedCallback RxReceivedCallback;
	drvUARTTxEmptyCallback TxEmptyCallback;

} halUARTConfigInfo;


/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void halUARTInit(void);
void halUARTConfig(uint8_t in_uart_index, halUARTConfigInfo* in_config_info);
void halUARTConfigInfoInit(halUARTConfigInfo* in_config_info);
bool halUARTSetBaudRate(uint8_t in_uart_index, uint32_t in_baud_rate);
bool halUARTSendBlock(uint8_t in_uart_index, uint8_t* in_buffer, uint16_t in_buffer_length);

#endif
