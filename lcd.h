/*
 * lcd.h
 *
 *  Created on: 2014/03/23
 *      Author: masayuki
 */

#ifndef LCD_H_
#define LCD_H_

#include "stm32f4xx_hal.h"

#include "pcf_font.h"
#include "icon.h"
#include "settings.h"


#define LCD_WIDTH  160
#define LCD_HEIGHT 128

#define ARCHIVE_COLOR WHITE
#define DIR_COLOR WHITE
#define HEADER_COLOR WHITE


#define PAGE_NUM_ITEMS 5
#define HEIGHT_ITEM 22 // 24px

#define SPRINTF sprintf

#define ART_WORK_X 115
#define ART_WORK_Y 43
#define ART_WORK_SIZE   40

#define TIME_X 24
#define TIME_Y 114
#define TIME_REMAIN_X 108
#define TIME_REMAIN_Y 114

#define UI_POS_X 18
#define UI_POS_Y 99


#define FRAME_BUFFER_SIZE (160 * 128 * sizeof(uint16_t))


typedef struct
{
	uint16_t pos, pages, pageIdx;
}cursor_typedef;

extern cursor_typedef cursor;

typedef struct
{
	int waitExitKey;
	int16_t idEntry;
}LCDStatusStruct_typedef;

typedef struct{
	int16_t curTime, prevTime;
	union{
		uint8_t d8;
		struct {
			uint8_t dimLight : 1;
			uint8_t enable : 1;
			uint8_t stop_mode : 1;
			uint8_t wakeup : 1;
			uint8_t reserved : 4;
		};
	}flags;
}time_typedef;

extern volatile time_typedef sleep_time;

#define red	        0xf800
#define green		0x07e0
#define blue		0x001f
#define skyblue     0x7e9f
#define white		0xffff
//#define gray		0x8c51
#define gray		0x39c7
#define yellow		0xFFE0
#define cyan	 	0x07FF
#define purple		0xF81F
#define black		0x0000

typedef enum
{
	RED,
	GREEN,
	BLUE,
	SKYBLUE,
	WHITE,
	GRAY,
	YELLOW,
	CYAN,
	PURPLE,
	BLACK
}colors;

typedef enum
{
	FILE_TYPE_WAV = 2,
	FILE_TYPE_MP3,
	FILE_TYPE_AAC,
	FILE_TYPE_JPG,
	FILE_TYPE_MOV,
	FILE_TYPE_PCF,
	FILE_TYPE_COMPASS
}selected_file_type;


enum
{
	LCD_DMA_TRANSMIT_NOBLOCKING = 0,
	LCD_DMA_TRANSMIT_BLOCKING,
	LCD_DMA_TRANSMIT_COMPBLOCKING
};


typedef struct{
	union{
		uint16_t d16;
		struct {
			uint16_t B : 5;
			uint16_t G : 6;
			uint16_t R : 5;
		};
	}color;
}pixel_fmt_typedef;


static const uint16_t colorc[] = {red, green, blue, skyblue, white, gray, yellow, cyan, purple, black};

extern uint16_t clx, cly;
extern uint16_t frame_buffer[160 * 128];
extern LCDStatusStruct_typedef LCDStatusStruct;


volatile struct
{
	void (*putChar)(uint16_t asc, void *cp);
	void (*putWideChar)(uint16_t code, void *cp);
	uint16_t (*getCharLength)(uint16_t code, uint16_t font_width);
}LCD_FUNC;


/* SPI_LCD_RS_PIN_BB */
/* --- GPIOC ODR Register ---*/
/* Alias word address of GPIOC ODR 5 bit */
#define GPIOC_ODR_OFFSET       (GPIOC_BASE - PERIPH_BASE + 0x14)
#define GPIOC_ODR_5_BitNumber  0x05
#define GPIOC_ODR_5_BB         (*(__IO uint32_t *)(PERIPH_BB_BASE + (GPIOC_ODR_OFFSET * 32) + (GPIOC_ODR_5_BitNumber * 4)))
#define SPI_LCD_RS_PIN_BB GPIOC_ODR_5_BB

#define SPI_LCD_RS_PIN_ASSERT 	(SPI_LCD_RS_PIN_BB = 0)
#define SPI_LCD_RS_PIN_DEASSERT (SPI_LCD_RS_PIN_BB = 1)

/* SPI_LCD_NSS_PIN_BB */
/* --- GPIOA ODR Register ---*/
/* Alias word address of GPIOA ODR 15 bit */
#define GPIOA_ODR_OFFSET       (GPIOA_BASE + 0x14)
#define GPIOA_ODR_15_BitNumber  0x0f
#define GPIOA_ODR_15_BB         (*(__IO uint32_t *)(PERIPH_BB_BASE + (GPIOA_ODR_OFFSET * 32) + (GPIOA_ODR_15_BitNumber * 4)))
#define SPI_LCD_NSS_PIN_BB GPIOA_ODR_15_BB

#define SPI_LCD_NSS_PIN_ASSERT 	(SPI_LCD_NSS_PIN_BB = 0)
#define SPI_LCD_NSS_PIN_DEASSERT (SPI_LCD_NSS_PIN_BB = 1)

extern void LCD_CMD(uint16_t index);
extern void LCD_DATA(uint16_t data);
extern void LCD_FRAME_BUFFER_Transmit(int8_t blocking);
extern void LCD_SetGramAddr(uint16_t x_start, uint16_t y_start);
extern void LCD_Clear(uint16_t Color);
extern void LCD_Init();
extern void LCD_DrawSquare(int x, int y, int width, int height, colors color);
extern void LCD_SetRegion(uint16_t x_start,uint16_t y_start,uint16_t x_end,uint16_t y_end);

extern void LCD_GotoXY(int x, int y);

extern void LCDPutString(const char *str, pcf_typedef *pcf);
extern void LCDRotateImage(int16_t sx, int16_t sy, uint16_t width, uint16_t height, const uint16_t *p_img, uint16_t bgColor, int16_t angle, uint8_t enableAntiAlias);
extern void LCDPrintFileList();
//extern void LCDPutString(const char *str, pcf_typedef *pcf);
extern void LCDSetPWMValue(int val);


extern uint16_t LCDGetStringUTF8PixelLength(uint8_t *s, uint16_t font_width);
extern uint16_t LCDPutStringUTF8(uint16_t startPosX, uint16_t endPosX, uint8_t lineCnt, uint8_t *s, pcf_typedef *pcf);
extern uint16_t LCDGetStringLFNPixelLength(void *s, uint16_t font_width);
extern uint16_t LCDPutStringLFN(uint16_t startPosX, uint16_t endPosX, uint8_t lineCnt, uint8_t *s, pcf_typedef *pcf);
extern uint16_t LCDPutStringSJISN(uint16_t startPosX, uint16_t endPosX, uint8_t lineCnt, uint8_t *s, pcf_typedef *pcf);
extern uint16_t LCDGetStringSJISPixelLength(uint8_t *s, uint16_t font_width);
extern void LCDPutStringSJIS(uint8_t *s, pcf_typedef *pcf);

extern int LCDArtWork(MY_FILE *infile, uint8_t *width, uint8_t *height);
extern void LCDFadeIn(uint8_t step, const uint16_t *bgimg);
extern void LCDFadeOut(uint8_t step);
extern void LCDPutIcon(int startPosX, int startPosY, int width, int height, const uint16_t *d, const uint8_t *a);
extern void LCDDrawReflection(uint16_t x, uint16_t y, uint16_t t, uint16_t width, uint16_t height, uint16_t *p_img);
extern void LCDPutBuffToBgImg(int startPosX, int startPosY, int width, int height, uint16_t *p);
extern void LCDStoreBgImgToBuff(int startPosX, int startPosY, int width, int height, uint16_t *p);
extern void LCD_StoreFrameBufferToFFTSquares(drawBuff_typedef *drawBuff);
extern void LCDPutBgImgToFramebuffer(int startPosX, int startPosY, int width, int height, const uint16_t *p);
extern void LCDPutBuffToBgImg(int startPosX, int startPosY, int width, int height, uint16_t *p);

extern void LCDPutCursorBar(int curPos);
extern void LCDStoreCursorBar(int curPos);
extern void LCDCursorUp();
extern void LCDCursorDown();
extern void LCDCursorEnter();

extern void DMA_ProgressBar_Start();
extern void DMA_ProgressBar_Stop();
extern void LCD_DrawSquare_DB(int x, int y, int width, int height, colors color);
extern void LCDZoomImageToBuff(uint16_t bufWidth, uint16_t bufHeight, uint16_t *p_buff, uint16_t srcWidth, uint16_t srcHeight, uint16_t *p_img, uint16_t bgColor, uint16_t zoom, uint8_t enableAntiAlias);
extern void LCDZoomImage(int16_t offsetX, int16_t offsetY, uint16_t width, uint16_t height, uint16_t *p_img, uint16_t bgColor, uint16_t zoom, uint8_t enableAntiAlias);
extern void LCDXor();
extern void LCDPrintSettingsList(char type, int select_id, settings_item_typedef *item);
extern void LCDCheckPattern();


#endif /* LCD_H_ */
