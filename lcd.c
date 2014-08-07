/*
 * lcd.c
 *
 *  Created on: 2014/03/23
 *      Author: masayuki
 */

#include "stm32f4xx_hal.h"

#include "main.h"
#include "lcd.h"
#include "sd.h"
#include "sound.h"
#include "usart.h"
#include "string.h"
#include "stdio.h"
#include "picojpeg.h"

#include <ctype.h>
#include <math.h>

#include "icon.h"

#include "sjis2utf16.h"

#include "settings.h"

uint16_t clx, cly;

uint16_t cursorRAM[LCD_WIDTH * 13];

volatile time_typedef sleep_time;

cursor_typedef cursor;

LCDStatusStruct_typedef LCDStatusStruct;

SPI_HandleTypeDef SpiLcdHandle;

uint16_t frame_buffer[160 * 128];

void LCD_Reset()
{
	HAL_GPIO_WritePin(SPI_LCD_RESET_GPIO_PORT, SPI_LCD_RESET_PIN, GPIO_PIN_RESET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(SPI_LCD_RESET_GPIO_PORT, SPI_LCD_RESET_PIN, GPIO_PIN_SET);
	HAL_Delay(50);

	HAL_GPIO_WritePin(SPI_LCD_NSS_GPIO_PORT, SPI_LCD_NSS_PIN, GPIO_PIN_SET);

}


void LCD_CMD(uint16_t index)
{
	SPI_LCD_NSS_PIN_ASSERT;
	SPI_LCD_RS_PIN_ASSERT;
	HAL_SPI_Transmit(&SpiLcdHandle, (uint8_t*)&index, 1, 10);
//    SpiLcdHandle.Instance->DR = index;
//	while((SpiLcdHandle.Instance->SR & SPI_FLAG_TXE) == RESET){};
//	while((SpiLcdHandle.Instance->SR & SPI_FLAG_BSY)){};

	/*
	switch(HAL_SPI_Transmit(&SpiLcdHandle, txData, 1, 10)){
	case HAL_TIMEOUT:
		debug.printf("\r\nHAL_TIMEOUT");
		break;
	case HAL_BUSY:
		debug.printf("\r\nHAL_BUSY");
		break;
	case HAL_ERROR:
		debug.printf("\r\nHAL_ERROR");
		break;
	case HAL_OK:
		debug.printf("\r\nHAL_OK");
		break;
	default:
		debug.printf("\r\nUnkown ERROR");
		break;
	}
*/
	SPI_LCD_NSS_PIN_DEASSERT;
}

void LCD_FRAME_BUFFER_Transmit(int8_t blocking)
{
	while(blocking && (SpiLcdHandle.State != HAL_SPI_STATE_READY)){};

	if(!blocking && (SpiLcdHandle.State != HAL_SPI_STATE_READY))
	{
		HAL_DMA_Abort(SpiLcdHandle.hdmatx);
	}

	LCD_CMD(0x002c);
	SPI_LCD_NSS_PIN_ASSERT;
	SPI_LCD_RS_PIN_DEASSERT;
	HAL_SPI_Transmit_DMA(&SpiLcdHandle, (uint8_t*)frame_buffer, FRAME_BUFFER_SIZE / sizeof(uint16_t));

	while((blocking == LCD_DMA_TRANSMIT_COMPBLOCKING) && (SpiLcdHandle.State != HAL_SPI_STATE_READY)){};
}


void LCD_DATA(uint16_t data)
{
	SPI_LCD_NSS_PIN_ASSERT;
	SPI_LCD_RS_PIN_DEASSERT;
	HAL_SPI_Transmit(&SpiLcdHandle, (uint8_t*)&data, 1, 10);
//    SpiLcdHandle.Instance->DR = data;
//	while((SpiLcdHandle.Instance->SR & SPI_FLAG_TXE) == RESET){};
//	while((SpiLcdHandle.Instance->SR & SPI_FLAG_BSY)){};

    /*
    while(1){
    	if(SpiLcdHandle.Instance->SR & SPI_FLAG_TXE){
    		break;
    	}
    }
*/
	/*
	switch(HAL_SPI_Transmit(&SpiLcdHandle, txData, 1, 10)){
	case HAL_TIMEOUT:
		debug.printf("\r\nHAL_TIMEOUT");
		break;
	case HAL_BUSY:
		debug.printf("\r\nHAL_BUSY");
		break;
	case HAL_ERROR:
		debug.printf("\r\nHAL_ERROR");
		break;
	case HAL_OK:
		debug.printf("\r\nHAL_OK");
		break;
	default:
		debug.printf("\r\nUnkown ERROR");
		break;
	}
*/
	SPI_LCD_NSS_PIN_DEASSERT;
}

void LCD_GET(uint16_t index, uint8_t *array)
{
	SPI_LCD_NSS_PIN_ASSERT;
	SPI_LCD_RS_PIN_ASSERT;
	HAL_SPI_Transmit(&SpiLcdHandle, (uint8_t*)&index, 1, 10);

	SPI_LCD_RS_PIN_DEASSERT;
	HAL_SPI_Receive(&SpiLcdHandle, array, 4, 10);

//    SpiLcdHandle.Instance->DR = data;
//	while((SpiLcdHandle.Instance->SR & SPI_FLAG_TXE) == RESET){};
//	while((SpiLcdHandle.Instance->SR & SPI_FLAG_BSY)){};

    /*
    while(1){
    	if(SpiLcdHandle.Instance->SR & SPI_FLAG_TXE){
    		break;
    	}
    }
*/
	/*
	switch(HAL_SPI_Transmit(&SpiLcdHandle, txData, 1, 10)){
	case HAL_TIMEOUT:
		debug.printf("\r\nHAL_TIMEOUT");
		break;
	case HAL_BUSY:
		debug.printf("\r\nHAL_BUSY");
		break;
	case HAL_ERROR:
		debug.printf("\r\nHAL_ERROR");
		break;
	case HAL_OK:
		debug.printf("\r\nHAL_OK");
		break;
	default:
		debug.printf("\r\nUnkown ERROR");
		break;
	}
*/
	SPI_LCD_NSS_PIN_DEASSERT;
}


void LCD_Data_N(uint8_t *txData, uint16_t n)
{
	SPI_LCD_NSS_PIN_ASSERT;
	SPI_LCD_RS_PIN_DEASSERT;
	HAL_SPI_Transmit(&SpiLcdHandle, txData, n, 100);
	/*
	switch(HAL_SPI_Transmit(&SpiLcdHandle, txData, 1, 10)){
	case HAL_TIMEOUT:
		debug.printf("\r\nHAL_TIMEOUT");
		break;
	case HAL_BUSY:
		debug.printf("\r\nHAL_BUSY");
		break;
	case HAL_ERROR:
		debug.printf("\r\nHAL_ERROR");
		break;
	case HAL_OK:
		debug.printf("\r\nHAL_OK");
		break;
	default:
		debug.printf("\r\nUnkown ERROR");
		break;
	}
*/
	SPI_LCD_NSS_PIN_DEASSERT;
}


void LCD_SetRegion(uint16_t x_start,uint16_t y_start,uint16_t x_end,uint16_t y_end)
{
	LCD_CMD(0x002a);
	LCD_DATA(x_start);
	LCD_DATA(x_end);

	LCD_CMD(0x002b);
	LCD_DATA(y_start);
	LCD_DATA(y_end);

	LCD_CMD(0x002c);
}

void LCD_SetGramAddr(uint16_t x_start, uint16_t y_start)
{
	LCD_CMD(0x002a);
	LCD_DATA(x_start);

	LCD_CMD(0x002b);
	LCD_DATA(y_start);
}


void LCD_Clear(uint16_t Color)
{
   unsigned int i,m;
   LCD_SetRegion(0,0,160-1,128-1);
   LCD_CMD(0x002C);
   for(i=0;i<128;i++)
    for(m=0;m<160;m++)
      {
	  	LCD_DATA(Color);
      }
}

void LCD_Config()
{
	LCD_Reset();

	LCD_CMD(0x0011);//Sleep exit
	HAL_Delay(120);

	//ST7735R Frame Rate
	LCD_CMD(0x00B1);
	LCD_DATA(0x012C);
	LCD_DATA(0x2D00);
	LCD_CMD(0x00B2);
	LCD_DATA(0x012C);
	LCD_DATA(0x2D00);

	LCD_CMD(0x00B3);
	LCD_DATA(0x012C);
	LCD_DATA(0x2D01);
	LCD_DATA(0x2C2D);

	LCD_CMD(0x00B4); //Column inversion
	LCD_DATA(0x0700);

	//ST7735R Power Sequence
	LCD_CMD(0x00C0);
	LCD_DATA(0xA202);
	LCD_DATA(0x8400);
	LCD_CMD(0x00C1);
	LCD_DATA(0xC500);
	LCD_CMD(0x00C2);
	LCD_DATA(0x0A00);
	LCD_CMD(0x00C3);
	LCD_DATA(0x8A2A);
	LCD_CMD(0x00C4);
	LCD_DATA(0x8AEE);

	LCD_CMD(0x00C5); //VCOM
	LCD_DATA(0x0E00);

	LCD_CMD(0x0036); //MX, MY, RGB mode
	LCD_DATA(0x6000);

	//ST7735R Gamma Sequence
	LCD_CMD(0x00e0);
	LCD_DATA(0x0f1a);
	LCD_DATA(0x0f18);
	LCD_DATA(0x2f28);
	LCD_DATA(0x2022);
	LCD_DATA(0x1f1b);
	LCD_DATA(0x2337);
	LCD_DATA(0x0007);
	LCD_DATA(0x0210);

	LCD_CMD(0x00e1);
	LCD_DATA(0x0f1b);
	LCD_DATA(0x0f17);
	LCD_DATA(0x332c);
	LCD_DATA(0x292e);
	LCD_DATA(0x3030);
	LCD_DATA(0x393f);
	LCD_DATA(0x0007);
	LCD_DATA(0x0310);


	LCD_CMD(0x00F0); //Enable test command
	LCD_DATA(0x0100);
	LCD_CMD(0x00F6); //Disable ram power save mode
	LCD_DATA(0x0000);


	LCD_CMD(0x003A); //65k mode
	LCD_DATA(0x0500);

	LCD_CMD(0x0029);//Display on

	LCD_SetRegion(0,0,160-1,128-1);

	LCD_Clear(colorc[BLACK]);
}

void LCD_Clear_Frame_Buffer(uint16_t *p_img, colors color)
{
	int i;
	for(i = 0;i < LCD_WIDTH * LCD_HEIGHT;i++){
		p_img[i] = colorc[color];
	}
}

void LCD_Init()
{
	GPIO_InitTypeDef GPIO_InitStruct;
	HAL_SPI_DeInit(&SpiLcdHandle);

	/*##-1- Configure the SPI peripheral #######################################*/
	/* Set the SPI parameters */
	SpiLcdHandle.Instance               = SPI_LCD;

	SpiLcdHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
	SpiLcdHandle.Init.Direction         = SPI_DIRECTION_1LINE;
	SpiLcdHandle.Init.CLKPhase          = SPI_PHASE_1EDGE;//SPI_PHASE_2EDGE;
	SpiLcdHandle.Init.CLKPolarity       = SPI_POLARITY_HIGH;
	SpiLcdHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLED;
	SpiLcdHandle.Init.CRCPolynomial     = 7;
	SpiLcdHandle.Init.DataSize          = SPI_DATASIZE_16BIT;
	SpiLcdHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
	SpiLcdHandle.Init.NSS               = SPI_NSS_SOFT;
	SpiLcdHandle.Init.TIMode            = SPI_TIMODE_DISABLED;
	SpiLcdHandle.Init.Mode 				= SPI_MODE_MASTER;

	if(HAL_SPI_Init(&SpiLcdHandle) != HAL_OK)
	{
		/* Initialization Error */
	    //Error_Handler();
		debug.printf("\r\nSPI Init Error");
	}


	LCD_Config();
}

void LCD_GotoXY(int x, int y)
{
	clx = x;
	cly = y;
}


void LCDPutWideCharDefault(uint16_t code, void *cp)
{
	if(pcf_font.ext_loaded){
		LCD_FUNC.putChar(code, cp);
	} else if(code >= 0x0020 && code <= 0x00DF){
//		LCDPutAscii(code, color);
//		PCFPutChar(code, cp);
		LCD_FUNC.putChar(code, cp);
	} else {
//		LCDPutAscii(0x00e0, color);
//		PCFPutChar(0x0080, cp);
		LCD_FUNC.putChar(0x0080, cp);
	}
}
/*
void LCDPutWideChar16pxDefault(uint16_t code, void *cp)
{
	if(pcf_font.ext_loaded){
		LCD_FUNC.putChar16px(code, cp);
	} else if(code >= 0x0020 && code <= 0x00DF){
//		LCDPutAscii(code, color);
//		PCFPutChar16px(code, cp);
		LCD_FUNC.putChar16px(code, cp);
	} else {
//		LCDPutAscii(0x00e0, color);
//		PCFPutChar16px(0x0080, cp);
		LCD_FUNC.putChar16px(0x0080, cp);
	}
}
*/
/*
void LCDPutString(const uint16_t *uni_str, int n, pcf_typedef *pcf)
{
	while(n-- > 0){
		LCDPutWideCharDefault(*uni_str++, pcf);
	}
}
*/

void LCDPutString(const char *str, pcf_typedef *pcf)
{
	while(*str != '\0'){
		if((0x20 <= *str) && (*str <= 0xDF)){
			LCD_FUNC.putChar(*str++, pcf);
			continue;
		} else if(*str++ == '\n') {
			clx = 0;
			cly += 15;
			continue;
		} else if(!pcf_font.ext_loaded){
			LCD_FUNC.putChar(0x0080, pcf);
			debug.printf("\r\nunkown:%02x", *str);
		}
	}
}

void LCDPutStringN(const char *str, uint16_t endPosX, pcf_typedef *pcf)
{
	while(*str != '\0'){
		if(clx >= endPosX){
			LCDPutString("..", pcf);
			return;
		}
		if((0x20 <= *str) && (*str <= 0xDF)){
			LCD_FUNC.putChar(*str++, pcf);
		} else if(*str++ == '\n') {
			clx = 0;
			cly += 13;
			continue;
		} else if(!pcf_font.ext_loaded){
			LCD_FUNC.putChar(0x0080, pcf);
		}
	}
}

uint16_t LCDPutStringUTF8(uint16_t startPosX, uint16_t endPosX, uint8_t lineCnt, uint8_t *s, pcf_typedef *pcf)
{
	uint16_t tc, yPos = 0;

	while(*s != '\0'){
		if(clx > endPosX){
			if(lineCnt-- > 1){
				if(pcf->size == 16){
					cly += 17;
					yPos += 17;
				} else {
					cly += 13;
					yPos += 13;
				}
				clx = startPosX;
			} else {
				LCDPutString("..", pcf);
				return yPos;
			}
		}

		if((*s >= 0x20 && *s <= 0x7F)){
			LCD_FUNC.putChar(*s++, pcf);
			continue;
		}

		if(*s >= 0xE0){
			tc  = ((uint16_t)*s++ << 12) & 0xF000;
			tc |= ((uint16_t)*s++ << 6) & 0x0FC0;
			tc |=  (uint16_t)*s++ & 0x003F;

			if(!pcf_font.ext_loaded){
				LCD_FUNC.putChar(C_FONT_UNDEF_CODE, pcf);
			} else {
				LCD_FUNC.putChar(tc, pcf);
			}
		} else {
			s++;
		}
	}

	return yPos;
}

uint16_t LCDGetStringUTF8PixelLength(uint8_t *s, uint16_t font_width)
{
	uint16_t tc, len = 0;

	while(*s != '\0'){
		if((*s >= 0x20 && *s <= 0x7F)){
			len += LCD_FUNC.getCharLength(*s++, font_width);
			continue;
		}

		if(*s >= 0xE0){
			tc  = ((uint16_t)*s++ << 12) & 0xF000;
			tc |= ((uint16_t)*s++ << 6) & 0x0FC0;
			tc |=  (uint16_t)*s++ & 0x003F;

			if(!pcf_font.ext_loaded){
				len += LCD_FUNC.getCharLength(C_FONT_UNDEF_CODE, font_width);
			} else {
				len += LCD_FUNC.getCharLength(tc, font_width);
			}
		} else {
			s++;
		}
	}

	return len;
}


static uint16_t sjis_utf16_conv(uint16_t code)
{
	return sjis2utf16_table[code - 0x8140];
}

void LCDPutStringSJIS(uint8_t *s, pcf_typedef *pcf)
{
	uint16_t tc;

	while(*s != '\0'){
		if((*s >= 0x20 && *s <= 0x7F) || (*s >= 0xA0 && *s <= 0xDF)){
			LCD_FUNC.putChar(*s++, pcf);
			continue;
		}

		tc  = *s++ << 8;
		tc |= *s++;

		if(!pcf_font.ext_loaded){
			LCD_FUNC.putWideChar(C_FONT_UNDEF_CODE, pcf);
		} else {
			LCD_FUNC.putWideChar(sjis_utf16_conv(tc), pcf);
		}
	}
}

uint16_t LCDPutStringSJISN(uint16_t startPosX, uint16_t endPosX, uint8_t lineCnt, uint8_t *s, pcf_typedef *pcf)
{
	uint16_t tc, yPos = 0;

	while(*s != '\0'){
		/*
		if(clx >= endPosX){
			LCDPutString("..", color);
			return;
		}
		*/
		if(clx > endPosX){
			if(lineCnt-- > 1){
				if(pcf->size == 16){
					cly += 17;
					yPos += 17;
				} else {
					cly += 13;
					yPos += 13;
				}
				clx = startPosX;
			} else {
				LCDPutString("..", pcf);
				return yPos;
			}
		}

		if((*s >= 0x20 && *s <= 0x7F) || (*s >= 0xA0 && *s <= 0xDF)){
			LCD_FUNC.putChar(*s++, (void*)pcf);
			continue;
		}

		tc  = *s++ << 8;
		tc |= *s++;

		if(!pcf_font.ext_loaded){
			LCD_FUNC.putWideChar(C_FONT_UNDEF_CODE, (void*)pcf);
		} else {
			LCD_FUNC.putWideChar(sjis_utf16_conv(tc), (void*)pcf);
		}
	}

	return yPos;
}

uint16_t LCDGetStringSJISPixelLength(uint8_t *s, uint16_t font_width)
{
	uint16_t tc, len = 0;

	while(*s != '\0'){
		if((*s >= 0x20 && *s <= 0x7F) || (*s >= 0xA0 && *s <= 0xDF)){
			len += LCD_FUNC.getCharLength(*s++, font_width);
			continue;
		}

		tc  = *s++ << 8;
		tc |= *s++;

		if(!pcf_font.ext_loaded){
			len += LCD_FUNC.getCharLength(C_FONT_UNDEF_CODE, font_width);
		} else {
			len += LCD_FUNC.getCharLength(sjis_utf16_conv(tc), font_width);
		}
	}

	return len;
}

uint16_t LCDGetStringLFNPixelLength(void *s, uint16_t font_width){
	uint16_t *tc, len = 0;

	tc = (uint16_t*)s;

	while(1){
//		debug.printf("\r\ntc:%04x", *tc);

		if(*tc == 0x0000){
			return len;
		}
		if(*tc <= 0x007F){
//			debug.printf("\r\nother:%04x", *tc);
//			len += PCFGetCharPixelLength(tc, font_width);
			len += LCD_FUNC.getCharLength(*tc++, font_width);
			continue;
		}
		if(!pcf_font.ext_loaded){
//			len += PCFGetCharPixelLength(0x0080, font_width);
			len += LCD_FUNC.getCharLength(0x0080, font_width);
			tc++;
			continue;
		}
//		debug.printf("\r\nother2:%04x", *tc);
//		len += PCFGetCharPixelLength(tc, font_width);
		len += LCD_FUNC.getCharLength(*tc++, font_width);
	}

	return len;
}


uint16_t LCDPutStringLFN(uint16_t startPosX, uint16_t endPosX, uint8_t lineCnt, uint8_t *s, pcf_typedef *pcf)
{
	uint16_t tc, yPos = 0;

	while(1){
		if(clx > endPosX){
			if(lineCnt-- > 1){
				if(pcf->size == 16){
					cly += 17;
					yPos += 17;
				} else {
					cly += 13;
					yPos += 13;
				}
				clx = startPosX;
			} else {
				LCDPutString("..", pcf);
				return yPos;
			}
		}

		tc  = *(uint8_t*)s++;
		tc |= *(uint8_t*)s++ << 8;

		if(tc == 0x0000){
			return yPos;
		}
		if(tc <= 0x007F){
			LCD_FUNC.putChar(tc, pcf);
			continue;
		} else if(!pcf_font.ext_loaded){
			LCD_FUNC.putChar(0x0080, pcf);
		}
		LCD_FUNC.putWideChar(tc, pcf);
	}

	return yPos;
}

__attribute__( ( always_inline ) ) static __INLINE uint16_t get_image_data(const uint16_t *p_img, uint16_t width, uint16_t x, uint16_t y)
{
	return (p_img[y * width + x]);
}

__attribute__( ( always_inline ) ) static __INLINE void set_image_data(uint16_t *p_img, uint16_t width, uint16_t x, uint16_t y, uint16_t data)
{
	if((y * width + x) >= FRAME_BUFFER_SIZE){
		return;
	}
	p_img[y * width + x] = data;
}

void LCDStoreBgImgToBuff(int startPosX, int startPosY, int width, int height, uint16_t *p)
{
	uint32_t x, y;
	for(y = 0;y < height;y++){
//		LCDSetGramAddr(startPosX, startPosY + y);
//		LCDPutCmd(0x0022);
//		LCD->RAM;
		for(x = 0;x < width;x++){
//			*p++ = LCD->RAM;
			*p++ = get_image_data(frame_buffer, LCD_WIDTH, startPosX + x, startPosY +y);
		}
	}
}
/*
void LCDStoreBgImgToBuff2(fft_analyzer_typedef *fftDrawBuff)
{
	uint32_t x, y, i, j;
	for(j = 0;j < fftDrawBuff->numRow;j++){
		for(i = 0;i < fftDrawBuff->numCol;i++){
			for(y = 0;y < fftDrawBuff->height;y++){
				for(x = 0;x < fftDrawBuff->width;x++){
					fftDrawBuff->bar[i][j][y * fftDrawBuff->width + x] = get_image_data(frame_buffer, LCD_WIDTH, startPosX + x, startPosY +y);
				}
			}
		}
	}
}
*/

void LCDPutBgImgToFramebuffer(int startPosX, int startPosY, int width, int height, const uint16_t *p)
{
	uint32_t x, y;
	for(y = 0;y < height;y++){
		for(x = 0;x < width;x++){
			set_image_data(frame_buffer, LCD_WIDTH, startPosX + x, startPosY + y, get_image_data(p, LCD_WIDTH, startPosX + x, startPosY + y));
		}
	}
}

void LCDPutBuffToBgImg(int startPosX, int startPosY, int width, int height, uint16_t *p)
{
	uint32_t x, y;
	for(y = 0;y < height;y++){
//		LCDSetGramAddr(startPosX, startPosY + y);
//		LCDPutCmd(0x0022);
		for(x = 0;x < width;x++){
//			LCD->RAM = *p++;
			set_image_data(frame_buffer, LCD_WIDTH, startPosX + x, startPosY + y, *p++);
		}
	}
}

void LCDXor()
{
	int x, y;
	uint16_t data;

	for(y = 0;y < LCD_HEIGHT;y++)
	{
		for(x = 0;x < LCD_WIDTH;x++)
		{
			data = get_image_data(frame_buffer, LCD_WIDTH, x, y);
			set_image_data(frame_buffer, LCD_WIDTH, x, y, data ^ 0xFFFF);
		}
	}
	LCD_FRAME_BUFFER_Transmit(LCD_DMA_TRANSMIT_BLOCKING);
}

void LCDFadeIn(uint8_t step, const uint16_t *bgimg)
{
	int x, y, i;
	pixel_fmt_typedef pixel;

	for(i = 31;i >= 0;i -= step)
	{
		memcpy((void*)frame_buffer, bgimg, sizeof(frame_buffer));
		for(y = 0;y < LCD_HEIGHT;y++)
		{
			for(x = 0;x < LCD_WIDTH;x++)
			{
				pixel.color.d16 = get_image_data(frame_buffer, LCD_WIDTH, x, y);
				pixel.color.R = __USAT(pixel.color.R - i, 5);
				pixel.color.G = __USAT(pixel.color.G - i * 2, 6);
				pixel.color.B = __USAT(pixel.color.B - i, 5);
				set_image_data(frame_buffer, LCD_WIDTH, x, y, pixel.color.d16);
			}
		}
		LCD_FRAME_BUFFER_Transmit(LCD_DMA_TRANSMIT_BLOCKING);
		HAL_Delay(30);
	}
}


void LCDFadeOut(uint8_t step)
{
	int x, y, i;
	pixel_fmt_typedef pixel;

	for(i = 0;i < 32;i += step)
	{
		for(y = 0;y < LCD_HEIGHT;y++)
		{
			for(x = 0;x < LCD_WIDTH;x++)
			{
				pixel.color.d16 = get_image_data(frame_buffer, LCD_WIDTH, x, y);
				pixel.color.R = __USAT(pixel.color.R - step, 5);
				pixel.color.G = __USAT(pixel.color.G - step * 2, 6);
				pixel.color.B = __USAT(pixel.color.B - step, 5);
				set_image_data(frame_buffer, LCD_WIDTH, x, y, pixel.color.d16);
			}
		}
		LCD_FRAME_BUFFER_Transmit(LCD_DMA_TRANSMIT_BLOCKING);
		HAL_Delay(20);
	}
}

#define RAD_TO_DEG_COEF ((float)M_PI / 180.0f)
#define GET_IMAGE_DATA(img, x, y) (img[(y) * LCD_WIDTH + x])


void LCDRotateImage(int16_t sx, int16_t sy, uint16_t width, uint16_t height, const uint16_t *p_img, uint16_t bgColor, int16_t angle, uint8_t enableAntiAlias)
{
    int x, y, x1, y1, inverse;
    int cx = width >> 1, cy = height >> 1;

    sx = sx - cx + 1, sy = sy - cy + 1;

    float f_sin, f_cos;

    f_sin = sinf(angle * RAD_TO_DEG_COEF);
    f_cos = cosf(angle * RAD_TO_DEG_COEF);

    uint16_t *p_u16;

    for(y = 0;y < LCD_HEIGHT;y++){

    	p_u16 = &frame_buffer[LCD_WIDTH * y];

        for(x = 0;x < LCD_WIDTH;x++){
    		if(enableAntiAlias){ // Antialias (Biliner interpolation)

    		    float fx1, fy1;
    			float ratioX0, ratioX1, ratioY0, ratioY1;

        		fx1 = ((x - sx) - cx) * f_cos - ((y - sy) - cy) * f_sin + cx;
        		fy1 = ((x - sx) - cx) * f_sin + ((y - sy) - cy) * f_cos + cy;

        		x1 = (int)fx1;
        		y1 = (int)fy1;

    			if(x1 >= 0 && x1 < width && y1 >= 0 && y1 < height){
    				pixel_fmt_typedef pixel, pixelBox[2][2], pixelH0, pixelH1;

        			if(fx1 < 0.0f){
        				inverse = 2; // H inverse
        				fx1 = fabsf(fx1);
        			}else if(fy1 < 0.0f){
        				inverse = 1; // V inverse
        				fy1 = fabsf(fy1);
        			} else {
        				inverse = 0; // Normal
        			}

        			ratioX1 = fx1 - x1;
        			ratioX0 = 1.0f - ratioX1;

        			ratioY1 = fy1 - y1;
        			ratioY0 = 1.0f - ratioY1;

    				pixelBox[0][0].color.d16 = get_image_data(p_img, width, x1, y1);

    				if(!inverse){
    					if((x1 + 1) < width && (y1 + 1) < height){
    						pixelBox[1][1].color.d16 = get_image_data(p_img, width, x1 + 1, y1 + 1);
    					} else {
    						pixelBox[1][1].color.d16 = bgColor;
    					}
    					if((x1 + 1) < width){
    						pixelBox[1][0].color.d16 = get_image_data(p_img, width, x1 + 1, y1);
    					} else {
    						pixelBox[1][0].color.d16 = bgColor;
    					}
    					if((y1 + 1) < height){
    						pixelBox[0][1].color.d16 = get_image_data(p_img, width, x1, y1 + 1);
    					} else {
    						pixelBox[0][1].color.d16 = bgColor;
    					}
    				} else if(inverse == 1) { // V inverse
    					if((x1 + 1) < width){
    						pixelBox[1][0].color.d16 = get_image_data(p_img, width, x1 + 1, y1);
    					} else {
    						pixelBox[1][0].color.d16 = bgColor;
    					}
    					pixelBox[0][1].color.d16 = bgColor;
    					pixelBox[1][1].color.d16 = bgColor;
    				} else { // H inverse
    					if((x1 - 1) >= 0){
    						pixelBox[1][0].color.d16 = get_image_data(p_img, width, x1 - 1, y1);
    					} else {
    						pixelBox[1][0].color.d16 = bgColor;
    					}
    					if((y1 + 1) < height){
    						pixelBox[0][1].color.d16 = get_image_data(p_img, width, x1, y1 + 1);
    					} else {
    						pixelBox[0][1].color.d16 = bgColor;
    					}
    					if((x1 - 1) >= 0 && (y1 + 1) < height){
    						pixelBox[1][1].color.d16 = get_image_data(p_img, width, x1 - 1, y1 + 1);
    					} else {
    						pixelBox[1][1].color.d16 = bgColor;
    					}
    				}

    				pixelH0.color.R = __USAT(pixelBox[0][0].color.R * ratioX0 + pixelBox[1][0].color.R * ratioX1, 5);
    				pixelH0.color.G = __USAT(pixelBox[0][0].color.G * ratioX0 + pixelBox[1][0].color.G * ratioX1, 6);
    				pixelH0.color.B = __USAT(pixelBox[0][0].color.B * ratioX0 + pixelBox[1][0].color.B * ratioX1, 5);

    				pixelH1.color.R = __USAT(pixelBox[0][1].color.R * ratioX0 + pixelBox[1][1].color.R * ratioX1, 5);
    				pixelH1.color.G = __USAT(pixelBox[0][1].color.G * ratioX0 + pixelBox[1][1].color.G * ratioX1, 6);
    				pixelH1.color.B = __USAT(pixelBox[0][1].color.B * ratioX0 + pixelBox[1][1].color.B * ratioX1, 5);

    				pixel.color.R = __USAT(pixelH0.color.R * ratioY0 + pixelH1.color.R * ratioY1, 5);
    				pixel.color.G = __USAT(pixelH0.color.G * ratioY0 + pixelH1.color.G * ratioY1, 6);
    				pixel.color.B = __USAT(pixelH0.color.B * ratioY0 + pixelH1.color.B * ratioY1, 5);

    				*p_u16 = pixel.color.d16;
    			} else {
    				*p_u16 = bgColor;
    			}
    		} else { // No Antialias (Nearest neighbor)

        		x1 = ((x - sx) - cx) * f_cos - ((y - sy) - cy) * f_sin + cx;
        		y1 = ((x - sx) - cx) * f_sin + ((y - sy) - cy) * f_cos + cy;

        		if(x1 >= 0 && x1 < width && y1 >= 0 && y1 < height){
        			*p_u16 = get_image_data(p_img, width, x1, y1);
        		} else {
    				*p_u16 = bgColor;
        		}
    		}
            p_u16++;
    	}
    }
}

void LCDZoomImage(int16_t offsetX, int16_t offsetY, uint16_t width, uint16_t height, uint16_t *p_img, uint16_t bgColor, uint16_t zoom, uint8_t enableAntiAlias)
{
	int x, y, x1, y1;
	float t =100.0f / (float)zoom;

	for(y = 0;y < LCD_HEIGHT;y++){

		for(x = 0;x < LCD_WIDTH;x++){
			if(enableAntiAlias){
			    float fx1, fy1;

				fx1 = x * t;
				fy1 = y * t;

				x1 = (int)fx1;
				y1 = (int)fy1;

				if(x1 >= 0 && x1 < width && y1 >= 0 && y1 < height){

					pixel_fmt_typedef pixel, pixelBox[2][2], pixelH0, pixelH1;
					float ratioX0, ratioX1, ratioY0, ratioY1;


	    			ratioX1 = fx1 - x1;
	    			ratioX0 = 1.0f - ratioX1;

	    			ratioY1 = fy1 - y1;
	    			ratioY0 = 1.0f - ratioY1;

					pixelBox[0][0].color.d16 = get_image_data(p_img, width, x1, y1);

					if((x1 + 1) < width && (y1 + 1) < height){
						pixelBox[1][1].color.d16 = get_image_data(p_img, width, x1 + 1, y1 + 1);
					} else {
						pixelBox[1][1].color.d16 = bgColor;
					}
					if((x1 + 1) < width){
						pixelBox[1][0].color.d16 = get_image_data(p_img, width, x1 + 1, y1);
					} else {
						pixelBox[1][0].color.d16 = bgColor;
					}
					if((y1 + 1) < height){
						pixelBox[0][1].color.d16 = get_image_data(p_img, width, x1, y1 + 1);
					} else {
						pixelBox[0][1].color.d16 = bgColor;
					}

					pixelH0.color.R = __USAT(pixelBox[0][0].color.R * ratioX0 + pixelBox[1][0].color.R * ratioX1, 5);
    				pixelH0.color.G = __USAT(pixelBox[0][0].color.G * ratioX0 + pixelBox[1][0].color.G * ratioX1, 6);
    				pixelH0.color.B = __USAT(pixelBox[0][0].color.B * ratioX0 + pixelBox[1][0].color.B * ratioX1, 5);

    				pixelH1.color.R = __USAT(pixelBox[0][1].color.R * ratioX0 + pixelBox[1][1].color.R * ratioX1, 5);
    				pixelH1.color.G = __USAT(pixelBox[0][1].color.G * ratioX0 + pixelBox[1][1].color.G * ratioX1, 6);
    				pixelH1.color.B = __USAT(pixelBox[0][1].color.B * ratioX0 + pixelBox[1][1].color.B * ratioX1, 5);

    				pixel.color.R = __USAT(pixelH0.color.R * ratioY0 + pixelH1.color.R * ratioY1, 5);
    				pixel.color.G = __USAT(pixelH0.color.G * ratioY0 + pixelH1.color.G * ratioY1, 6);
    				pixel.color.B = __USAT(pixelH0.color.B * ratioY0 + pixelH1.color.B * ratioY1, 5);

    				LCD_SetGramAddr(x + offsetX, y + offsetY);
    				LCD_CMD(0x002C);
    				LCD_DATA(pixel.color.d16);
//    				set_image_data(frame_buffer, width, x, y, pixel.color.d16);
				} else {
					if(offsetX != 0 || offsetY != 0){
						continue;
					}
    				LCD_SetGramAddr(x, y);
    				LCD_CMD(0x002C);
					LCD_DATA(bgColor);
//    				set_image_data(frame_buffer, width, x, y, bgColor);
				}

			} else {

				x1 = x * t - offsetX;
				y1 = y * t - offsetY;

				if(x1 >= 0 && x1 < width && y1 >= 0 && y1 < height){
    				LCD_SetGramAddr(x, y);
    				LCD_CMD(0x002C);
					LCD_DATA(get_image_data(p_img, width, x1, y1));
//    				set_image_data(frame_buffer, width, x, y, get_image_data(p_img, width, x1, y1));
				} else {
    				LCD_SetGramAddr(x, y);
    				LCD_CMD(0x002C);
					LCD_DATA(bgColor);
//    				set_image_data(frame_buffer, width, x, y, bgColor);
				}
			}
		}
	}
}


void LCDZoomImageToBuff(uint16_t bufWidth, uint16_t bufHeight, uint16_t *p_buff, uint16_t srcWidth, uint16_t srcHeight, uint16_t *p_img, uint16_t bgColor, uint16_t zoom, uint8_t enableAntiAlias)
{
	int x, y, x1, y1, offsetX = 0, offsetY = 0;
	float t =100.0f / (float)zoom;

//	offsetX = ((int)(LCD_WIDTH - width * t) >> 1) - 1;
//	offsetY = ((int)(LCD_HEIGHT - height * t) >> 1) - 1;

	for(y = 0;y < bufHeight;y++){

		for(x = 0;x < bufWidth;x++){
			if(enableAntiAlias){
			    float fx1, fy1;

				fx1 = x * t + offsetX;
				fy1 = y * t + offsetY;

				x1 = (int)fx1;
				y1 = (int)fy1;

				if(x1 >= 0 && x1 < srcWidth && y1 >= 0 && y1 < srcHeight){

					pixel_fmt_typedef pixel, pixelBox[2][2], pixelH0, pixelH1;
					float ratioX0, ratioX1, ratioY0, ratioY1;


	    			ratioX1 = fx1 - x1;
	    			ratioX0 = 1.0f - ratioX1;

	    			ratioY1 = fy1 - y1;
	    			ratioY0 = 1.0f - ratioY1;

					pixelBox[0][0].color.d16 = get_image_data(p_img, srcWidth, x1, y1);

					if((x1 + 1) < srcWidth && (y1 + 1) < srcHeight){
						pixelBox[1][1].color.d16 = get_image_data(p_img, srcWidth, x1 + 1, y1 + 1);
					} else {
						pixelBox[1][1].color.d16 = bgColor;
					}
					if((x1 + 1) < srcWidth){
						pixelBox[1][0].color.d16 = get_image_data(p_img, srcWidth, x1 + 1, y1);
					} else {
						pixelBox[1][0].color.d16 = bgColor;
					}
					if((y1 + 1) < srcHeight){
						pixelBox[0][1].color.d16 = get_image_data(p_img, srcWidth, x1, y1 + 1);
					} else {
						pixelBox[0][1].color.d16 = bgColor;
					}

					pixelH0.color.R = __USAT(pixelBox[0][0].color.R * ratioX0 + pixelBox[1][0].color.R * ratioX1, 5);
    				pixelH0.color.G = __USAT(pixelBox[0][0].color.G * ratioX0 + pixelBox[1][0].color.G * ratioX1, 6);
    				pixelH0.color.B = __USAT(pixelBox[0][0].color.B * ratioX0 + pixelBox[1][0].color.B * ratioX1, 5);

    				pixelH1.color.R = __USAT(pixelBox[0][1].color.R * ratioX0 + pixelBox[1][1].color.R * ratioX1, 5);
    				pixelH1.color.G = __USAT(pixelBox[0][1].color.G * ratioX0 + pixelBox[1][1].color.G * ratioX1, 6);
    				pixelH1.color.B = __USAT(pixelBox[0][1].color.B * ratioX0 + pixelBox[1][1].color.B * ratioX1, 5);

    				pixel.color.R = __USAT(pixelH0.color.R * ratioY0 + pixelH1.color.R * ratioY1, 5);
    				pixel.color.G = __USAT(pixelH0.color.G * ratioY0 + pixelH1.color.G * ratioY1, 6);
    				pixel.color.B = __USAT(pixelH0.color.B * ratioY0 + pixelH1.color.B * ratioY1, 5);

    				set_image_data(p_buff, bufWidth, x, y, pixel.color.d16);
				} else {
    				set_image_data(p_buff, bufWidth, x, y, bgColor);
				}

			} else {

				x1 = x * t + offsetX;
				y1 = y * t + offsetY;

				if(x1 >= 0 && x1 < srcWidth && y1 >= 0 && y1 < srcHeight){
    				set_image_data(p_buff, bufWidth, x, y, get_image_data(p_img, srcWidth, x1, y1));
				} else {
    				set_image_data(p_buff, bufWidth, x, y, bgColor);
				}
			}
		}
	}
}


void LCDPutIcon(int startPosX, int startPosY, int width, int height, const uint16_t *d, const uint8_t *a)
{
	int i, j, x, y = startPosY;
	float alpha_ratio;
	pixel_fmt_typedef pixel_fg, pixel_bg;
#define F_INV_255 (1.0f / 255.0f)

	if(a == '\0'){
		for(y = startPosY;y < startPosY + height;y++){
//			LCDSetGramAddr(startPosX, y);
//			LCDPutCmd(0x0022);
			for(x = 0;x < width;x++){
//				LCDPutData(*d++);
				set_image_data(frame_buffer, LCD_WIDTH, startPosX + x, y, *d++);
			}
		}
		return;
	}

	for(i = 0;i < height;i++){
		x = startPosX;
		for(j = 0;j < width;j++){
			pixel_bg.color.d16 = get_image_data(frame_buffer, LCD_WIDTH, x, y);
			pixel_fg.color.d16 = *d++;
			alpha_ratio = *a++ * F_INV_255; // Gray Scale

			// Foreground Color
			pixel_fg.color.R *= alpha_ratio;
			pixel_fg.color.G *= alpha_ratio;
			pixel_fg.color.B *= alpha_ratio;

			// Background Color
			pixel_bg.color.R *= (1.0f - alpha_ratio);
			pixel_bg.color.G *= (1.0f - alpha_ratio);
			pixel_bg.color.B *= (1.0f - alpha_ratio);

			// Add colors
			pixel_fg.color.R += pixel_bg.color.R;
			pixel_fg.color.G += pixel_bg.color.G;
			pixel_fg.color.B += pixel_bg.color.B;

			set_image_data(frame_buffer, LCD_WIDTH, x, y, pixel_fg.color.d16);
			x++;
		}
		y++;
	}
}


void LCDPutCursorBar(int curPos)
{
	int i = 0, x, y;

//	LCDSetGramAddr(0, (curPos + 1) * HEIGHT_ITEM);
//	LCDPutCmd(0x0022);

	//	for(i = 0;i < LCD_WIDTH * 13;i++){
	//		LCDPutData(cursorRAM[i]);
	//	}

	for(y = 0;y < 13;y++){
		for(x = 0;x < LCD_WIDTH;x++){
			set_image_data(frame_buffer, LCD_WIDTH, x, (curPos + 1) * HEIGHT_ITEM + y, cursorRAM[i++]);
		}
	}

	LCD_FRAME_BUFFER_Transmit(LCD_DMA_TRANSMIT_COMPBLOCKING);
}

void LCDSelectCursorBar(int curPos)
{
	int  i, j;
	uint16_t startPosX = 26, width = 120;
	pixel_fmt_typedef pixel;

	for(j = 0;j < 13;j++){
//		LCDSetGramAddr(startPosX, (curPos + 1) * HEIGHT_ITEM + j);
//		LCDPutCmd(0x0022);
		for(i = startPosX;i < (startPosX + width);i++){

			pixel.color.d16 = cursorRAM[i + j * LCD_WIDTH];

			pixel.color.R = __USAT(pixel.color.R + 6, 5);
			pixel.color.G = __USAT(pixel.color.G + 12, 6);
			pixel.color.B = __USAT(pixel.color.B + 6, 5);

//			LCDPutData(pixel.color.d16);
			set_image_data(frame_buffer, LCD_WIDTH, i, (curPos + 1) * HEIGHT_ITEM + j, pixel.color.d16);
		}
	}

	LCD_FRAME_BUFFER_Transmit(LCD_DMA_TRANSMIT_COMPBLOCKING);
}


void LCDStoreCursorBar(int curPos)
{
	int i = 0, x, y;

	for(y = 0;y < 13;y++){
		for(x = 0;x < LCD_WIDTH;x++){
			cursorRAM[i++] = get_image_data(frame_buffer, LCD_WIDTH, x, (curPos + 1) * HEIGHT_ITEM + y);
		}
	}

	/*
	int i;

	LCDSetGramAddr(0, (curPos + 1) * HEIGHT_ITEM);
	LCDPutCmd(0x0022);
	LCD->RAM;

	for(i = 0;i < LCD_WIDTH * 13;i++){
		cursorRAM[i] = LCD->RAM;
	}
	*/
}


void LCDCursorUp()
{
	if(cursor.pos-- <= 0) {
		if(cursor.pageIdx-- <= 0){
			cursor.pageIdx = cursor.pos = 0;
			return;
		} else {
			cursor.pos = PAGE_NUM_ITEMS - 1;
			if(!settings_mode){
				LCDPrintFileList();
			} else {
//				LCDPrintSettingsList(SETTING_TYPE_DIR, 0, NULL);
				LCDPrintSettingsList(settings_parent->type, settings_parent->item->selected_id + 1, settings_parent->item);
			}
			return;
		}
	}

	LCDPutCursorBar(cursor.pos + 1); // 前項消去
	LCDStoreCursorBar(cursor.pos); // 現在の項目を保存
	LCDSelectCursorBar(cursor.pos);  // 現在の項目を選択
}

void LCDCursorDown()
{
	if( (cursor.pos + cursor.pageIdx * PAGE_NUM_ITEMS + 1) >= fat.fileCnt ){
		return;
	}

	if(++cursor.pos >= PAGE_NUM_ITEMS) {
		cursor.pos = 0;
		if(++cursor.pageIdx > cursor.pages) cursor.pageIdx = cursor.pages - 1;
		if(!settings_mode){
			LCDPrintFileList();
		} else {
//			LCDPrintSettingsList(SETTING_TYPE_DIR, 0, NULL);
			LCDPrintSettingsList(settings_parent->type, settings_parent->item->selected_id + 1, settings_parent->item);
		}
		return;
	}

	LCDPutCursorBar(cursor.pos - 1); // 前項消去
	LCDStoreCursorBar(cursor.pos); // 現在の項目を保存
	LCDSelectCursorBar(cursor.pos);  // 現在の項目を選択表示
}


void LCDCursorEnter()
{
	uint16_t idEntry = cursor.pos + cursor.pageIdx * PAGE_NUM_ITEMS, entryPointOffset;
	char fileType[4];

	memset(fileType, '\0', sizeof(fileType));

	LCDPutCursorBar(cursor.pos);
	HAL_Delay(100);
	LCDSelectCursorBar(cursor.pos);

	if(settings_mode){ // settings mode
		if(strncmp(settings_p[idEntry].name, "..", 2) == 0){ // selected item is parent directry
			settings_p = (settings_list_typedef*)settings_p[idEntry].next;
			if(settings_p == NULL){ // back to root
				settings_mode = 0;
				fat.currentDirEntry = fat.rootDirEntry;
				memcpy((char*)fat.currentDirName, (char*)root_str, sizeof(root_str));
				free(fat.pfileList);
				if(cardInfo.ready){
					makeFileList();
				}
				LCDPrintFileList();
				return;
			}
			strcpy((char*)fat.currentDirName, (char*)&settings_stack.name[--settings_stack.idx][0]);
			fat.fileCnt = settings_stack.items[settings_stack.idx];
			cursor.pages = fat.fileCnt / PAGE_NUM_ITEMS;
			if(cursor.pages > 0){
				if(fat.fileCnt % PAGE_NUM_ITEMS){
					cursor.pages = cursor.pages + 1;
				}
			}
			cursor.pageIdx = settings_stack.pos[settings_stack.idx] / PAGE_NUM_ITEMS;
			cursor.pos = settings_stack.pos[settings_stack.idx] % PAGE_NUM_ITEMS;
			LCDPrintSettingsList(SETTING_TYPE_DIR, 0, NULL);
			return;
		} else {
			settings_list_typedef *settings_cur = &settings_p[idEntry];
			if(settings_cur->type != 0){
				settings_parent = settings_cur;
			}
			if(settings_cur->next == NULL){
				LCDPrintSettingsList(SETTING_TYPE_ITEM, idEntry, settings_cur->item);
				return;
			}
			settings_stack.items[settings_stack.idx] = fat.fileCnt;
			strcpy((char*)&settings_stack.name[settings_stack.idx][0], (char*)fat.currentDirName);
			strcpy((char*)fat.currentDirName, settings_p[idEntry].name);
			fat.fileCnt = settings_p[idEntry].itemCnt;
			settings_p = settings_p[idEntry].next;
			settings_stack.pos[settings_stack.idx] = idEntry;
			cursor.pages = fat.fileCnt / PAGE_NUM_ITEMS;
			if(cursor.pages > 0){
				if(fat.fileCnt % PAGE_NUM_ITEMS){
					cursor.pages = cursor.pages + 1;
				}
			}
			cursor.pos = cursor.pageIdx = 0;
			settings_stack.idx++;
			LCDPrintSettingsList(settings_cur->type, 0, settings_cur->item);
			if(settings_cur->func){
				settings_cur->func(NULL);
			}
			return;
		}
	}

	if((idEntry == 0) && (fat.currentDirEntry == fat.rootDirEntry)){ // exception for settings item
		settings_mode = 1;
		settings_stack.idx = 1;
		strcpy((char*)fat.currentDirName, "Settings");
//		fat.fileCnt = sizeof(settings_root_list) / sizeof(settings_root_list[0]);
		fat.fileCnt = settings_root_list_fileCnt;
		cursor.pages = fat.fileCnt / PAGE_NUM_ITEMS;
		cursor.pos = cursor.pageIdx = 0;
		settings_p = (settings_list_typedef*)settings_root_list;
		LCDPrintSettingsList(0, 0, NULL);
		return;
	}
//	if((idEntry == 0) && (fat.currentDirEntry == fat.rootDirEntry)){ // exception for settings item
//		LCDStatusStruct.idEntry = 0;
//		LCDStatusStruct.waitExitKey = FILE_TYPE_COMPASS;
//		return;
//	}

	if((idEntry == 1) && (fat.currentDirEntry == fat.rootDirEntry)){ // exception for compass item
		LCDStatusStruct.idEntry = 1;
		LCDStatusStruct.waitExitKey = FILE_TYPE_COMPASS;
		return;
	}

	entryPointOffset = getListEntryPoint(idEntry); // リスト上にあるIDファイルエントリの先頭位置をセット

	if(!(fbuf[entryPointOffset + ATTRIBUTES] & ATTR_DIRECTORY)){ // ファイルの属性をチェック
		strncpy(fileType, (char*)&fbuf[entryPointOffset + 8], 3);
		if(strcmp(fileType, "WAV") == 0){
			LCDStatusStruct.idEntry = idEntry;
			LCDStatusStruct.waitExitKey = FILE_TYPE_WAV;
			return;
		}
		if(strcmp(fileType, "MP3") == 0){
			LCDStatusStruct.idEntry = idEntry;
			LCDStatusStruct.waitExitKey = FILE_TYPE_MP3;
			return;
		}
		if(strcmp(fileType, "MP4") == 0 || \
		   strcmp(fileType, "M4A") == 0 || \
		   strcmp(fileType, "M4P") == 0 || \
		   strcmp(fileType, "AAC") == 0){
			LCDStatusStruct.idEntry = idEntry;
			LCDStatusStruct.waitExitKey = FILE_TYPE_AAC;
			return;
		}
		if(strcmp(fileType, "JPG") == 0 || strcmp(fileType, "JPE") == 0){
			LCDStatusStruct.idEntry = idEntry;
			LCDStatusStruct.waitExitKey = FILE_TYPE_JPG;
			return;
		}
		if(strcmp(fileType, "MOV") == 0){
			LCDStatusStruct.idEntry = idEntry;
			LCDStatusStruct.waitExitKey = FILE_TYPE_MOV;
			return;
		}
		if(strcmp(fileType, "PCF") == 0){
			LCDStatusStruct.idEntry = idEntry;
			LCDStatusStruct.waitExitKey = FILE_TYPE_PCF;
			return;
		}
	} else { // ファイルの属性がディレクトリ

		extern shuffle_play_typedef shuffle_play;
		shuffle_play.flag_make_rand = 0; // clear shuffle flag

		changeDir(idEntry); // idEntryのディレクトリに移動する
		LCDPrintFileList(); // ファイルリスト表示
	}
}

void LCDPrintSettingsList(char type, int select_id, settings_item_typedef *item)
{
	int i, selected_entry;
	volatile uint16_t idEntry = cursor.pageIdx * PAGE_NUM_ITEMS;
	uint16_t step;

	LCDCheckPattern();

	LCDPutIcon(0, 0, 160, 22, menubar_160x22, menubar_160x22_alpha);

	pcf_typedef pcf;
	pcf.dst_gram_addr = (uint32_t)frame_buffer;
	pcf.pixelFormat = PCF_PIXEL_FORMAT_RGB565;
	pcf.size = 12;
	pcf.color = WHITE;
	pcf.colorShadow = BLACK;
	pcf.alphaSoftBlending = 1;
	pcf.enableShadow = 1;
	pcf_font.metrics.hSpacing = 2;

	LCD_GotoXY(5, 2);
	LCDPutString((char*)fat.currentDirName, &pcf);

	step = fat.fileCnt - idEntry;
	if(step >= PAGE_NUM_ITEMS){
		step = PAGE_NUM_ITEMS;
	}

	cly = HEIGHT_ITEM;

	if(type == SETTING_TYPE_ITEM){
		if(select_id == 0){
			selected_entry = item->selected_id + 1;
		} else {
			selected_entry = select_id;
		}

		for(i = idEntry;i < (idEntry + step);i++){
			if(strncmp(settings_p[i].name, "..", 2) == 0){
				LCDPutIcon(2, cly - 5, 22, 22, parent_arrow_22x22, parent_arrow_22x22_alpha);
			} else {
				if(i == selected_entry){
					LCDPutIcon(2, cly - 3, 22, 22, radiobutton_checked_22x22, radiobutton_22x22_alpha);
				} else {
					LCDPutIcon(2, cly - 3, 22, 22, radiobutton_unchecked_22x22, radiobutton_22x22_alpha);
				}
			}
			clx = 28;
			LCDPutString(settings_p[i].name, &pcf);

			cly += HEIGHT_ITEM;
		}
		item->selected_id = selected_entry - 1;

		if(item->func != NULL && select_id != 0){
			item->func((void*)item);
		}
	} else {
		for(i = idEntry;i < (idEntry + step);i++){
			if(strncmp(settings_p[i].name, "..", 2) == 0){
				LCDPutIcon(2, cly - 5, 22, 22, parent_arrow_22x22, parent_arrow_22x22_alpha);
			} else {
				if(settings_p[i].icon != NULL){
					LCDPutIcon(2, cly - 5, 22, 22, settings_p[i].icon->data, settings_p[i].icon->alpha);
				} else {
					LCDPutIcon(2, cly - 5, 22, 22, select_22x22, select_22x22_alpha);
				}
			}
			clx = 28;
			LCDPutString(settings_p[i].name, &pcf);

			cly += HEIGHT_ITEM;
		}
	}

	if(cursor.pages > 1){
		// Scroll Bar
		int scrollHeight = 98 / cursor.pages, \
			scrollStartPos = (98 - 14) * (float)cursor.pageIdx / (float)cursor.pages + 25, \
		    height = scrollHeight - 14;
		height = height < 0 ? 0 : height;
		scrollStartPos = cursor.pageIdx >= (cursor.pages - 1) ? (98 - height - 14) + 25 : scrollStartPos;

		LCDPutIcon(151, 25, 6, 98, scrollbar_6x98, scrollbar_6x98_alpha); // scroll background
		LCDPutIcon(151, scrollStartPos, 6, 7, scrollbar_top_6x7, scrollbar_top_6x7_alpha);
		for(i = scrollStartPos + 7;i < ((scrollStartPos + 7) + height);i++){
			LCDPutIcon(151, i, 6, 1, scrollbar_hline_6x1, scrollbar_hline_6x1_alpha);
		}
		LCDPutIcon(151, i, 6, 7, scrollbar_bottom_6x7, scrollbar_bottom_6x7_alpha);
	}


	LCDStoreCursorBar(cursor.pos);
	LCDSelectCursorBar(cursor.pos);
//	LCDBackLightTimerInit();
//	LCD_FRAME_BUFFER_Transmit(LCD_DMA_TRANSMIT_BLOCKING);
}


void LCDPrintFileList()
{
	int i;
	uint32_t var32;
	volatile uint16_t idEntry = cursor.pageIdx * PAGE_NUM_ITEMS;
	uint16_t entryPointOffset, step;

	uint8_t pLFNname[80];
	char fileNameStr[13], fileSizeStr[13], fileTypeStr[4];


	memcpy((void*)frame_buffer, (void*)filer_bgimg_160x128, (size_t)_sizeof_filer_bgimg_160x128);

	LCDPutIcon(0, 0, 160, 22, menubar_160x22, menubar_160x22_alpha);

//	LCD_Clear_Frame_Buffer(frame_buffer, BLACK);

	pcf_typedef pcf;
	//  pcf.dst_gram_addr = SDRAM_FRAMEBUFF + (320 * 240 * sizeof(uint16_t)) * 2;
	pcf.dst_gram_addr = (uint32_t)frame_buffer;
	pcf.pixelFormat = PCF_PIXEL_FORMAT_RGB565;
	pcf.size = 12;
	pcf.color = WHITE;
	pcf.colorShadow = BLACK;
	pcf.alphaSoftBlending = 1;
	pcf.enableShadow = 1;
	pcf_font.metrics.hSpacing = 2;

	LCD_GotoXY(5, 1);
	LCDPutStringLFN(5, 140, 1, (uint8_t*)fat.currentDirName, &pcf); // HEADER_COLOR


	step = fat.fileCnt - idEntry;
	if(step >= PAGE_NUM_ITEMS){
		step = PAGE_NUM_ITEMS;
	}

	cly = HEIGHT_ITEM;

	extern volatile card_info_typedef cardInfo;
	if(!cardInfo.ready){
		LCDPutIcon(2, cly - 3, 22, 22, settings_22x22, settings_22x22_alpha);
		clx = 28;
		LCDPutString("Settings", &pcf); // ARCHIVE_COLOR
		cly += HEIGHT_ITEM;

		LCDPutIcon(2, cly - 3, 22, 22, compass_22x22, compass_22x22_alpha);
		clx = 28;
		LCDPutString("Compass", &pcf); // ARCHIVE_COLOR

		fat.fileCnt = 2;

		LCDStoreCursorBar(cursor.pos);
		LCDSelectCursorBar(cursor.pos);

		LCD_FRAME_BUFFER_Transmit(LCD_DMA_TRANSMIT_COMPBLOCKING);
		return;
	}

	for(i = idEntry;i < (idEntry + step);i++){
		if((i == 0) && (fat.currentDirEntry == fat.rootDirEntry)){ // exception for settings
			LCDPutIcon(2, cly - 3, 22, 22, settings_22x22, settings_22x22_alpha);
			clx = 28;
			LCDPutString("Settings", &pcf); // ARCHIVE_COLOR
			cly += HEIGHT_ITEM;
			continue;
		}
		if((i == 1) && (fat.currentDirEntry == fat.rootDirEntry)){ // exception for compass
			LCDPutIcon(2, cly - 3, 22, 22, compass_22x22, compass_22x22_alpha);
			clx = 28;
			LCDPutString("Compass", &pcf); // ARCHIVE_COLOR
			cly += HEIGHT_ITEM;
			continue;
		}


		entryPointOffset = getListEntryPoint(i); // リスト上にあるIDファイルエントリの先頭位置をセット

		memset(fileNameStr, '\0', sizeof(fileNameStr));
		memset(fileSizeStr, '\0', sizeof(fileSizeStr));
		memset(fileTypeStr, '\0', sizeof(fileTypeStr));

		strncpy(fileNameStr, (char*)&fbuf[entryPointOffset], 8); // 8文字ファイル名をコピー
		strtok(fileNameStr, " "); // スペースがあればNULLに置き換える

		if(fbuf[entryPointOffset + NT_Reserved] & NT_U2L_NAME){ // Name Upper to Lower
			fileNameStr[0] = tolower((int)fileNameStr[0]);
			fileNameStr[1] = tolower((int)fileNameStr[1]);
			fileNameStr[2] = tolower((int)fileNameStr[2]);
			fileNameStr[3] = tolower((int)fileNameStr[3]);
			fileNameStr[4] = tolower((int)fileNameStr[4]);
			fileNameStr[5] = tolower((int)fileNameStr[5]);
			fileNameStr[6] = tolower((int)fileNameStr[6]);
			fileNameStr[7] = tolower((int)fileNameStr[7]);
		}

		var32 = *((uint32_t*)&fbuf[entryPointOffset + FILESIZE]); // ファイルサイズ取得

		if(var32 >= 1000000){
			var32 /= 1000000;
			SPRINTF(fileSizeStr, "%dMB", (int)var32);
		} else if(var32 >= 1000){
			var32 /= 1000;
			SPRINTF(fileSizeStr, "%dKB", (int)var32);
		} else {
			SPRINTF(fileSizeStr, "%dB", (int)var32);
		}

		if(!(fbuf[entryPointOffset + ATTRIBUTES] & ATTR_DIRECTORY)){
			if(fbuf[entryPointOffset + 8] != 0x20){
				strncpy(fileTypeStr, (char*)&fbuf[entryPointOffset + 8], 3);
				//strcat(fileNameStr, ".");
				//strcat(fileNameStr, fileTypeStr);
			} else {
				strcpy(fileTypeStr, "---");
			}

			if(strcmp(fileTypeStr, "MOV") == 0){
				LCDPutIcon(2, cly - 5, 22, 22, movie_22x22, movie_22x22_alpha);
			} else if(strcmp(fileTypeStr, "MP3") == 0 || \
					  strcmp(fileTypeStr, "AAC") == 0 || \
					  strcmp(fileTypeStr, "MP4") == 0 || \
					  strcmp(fileTypeStr, "M4A") == 0 || \
					  strcmp(fileTypeStr, "M4P") == 0 || \
					  strcmp(fileTypeStr, "WAV") == 0
					) {
				LCDPutIcon(2, cly - 5, 22, 22, onpu_22x22, onpu_22x22_alpha);
			} else if(strcmp(fileTypeStr, "PCF") == 0) {
				LCDPutIcon(2, cly - 5, 22, 22, font_22x22, font_22x22_alpha);
			} else if(strcmp(fileTypeStr, "JPG") == 0 || strcmp(fileTypeStr, "JPE") == 0) {
				LCDPutIcon(2, cly - 5, 22, 22, jpeg_22x22, jpeg_22x22_alpha);
			} else {
				LCDPutIcon(2, cly - 5, 22, 22, archive_22x22, archive_22x22_alpha);
			}
		} else {
			strcat(fileTypeStr, "DIR");
			fileSizeStr[0] = '\0';
			if(strncmp(fileNameStr, "..", 2) == 0){
				LCDPutIcon(2, cly - 5, 22, 22, parent_arrow_22x22, parent_arrow_22x22_alpha);
			} else {
				LCDPutIcon(2, cly - 5, 22, 22, folder_22x22, folder_22x22_alpha);
			}
		}

		clx = 28;
		if(!setLFNname(pLFNname, i, LFN_WITHOUT_EXTENSION, sizeof(pLFNname))){ // エントリがSFNの場合
			LCDPutString(fileNameStr, &pcf);  // color
		} else { // LFNの場合
			LCDPutStringLFN(clx, 130, 1, pLFNname, &pcf); // color
		}

//		clx = 130;
//		LCDPutString(fileTypeStr, &pcf); // color

//		clx = 130;
//		LCDPutString(fileSizeStr, &pcf);  // color

		cly += HEIGHT_ITEM;
	}

	if(cursor.pages > 1){
		// Scroll Bar
		int scrollHeight = 98 / cursor.pages, \
			scrollStartPos = (98 - 14) * (float)cursor.pageIdx / (float)cursor.pages + 25, \
		    height = scrollHeight - 14;
		height = height < 0 ? 0 : height;
		scrollStartPos = cursor.pageIdx >= (cursor.pages - 1) ? (98 - height - 14) + 25 : scrollStartPos;

		LCDPutIcon(151, 25, 6, 98, scrollbar_6x98, scrollbar_6x98_alpha); // scroll background
		LCDPutIcon(151, scrollStartPos, 6, 7, scrollbar_top_6x7, scrollbar_top_6x7_alpha);
		for(i = scrollStartPos + 7;i < ((scrollStartPos + 7) + height);i++){
			LCDPutIcon(151, i, 6, 1, scrollbar_hline_6x1, scrollbar_hline_6x1_alpha);
		}
		LCDPutIcon(151, i, 6, 7, scrollbar_bottom_6x7, scrollbar_bottom_6x7_alpha);
	}

	LCDStoreCursorBar(cursor.pos);
	LCDSelectCursorBar(cursor.pos);

//	LCD_FRAME_BUFFER_Transmit(LCD_DMA_TRANSMIT_COMPBLOCKING);
}


extern MY_FILE *g_pInFile;
//extern unsigned char pjpeg_need_bytes_callback(unsigned char* pBuf, unsigned char buf_size, unsigned char *pBytes_actually_read, void *pCallback_data);
//extern uint g_pInFileOfs, g_pInFileSize;


int LCDArtWork(MY_FILE *infile, uint8_t *width, uint8_t *height)
{
	int comps, i;
	pjpeg_scan_type_t scan_type;
	const char* p = "?";
	uint8_t *pImage;
	int reduce = 0;

	uint8_t checkHeader[512];

	if(infile->clusterOrg == 0){
		return 0;
	}

   if (!infile)
   {
	   debug.printf("\r\nfopen error - picojpeg");
	   return 0;
   }

	my_fread(checkHeader, 1, 512, infile);

	for(i = 0;i < 511;i++){
		if((checkHeader[i] == 0xff) && (checkHeader[i + 1] == 0xd8)){
			break;
		}
	}

	if(i >= 511){
		debug.printf("\r\nArtWork: doesn't start with 0xffd8");
		return 0;
	}
	my_fseek(infile, -512 + i, SEEK_CUR);
/*
	my_fread(checkHeader, 1, 512, infile);
	for(i = 0;i < 512;i++)
	{
		if((i % 16) == 0){
			debug.printf("\r\n");
		}
		debug.printf("%02x ", checkHeader[i]);
	}
	my_fseek(infile, -512, SEEK_CUR);
*/
   g_pInFile = infile;
   pImage = pjpeg_load_from_file2(width, height, &comps, &scan_type, reduce);

   if (!pImage)
   {
      debug.printf("\r\nFailed loading source image!");
      return 0;
   }

   debug.printf("\r\nWidth: %i, Height: %i, Comps: %i", *width, *height, comps);

   switch (scan_type)
   {
      case PJPG_GRAYSCALE: p = "GRAYSCALE"; break;
      case PJPG_YH1V1: p = "H1V1"; break;
      case PJPG_YH2V1: p = "H2V1"; break;
      case PJPG_YH1V2: p = "H1V2"; break;
      case PJPG_YH2V2: p = "H2V2"; break;
   }
   debug.printf("\r\nScan type: %s", p);

   return 1;
}

void LCDDrawReflection(uint16_t x, uint16_t y, uint16_t t, uint16_t width, uint16_t height, uint16_t *p_img)
{
	pixel_fmt_typedef pixel_fg, pixel_bg;
	int i, j, xx, yy;
	float alpha, alpha_ratio;

	yy = height - 1;
	for(j = y + height - 1;j >= y;j--){
		alpha = ((float)t * 2.8f) / (float)(height);
		alpha = alpha <= 1.0f ? alpha : 1.0f;
		alpha_ratio = 1.0f - alpha;
		xx = 0;
		for(i = x;i < x + width;i++){
			pixel_fg.color.d16 = get_image_data(p_img, width, xx, yy);

			// Foreground Image
			pixel_fg.color.R *= alpha_ratio;
			pixel_fg.color.G *= alpha_ratio;
			pixel_fg.color.B *= alpha_ratio;

			pixel_bg.color.d16 = get_image_data(frame_buffer, LCD_WIDTH, i, y + height + t);

			// Background Image
			pixel_bg.color.R *= (1.0f - alpha_ratio);
			pixel_bg.color.G *= (1.0f - alpha_ratio);
			pixel_bg.color.B *= (1.0f - alpha_ratio);

			// Add colors
			pixel_fg.color.R += pixel_bg.color.R;
			pixel_fg.color.G += pixel_bg.color.G;
			pixel_fg.color.B += pixel_bg.color.B;

			set_image_data(frame_buffer, LCD_WIDTH, i, y + height + t, pixel_fg.color.d16);
			xx++;
		}
		t++;
		yy--;
	}
}


void DMA_ProgressBar_Start()
{
	TimProgressBar.Instance = TIM_PROGRESS_BAR;
	HAL_TIM_Base_DeInit(&TimProgressBar);
	TimProgressBar.Init.Period = 500 - 1; // 50ms
	TimProgressBar.Init.Prescaler = (SystemCoreClock / 10000UL) - 1; // 100us
	TimProgressBar.Init.CounterMode = TIM_COUNTERMODE_UP;
	TimProgressBar.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	HAL_TIM_Base_Init(&TimProgressBar);

	while(SpiLcdHandle.State != HAL_SPI_STATE_READY){};

    LCD_SetRegion(71, 55, 71 + 16 - 1, 55 + 16 - 1);
    LCD_SetGramAddr(71, 55);

	HAL_TIM_Base_Start_IT(&TimProgressBar);
    HAL_NVIC_EnableIRQ(TIM_PROGRESS_BAR_IRQn);
}

void DMA_ProgressBar_Stop()
{
	while(SpiLcdHandle.State != HAL_SPI_STATE_READY){};

    HAL_NVIC_DisableIRQ(TIM_PROGRESS_BAR_IRQn);
	HAL_TIM_Base_Stop_IT(&TimProgressBar);
    LCD_SetRegion(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
}

void LCDCheckPattern(){
	int i, j, k;
	uint16_t *p16;
	p16 = frame_buffer;

	for(k = 0;k < 4;k++){
		for(j = 0;j < 5 * 16;j++){
			for(i = 0;i < 16;i++){
				*p16++ = 0x0000;
			}
			for(i = 0;i < 16;i++){
				*p16++ = 0xA800;
			}
		}
		for(j = 0;j < 5 * 16;j++){
			for(i = 0;i < 16;i++){
				*p16++ = 0xA800;
			}
			for(i = 0;i < 16;i++){
				*p16++ = 0x0000;
			}
		}
	}
}

void LCDSetPWMValue(int val)
{
	extern TIM_HandleTypeDef TimLcdPwm;

	TIM_OC_InitTypeDef sConfig;

	sConfig.OCMode = TIM_OCMODE_PWM1;
	sConfig.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfig.OCFastMode = TIM_OCFAST_DISABLE;

	sConfig.Pulse = (float)val / 100.0f * LCD_PWM_PERIOD - 1;
	HAL_TIM_PWM_ConfigChannel(&TimLcdPwm, &sConfig, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&TimLcdPwm, TIM_CHANNEL_2);
}

