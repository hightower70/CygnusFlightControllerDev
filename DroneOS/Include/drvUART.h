/*****************************************************************************/
/* UART Driver                                                               */
/*                                                                           */
/* Copyright (C) 2014 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the BSD license.  See the LICENSE file for details.                    */
/*****************************************************************************/
#ifndef __drvUART_h
#define __drvUART_h

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

} drvUARTConfigInfo;


/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void drvUARTInit(uint8_t in_uart_index);
void drvUARTConfig(uint8_t in_uart_index, drvUARTConfigInfo* in_config_info);
void drvUARTConfigInfoInit(drvUARTConfigInfo* in_config_info);
bool drvUARTSetBaudRate(uint8_t in_uart_index, uint32_t in_baud_rate);
void drvUARTSendBlock(uint8_t in_uart_index, uint8_t* in_buffer, uint16_t in_buffer_length);

#endif
