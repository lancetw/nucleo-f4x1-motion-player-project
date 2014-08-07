/*
 * usart.h
 *
 *  Created on: 2014/03/04
 *      Author: masayuki
 */

#ifndef USART_H_
#define USART_H_

#define CURSOR_LEFT  0x01
#define CURSOR_RIGHT 0x06
#define CURSOR_UP    0x10
#define CURSOR_DOWN  0x0e
#define CURSOR_ENTER 0x0D

#define VOL_UP		       0x10
#define VOL_DOWN	       0x11
#define VOL_MUTE	       0x12
#define PLAY_PAUSE	       0x13
#define PLAY_LOOP_MODE     0x14
#define PLAY_NEXT	       0x15
#define PLAY_PREV	       0x16
#define PLAY_SW_HOLD_LEFT  0x17
#define PLAY_SW_HOLD_RIGHT 0x18

typedef struct {
	int (*printf)(const char *, ...);
} debug_printf_typedef;

extern volatile debug_printf_typedef debug;

extern void USART_Init();
extern void USARTPutData(const uint8_t data);
extern void USARTPutString(const char *str);
int USARTPrintf(const char *fmt, ...);


#endif /* USART_H_ */
