/*
 * mjpeg.c
 *
 *  Created on: 2011/07/10
 *      Author: Tonsuke
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "lcd.h"
#include "sound.h"
#include "fat.h"
#include "usart.h"
#include "pcf_font.h"
#include "main.h"

#include "wm8731.h"
#include "movie.h"


extern int8_t vol;
extern I2S_HandleTypeDef haudio_i2s;
extern SPI_HandleTypeDef SpiLcdHandle;
extern uint16_t cursorRAM[LCD_WIDTH * 13];

#define FUNC_VIDEO_BGIMG LCD_Clear(colorc[BLACK]);


extern LCDStatusStruct_typedef LCDStatusStruct;

typedef struct{
	int x, y, width, height;
	uint16_t p[30 * 30];
}pic_arrow_buf_typedef;


__attribute__( ( always_inline ) ) static __INLINE size_t getAtomSize(void* atom)
{
/*
	ret =  *(uint8_t*)(atom + 0) << 24;
	ret |= *(uint8_t*)(atom + 1) << 16;
	ret |= *(uint8_t*)(atom + 2) << 8;
	ret |= *(uint8_t*)(atom + 3);
*/
	return __REV(*(uint32_t*)atom);
}

__attribute__( ( always_inline ) ) static __INLINE size_t getSampleSize(void* atombuf, int bytes, MY_FILE *fp)
{
	my_fread(atombuf, 1, bytes, fp);
	return getAtomSize(atombuf);
}


uint32_t b2l(void* val, size_t t){
	uint32_t ret = 0;
	size_t tc = t;

	for(;t > 0;t--){
		ret |= *(uint8_t*)(val + tc - t) << 8 * (t - 1);
	}

	return ret;
}


int collectAtoms(MY_FILE *fp, size_t parentAtomSize, size_t child)
{
	int index, is;
	size_t atomSize, totalAtomSize = 0;
	uint32_t timeScale = 0, duration = 0;
	uint8_t atombuf[512];

	MY_FILE fp_tmp;

	do{
		memset(atombuf, '\0', sizeof(atombuf));
		atomSize = getSampleSize(atombuf, 8, fp);

		debug.printf("\r\n");
		for(is = child;is > 0;is--){
			debug.printf(" ");
		}
		debug.printf("%s %d", (char*)&atombuf[4], atomSize);


		for(index = 0;index < ATOM_ITEMS;index++){
			if(strncmp((char*)&atombuf[4], (char*)&atomTypeString[index][0], 4) == 0){
				if(atomHasChild[index]){
					debug.printf(" +");
				}
				break;
			}
		}

		if(index >= ATOM_ITEMS){ // unrecognized atom
			if(atomSize == 0){
				return -1;
			}
			goto NEXT;
		}

		memcpy((void*)&fp_tmp, (void*)fp, sizeof(MY_FILE));

		switch(index){
		case MDHD:
			my_fseek(fp, 12, SEEK_CUR); // skip ver/flag  creationtime modificationtime

			timeScale = getSampleSize(atombuf, 4, fp); // time scale
			duration = getSampleSize(atombuf, 4, fp); // duration

			break;
		case HDLR:
			my_fseek(fp, 4, SEEK_CUR); // skip flag ver
			my_fread(atombuf, 1, 4, fp); // Component type
			my_fread(atombuf, 1, 4, fp); // Component subtype
			if(!strncmp((char*)atombuf, (const char*)"soun", 4)){
				media.sound.flag.process = 1;
				media.sound.timeScale = timeScale;
				media.sound.duration = duration;
			}
			if(!strncmp((char*)atombuf, (const char*)"vide", 4)){
				media.video.flag.process = 1;
				media.video.timeScale = timeScale;
				media.video.duration = duration;
			}
			break;
		case TKHD:
			my_fseek(fp, 74, SEEK_CUR); // skip till width, height data
			my_fread(atombuf, 1, 4, fp); // width
			if(getAtomSize(atombuf)){
				media.video.width = getAtomSize(atombuf);
			}
			my_fread(atombuf, 1, 4, fp); // height
			if(getAtomSize(atombuf)){
				media.video.height = getAtomSize(atombuf);
			}
			break;
		case STSD:
			my_fseek(fp, 8, SEEK_CUR); // skip Reserved(6bytes)/Data Reference Index
			my_fread(atombuf, 1, 4, fp); // next atom size
			my_fread(atombuf, 1, getAtomSize(atombuf) - 4, fp);
			if(media.video.flag.process && !media.video.flag.complete){
				memset((void*)media.video.videoFmtString, '\0', sizeof(media.video.videoFmtString));
				memset((void*)media.video.videoCmpString, '\0', sizeof(media.video.videoCmpString));
				memcpy((void*)media.video.videoFmtString, (void*)atombuf, 4);
				memcpy((void*)media.video.videoCmpString, (void*)&atombuf[47], atombuf[46]);
				if(strncmp((char*)media.video.videoFmtString, "jpeg", 4) == 0){
					media.video.playJpeg = 1; // JPEG
				} else {
					media.video.playJpeg = 0; // Uncompression
				}
			}
			if(media.sound.flag.process && !media.sound.flag.complete){
				memcpy((void*)&media.sound.format, (void*)atombuf, sizeof(sound_format));
				media.sound.format.version = (uint16_t)b2l((void*)&media.sound.format.version, sizeof(uint16_t));
				media.sound.format.revision = (uint16_t)b2l((void*)&media.sound.format.revision, sizeof(media.sound.format.revision));
				media.sound.format.vendor = (uint16_t)b2l((void*)&media.sound.format.vendor, sizeof(media.sound.format.vendor));
				media.sound.format.numChannel = (uint16_t)b2l((void*)&media.sound.format.numChannel, sizeof(media.sound.format.numChannel));
				media.sound.format.sampleSize = (uint16_t)b2l((void*)&media.sound.format.sampleSize, sizeof(media.sound.format.sampleSize));
				media.sound.format.complesionID = (uint16_t)b2l((void*)&media.sound.format.complesionID, sizeof(media.sound.format.complesionID));
				media.sound.format.packetSize = (uint16_t)b2l((void*)&media.sound.format.packetSize, sizeof(media.sound.format.packetSize));
				media.sound.format.sampleRate = (uint16_t)b2l((void*)&media.sound.format.sampleRate, sizeof(uint16_t));
			}
			break;
		case STTS:
			my_fseek(fp, 4, SEEK_CUR); // skip flag ver
			if(media.video.flag.process && !media.video.flag.complete){
				video_stts.numEntry = getSampleSize(atombuf, 4, fp);
				memcpy((void*)&video_stts.fp, (void*)fp, sizeof(MY_FILE));
			}
			if(media.sound.flag.process && !media.sound.flag.complete){
				sound_stts.numEntry = getSampleSize(atombuf, 4, fp);
				memcpy((void*)&sound_stts.fp, (void*)fp, sizeof(MY_FILE));
			}
			break;
		case STSC:
			my_fseek(fp, 4, SEEK_CUR); // skip flag ver
			if(media.video.flag.process && !media.video.flag.complete){
				video_stsc.numEntry = getSampleSize(atombuf, 4, fp);
				memcpy((void*)&video_stsc.fp, (void*)fp, sizeof(MY_FILE));
			}
			if(media.sound.flag.process && !media.sound.flag.complete){
				sound_stsc.numEntry = getSampleSize(atombuf, 4, fp);
				memcpy((void*)&sound_stsc.fp, (void*)fp, sizeof(MY_FILE));
			}
			break;
		case STSZ:
			my_fseek(fp, 4, SEEK_CUR); // skip flag ver
			if(media.video.flag.process && !media.video.flag.complete){
				video_stsz.sampleSize = getSampleSize(atombuf, 4, fp);
				video_stsz.numEntry = getSampleSize(atombuf, 4, fp);
				memcpy((void*)&video_stsz.fp, (void*)fp, sizeof(MY_FILE));
			}
			if(media.sound.flag.process && !media.sound.flag.complete){
				sound_stsz.sampleSize = getSampleSize(atombuf, 4, fp);
				memcpy((void*)&sound_stsz.fp, (void*)fp, sizeof(MY_FILE));
				sound_stsz.numEntry = getSampleSize(atombuf, 4, fp);
				memcpy((void*)&sound_stsz.fp, (void*)fp, sizeof(MY_FILE));
			}
			break;
		case STCO:
			my_fseek(fp, 4, SEEK_CUR); // skip flag ver
			if(media.video.flag.process && !media.video.flag.complete){
				video_stco.numEntry = getSampleSize(atombuf, 4, fp);
				memcpy((void*)&video_stco.fp, (void*)fp, sizeof(MY_FILE));

				media.video.flag.process = 0;
				media.video.flag.complete = 1;
			}
			if(media.sound.flag.process && !media.sound.flag.complete){
				sound_stco.numEntry = getSampleSize(atombuf, 4, fp);
				memcpy((void*)&sound_stco.fp, (void*)fp, sizeof(MY_FILE));

				media.sound.flag.process = 0;
				media.sound.flag.complete = 1;
			}
			break;
		default:
			break;
		}

		memcpy((void*)fp, (void*)&fp_tmp, sizeof(MY_FILE));

		if(index < ATOM_ITEMS && atomHasChild[index]){
//			memcpy((void*)&fp_tmp, (void*)fp, sizeof(MY_FILE));

			if(collectAtoms(fp, atomSize - 8, child + 1) != 0){ // Re entrant
				return -1;
			}

			memcpy((void*)fp, (void*)&fp_tmp, sizeof(MY_FILE));
		}

NEXT:
		my_fseek(fp, atomSize - 8, SEEK_CUR);
		totalAtomSize += atomSize;
	}while(parentAtomSize > (totalAtomSize + 8));

	return 0;
}

static inline uint32_t getVideoSampleTime(uint8_t *atombuf, uint32_t sampleID)
{
	static uint32_t numSamples, count, duration;
	static MY_FILE fp;

	if(sampleID == 0){
		numSamples = count = 0;
		memcpy((void*)&fp, (void*)&video_stts.fp, sizeof(MY_FILE));
	}

	if(numSamples < ++count){
		count = 1;
		numSamples = getSampleSize(atombuf, 8, &fp);
		duration = getAtomSize(&atombuf[4]);
	}

	return duration;
}

drawBuff_typedef *drawBuff;

void mjpegTouch(int id, uint32_t pointed_chunk) // タッチペン割込み処理
{
	uint32_t firstChunk, prevChunk, prevSamples, samples, totalSamples = 0, jFrameOffset;
	MY_FILE fp_stsc, fp_stsz, fp_stco, fp_frame, fp_frame_cp;
	uint8_t atombuf[12];

	raw_video_typedef raw;

	memcpy((void*)&fp_stsc, (void*)&video_stsc.fp, sizeof(MY_FILE));
	memcpy((void*)&fp_stsz, (void*)&video_stsz.fp, sizeof(MY_FILE));
	memcpy((void*)&fp_stco, (void*)&video_stco.fp, sizeof(MY_FILE));
	memcpy((void*)&fp_frame, (void*)&fp_global, sizeof(MY_FILE));

//		debug.printf("\r\npointed_chunk:%d video_stco.numEntry:%d", pointed_chunk, video_stco.numEntry);

	prevChunk = getSampleSize(atombuf, 12, &fp_stsc); // firstChunk samplesPerChunk sampleDescriptionID 一つ目のfirstChunkをprevChunkに
	prevSamples = getAtomSize(&atombuf[4]); // 一つ目のsamplesPerChunkをprevSamplesに
	firstChunk = getSampleSize(atombuf, 4, &fp_stsc); // 二つ目のfirstChunk

	while(1){
		if(prevChunk <= pointed_chunk && firstChunk >= pointed_chunk){
			samples = (firstChunk - pointed_chunk) * prevSamples;
			totalSamples += (pointed_chunk - prevChunk) * prevSamples;
			break;
		}
		samples = (firstChunk - prevChunk) * prevSamples;
		totalSamples += samples;

		prevChunk = firstChunk; // 今回のfirstChunkをprevChunkに
		prevSamples = getSampleSize(atombuf, 8, &fp_stsc); // samplesPerChunk sampleDescriptionID
		firstChunk = getSampleSize(atombuf, 4, &fp_stsc); // 次のfirstChunk
	}

	my_fseek(&fp_stco, (pointed_chunk - 1) * 4, SEEK_CUR);
	jFrameOffset = getSampleSize(atombuf, 4, &fp_stco);

	memcpy((void*)&fp_stco, (void*)&video_stco.fp, sizeof(MY_FILE));
	my_fseek(&fp_stco, (pointed_chunk - 1) * 4, SEEK_CUR); //

	my_fseek(&fp_frame, jFrameOffset, SEEK_SET);

	memcpy((void*)&fp_frame_cp, (void*)&fp_frame, sizeof(MY_FILE));
	my_fseek(&fp_frame, raw.frame_size, SEEK_CUR);

	*pv_src.firstChunk = firstChunk;
	*pv_src.prevChunk = prevChunk;
	*pv_src.prevSamples = prevSamples;
	*pv_src.samples = samples;
	*pv_src.totalSamples = totalSamples;
	*pv_src.videoStcoCount = pointed_chunk + 1;

	memcpy((void*)pv_src.fp_video_stsc, (void*)&fp_stsc, sizeof(MY_FILE));
	memcpy((void*)pv_src.fp_video_stsz, (void*)&fp_stsz, sizeof(MY_FILE));
	memcpy((void*)pv_src.fp_video_stco, (void*)&fp_stco, sizeof(MY_FILE));
	memcpy((void*)pv_src.fp_frame, (void*)&fp_frame, sizeof(MY_FILE));


	// Sound
	memcpy((void*)&fp_stsc, (void*)&sound_stsc.fp, sizeof(MY_FILE));
	memcpy((void*)&fp_stsz, (void*)&sound_stsz.fp, sizeof(MY_FILE));
	memcpy((void*)&fp_stco, (void*)&sound_stco.fp, sizeof(MY_FILE));
	memcpy((void*)&fp_frame, (void*)&fp_global, sizeof(MY_FILE));

//		debug.printf("\r\npointed_chunk:%d video_stco.numEntry:%d", pointed_chunk, video_stco.numEntry);

	prevChunk = getSampleSize(atombuf, 12, &fp_stsc); // firstChunk samplesPerChunk sampleDescriptionID 一つ目のfirstChunkをprevChunkに
	prevSamples = getAtomSize(&atombuf[4]); // 一つ目のsamplesPerChunkをprevSamplesに
	firstChunk = getSampleSize(atombuf, 4, &fp_stsc); // 二つ目のfirstChunk

	totalSamples = 0;
	while(1){
		if(prevChunk <= pointed_chunk && firstChunk >= pointed_chunk){
			samples = (firstChunk - pointed_chunk) * prevSamples;
			totalSamples += (pointed_chunk - prevChunk) * prevSamples;
			break;
		}
		samples = (firstChunk - prevChunk) * prevSamples;
		totalSamples += samples;

		prevChunk = firstChunk; // 今回のfirstChunkをprevChunkに
		prevSamples = getSampleSize(atombuf, 8, &fp_stsc); // samplesPerChunk sampleDescriptionID
		firstChunk = getSampleSize(atombuf, 4, &fp_stsc); // 次のfirstChunk
	}
	my_fseek(&fp_stco, (pointed_chunk - 1) * 4, SEEK_CUR);

	*ps_src.firstChunk = firstChunk;
	*ps_src.prevChunk = prevChunk;
	*ps_src.prevSamples = prevSamples;
	*ps_src.samples = samples;
	*ps_src.soundStcoCount = pointed_chunk;

	memcpy((void*)ps_src.fp_sound_stsc, (void*)&fp_stsc, sizeof(MY_FILE));
	memcpy((void*)ps_src.fp_sound_stsz, (void*)&fp_stsz, sizeof(MY_FILE));
	memcpy((void*)ps_src.fp_sound_stco, (void*)&fp_stco, sizeof(MY_FILE));

	if(mjpeg_touch.resynch){
		return;
	}

	if(media.video.width != LCD_WIDTH || media.video.height != LCD_HEIGHT){
		LCD_DrawSquare(0, 0, LCD_WIDTH, media.video.startPosY, BLACK);
		LCD_DrawSquare(0, media.video.startPosY, (LCD_WIDTH - media.video.width) / 2, media.video.height, BLACK);
		LCD_DrawSquare(media.video.startPosX + media.video.width, media.video.startPosY, (LCD_WIDTH - media.video.width) / 2, media.video.height, BLACK);
		LCD_DrawSquare(0, media.video.startPosY + media.video.height, LCD_WIDTH, LCD_HEIGHT - (media.video.startPosY + media.video.height), BLACK);
	}

	int v;
	for(v = 0;v < media.video.height;v++){
		my_fread(&frame_buffer[media.video.startPosX + v * LCD_WIDTH + media.video.startPosY * LCD_WIDTH], 2, media.video.width, &fp_frame_cp);
	}


	int curX, prevX, duration = (int)((float)media.video.duration / (float)media.video.timeScale + 0.5f);
	int time = duration * (float)*pv_src.videoStcoCount / (float)video_stco.numEntry;
	char timeStr[20];

	LCDPutIcon(4, 100, 12, 12, pause_icon_12x12, pause_icon_12x12_alpha);
	LCDPutIcon(UI_POS_X, UI_POS_Y, 120, 14, seekbar_120x14, seekbar_120x14_alpha);

	drawBuff->navigation_loop.x = 140;
	drawBuff->navigation_loop.y = UI_POS_Y;
	drawBuff->navigation_loop.width = 18;
	drawBuff->navigation_loop.height = 14;
	LCDStoreBgImgToBuff(drawBuff->navigation_loop.x, drawBuff->navigation_loop.y, \
						drawBuff->navigation_loop.width, drawBuff->navigation_loop.height, drawBuff->navigation_loop.p);

	Update_Navigation_Loop_Icon(drawBuff, music_control.b.navigation_loop_mode);

	drawBuff->posision.width = 12;
	drawBuff->posision.height = 12;
	drawBuff->posision.x = UI_POS_X + 1;
	drawBuff->posision.y = UI_POS_Y + 1;
	prevX = UI_POS_X + 1;
	LCDStoreBgImgToBuff(prevX, drawBuff->posision.y, \
						drawBuff->posision.width, drawBuff->posision.height, drawBuff->posision.p);

	DRAW_SEEK_CIRCLE((float)time / (float)duration , seek_active_circle_12x12);

	pcf_typedef pcf;
	pcf.dst_gram_addr = (uint32_t)frame_buffer;
	pcf.pixelFormat = PCF_PIXEL_FORMAT_RGB565;
	pcf.size = 12;
	pcf.color = WHITE;
	pcf.colorShadow = GRAY;
	pcf.alphaSoftBlending = 1;
	pcf.enableShadow = 1;
	pcf_font.metrics.hSpacing = 2;

	char s[10];
	SPRINTF(s, "%d/%d", id, fat.fileCnt - 1);
	LCD_GotoXY(5, MUSIC_INFO_POS_Y + 1);
	LCDPutString(s, &pcf);

	DRAW_MOV_TIME_STR();

	DRAW_MOV_REMAIN_TIME_STR();

	LCD_FRAME_BUFFER_Transmit(LCD_DMA_TRANSMIT_BLOCKING);
}

extern I2S_HandleTypeDef haudio_i2s;


int mjpegPause(int id)
{
	int seekBytes, media_data_totalBytes, diff;
	int ret = RET_PLAY_NORM, curX, prevX = 0;
	int duration = media.video.duration / media.video.timeScale;
	int swHoldCnt, media_data_denom;
	int time = duration * (float)((float)*pv_src.videoStcoCount / (float)video_stco.numEntry);
	char timeStr[20];

	LCD_SetRegion(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);

	pcf_typedef pcf;
	pcf.dst_gram_addr = (uint32_t)frame_buffer;
	pcf.pixelFormat = PCF_PIXEL_FORMAT_RGB565;
	pcf.size = 12;
	pcf.color = WHITE;
	pcf.colorShadow = GRAY;
	pcf.alphaSoftBlending = 1;
	pcf.enableShadow = 1;
	pcf_font.metrics.hSpacing = 2;

	drawBuff = (drawBuff_typedef*)cursorRAM;

	char s[10];
	SPRINTF(s, "%d/%d", id, fat.fileCnt - 1);
	LCD_GotoXY(5, MUSIC_INFO_POS_Y + 1);
	LCDPutString(s, &pcf);

	mjpeg_touch.resynch = 1;
	mjpegTouch(id, *pv_src.videoStcoCount);
	mjpeg_touch.resynch = 0;

	LCDPutIcon(4, 100, 12, 12, pause_icon_12x12, pause_icon_12x12_alpha);
	LCDPutIcon(UI_POS_X, UI_POS_Y, 120, 14, seekbar_120x14, seekbar_120x14_alpha);

	drawBuff->navigation_loop.x = 140;
	drawBuff->navigation_loop.y = UI_POS_Y;
	drawBuff->navigation_loop.width = 18;
	drawBuff->navigation_loop.height = 14;
	LCDStoreBgImgToBuff(drawBuff->navigation_loop.x, drawBuff->navigation_loop.y, \
						drawBuff->navigation_loop.width, drawBuff->navigation_loop.height, drawBuff->navigation_loop.p);

	Update_Navigation_Loop_Icon(drawBuff, music_control.b.navigation_loop_mode);

	drawBuff->posision.width = 12;
	drawBuff->posision.height = 12;
	drawBuff->posision.x = UI_POS_X + 1;
	drawBuff->posision.y = UI_POS_Y + 1;
	prevX = UI_POS_X + 1;
	LCDStoreBgImgToBuff(prevX, drawBuff->posision.y, \
						drawBuff->posision.width, drawBuff->posision.height, drawBuff->posision.p);

	DRAW_SEEK_CIRCLE((float)time / (float)duration , seek_circle_12x12);

	DRAW_MOV_TIME_STR();

	DRAW_MOV_REMAIN_TIME_STR();

	LCD_FRAME_BUFFER_Transmit(LCD_DMA_TRANSMIT_BLOCKING);

	int pause_flag = 0;
	if(LCDStatusStruct.waitExitKey == PLAY_PAUSE || LCDStatusStruct.waitExitKey == PLAY_LOOP_MODE)
	{
		pause_flag = 1;
		LCDStatusStruct.waitExitKey = 1;
	}
extern int8_t vol;
	while(1){
		switch(LCDStatusStruct.waitExitKey)
		{
		case 0:
			return RET_PLAY_STOP;
			break;
		case VOL_UP:
			vol = ++vol < 6 ? vol : 6;
//			wm8731_left_headphone_volume_set(121 + vol);
			LCDStatusStruct.waitExitKey = 1;
			break;
		case VOL_DOWN:
			vol = --vol > -121 ? vol : -121;
//			wm8731_left_headphone_volume_set(121 + vol);
			LCDStatusStruct.waitExitKey = 1;
			break;
		case PLAY_PAUSE:
			ret = 1;
			LCDStatusStruct.waitExitKey = 1;
			goto PAUSE_EXIT;
			break;
		case PLAY_LOOP_MODE:
			Update_Navigation_Loop_Icon(drawBuff, music_control.b.navigation_loop_mode = ++music_control.b.navigation_loop_mode % 5);
			LCD_FRAME_BUFFER_Transmit(LCD_DMA_TRANSMIT_NOBLOCKING);
			LCDStatusStruct.waitExitKey = 1;
			break;
		case PLAY_SW_HOLD_LEFT:
			media_data_totalBytes = video_stco.numEntry;
			seekBytes = *pv_src.videoStcoCount;
			media_data_denom = 100;
			swHoldCnt = SW_HOLD_CNT_VAL;
			while(HAL_GPIO_ReadPin(SW_PUSH_LEFT_PORT, SW_PUSH_LEFT_PIN) == GPIO_PIN_RESET){
				diff = seekBytes - media_data_totalBytes / media_data_denom;
				if(diff < 0){
					seekBytes = 2;
				} else {
					seekBytes += -media_data_totalBytes / media_data_denom;
				}

				mjpegTouch(id, seekBytes);

				HAL_Delay(20);
				media_data_denom = 100 / (++swHoldCnt / SW_HOLD_CNT_VAL);
			}
			ret = 1;
			LCDStatusStruct.waitExitKey = 1;
			if(!pause_flag){
				goto PAUSE_EXIT;
			}
			break;
		case PLAY_SW_HOLD_RIGHT:
			media_data_totalBytes = video_stco.numEntry;
			seekBytes = *pv_src.videoStcoCount;
		 	media_data_denom = 100;
			swHoldCnt = SW_HOLD_CNT_VAL;
			while(HAL_GPIO_ReadPin(SW_PUSH_RIGHT_PORT, SW_PUSH_RIGHT_PIN) == GPIO_PIN_RESET){
				if((seekBytes + media_data_totalBytes / media_data_denom) > media_data_totalBytes){
					continue;
				}
				seekBytes += media_data_totalBytes / media_data_denom;
				mjpegTouch(id, seekBytes);

	 	 		HAL_Delay(20);
	 	 		media_data_denom = 100 / (++swHoldCnt / SW_HOLD_CNT_VAL);
			}
			ret = 1;
			LCDStatusStruct.waitExitKey = 1;
			if(!pause_flag){
				goto PAUSE_EXIT;
			}
			break;
		case PLAY_NEXT:
			ret = RET_PLAY_NEXT;
			return ret;
		case PLAY_PREV:
			ret = RET_PLAY_PREV;
			return ret;
			break;
		default:
			break;
		}
		HAL_Delay(1);
	}

PAUSE_EXIT:
	if(media.video.width != LCD_WIDTH || media.video.height != LCD_HEIGHT){
		LCD_DrawSquare(0, 0, LCD_WIDTH, media.video.startPosY, BLACK);
		LCD_DrawSquare(0, media.video.startPosY, (LCD_WIDTH - media.video.width) / 2, media.video.height, BLACK);
		LCD_DrawSquare(media.video.startPosX + media.video.width, media.video.startPosY, (LCD_WIDTH - media.video.width) / 2, media.video.height, BLACK);
		LCD_DrawSquare(0, media.video.startPosY + media.video.height, LCD_WIDTH, LCD_HEIGHT - (media.video.startPosY + media.video.height), BLACK);
	}

	LCD_FRAME_BUFFER_Transmit(LCD_DMA_TRANSMIT_BLOCKING);
	while((SpiLcdHandle.State != HAL_SPI_STATE_READY)){};

	memset(dac_intr.buff, '\0', dac_intr.bufferSize);

	LCD_SetRegion(media.video.startPosX, media.video.startPosY, media.video.startPosX + media.video.width - 1, media.video.startPosY + media.video.height - 1);

	HAL_I2S_DMAResume(&haudio_i2s);
	DMA_SOUND_IT_ENABLE;

    return ret;
}

int PlayMovie(int id){
	register uint32_t i, j;
	int ret = 0;

	uint32_t fps, frames, prevFrames, sample_time_limit;
	uint32_t samples, frameDuration, numEntry;
	uint32_t prevChunkSound, prevSamplesSound, firstChunkSound, samplesSound;
	uint32_t firstChunk = 0, totalSamples = 0, prevChunk = 0, prevSamples = 0, totalBytes = 0;
	uint32_t videoStcoCount, soundStcoCount, stco_reads;
	uint32_t prevSamplesBuff[60];

	uint8_t atombuf[512];
	uint8_t fpsCnt = 0;
	const char fps1Hz[] = "|/-\\";
	char timeStr[20];

	raw_video_typedef raw;

	MY_FILE fp_sound, fp_frame, fp_frame_cp, \
			fp_stsc, fp_stsz, fp_stco, \
			fp_sound_stsc, fp_sound_stsz, fp_sound_stco, \
			*fp = '\0';

	media.sound.flag.process = 0;
	media.sound.flag.complete = 0;
	media.video.flag.process = 0;
	media.video.flag.complete = 0;

	debug.printf("\r\n\n*** Atom Info ***");

	fp = my_fopen(id);
	memcpy((void*)&fp_global, (void*)fp, sizeof(MY_FILE));

	int hasChild = atomHasChild[UDTA];
	atomHasChild[UDTA] = 0;

	debug.printf("\r\n[Atoms]");
	if(collectAtoms(fp, fp->fileSize, 0) != 0){
		debug.printf("\r\nread error file contents.");
		my_fclose(fp);
		LCDStatusStruct.waitExitKey = 0;
		atomHasChild[UDTA] = hasChild;
		return -99;
	}

	atomHasChild[UDTA] = hasChild;

	debug.printf("\r\n\n[Video Sample Tables]");
	debug.printf("\r\nstts:%d", video_stts.numEntry);
	debug.printf("\r\nstsc:%d", video_stsc.numEntry);
	debug.printf("\r\nstsz:%d %d", video_stsz.sampleSize, video_stsz.numEntry);
	debug.printf("\r\nstco:%d", video_stco.numEntry);

	debug.printf("\r\n\n[Sound Sample Tables]");
	debug.printf("\r\nstts:%d", sound_stts.numEntry);
	debug.printf("\r\nstsc:%d", sound_stsc.numEntry);
	debug.printf("\r\nstsz:%d %d", sound_stsz.sampleSize, sound_stsz.numEntry);
	debug.printf("\r\nstco:%d", sound_stco.numEntry);

	debug.printf("\r\n\n[Video Track]");
	debug.printf("\r\nformat:%s", media.video.videoFmtString);
	debug.printf("\r\ncompression:%s", media.video.videoCmpString);
	debug.printf("\r\nwidth:%d", media.video.width);
	debug.printf("\r\nheight:%d", media.video.height);
	debug.printf("\r\ntimeScale:%d", media.video.timeScale);
	debug.printf("\r\nduration:%d", media.video.duration);
	setStrSec(timeStr, (int)((float)media.video.duration / (float)media.video.timeScale + 0.5f));
	media.video.frameRate = (int16_t)((float)(media.video.timeScale * video_stsz.numEntry) / media.video.duration + 0.5f);
	debug.printf("\r\nframe rate:%d", media.video.frameRate);
	debug.printf("\r\ntime:%s", timeStr);

	debug.printf("\r\n\n[Sound Track]");
	char s[5];
	s[4] = '\0';
	memcpy(s, (void*)media.sound.format.audioFmtString, 4);
	debug.printf("\r\ntype:%s", s);
	debug.printf("\r\nnumChannel:%d", media.sound.format.numChannel);
	debug.printf("\r\nsampleSize:%d", media.sound.format.sampleSize);
	debug.printf("\r\nsampleRate:%d", media.sound.format.sampleRate);
	debug.printf("\r\ntimeScale:%d", media.sound.timeScale);
	debug.printf("\r\nduration:%d", media.sound.duration);
	setStrSec(timeStr, (int)((float)media.sound.duration / (float)media.sound.timeScale + 0.5f));
	debug.printf("\r\ntime:%s", timeStr);

	if(media.video.width > LCD_WIDTH || media.video.height > LCD_HEIGHT){
		debug.printf("\r\ntoo large video dimension size.");
		my_fclose(fp);
		LCDStatusStruct.waitExitKey = 0;
		atomHasChild[UDTA] = hasChild;
		return RET_PLAY_STOP;
	}

	FUNC_VIDEO_BGIMG;
	media.video.startPosX = (LCD_WIDTH - media.video.width) / 2 - 1;
	media.video.startPosY = (LCD_HEIGHT - media.video.height) / 2 - 1;
	media.video.startPosX = media.video.startPosX > 0 ? media.video.startPosX : 0;
	media.video.startPosY = media.video.startPosY > 0 ? media.video.startPosY : 0;
//	media.video.height += (media.video.height % 2); // if value is odd number, convert to even


	debug.printf("\r\nmedia.video.startPosX:%d", media.video.startPosX);
	debug.printf("\r\nmedia.video.startPosY:%d", media.video.startPosY);
	debug.printf("\r\nmedia.video.width:%d", media.video.width);
	debug.printf("\r\nmedia.video.height:%d", media.video.height);

/*
 	// DEBUG SAMPLE TABLES >>
	uint32_t sampleCount, sampleDuration, totalSampleCount, totalSampleDuration, samplesPerChunk, sampleDscId, chunkOffset;
 	MY_FILE fp_tmp;
	memcpy((void*)&fp_tmp, (void*)&video_stts.fp, sizeof(MY_FILE));

	debug.printf("\r\n\n*** Video Info ***");

	// Time to  Sample (stts)

	debug.printf("\r\n\nTimeToSample(stts)");

	for(i = 0;i < video_stts.numEntry;i++){
		my_fread(atombuf, 1, 4, &fp_tmp);
		sampleCount = getAtomSize(atombuf);

		my_fread(atombuf, 1, 4, &fp_tmp);
		sampleDuration = getAtomSize(atombuf);


		debug.printf("\r\n%04d:%04x %04x", i, sampleCount, sampleDuration);


		totalSampleCount += sampleCount;
		totalSampleDuration += sampleDuration * sampleCount;
	}
	debug.printf("\r\ntotalSamplesCount:%d totalSampleDuration:%d", totalSampleCount, totalSampleDuration);




	// Samples To Chunk (stsc)

	debug.printf("\r\n\nSamplesToChunk(stsc)");

	memcpy((void*)&fp_tmp, (void*)&video_stsc.fp, sizeof(MY_FILE));

	for(i = 0;i < video_stsc.numEntry;i++){
		my_fread(atombuf, 1, 4, &fp_tmp);
		firstChunk = getAtomSize(atombuf);

		my_fread(atombuf, 1, 4, &fp_tmp);
		samplesPerChunk = getAtomSize(atombuf);

		my_fread(atombuf, 1, 4, &fp_tmp);
		sampleDscId = getAtomSize(atombuf);

		debug.printf("\r\n%02d:%04x %04x %04x", i, firstChunk, samplesPerChunk, sampleDscId);


		totalSamples += (firstChunk - prevChunk) * prevSamples;

		prevChunk = firstChunk;
		prevSamples  = samplesPerChunk;
	}
	totalSamples += prevSamples;

	debug.printf("\r\ntotalSamples:%d", totalSamples);

	// Chunk To Offset (stco)

	debug.printf("\r\n\nChunkToOffset(stco)");

	memcpy((void*)&fp_tmp, (void*)&video_stco.fp, sizeof(MY_FILE));


	for(i = 0;i < video_stco.numEntry;i++){
		my_fread(atombuf, 1, 4, &fp_tmp);
		chunkOffset = getAtomSize(atombuf);

		debug.printf("\r\n%04d:%08x", i, chunkOffset);
	}



	debug.printf("\r\n\n*** Sound Info ***");

	totalSampleCount = 0, totalSampleDuration = 0;
	firstChunk = 0, samplesPerChunk = 0, sampleDscId, totalSamples = 0, \
	prevChunk = 0, prevSamples = 0;
	// Time to  Sample (stts)

	memcpy((void*)&fp_tmp, (void*)&sound_stts.fp, sizeof(MY_FILE));

	debug.printf("\r\n\nTimeToSample(stts)");

	for(i = 0;i < sound_stts.numEntry;i++){
		my_fread(atombuf, 1, 4, &fp_tmp);
		sampleCount = getAtomSize(atombuf);

		my_fread(atombuf, 1, 4, &fp_tmp);
		sampleDuration = getAtomSize(atombuf);


		debug.printf("\r\n%04d:%04x %04x", i, sampleCount, sampleDuration);


		totalSampleCount += sampleCount;
		totalSampleDuration += sampleDuration * sampleCount;
	}
	debug.printf("\r\ntotalSamplesCount:%d totalSampleDuration:%d", totalSampleCount, totalSampleDuration);




	// Samples To Chunk (stsc)

	debug.printf("\r\n\nSamplesToChunk(stsc)");

	memcpy((void*)&fp_tmp, (void*)&sound_stsc.fp, sizeof(MY_FILE));

	for(i = 0;i < sound_stsc.numEntry;i++){
		my_fread(atombuf, 1, 4, &fp_tmp);
		firstChunk = getAtomSize(atombuf);

		my_fread(atombuf, 1, 4, &fp_tmp);
		samplesPerChunk = getAtomSize(atombuf);

		my_fread(atombuf, 1, 4, &fp_tmp);
		sampleDscId = getAtomSize(atombuf);

		debug.printf("\r\n%02d:%04x %04x %04x", i, firstChunk, samplesPerChunk, sampleDscId);


		totalSamples += (firstChunk - prevChunk) * prevSamples;

		prevChunk = firstChunk;
		prevSamples  = samplesPerChunk;
	}
	totalSamples += prevSamples;

	debug.printf("\r\ntotalSamples:%d", totalSamples);

	// Chunk To Offset (stco)

	debug.printf("\r\n\nChunkToOffset(stco)");

	memcpy((void*)&fp_tmp, (void*)&sound_stco.fp, sizeof(MY_FILE));


	for(i = 0;i < sound_stco.numEntry;i++){
		my_fread(atombuf, 1, 4, &fp_tmp);
		chunkOffset = getAtomSize(atombuf);

		debug.printf("\r\n%04d:%08x", i, chunkOffset);
	}
	// << DEBUG SAMPLE TABLES
*/
	/*** MotionJPEG Play Process ***/
	debug.printf("\r\n\n[Play]\n");

	my_fseek(fp, 0, SEEK_SET);

	memcpy((void*)&fp_frame, (void*)fp, sizeof(MY_FILE));
	memcpy((void*)&fp_stsz, (void*)&video_stsz.fp, sizeof(MY_FILE));
	memcpy((void*)&fp_stco, (void*)&video_stco.fp, sizeof(MY_FILE));
	memcpy((void*)&fp_stsc, (void*)&video_stsc.fp, sizeof(MY_FILE));
	numEntry = video_stsc.numEntry;

	fps = frames = prevFrames = 0;
	totalSamples = firstChunk = prevChunk = prevSamples = 0;

//	if(!pcf_font.c_loaded){
//		LCD_FUNC.putChar = PCFPutCharCache;
//		LCD_FUNC.putWideChar = PCFPutCharCache;
//
//		PCFSetGlyphCacheStartAddress((void*)cursorRAM);
//		PCFCachePlayTimeGlyphs(12);
//	} else {
//		LCD_FUNC.putChar = C_PCFPutChar;
//		LCD_FUNC.putWideChar = C_PCFPutChar;
//	}

	if(abs(video_stco.numEntry - sound_stco.numEntry) > 50){ // not interleaved correctly
		debug.printf("\r\nError!! this is not an interleaved media.");
		debug.printf("\r\nTry command:");
		debug.printf("\r\nMP4Box -inter 500 this.mov");

//		LCD_DrawSquare(0, 0, 320, 13, BLACK);
//		LCDSetWindowArea(0, 0, LCD_WIDTH, LCD_HEIGHT);
//		LCDClear(LCD_WIDTH, LCD_HEIGHT, BLACK);
//		LCDGotoXY(0, 0);
		LCDPutString("Error!!  this  is  not  an  interleaved  media.\n", RED);
		LCDPutString("Try  command:\n", RED);
		LCDPutString("MP4Box  -inter  500  this.mov", RED);
//		Delay(5000);
//		while(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4) != Bit_RESET); // タッチパネルが押されたか？

		goto EXIT_PROCESS;
	} else {
		prevChunk = getSampleSize(atombuf, 12, &fp_stsc); // firstChunk samplesPerChunk sampleDescriptionID 一つ目のfirstChunkをprevChunkに
		prevSamples = getAtomSize(&atombuf[4]); // 一つ目のsamplesPerChunkをprevSamplesに
		firstChunk = getSampleSize(atombuf, 4, &fp_stsc); // 二つ目のfirstChunk
		samples = firstChunk - prevChunk;
	}

	frameDuration = getVideoSampleTime(atombuf, totalSamples);

	// SOUND
	memcpy((void*)&fp_sound_stsz, (void*)&sound_stsz.fp, sizeof(MY_FILE));
	memcpy((void*)&fp_sound_stco, (void*)&sound_stco.fp, sizeof(MY_FILE));
	memcpy((void*)&fp_sound_stsc, (void*)&sound_stsc.fp, sizeof(MY_FILE));
	memcpy((void*)&fp_sound, (void*)fp, sizeof(MY_FILE));

	prevChunkSound = getSampleSize(atombuf, 12, &fp_sound_stsc); // firstChunk samplesPerChunk sampleDescriptionID　一つ目のfirstChunkをprevChunkに
	prevSamplesSound = (getAtomSize(&atombuf[4]) / 100) * 100; // 一つ目のsamplesPerChunkをprevSamplesに サウンドバッファの半端がでないようにする
	firstChunkSound = getSampleSize(atombuf, 4, &fp_sound_stsc); // 二つ目のfirstChunk

	samplesSound = (firstChunkSound - prevChunkSound) * prevSamplesSound;

//	uint8_t SOUND_BUFFER[38400];
//	uint8_t SOUND_BUFFER[12800];
	uint16_t soundSampleByte = media.sound.format.sampleSize / 8;
	uint32_t soundSampleBlocks = soundSampleByte * media.sound.format.numChannel;

	float timeScaleCoeff = (1.0f / media.video.timeScale) * 100000;
	extern uint16_t frame_buffer[160 * 128];

	dac_intr.fp = &fp_sound;
	dac_intr.buff = (uint8_t*)frame_buffer;
//	dac_intr.buff = SOUND_BUFFER;
	dac_intr.bufferSize = ((media.sound.format.sampleRate / 10) * 2) * soundSampleByte * media.sound.format.numChannel;
//	if(media.sound.format.sampleSize == 16){
//		dac_intr.func = DAC_Buffer_Process_Stereo_S16bit;
//	} else {
//		dac_intr.func = DAC_Buffer_Process_Mono_U8bit;
//	}

	memset(dac_intr.buff, 0, dac_intr.bufferSize);

	my_fseek(&fp_sound, getSampleSize(atombuf, 4, &fp_sound_stco), SEEK_SET);

	dac_intr.sound_reads = 0;
	stco_reads = 1;

	debug.printf("\r\nframeDuration:%d", frameDuration);

	TIM_HandleTypeDef Tim1SecHandle, TimDurationHandle;

	/* TIM3 サンプルタイム用 0.01ms秒単位で指定 */
	TimDurationHandle.Instance = TIM_DURATION;
	HAL_TIM_Base_DeInit(&TimDurationHandle);
	TimDurationHandle.Init.Period = (100000 * frameDuration) / media.video.timeScale - 1;
	TimDurationHandle.Init.Prescaler = ((SystemCoreClock / 2) / 100000) * 2 - 1; // 0.01ms
	TimDurationHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
	TimDurationHandle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	if(HAL_TIM_Base_Init(&TimDurationHandle) != HAL_OK){
		while(1);
	}
	HAL_TIM_Base_Start(&TimDurationHandle);

	/*
	while(1)
	{
		if(TIM3_SR_UIF_BB){
			TIM3_SR_UIF_BB = 0;
			HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
			TIM3->CNT = 0;
		}
	}
	*/

	Tim1SecHandle.Instance = TIM_1SEC;
	HAL_TIM_Base_DeInit(&Tim1SecHandle);
	Tim1SecHandle.Init.Prescaler = 100 - 1;
	Tim1SecHandle.Init.Period = 10000 - 1;
	Tim1SecHandle.Init.RepetitionCounter = (SystemCoreClock / 1000000UL) - 1;
	Tim1SecHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
	Tim1SecHandle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	if(HAL_TIM_Base_Init(&Tim1SecHandle) != HAL_OK){
		while(1);
	}
	HAL_TIM_Base_Start_IT(&Tim1SecHandle);



	// Video
	pv_src.firstChunk = &firstChunk;
	pv_src.prevChunk = &prevChunk;
	pv_src.prevSamples = &prevSamples;
	pv_src.samples = &samples;
	pv_src.totalSamples = &totalSamples;
	pv_src.videoStcoCount = &videoStcoCount;

	pv_src.fp_video_stsc = &fp_stsc;
	pv_src.fp_video_stsz = &fp_stsz;
	pv_src.fp_video_stco = &fp_stco;
	pv_src.fp_frame = &fp_frame;

	// Sound
	ps_src.firstChunk = &firstChunkSound;
	ps_src.prevChunk = &prevChunkSound;
	ps_src.prevSamples = &prevSamplesSound;
	ps_src.samples = &samplesSound;
	ps_src.soundStcoCount = &soundStcoCount;

	ps_src.fp_sound_stsc = &fp_sound_stsc;
	ps_src.fp_sound_stsz = &fp_sound_stsz;
	ps_src.fp_sound_stco = &fp_sound_stco;

	mjpeg_touch.resynch = 0;
	LCD_SetRegion(media.video.startPosX, media.video.startPosY, media.video.startPosX + media.video.width - 1, media.video.startPosY + media.video.height - 1);

	float limitter;

	switch(SystemCoreClock){
	case 168000000:
		limitter = 0.91f;
		break;
	case 200000000:
		limitter = 0.93f;
		break;
	case 240000000:
		limitter = 0.96f;
		break;
	case 250000000:
		limitter = 0.98f;
		break;
	default:
		limitter = 0.8f;
		break;
	}

	videoStcoCount = 0, soundStcoCount = 0;
	int soundEndFlag = 0;

	pcf_font_typedef pcf_font_bak;
	if(pcf_font.ext_loaded)
	{
		memcpy((void*)&pcf_font_bak, (void*)&pcf_font, sizeof(pcf_font_typedef));
		/* internal flash pcf font */
		C_PCFFontInit((uint32_t)internal_flash_pcf_font, (size_t)_sizeof_internal_flash_pcf_font);
		PCF_RENDER_FUNC_C_PCF();
	}

#define RASTER 30

	uint8_t LINE_BUFFER[RASTER * LCD_WIDTH * 2];

    BSP_AUDIO_OUT_Init(0, 0, media.sound.format.sampleSize, media.sound.format.numChannel >= 2 ? media.sound.format.sampleRate : media.sound.format.sampleRate / 2);
//	wm8731_left_headphone_volume_set(121 + vol);

	debug.printf("\r\nhaudio_i2s.State:%d", haudio_i2s.State);

	//    HAL_StatusTypeDef errorState;
	HAL_I2S_Transmit_DMA(&haudio_i2s, (uint16_t*)dac_intr.buff, DMA_MAX(dac_intr.bufferSize / ( AUDIODATA_SIZE )));

	//	SOUNDInitDAC(media.sound.format.sampleRate);
//	if(media.sound.format.sampleSize == 8){
//		SOUNDDMAConf((void*)&DAC->DHR8RD, (media.sound.format.sampleSize / 8) * media.sound.format.numChannel, (media.sound.format.sampleSize / 8) * media.sound.format.numChannel);
//	} else {
//		SOUNDDMAConf((void*)&DAC->DHR12LD, (media.sound.format.sampleSize / 8) * media.sound.format.numChannel, (media.sound.format.sampleSize / 8) * media.sound.format.numChannel);
//	}

	DMA_SOUND_IT_ENABLE;

	LCDStatusStruct.waitExitKey = 1;

	int outflag = 0, count = 0, pause = 0;

	while(1){
		CHUNK_OFFSET_HEAD:
		for(j = 0;j < samples;j++){

			my_fseek(&fp_frame, getSampleSize(atombuf, 4, &fp_stco), SEEK_SET);
			if(media.video.playJpeg){
				my_fread(prevSamplesBuff, 1, prevSamples * 4, &fp_stsz);
			}

			for(i = 0;i < prevSamples;i++){

				sample_time_limit = TIM_DURATION->ARR * limitter;

				frameDuration = getVideoSampleTime(atombuf, ++totalSamples); // get next frame duration

				LCD_SetGramAddr(media.video.startPosX, media.video.startPosY);
				LCD_CMD(0x002C);

				raw.output_scanline = 0;
				raw.frame_size = media.video.width * media.video.height * sizeof(uint16_t);
				raw.rasters = RASTER;
				raw.buf_size = raw.rasters * media.video.width * sizeof(uint16_t);
				memcpy((void*)&fp_frame_cp, (void*)&fp_frame, sizeof(MY_FILE));
				my_fseek(&fp_frame, raw.frame_size, SEEK_CUR);
				totalBytes += raw.frame_size;

				DMA_SOUND_IT_ENABLE; // Enable DAC interrupt
				while(!TIM3_SR_UIF_BB){ // while TIM3->SR Update Flag is unset
					 if ((raw.output_scanline < media.video.height) && (TIM_DURATION->CNT < sample_time_limit)){ // Uncompress draw rasters
						if(raw.frame_size < raw.buf_size){
							raw.buf_size = raw.frame_size;
						}

						while(SpiLcdHandle.State != HAL_SPI_STATE_READY)
						{
							if((TIM_DURATION->CNT >= sample_time_limit)){
								HAL_DMA_Abort(SpiLcdHandle.hdmatx);
								SPI_LCD_NSS_PIN_DEASSERT;
								goto EXIT_LOOP;
							}
						}

						DMA_SOUND_IT_DISABLE;
						my_fread((void*)LINE_BUFFER, 1, raw.buf_size, &fp_frame_cp);
						DMA_SOUND_IT_ENABLE;

						SPI_LCD_NSS_PIN_ASSERT;
						SPI_LCD_RS_PIN_DEASSERT;
						HAL_SPI_Transmit_DMA(&SpiLcdHandle, (uint8_t*)LINE_BUFFER, raw.buf_size / sizeof(uint16_t));

						raw.frame_size -= raw.buf_size;
						raw.output_scanline += raw.rasters;
					}
					if((abs(soundStcoCount - videoStcoCount) > 1) && !soundEndFlag){ //　correct synch unmatch
						if(soundStcoCount >= (sound_stco.numEntry - 2) || videoStcoCount >= (video_stco.numEntry - 2)){
							goto END_PROCESS;
						}
						mjpeg_touch.resynch = 1;
						mjpeg_touch.resynch_entry = soundStcoCount > videoStcoCount ? videoStcoCount : soundStcoCount;
						debug.printf("\r\n*synch unmatch at video_stco:%d sound_stco:%d\n", videoStcoCount, soundStcoCount);
						DMA_SOUND_IT_DISABLE; // Disable DAC interrupt
						mjpegTouch(id, mjpeg_touch.resynch_entry);
						samples /= prevSamples;
						mjpeg_touch.resynch = 0;
						getVideoSampleTime(atombuf, 0); // reset sample time
						getVideoSampleTime(atombuf, totalSamples); // get next sample time
						dac_intr.sound_reads = prevSamplesSound * soundSampleBlocks; // fill DAC buffer
						videoStcoCount -= 2, soundStcoCount -= 2;
						goto CHUNK_OFFSET_HEAD;
					}
					if(dac_intr.sound_reads >= (prevSamplesSound * soundSampleBlocks)){
						if(++soundStcoCount < sound_stco.numEntry){
							soundEndFlag = 0;

							totalBytes += dac_intr.sound_reads;

							my_fseek(dac_intr.fp, getSampleSize(atombuf, 4, &fp_sound_stco), SEEK_SET);

							dac_intr.sound_reads = 0;
							if(++stco_reads > samplesSound){
								stco_reads = 0;
								prevChunkSound = firstChunkSound; // 今回のfirstChunkをprevChunkに
								prevSamplesSound = getSampleSize(atombuf, 12, &fp_sound_stsc); // samplesPerChunk sampleDescriptionID
								firstChunkSound = getAtomSize(&atombuf[8]); // 次のfirstChunk
								samplesSound = firstChunkSound - prevChunkSound; // 次回再生チャンクのサンプル数
							}
						} else {
							soundEndFlag = 1;
							dac_intr.sound_reads = 0;
							DMA_SOUND_IT_DISABLE;
						}
					}

					if(!outflag && (++count >= 100000)){
						outflag = 1;
						if(!music_control.b.mute){
							HAL_I2S_DMAPause(&haudio_i2s);
							Delay_us(3);
							wm8731_left_headphone_volume_set(121 + vol);
							HAL_I2S_DMAResume(&haudio_i2s);
						}
					}

					switch(LCDStatusStruct.waitExitKey)
		 			{
		 			case VOL_UP:
		 				HAL_I2S_DMAPause(&haudio_i2s);
		 				Delay_us(3);
		 				vol = ++vol < 6 ? vol : 6;
		 				wm8731_left_headphone_volume_set(121 + vol);
		 				HAL_I2S_DMAResume(&haudio_i2s);
		 				music_control.b.mute = 0;
		 				LCDStatusStruct.waitExitKey = 1;
		 				break;
		 			case VOL_DOWN:
		 				HAL_I2S_DMAPause(&haudio_i2s);
		 				Delay_us(3);
		 				vol = --vol > -121 ? vol : -121;
		 				wm8731_left_headphone_volume_set(121 + vol);
		 				HAL_I2S_DMAResume(&haudio_i2s);
		 				music_control.b.mute = 0;
		 				LCDStatusStruct.waitExitKey = 1;
		 				break;
		 			case VOL_MUTE:
		 				HAL_I2S_DMAPause(&haudio_i2s);
		 				Delay_us(3);
		 				if(music_control.b.mute){
		 					music_control.b.mute = 0;
		 	 				wm8731_left_headphone_volume_set(121 + vol);
		 				} else {
		 					music_control.b.mute = 1;
		 	 				wm8731_left_headphone_volume_set(0);
		 				}
		 				HAL_I2S_DMAResume(&haudio_i2s);
		 				LCDStatusStruct.waitExitKey = 1;
		 				break;
		 			case PLAY_PAUSE:
		 			case PLAY_SW_HOLD_LEFT:
		 			case PLAY_SW_HOLD_RIGHT:
		 			case PLAY_LOOP_MODE:
		 				HAL_I2S_DMAPause(&haudio_i2s);
		 				Delay_us(3);
	 	 				wm8731_left_headphone_volume_set(0);
		 				HAL_I2S_DMAResume(&haudio_i2s);
		 				DMA_SOUND_IT_DISABLE;

						raw.frame_size = media.video.width * media.video.height * sizeof(uint16_t);
						memcpy((void*)&fp_frame_cp, (void*)&fp_frame, sizeof(MY_FILE));
		 				my_fseek(&fp_frame_cp, -raw.frame_size, SEEK_CUR);
						memset((void*)frame_buffer, 0, FRAME_BUFFER_SIZE);

						int v;
						for(v = 0;v < media.video.height;v++){
							my_fread(&frame_buffer[media.video.startPosX + v * LCD_WIDTH + media.video.startPosY * LCD_WIDTH], 2, media.video.width, &fp_frame_cp);
						}

						ret = mjpegPause(id);
						outflag = 0, count = 0;
						if(ret == RET_PLAY_STOP || ret == RET_PLAY_NEXT || ret == RET_PLAY_PREV){
							goto END_PROCESS;
						}
						if(ret == 1){ // 一時停止処理へ飛ぶ 返り値:0 そのまま復帰 :1 移動先のサンプルタイムを取得
							samples /= prevSamples;
							getVideoSampleTime(atombuf, 0); // サンプルタイム初期化
							getVideoSampleTime(atombuf, totalSamples); // 移動先のサンプルタイム取得
							dac_intr.sound_reads = prevSamplesSound * soundSampleBlocks; // DAC読み込みバッファ数を埋めておく
//							videoStcoCount -= 2, soundStcoCount -= 2;
							ret = 0;
							goto CHUNK_OFFSET_HEAD;
						}
		 				LCDStatusStruct.waitExitKey = 1;
						break;
		 			case PLAY_NEXT:
		 				ret = RET_PLAY_NEXT;
		 				goto END_PROCESS;
		 				break;
		 			case PLAY_PREV:
		 				ret = RET_PLAY_PREV;
		 				goto END_PROCESS;
		 				break;
		 			default:
		 				break;
		 			}
				}
				EXIT_LOOP:
				DMA_SOUND_IT_DISABLE; // DAC割込み不許可

				// フレームあたりのタイムデュレーションタイマ(1/100ms単位で指定)
				TIM_DURATION->ARR = frameDuration * timeScaleCoeff - 1;
				TIM_DURATION->CR1 = 0;
				TIM_DURATION->CNT = 0; // clear counter
				TIM3_SR_UIF_BB = 0; // clear update flag
				TIM3_DIER_UIE_BB = 1; // set update interrupt
				TIM3_CR1_CEN_BB = 1; // enable tim3

				frames++;
/*
				if(TIM1_SR_UIF_BB){ // フレームレート表示用
					TIM1_SR_UIF_BB = 0;
					fps = frames - prevFrames;
					debug.printf("\r%c%dfps %dkbps v:%d s:%d  ", fps1Hz[fpsCnt++ & 3], fps, (int)((float)(totalBytes * 8) * 0.001f), videoStcoCount, soundStcoCount);
					prevFrames = frames;
					totalBytes = 0;
				}
				*/
				if(!LCDStatusStruct.waitExitKey){
					ret = RET_PLAY_STOP;
					goto END_PROCESS; // play abort
				}
			}
//			AUDIO_OUT_ENABLE;
			if(++videoStcoCount >= video_stco.numEntry){// || soundStcoCount >= (sound_stco.numEntry)){
				goto END_PROCESS; // ビデオチャンクカウントが最後までいったら再生終了
			}
		}
		prevChunk = firstChunk; // 今回のfirstChunkをprevChunkに
		prevSamples = getSampleSize(atombuf, 12, &fp_stsc); // samplesPerChunk sampleDescriptionID
		firstChunk = getAtomSize(&atombuf[8]); // 次のfirstChunk
		samples = firstChunk - prevChunk; // 次回再生チャンクのサンプル数
	}

	END_PROCESS: // 再生終了処理
//	AUDIO_OUT_SHUTDOWN;
	debug.printf("\r\ntotal_samples:%d video_stco_count:%d sound_stco_count:%d", totalSamples, videoStcoCount, soundStcoCount);
//	debug.printf("\r\ntotalRasters:%d", totalRasters);

	HAL_I2S_DMAStop(&haudio_i2s);
	DMA_SOUND_IT_DISABLE;
	Delay_us(10);
//	wm8731_set_active(0);

	wm8731_left_headphone_volume_set(121 -121);

//	if(media.video.playJpeg){
//		(void) jpeg_finish_decompress(&jdinfo);
//		jpeg_destroy_decompress(&jdinfo);
//	}

	EXIT_PROCESS: //

	memset(dac_intr.buff, 0, dac_intr.bufferSize);
	HAL_I2S_Transmit(&haudio_i2s, (uint16_t*)dac_intr.buff, dac_intr.bufferSize / sizeof(uint16_t), 100);

	dac_intr.func = '\0';
	my_fclose(fp);

	LCD_SetRegion(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);

	LCD_DrawSquare(0, 0, LCD_WIDTH, LCD_HEIGHT, BLACK);

	if(pcf_font.ext_loaded)
	{
		memcpy((void*)&pcf_font, (void*)&pcf_font_bak, sizeof(pcf_font_typedef));
		PCF_RENDER_FUNC_PCF();
	}

	LCDStatusStruct.waitExitKey = 0;

	return ret;
}

