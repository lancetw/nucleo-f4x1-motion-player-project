/*
 * sound.h
 *
 *  Created on: 2011/03/12
 *      Author: Tonsuke
 */

#ifndef SOUND_H_
#define SOUND_H_

#include "stm32f4xx_hal_conf.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "fat.h"
#include "usart.h"
#include "icon.h"

#define MUSIC_INFO_POS_Y 3

/*
typedef struct {
	char headStrRIFF[4];
	uint32_t fileSize;
	char headStrWAVE[4], chunkfmt[4];
	uint32_t fmtChunkSize;
	uint16_t formatID, numChannel;
	uint32_t sampleRate, dataSpeed;
	uint16_t blockSize, bitPerSample;
	char chunkdata[4];
	uint32_t bytesWaveData;
}WAVEFormatStruct;
*/

typedef struct {
	uint16_t formatID, numChannel;
	uint32_t sampleRate, dataSpeed;
	uint16_t blockSize, bitPerSample;
}WAVEFormatStruct;


typedef struct {
	char headStrRIFF[4];
	uint32_t fileSize;
	char headStrWAVE[4];
}WAVEFormatHeaderStruct;


typedef struct {
	char chunkfmt[4];
	uint32_t chunkSize;
}WAVEFormatChunkStruct;


typedef struct {
	uint8_t *buff, comp;
	uint32_t bufferSize,\
			 sound_reads, \
			 contentSize;
	void (*func)(void);
	MY_FILE *fp;
} dac_intr_typedef;

extern dac_intr_typedef dac_intr;

typedef struct{
	int8_t flag_make_rand, play_continuous, mode_changed, initial_mode;
	uint8_t *pRandIdEntry;
} shuffle_play_typedef;

extern shuffle_play_typedef shuffle_play;


typedef struct{
	int currentTotalSec;
	uint8_t update, \
			comp;
}music_play_typedef;

typedef struct {
	int enable, type, tic, cnt, shifts;
	int16_t sLeft, sRight;
}sound_fade_typedef;


#define RET_PLAY_NORM  0
#define RET_PLAY_STOP -1
#define RET_PLAY_NEXT -2
#define RET_PLAY_PREV -3
#define RET_PLAY_INVALID -99

extern music_play_typedef music_play;


typedef union
{
	uint8_t d8;
	struct
	{
		uint8_t navigation_loop_mode : 3;
		uint8_t mute: 1;
		uint8_t reserved: 4;
	}b;
}music_control_typedef;

extern music_control_typedef music_control;

//uint8_t SOUND_BUFFER[9216]; // interruption occurs 50msec each

#define UPPER_OF_WORD(x) (x >> 16)
#define LOWER_OF_WORD(x) (x & 0x0000ffff)


#define NAV_ONE_PLAY_EXIT         0
#define NAV_PLAY_ENTIRE           1
#define NAV_INFINITE_PLAY_ENTIRE  2
#define NAV_INFINITE_ONE_PLAY     3
#define NAV_SHUFFLE_PLAY          4


//extern void DAC_Buffer_Process_Stereo_S16bit();
//extern void DAC_Buffer_Process_Mono_U8bit();
extern char* setStrSec(char *timeStr, int totalSec);
extern int PlaySound(int id);
extern int PlayMusic(int id);
//extern void SOUNDDMAConf(void *dacOutputReg, size_t blockSize, size_t periphSize);
//extern void SOUNDInitDAC(uint32_t sampleRate);

//extern void musicTouch();
//extern int musicPause();

#define DRAW_PAUSE_ICON() do{LCDPutBuffToBgImg(drawBuff->play_pause.x, drawBuff->play_pause.y, \
							 drawBuff->play_pause.width, drawBuff->play_pause.height, drawBuff->play_pause.p); \
							 LCDPutIcon(drawBuff->play_pause.x, drawBuff->play_pause.y, 12, 12, pause_icon_12x12, pause_icon_12x12_alpha);}while(0)

#define DRAW_PLAY_ICON() do{LCDPutBuffToBgImg(drawBuff->play_pause.x, drawBuff->play_pause.y, \
							 drawBuff->play_pause.width, drawBuff->play_pause.height, drawBuff->play_pause.p); \
							 LCDPutIcon(drawBuff->play_pause.x, drawBuff->play_pause.y, 12, 12, play_icon_12x12, play_icon_12x12_alpha);}while(0)

#define DRAW_MUTE_ICON() do{LCDPutBuffToBgImg(drawBuff->play_pause.x, drawBuff->play_pause.y, \
							 drawBuff->play_pause.width, drawBuff->play_pause.height, drawBuff->play_pause.p); \
							 LCDPutIcon(drawBuff->play_pause.x, drawBuff->play_pause.y, 12, 12, mute_icon_12x12, mute_icon_12x12_alpha);}while(0)


#define	DRAW_SEEK_CIRCLE(pos, seek_icon) do{LCDPutBuffToBgImg(prevX, drawBuff->posision.y, \
										drawBuff->posision.width, drawBuff->posision.height, drawBuff->posision.p); \
										curX = (126 - drawBuff->posision.x) * pos + drawBuff->posision.x; \
										LCDStoreBgImgToBuff(curX, drawBuff->posision.y, drawBuff->posision.width, drawBuff->posision.height, drawBuff->posision.p); \
										LCDPutIcon(curX, drawBuff->posision.y, drawBuff->posision.width, drawBuff->posision.height, seek_icon, seek_circle_12x12_alpha); \
										prevX = curX;}while(0)

#define DRAW_TIME_STR() do{LCD_GotoXY(TIME_X, TIME_Y); \
						   LCDPutBgImgToFramebuffer(TIME_X - 1, TIME_Y, 50, 13, music_bgimg_160x128); \
						   LCDPutString(setStrSec(timeStr, time), &pcf);}while(0)

#define DRAW_REMAIN_TIME_STR() do{LCD_GotoXY(abs(time - duration) > (99 * 60 + 99) ? TIME_REMAIN_X - 6 : TIME_REMAIN_X, TIME_REMAIN_Y); \
								  LCDPutBgImgToFramebuffer(TIME_REMAIN_X - 7, TIME_REMAIN_Y, 60, 13, music_bgimg_160x128); \
								  LCDPutString(setStrSec(timeStr, time - duration), &pcf);}while(0)

/* SOUND_DMA_HALF_TRANS_BB */
/* --- DMA1 HISR Register ---*/
/* Alias word address of DMA1 HISR DMA_HISR_HTIF4 bit */
#define DMA1_HISR_OFFSET       (DMA1_BASE + 0x4)
#define DMA1_HISR_DMA_HISR_HTIF4_BitNumber  4
#define DMA1_HISR_DMA_HISR_HTIF4_BB         (*(__IO uint32_t *)(PERIPH_BB_BASE + (DMA1_HISR_OFFSET * 32) + (DMA1_HISR_DMA_HISR_HTIF4_BitNumber * 4)))
#define SOUND_DMA_HALF_TRANS_BB DMA1_HISR_DMA_HISR_HTIF4_BB


/* SOUND_DMA_CLEAR_HALF_TRANS_BB */
/* --- DMA1 HIFCR Register ---*/
/* Alias word address of DMA1 HIFCR DMA_HIFCR_CHTIF4 bit */
#define DMA1_HIFCR_OFFSET       (DMA1_BASE + 0xC)
#define DMA1_HIFCR_DMA_HIFCR_CHTIF4_BitNumber  4
#define DMA1_HIFCR_DMA_HIFCR_CHTIF4_BB         (*(__IO uint32_t *)(PERIPH_BB_BASE + (DMA1_HIFCR_OFFSET * 32) + (DMA1_HIFCR_DMA_HIFCR_CHTIF4_BitNumber * 4)))
#define SOUND_DMA_CLEAR_HALF_TRANS_BB DMA1_HIFCR_DMA_HIFCR_CHTIF4_BB

/* SOUND_DMA_FULL_TRANS_BB */
/* --- DMA1 LISR Register ---*/
/* Alias word address of DMA1 LISR DMA_LISR_TCIF1 bit */
#define DMA1_HISR_OFFSET       (DMA1_BASE + 0x4)
#define DMA1_HISR_DMA_HISR_TCIF4_BitNumber  5
#define DMA1_HISR_DMA_HISR_TCIF4_BB         (*(__IO uint32_t *)(PERIPH_BB_BASE + (DMA1_HISR_OFFSET * 32) + (DMA1_HISR_DMA_HISR_TCIF4_BitNumber * 4)))
#define SOUND_DMA_FULL_TRANS_BB DMA1_HISR_DMA_HISR_TCIF4_BB

/* SOUND_DMA_CLEAR_FULL_TRANS_BB */
/* --- DMA1 LIFCR Register ---*/
/* Alias word address of DMA1 LIFCR DMA_LIFCR_CTCIF1 bit */
#define DMA1_HIFCR_OFFSET       (DMA1_BASE + 0xC)
#define DMA1_HIFCR_DMA_HIFCR_CTCIF5_BitNumber  5
#define DMA1_HIFCR_DMA_HIFCR_CTCIF5_BB         (*(__IO uint32_t *)(PERIPH_BB_BASE + (DMA1_HIFCR_OFFSET * 32) + (DMA1_HIFCR_DMA_HIFCR_CTCIF5_BitNumber * 4)))
#define SOUND_DMA_CLEAR_FULL_TRANS_BB DMA1_HIFCR_DMA_HIFCR_CTCIF5_BB

#define AUDIO_I2S_IRQ_ENABLE()   HAL_NVIC_EnableIRQ(AUDIO_I2Sx_DMAx_IRQ);
#define AUDIO_I2S_IRQ_DISABLE()  HAL_NVIC_DisableIRQ(AUDIO_I2Sx_DMAx_IRQ);


#endif /* SOUND_H_ */
