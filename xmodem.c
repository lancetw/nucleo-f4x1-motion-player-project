#include "stm32f4xx_hal_conf.h"
#include "xmodem.h"

#include "lcd.h"
#include "usart.h"
#include <string.h>

#include "main.h"

volatile uint8_t Crcflg, seqno;

extern UART_HandleTypeDef UartHandle;

uint16_t crc_xmodem_update (uint16_t crc, uint8_t data)
{
    int i;

    crc = crc ^ ((uint16_t)data << 8);
    for (i = 0; i < 8; i++)
    {
        if (crc & 0x8000)
            crc = (crc << 1) ^ 0x1021;
        else
            crc <<= 1;
    }

    return crc;
}

int16_t usart_polling_timeout(uint32_t t)		/* タイムアウト付きUSARTポーリング */
{
	uint8_t recv;

	if(HAL_UART_Receive(&UartHandle, &recv, 1, t) != HAL_OK)
	{
		return -1;
	}

	return recv;
}

void xmodem_init()
{
}

int xmodem_start_session()
{
	uint8_t retry = 0;
	
	seqno = 1;
	
	debug.printf("\r\nstarting session...\r\n");

	HAL_NVIC_DisableIRQ(USARTx_IRQn); /* USART受信割り込み一時停止 */

WAIT_NACK:
	switch(usart_polling_timeout(WAIT_NACK_TIMEOUT)){		/* 接続要求待ち */
		case NAK:							/* NAK受信 Checksum ブロック送信へ*/
			Crcflg = FALSE;
			return 1;
		case 'C':							/* C文字受信 CRC ブロック送信へ*/
			Crcflg = TRUE;
			return 1;
		case CAN:							/* CANまたはETX受信　XMODEM終了 */
		case ETX:
			break;
		case -1:							/* タイムアウト 接続要求待ちへ */
		default:
			if(++retry <= WAIT_NACK_RETRY) goto WAIT_NACK;
			break;
	}

	HAL_NVIC_EnableIRQ(USARTx_IRQn); /* USART受信割り込み再開 */

	return (-1);
}

int xmodem_transmit(void* p, uint32_t blocks)	/* XMODEM送信ルーチン */
{
	register uint32_t i, n;
	uint8_t xbuf[133], *buf = (uint8_t*)p;
	uint8_t retry = 0;
	uint16_t crc;
	
	for(n = 0;n < blocks;n++){					/* n個のブロックを送信 */
	REQ_NACK:
		xbuf[HEAD] = SOH;						/* SOH付加 */
		xbuf[SEQ] = seqno;						/* 現在のシーケンス番号付加 */
		xbuf[COM] = seqno ^ 255;				/* シーケンス番号の補数付加 */
		
		memcpy((uint8_t*)&xbuf[DAT], (uint8_t*)&buf[n * 128], 128);

		if(!Crcflg) {							/* Checksum計算 */
			xbuf[CHK] = 0;
			for(i = DAT;i < CHK;i++)
				xbuf[CHK] += xbuf[i];
			
//			for(i = HEAD;i <= CHK;i++)			/* ブロック送出 */
//				USARTPutData(xbuf[i]);
			HAL_UART_Transmit(&UartHandle, xbuf, CHK + 1, 100);

		} else {									/* CRC計算 */
			crc = 0;
			for(i = DAT;i < CHK;i++)
				crc = crc_xmodem_update(crc, xbuf[i]);	/* CRCコード生成 */
			
			xbuf[CRCH] = (crc >> 8) & 0x00FF;
			xbuf[CRCL] = crc & 0x00FF;
			
//			for(i = HEAD;i <= CRCL;i++)			/* ブロック送出 */
//				USARTPutData(xbuf[i]);
			HAL_UART_Transmit(&UartHandle, xbuf, CRCL + 1, 100);
		}

	WAIT_ACK:
		switch(usart_polling_timeout(WAIT_ACK_TIMEOUT)){		/* ACK応答待ち */
			case ACK:							/* ACK受信 次のシーケンスへ */
				retry = 0;
				break;
			case NAK:							/* NAK受信 ブロック再送 */
				if(++retry < REQ_NACK_RETRY) goto REQ_NACK;
				goto END;
			case CAN:							/* CAN受信 XMODEM終了 */
				goto END;
			case -1:							/* タイムアウト CAN送信後 XMODEM終了*/
			default:
				if(++retry <= WAIT_ACK_RETRY) goto WAIT_ACK;
				goto CAN_END;
		}
		seqno++;								/* 次のシーケンス番号に更新 */
		retry = 0;
	}
	
	return 0;
	
CAN_END:
	USARTPutData(CAN);						/* CANを送信して終了 */
END:

	HAL_NVIC_EnableIRQ(USARTx_IRQn); /* USART受信割り込み再開 */

	HAL_Delay(100);

	return -1;
}

void xmodem_end_session()
{
	USARTPutData(EOT);						/* ブロック送信完了 EOT送出 */

	while(usart_polling_timeout(100) != ACK){};	/* ACK応答待ち */
	
	HAL_NVIC_EnableIRQ(USARTx_IRQn); /* USART受信割り込み再開 */

	HAL_Delay(1000);

	debug.printf("\r\ncomplete:%d", Crcflg);
}

__attribute__( ( always_inline ) ) static __INLINE uint16_t get_image_data(uint16_t *p_img, uint16_t width, uint16_t x, uint16_t y)
{
	return (p_img[(y) * width + x]);
}

#define LCD_WIDTH_LOW_BYTE (LCD_WIDTH & 0xff)
#define LCD_WIDTH_HIGH_BYTE (LCD_WIDTH >> 8)

#define LCD_HEIGHT_LOW_BYTE (LCD_HEIGHT & 0xff)
#define LCD_HEIGHT_HIGH_BYTE (LCD_HEIGHT >> 8)


void xput()
{
	const uint8_t bmp_header[] = {
		0x42, 0x4d, 0x46, 0x58, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46, 0x00, 0x00, 0x00, 0x38, 0x00,
		0x00, 0x00, LCD_WIDTH_LOW_BYTE, LCD_WIDTH_HIGH_BYTE, 0x00, 0x00, LCD_HEIGHT_LOW_BYTE, LCD_HEIGHT_HIGH_BYTE, 0x00, 0x00, 0x01, 0x00, 0x10, 0x00, 0x03, 0x00,
		0x00, 0x00, 0x00, 0x58, 0x02, 0x00, 0x13, 0x0b, 0x00, 0x00, 0x13, 0x0b, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0x1f, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	};

	uint8_t buf[1024];
	int totalPixel = LCD_WIDTH * LCD_HEIGHT + sizeof(bmp_header) / sizeof(uint16_t);
	int i, n, x, y;
	uint16_t *p_ram;

	extern uint16_t frame_buffer[160 * 128];

	debug.printf("\r\nXMODEM");

	xmodem_init();
	if(xmodem_start_session() == -1){
		debug.printf("\r\nsesson timeout...");
		return;
	}

	memcpy((uint8_t*)buf, (uint8_t*)bmp_header, sizeof(bmp_header));
	p_ram = (uint16_t*)&buf[sizeof(bmp_header)];
	n = ( sizeof(buf) -  sizeof(bmp_header) ) / sizeof(uint16_t);

	x = 0, y = LCD_HEIGHT - 1;

	do{
		for(i = 0;i < n;i++){
			*p_ram++ = get_image_data(frame_buffer, LCD_WIDTH, x, y);

			if(++x >= LCD_WIDTH)
			{
				x = 0;
				if(--y < 0)
				{
					y = LCD_HEIGHT - 1;
				}
			}
		}

		xmodem_transmit((uint8_t*)buf, sizeof(buf) / 128);

		totalPixel -= n;
		n = sizeof(buf) / sizeof(uint16_t);
		p_ram = (uint16_t*)buf;

	}while(totalPixel > 0);

	xmodem_end_session();
}
