/*
 * usart.c
 *
 *  Created on: 2014/03/04
 *      Author: masayuki
 */

#include "stm32f4xx_hal_conf.h"
#include "usart.h"
#include "lcd.h"
#include "xmodem.h"
#include "main.h"
#include "settings.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

static void Error_Handler();

volatile debug_printf_typedef debug;

UART_HandleTypeDef UartHandle;

uint8_t recv;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	extern cursor_typedef cursor;
	extern LCDStatusStruct_typedef LCDStatusStruct;

	sleep_time.prevTime = sleep_time.curTime;

	if(sleep_time.flags.dimLight)
	{
		LCDSetPWMValue(settings_group.disp_conf.brightness);
		sleep_time.flags.dimLight = 0;
	}

	if(huart->Instance == USARTx)
	{
//		debug.printf("\r\nrecv:%c", recv);

		if(recv == 'p')
		{
			sleep_time.flags.enable = 0;
			xput();
			sleep_time.flags.enable = 1;

			goto EXIT_UART;
		}

		if(LCDStatusStruct.waitExitKey != 0){
			if(recv == 'e'){
				LCDStatusStruct.waitExitKey = 0;
			} else {
				LCDStatusStruct.waitExitKey = recv;
			}
			goto EXIT_UART;
		}

		switch(recv){
			case CURSOR_LEFT:
				LCDPutCursorBar(cursor.pos);
				LCDStoreCursorBar(0);
				cursor.pos = 0, cursor.pageIdx = 0;
				LCDCursorEnter();
				break;
			case CURSOR_UP:
				LCDCursorUp();
				break;
			case CURSOR_DOWN:
				LCDCursorDown();
				break;
			case CURSOR_ENTER:
				LCDCursorEnter();
				break;
			case CURSOR_RIGHT:
				break;
			default:
				break;
		}

		EXIT_UART:
		if(HAL_UART_Receive_IT(&UartHandle, &recv, 1) != HAL_OK)
		{
			Error_Handler();
		}

	}
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	/*
	while(1)
	{

	};
	*/
}

void USART_Init()
{
	/*##-1- Configure the UART peripheral ######################################*/
	/* Put the USART peripheral in the Asynchronous mode (UART Mode) */
	/* UART1 configured as follow:
	      - Word Length = 8 Bits
	      - Stop Bit = One Stop bit
	      - Parity = None
	      - BaudRate = 9600 baud
	      - Hardware flow control disabled (RTS and CTS signals) */
	UartHandle.Instance        = USARTx;
	UartHandle.Init.BaudRate   = 9600;
	UartHandle.Init.WordLength = UART_WORDLENGTH_8B;
	UartHandle.Init.StopBits   = UART_STOPBITS_1;
	UartHandle.Init.Parity     = UART_PARITY_NONE;
	UartHandle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
	UartHandle.Init.Mode       = UART_MODE_TX_RX;

	if(HAL_UART_Init(&UartHandle) != HAL_OK)
	{
		Error_Handler();
	}

	debug.printf = USARTPrintf;

	if(HAL_UART_Receive_IT(&UartHandle, &recv, 1) != HAL_OK)
	{
		Error_Handler();
	}
}

void USARTPutData(const uint8_t data)
{
	HAL_UART_Transmit(&UartHandle, (uint8_t*)&data, 1, 10);
}

void USARTPutString(const char *str)
{
	HAL_UART_Transmit(&UartHandle, (uint8_t*)str, strlen(str), 10);
}

int USARTPrintf(const char *fmt, ...)
{
	static char s[100];
	int ret;
	va_list ap;

	va_start(ap, fmt);
	ret = vsprintf(s, fmt, ap);
	USARTPutString(s);
	va_end(ap);

	return ret;
}

static void Error_Handler()
{
	while(1){};
}
