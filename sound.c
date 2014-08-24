/*
 * sound.c
 *
 *  Created on: 2011/03/12
 *      Author: Tonsuke
 */


#include "stm32f4xx_hal_conf.h"
#include "main.h"
#include "sound.h"

#include "aac.h"
#include "mp3.h"
#include "movie.h"

#include "lcd.h"
#include "icon.h"

#include "wm8731.h"

#include "pcf_font.h"

#include "settings.h"

//#include "arm_math.h"
#include "fft.h"

int8_t vol = -28;

dac_intr_typedef dac_intr;
music_control_typedef music_control;
shuffle_play_typedef shuffle_play;


void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
	int halfBufferSize = dac_intr.bufferSize >> 1;
	uint8_t *outbuf;

	outbuf = (uint8_t*)dac_intr.buff;

	my_fread(outbuf, 1, halfBufferSize, dac_intr.fp);

	dac_intr.sound_reads += halfBufferSize;
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
	int halfBufferSize = dac_intr.bufferSize >> 1;
	uint8_t *outbuf;

	outbuf = (uint8_t*)&dac_intr.buff[halfBufferSize];

	my_fread(outbuf, 1, halfBufferSize, dac_intr.fp);

	dac_intr.sound_reads += halfBufferSize;
}


void DAC_Buffer_Process_Stereo_S16bit()
{
	int halfBufferSize = dac_intr.bufferSize >> 1;
	uint8_t *outbuf;

	if(SOUND_DMA_HALF_TRANS_BB){ // Half
		SOUND_DMA_CLEAR_HALF_TRANS_BB = 1;
		outbuf = (uint8_t*)dac_intr.buff;
	} else if(SOUND_DMA_FULL_TRANS_BB) {	// Full
		SOUND_DMA_CLEAR_FULL_TRANS_BB = 1;
		outbuf = (uint8_t*)&dac_intr.buff[halfBufferSize];
	}

	my_fread(outbuf, 1, halfBufferSize, dac_intr.fp);

	dac_intr.sound_reads += halfBufferSize;
}


char* setStrSec(char *timeStr, int duration)
{
	int minute, sec, it = 0, temp;
	const char asciiTable[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

	if(duration < 0){
		timeStr[it++] = '-';
		duration = abs(duration);
	}

	minute = duration / 60;
	sec = duration % 60;
	if((temp = minute / 100) > 0){
		timeStr[it++] = asciiTable[temp];
		timeStr[it++] = asciiTable[(minute % 100) / 10];
	} else if((temp = minute / 10) > 0) {
		timeStr[it++] = asciiTable[temp];
	}
	timeStr[it++] = asciiTable[minute % 10];
	timeStr[it++] = ':';
	timeStr[it++] = asciiTable[sec / 10];
	timeStr[it++] = asciiTable[sec % 10];
	timeStr[it] = '\0';

	return timeStr;
}

void volFadeIn(int8_t vol)
{
    int t, step, t_vol = 121 + vol;
    step = t_vol / 32;
    for(t = 0;t < t_vol;t += step){
    	wm8731_left_headphone_volume_set(t);
    	Delay_us(20);
    }
	wm8731_left_headphone_volume_set(t_vol);
}

int PlaySound(int id)
{
	int i, duration, curX, prevX, media_data_totalBytes;
	volatile int ret = RET_PLAY_NORM;

	uint32_t *dst32p;
	uint8_t *outbuf;
	char str[10];

	WAVEFormatStruct wav;
	WAVEFormatHeaderStruct wavHeader;
	WAVEFormatChunkStruct wavChunk;

	MY_FILE *infile;

	char timeStr[20];


	if(!(infile = my_fopen(id))){
		ret = RET_PLAY_STOP;
		goto EXIT_WAV;
	}

	my_fread(&wavHeader, 1, sizeof(WAVEFormatHeaderStruct), infile);

	debug.printf("\r\n\n[WAVE]");

	if(strncmp(wavHeader.headStrRIFF, "RIFF", 4) != 0){
		debug.printf("\r\nNot contain RIFF chunk");
		ret = RET_PLAY_STOP;
		goto EXIT_WAV;
	}

	debug.printf("\r\nFile Size:%d", wavHeader.fileSize);

	if(strncmp(wavHeader.headStrWAVE, "WAVE", 4) != 0){
		debug.printf("\r\nThis is not WAVE file.");
		ret = RET_PLAY_STOP;
		goto EXIT_WAV;
	}

	int restBytes = wavHeader.fileSize;

	while(1){ // loop until format chunk is found
		my_fread(&wavChunk, 1, sizeof(WAVEFormatChunkStruct), infile);
		if(strncmp(wavChunk.chunkfmt, "fmt ", 4) == 0){
			break;
		}
		memset(str, '\0', sizeof(str));
		debug.printf("\r\n\nchunkType:%s", strncpy(str, wavChunk.chunkfmt, sizeof(wavChunk.chunkfmt)));
		debug.printf("\r\nchunkSize:%d", wavChunk.chunkSize);
		restBytes = restBytes - wavChunk.chunkSize - sizeof(WAVEFormatChunkStruct);
		if(restBytes <= 0){
			debug.printf("\r\nNot Found Format Chunk.");
			ret = RET_PLAY_STOP;
			goto EXIT_WAV;
		}
		my_fseek(infile, wavChunk.chunkSize, SEEK_CUR);
	}

	my_fread(&wav, 1, sizeof(WAVEFormatStruct), infile);
	my_fseek(infile, wavChunk.chunkSize - sizeof(WAVEFormatStruct), SEEK_CUR);

	restBytes = restBytes - wavChunk.chunkSize - sizeof(WAVEFormatChunkStruct);

	while(1){ // loop until data chunk is found
		my_fread(&wavChunk, 1, sizeof(WAVEFormatChunkStruct), infile);
		if(strncmp(wavChunk.chunkfmt, "data", 4) == 0){
			break;
		}
		memset(str, '\0', sizeof(str));
		debug.printf("\r\n\nchunkType:%s", strncpy(str, wavChunk.chunkfmt, sizeof(wavChunk.chunkfmt)));
		debug.printf("\r\nchunkSize:%d", wavChunk.chunkSize);
		restBytes = restBytes - wavChunk.chunkSize - sizeof(WAVEFormatChunkStruct);
		if(restBytes <= 0){
			debug.printf("\r\nNot Found Format Chunk.");
			ret = RET_PLAY_STOP;
			goto EXIT_WAV;
		}
		my_fseek(infile, wavChunk.chunkSize, SEEK_CUR);
	}

	memset(str, '\0', sizeof(str));
	debug.printf("\r\n\nchunkType:%s", strncpy(str, wavChunk.chunkfmt, sizeof(wavChunk.chunkfmt)));
	debug.printf("\r\nchunkSize:%d", wavChunk.chunkSize);

	debug.printf("\r\n\nformatID:%d", wav.formatID);
	debug.printf("\r\nNum Channel:%d", wav.numChannel);
	debug.printf("\r\nSampling Rate:%d", wav.sampleRate);
	debug.printf("\r\nData Speed:%d", wav.dataSpeed);
	debug.printf("\r\nBlock Size:%d", wav.blockSize);
	debug.printf("\r\nBit Per Sample:%d", wav.bitPerSample);
	debug.printf("\r\nBytes Wave Data:%d", wavChunk.chunkSize);

	if(wav.formatID != 1){
		debug.printf("\r\nthis media is not a PCM.");
		ret = RET_PLAY_STOP;
		goto EXIT_WAV;
	}

	int seekBytesSyncWord = infile->seekBytes;

	media_data_totalBytes = wavChunk.chunkSize;
	duration = wavChunk.chunkSize / wav.dataSpeed;
	setStrSec(timeStr, duration);
	debug.printf("\r\nplay time:%s", timeStr);

	LCDFadeIn(2, music_bgimg_160x128);
	memcpy((void*)frame_buffer, (void*)music_bgimg_160x128, sizeof(frame_buffer));

	short xTag = 5, yTag = 50, disp_limit = 150, strLen;

	pcf_typedef pcf;
	pcf.dst_gram_addr = (uint32_t)frame_buffer;
	pcf.pixelFormat = PCF_PIXEL_FORMAT_RGB565;
	pcf.size = 16;
	pcf.color = WHITE;
	pcf.colorShadow = BLACK;
	pcf.alphaSoftBlending = 1;
	pcf.enableShadow = 1;
	pcf_font.metrics.hSpacing = 2;


	uint8_t strNameLFN[80];

	if(setLFNname(strNameLFN, id, LFN_WITHOUT_EXTENSION, sizeof(strNameLFN))){
		strLen = LCDGetStringLFNPixelLength(strNameLFN, pcf.size);
		if((xTag + strLen) < LCD_WIDTH){
			disp_limit = LCD_WIDTH - 16;
		} else {
			disp_limit = LCD_WIDTH - 16;
			yTag -= 12;
		}
		LCD_GotoXY(xTag, yTag);
		LCDPutStringLFN(xTag, disp_limit - 1, 3, strNameLFN, &pcf);
	} else {
		char strNameSFN[9];
		memset(strNameSFN, '\0', sizeof(strNameSFN));
		setSFNname(strNameSFN, id);
		LCD_GotoXY(xTag, yTag);
		LCDPutString(strNameSFN, &pcf);
	}


	if(settings_group.music_conf.b.musicinfo && (wav.bitPerSample > 16) && (wav.sampleRate >= 48000)){
		pcf.size = 12;
		pcf.color = RED;
		pcf.colorShadow = WHITE;
		LCD_GotoXY(5, 19);
		LCDPutString("Hi-Res ", &pcf);

		LCDPutString(wav.bitPerSample == 24 ? "24bit" : "32bit", &pcf);
	}

	pcf.size = 12;
	pcf.color = WHITE;
	pcf.colorShadow = GRAY;

	char s[30], s1[10];
	SPRINTF(s, "%d/%d", id, fat.fileCnt - 1);
	LCD_GotoXY(5, MUSIC_INFO_POS_Y + 1);
	if(settings_group.music_conf.b.musicinfo){
		strcat(s, "  WAV ");
		SPRINTF(s1, "%.1fM ", (float)(wav.numChannel * wav.bitPerSample * wav.sampleRate) / 1000000.0f);
		strcat(s, s1);
		SPRINTF(s1, "%dkHz", (int)(wav.sampleRate / 1000));
		strcat(s, s1);
	}
	LCDPutString(s, &pcf);

	uint16_t SOUND_BUFFER[12288];
	uint8_t SOUND_BUFFER_24BIT[((sizeof(SOUND_BUFFER) / 2) / sizeof(uint32_t)) * 3];

    dac_intr.fp = infile;
	dac_intr.buff = (uint8_t*)SOUND_BUFFER;
    if(wav.bitPerSample <= 16){
    	dac_intr.bufferSize = sizeof(SOUND_BUFFER) / 2;
    } else {
    	dac_intr.bufferSize = sizeof(SOUND_BUFFER);
    }

	FFT_Struct_Typedef FFT;
	FFT.bitPerSample = wav.bitPerSample;
	FFT.ifftFlag = 0;
	FFT.bitReverseFlag = 1;
	FFT.length = 16;
	FFT.samples = (dac_intr.bufferSize / (sizeof(int16_t) * wav.numChannel) / 2) / 1;
	if(wav.numChannel < 2){
		FFT.samples >>= 1;
	}
	FFT_Init(&FFT);

	drawBuff_typedef *drawBuff, _drawBuff;
	drawBuff = &_drawBuff;

	drawBuff->play_pause.x = 4;
	drawBuff->play_pause.y = UI_POS_Y + 1;
	drawBuff->play_pause.width = 12;
	drawBuff->play_pause.height = 12;
	LCDStoreBgImgToBuff(drawBuff->play_pause.x, drawBuff->play_pause.y, \
						drawBuff->play_pause.width, drawBuff->play_pause.height, drawBuff->play_pause.p);

	if(!music_control.b.mute){
		LCDPutIcon(drawBuff->play_pause.x, drawBuff->play_pause.y, 12, 12, play_icon_12x12, play_icon_12x12_alpha);
	} else {
		LCDPutIcon(drawBuff->play_pause.x, drawBuff->play_pause.y, 12, 12, mute_icon_12x12, mute_icon_12x12_alpha);
	}
	LCDPutIcon(UI_POS_X, UI_POS_Y, 120, 14, seekbar_120x14, seekbar_120x14_alpha);

	drawBuff->fft_analyzer_left.squareSize = 1;
	drawBuff->fft_analyzer_left.spacing = 1;
	drawBuff->fft_analyzer_left.x = 3;
	drawBuff->fft_analyzer_left.y = 125 - drawBuff->fft_analyzer_left.squareSize + 1;
	LCD_StoreFrameBufferToFFTSquares(drawBuff);


	drawBuff->navigation_loop.x = 140; // 140
	drawBuff->navigation_loop.y = UI_POS_Y; // 4
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


	pcf_font_typedef pcf_font_bak;
	if(pcf_font.ext_loaded)
	{
		memcpy((void*)&pcf_font_bak, (void*)&pcf_font, sizeof(pcf_font_typedef));
		/* internal flash pcf font */
		C_PCFFontInit((uint32_t)internal_flash_pcf_font, (size_t)_sizeof_internal_flash_pcf_font);
		PCF_RENDER_FUNC_C_PCF();
	}

	pcf.dst_gram_addr = (uint32_t)frame_buffer;
	pcf.pixelFormat = PCF_PIXEL_FORMAT_RGB565;
	pcf.size = 12;
	pcf.color = WHITE;
	pcf.colorShadow = GRAY;
	pcf.alphaSoftBlending = 1;
	pcf.enableShadow = 1;
	pcf_font.metrics.hSpacing = 3;

	int swHoldCnt, diff, noerror_cnt = 0;
	uint8_t draw_part_item = 0, media_data_denom, position_changed = 0, hires = 0;

    memset((void*)SOUND_BUFFER, 0, sizeof(SOUND_BUFFER));

    if((wav.sampleRate > 48000) || (wav.bitPerSample > 16)){
    	hires = 1;
    	if(wav.sampleRate == 192000){
    		SystemClock_Config(210, 7); // Overclock 120MHz
    	} else {
    		SystemClock_Config(480, 16); //Overclock 120MHz
    	}
    	HAL_Delay(10);

    	USART_Init();

    	debug.printf("\r\nSystemCoreClock:%d", SystemCoreClock);

    	Tim1SecHandle.Instance = TIM_1SEC;
    	HAL_TIM_Base_DeInit(&Tim1SecHandle);
    	Tim1SecHandle.Init.Prescaler = 100 - 1;
    	Tim1SecHandle.Init.Period = 10000 - 1;
    	Tim1SecHandle.Init.RepetitionCounter = (SystemCoreClock / 1000000UL) - 1;
    	Tim1SecHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
    	Tim1SecHandle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    	HAL_TIM_Base_Init(&Tim1SecHandle);
    	__HAL_TIM_CLEAR_IT(&Tim1SecHandle, TIM_IT_UPDATE);
    	TIM_1SEC->CNT = 0;
    }

    switch(wav.bitPerSample){
    case 32:
    	outbuf = (uint8_t*)&dac_intr.buff[0];
    	my_fread(outbuf, 1, dac_intr.bufferSize, dac_intr.fp);
    	dst32p = (uint32_t*)outbuf;
    	for(i = 0;i < dac_intr.bufferSize / sizeof(uint32_t);i++){
			*dst32p = __REV(*dst32p);
			*dst32p = __REV16(*dst32p);
			++dst32p;
    	}
    	break;
    case 24:
    	dst32p = (uint32_t*)SOUND_BUFFER;
    	my_fread(SOUND_BUFFER_24BIT, 1, sizeof(SOUND_BUFFER_24BIT), dac_intr.fp);
    	for(i = 0;i < sizeof(SOUND_BUFFER_24BIT) / 3;i++){
    		*dst32p  = SOUND_BUFFER_24BIT[i * 3 + 2] << 24;
    		*dst32p |= SOUND_BUFFER_24BIT[i * 3 + 1] << 16;
    		*dst32p |= SOUND_BUFFER_24BIT[i * 3 + 0] << 8;
			*dst32p = __REV(*dst32p);
			*dst32p = __REV16(*dst32p);
 			++dst32p;
    	}
       	my_fread(SOUND_BUFFER_24BIT, 1, sizeof(SOUND_BUFFER_24BIT), dac_intr.fp);
       	for(i = 0;i < sizeof(SOUND_BUFFER_24BIT) / 3;i++){
    		*dst32p  = SOUND_BUFFER_24BIT[i * 3 + 2] << 24;
    		*dst32p |= SOUND_BUFFER_24BIT[i * 3 + 1] << 16;
    		*dst32p |= SOUND_BUFFER_24BIT[i * 3 + 0] << 8;
			*dst32p = __REV(*dst32p);
			*dst32p = __REV16(*dst32p);
 			++dst32p;
       	}
       	break;
    default:
    	break;
    }

	extern I2S_HandleTypeDef haudio_i2s;
	HAL_SD_ErrorTypedef errorState;
	int8_t play_pause = 0;

	time = 0;
	int prevTime = 0;
	DRAW_TIME_STR();
	DRAW_REMAIN_TIME_STR();
	LCD_FRAME_BUFFER_Transmit(LCD_DMA_TRANSMIT_NOBLOCKING);

	HAL_Delay(30);

	LCD_SetRegion(0, 90, LCD_WIDTH - 1, LCD_HEIGHT - 1);


	__HAL_TIM_CLEAR_FLAG(&Tim1SecHandle, TIM_FLAG_UPDATE);
	__HAL_TIM_CLEAR_IT(&Tim1SecHandle, TIM_IT_UPDATE);
	TIM_1SEC->CNT = 0;

    BSP_AUDIO_OUT_Init(0, 0, wav.bitPerSample, wav.numChannel >= 2 ? wav.sampleRate : wav.sampleRate / 2);
//	wm8731_left_headphone_volume_set(121 + vol);
	debug.printf("\r\nhaudio_i2s.State:%d", haudio_i2s.State);
	//    HAL_StatusTypeDef errorState;
	HAL_I2S_Transmit_DMA(&haudio_i2s, SOUND_BUFFER, DMA_MAX(dac_intr.bufferSize / ( AUDIODATA_SIZE * (wav.bitPerSample > 16 ? 2 : 1) )));

	HAL_TIM_Base_Start_IT(&Tim1SecHandle);
	HAL_NVIC_EnableIRQ(TIM_1SEC_IRQn);

	LCDStatusStruct.waitExitKey = 1;

	uint32_t count = 0, outflag = 0;

	while(LCDStatusStruct.waitExitKey && ((infile->seekBytes - seekBytesSyncWord) < media_data_totalBytes)){

		if(SOUND_DMA_HALF_TRANS_BB){ // Half
 			SOUND_DMA_CLEAR_HALF_TRANS_BB = 1;
 			outbuf = (uint8_t*)dac_intr.buff;
 		} else if(SOUND_DMA_FULL_TRANS_BB){ // Full
 			SOUND_DMA_CLEAR_FULL_TRANS_BB = 1;
 			outbuf = (uint8_t*)&dac_intr.buff[dac_intr.bufferSize >> 1];
 		} else {
 			switch(LCDStatusStruct.waitExitKey)
 			{
 			case VOL_UP:
 				HAL_I2S_DMAPause(&haudio_i2s);
 				Delay_us(3);
 				vol = ++vol < 6 ? vol : 6;
 				wm8731_left_headphone_volume_set(121 + vol);
 				if(!play_pause){
 					HAL_I2S_DMAResume(&haudio_i2s);
 					if(music_control.b.mute){
 						music_control.b.mute = 0;
 						DRAW_PLAY_ICON();
 					}
 				}
 				LCDStatusStruct.waitExitKey = 1;
 				break;
 			case VOL_DOWN:
 				HAL_I2S_DMAPause(&haudio_i2s);
 				Delay_us(3);
 				vol = --vol > -121 ? vol : -121;
 				wm8731_left_headphone_volume_set(121 + vol);
 				if(!play_pause){
 					HAL_I2S_DMAResume(&haudio_i2s);
 					if(music_control.b.mute){
 						music_control.b.mute = 0;
 						DRAW_PLAY_ICON();
 					}
 				}
 				LCDStatusStruct.waitExitKey = 1;
 				break;
 			case VOL_MUTE:
 				HAL_I2S_DMAPause(&haudio_i2s);
 				Delay_us(3);
 				if(music_control.b.mute){
 					music_control.b.mute = 0;
 	 				wm8731_left_headphone_volume_set(121 + vol);
 	 				if(!play_pause){
 	 					DRAW_PLAY_ICON();
 	 				} else {
 	 					DRAW_PAUSE_ICON();
 	 				}
 				} else {
 					music_control.b.mute = 1;
 	 				wm8731_left_headphone_volume_set(0);
 	 				DRAW_MUTE_ICON();
 				}
 				if(!play_pause){
 	 				HAL_I2S_DMAResume(&haudio_i2s);
 				}
 				LCDStatusStruct.waitExitKey = 1;
 				break;
 			case PLAY_PAUSE:
 				if(!play_pause){
 					play_pause = 1;
 					TIM1->CR1 &= ~1;
 	 				HAL_I2S_DMAPause(&haudio_i2s);
 	 		 		FFT_Display(&FFT, drawBuff, 1); // erase fft
 	 		 		DRAW_PAUSE_ICON();
 	 	 	 		LCD_FRAME_BUFFER_Transmit_Music(LCD_DMA_TRANSMIT_COMPBLOCKING);
 	 				LCDStatusStruct.waitExitKey = 1;
 				} else {
 					if(music_control.b.mute){
 	 	 				DRAW_MUTE_ICON();
 					} else {
 						DRAW_PLAY_ICON();
 					}
 	 	 	 		LCD_FRAME_BUFFER_Transmit_Music(LCD_DMA_TRANSMIT_COMPBLOCKING);
 					play_pause = 0;
 					TIM1->CR1 |= 1;
 	 				HAL_I2S_DMAResume(&haudio_i2s);
 	 				LCDStatusStruct.waitExitKey = 1;
 				}
 				break;
 			case PLAY_LOOP_MODE:
 				Update_Navigation_Loop_Icon(drawBuff, music_control.b.navigation_loop_mode = ++music_control.b.navigation_loop_mode % 5);
 				LCD_FRAME_BUFFER_Transmit_Music(LCD_DMA_TRANSMIT_NOBLOCKING);
 				LCDStatusStruct.waitExitKey = 1;
 				break;
 			case PLAY_SW_HOLD_LEFT:
 				HAL_I2S_DMAPause(&haudio_i2s);
 				Delay_us(3);
 				wm8731_left_headphone_volume_set(0);
				HAL_I2S_DMAResume(&haudio_i2s);
 				FFT_Display(&FFT, drawBuff, 1); // erase fft
 				DRAW_PAUSE_ICON();
 				media_data_denom = 100;
 				swHoldCnt = SW_HOLD_CNT_VAL;
 				while(HAL_GPIO_ReadPin(SW_PUSH_LEFT_PORT, SW_PUSH_LEFT_PIN) == GPIO_PIN_RESET){
 					diff = infile->seekBytes - media_data_totalBytes / media_data_denom;
 					diff = diff - seekBytesSyncWord;
					if(diff < 0){
 						my_fseek(infile, seekBytesSyncWord, SEEK_SET);
 					} else {
 						my_fseek(infile, (-media_data_totalBytes / media_data_denom), SEEK_CUR);
 						my_fseek(infile, -((infile->seekBytes - seekBytesSyncWord) % wav.blockSize), SEEK_CUR);
 					}

 					time = duration * (float)(infile->seekBytes - seekBytesSyncWord) / (float)media_data_totalBytes;
 		 			DRAW_TIME_STR();
 		 			DRAW_REMAIN_TIME_STR();

 					DRAW_SEEK_CIRCLE((float)(infile->seekBytes - seekBytesSyncWord) / (float)media_data_totalBytes, seek_active_circle_12x12);
 		 	 		LCD_FRAME_BUFFER_Transmit_Music(LCD_DMA_TRANSMIT_BLOCKING);

 		 	 		HAL_Delay(100);
 		 	 		media_data_denom = 100 / (++swHoldCnt / SW_HOLD_CNT_VAL);
 				}
 				if(!play_pause){
 					if(music_control.b.mute){
 	 	 				DRAW_MUTE_ICON();
 					} else {
 						DRAW_PLAY_ICON();
 					}
 					HAL_I2S_DMAResume(&haudio_i2s);
 				}
 				__HAL_TIM_CLEAR_FLAG(&Tim1SecHandle, TIM_FLAG_UPDATE);
 				__HAL_TIM_CLEAR_IT(&Tim1SecHandle, TIM_IT_UPDATE);
 				TIM_1SEC->CNT = 0;
 				position_changed = 1;
 				noerror_cnt = 0;
 				outflag = 0, count = 0;
 				draw_part_item = 0;
 	 			prevTime = time;
 				LCDStatusStruct.waitExitKey = 1;
 				break;
 			case PLAY_SW_HOLD_RIGHT:
 				HAL_I2S_DMAPause(&haudio_i2s);
				Delay_us(3);
 				wm8731_left_headphone_volume_set(0);
				HAL_I2S_DMAResume(&haudio_i2s);
	 		 	FFT_Display(&FFT, drawBuff, 1); // erase fft
	 		 	DRAW_PAUSE_ICON();
	 		 	media_data_denom = 100;
 				swHoldCnt = SW_HOLD_CNT_VAL;
 				while(HAL_GPIO_ReadPin(SW_PUSH_RIGHT_PORT, SW_PUSH_RIGHT_PIN) == GPIO_PIN_RESET){
 					if((infile->seekBytes + media_data_totalBytes / media_data_denom) > infile->fileSize){
 						continue;
 					}
 					my_fseek(infile, (media_data_totalBytes / media_data_denom), SEEK_CUR);
 					my_fseek(infile, -((infile->seekBytes - seekBytesSyncWord) % wav.blockSize), SEEK_CUR);

 					time = duration * (float)(infile->seekBytes - seekBytesSyncWord) / (float)media_data_totalBytes;
 		 			DRAW_TIME_STR();
 		 			DRAW_REMAIN_TIME_STR();

 					DRAW_SEEK_CIRCLE((float)(infile->seekBytes - seekBytesSyncWord) / (float)media_data_totalBytes, seek_active_circle_12x12);
 		 	 		LCD_FRAME_BUFFER_Transmit_Music(LCD_DMA_TRANSMIT_BLOCKING);

 		 	 		HAL_Delay(100);
 		 	 		media_data_denom = 100 / (++swHoldCnt / SW_HOLD_CNT_VAL);
 				}
 				if(!play_pause){
 					if(music_control.b.mute){
 	 	 				DRAW_MUTE_ICON();
 					} else {
 						DRAW_PLAY_ICON();
 					}
 					HAL_I2S_DMAResume(&haudio_i2s);
 				}
 				__HAL_TIM_CLEAR_FLAG(&Tim1SecHandle, TIM_FLAG_UPDATE);
 				__HAL_TIM_CLEAR_IT(&Tim1SecHandle, TIM_IT_UPDATE);
 				TIM_1SEC->CNT = 0;
 				position_changed = 1;
 				noerror_cnt = 0;
 				outflag = 0, count = 0;
 				draw_part_item = 0;
 	 			prevTime = time;
 				LCDStatusStruct.waitExitKey = 1;
 				break;
 			case PLAY_NEXT:
 				ret = RET_PLAY_NEXT;
 				goto EXIT_PROCESS;
 				break;
 			case PLAY_PREV:
 				ret = RET_PLAY_PREV;
 				goto EXIT_PROCESS;
 				break;
 			default:
 				break;
 			}
 			continue;
 		}

		if(!outflag && ++count >= 10){
			outflag = 1;
			if(!music_control.b.mute){
				HAL_I2S_DMAPause(&haudio_i2s);
				Delay_us(3);
				wm8731_left_headphone_volume_set(121 + vol);
				HAL_I2S_DMAResume(&haudio_i2s);
			}
		}

		switch(wav.bitPerSample){
		case 24:
			if(!my_fread(SOUND_BUFFER_24BIT, 1, sizeof(SOUND_BUFFER_24BIT), dac_intr.fp)){
				ret = RET_PLAY_NORM;
				goto EXIT_PROCESS;
			}

			dst32p = (uint32_t*)outbuf;
	    	for(i = 0;i < sizeof(SOUND_BUFFER_24BIT) / 3;i++){
	    		*dst32p  = SOUND_BUFFER_24BIT[i * 3 + 2] << 24;
	    		*dst32p |= SOUND_BUFFER_24BIT[i * 3 + 1] << 16;
	    		*dst32p |= SOUND_BUFFER_24BIT[i * 3 + 0] << 8;
				*dst32p = __REV(*dst32p);
				*dst32p = __REV16(*dst32p);
	 			++dst32p;
	    	}

			break;
		case 32:
			if(!my_fread(outbuf, 1, dac_intr.bufferSize >> 1, dac_intr.fp)){
				ret = RET_PLAY_NORM;
				goto EXIT_PROCESS;
			}

			dst32p = (uint32_t*)outbuf;
			for(i = 0;i < (dac_intr.bufferSize >> 1) / sizeof(uint32_t);i++){
				*dst32p = __REV(*dst32p);
				*dst32p = __REV16(*dst32p);
				++dst32p;
			}
			break;
		default:
			if(!my_fread(outbuf, 1, dac_intr.bufferSize >> 1, dac_intr.fp)){
				ret = RET_PLAY_NORM;
				goto EXIT_PROCESS;
			}
			break;
		}

		if(settings_group.music_conf.b.fft && (wav.sampleRate < 192000)){
			/* sample audio data for FFT calcuration */
			FFT_Sample(&FFT, (uint32_t*)outbuf);

			/* FFT analyzer left */
			FFT_Display(&FFT, drawBuff, 0);

			LCD_FRAME_BUFFER_Transmit_Music(LCD_DMA_TRANSMIT_COMPBLOCKING);
		}

 		if(draw_part_item == 0)
 		{
 			if(SpiLcdHandle.State != HAL_SPI_STATE_READY){
 				continue;
 			}
 			draw_part_item = 1;
 			DRAW_SEEK_CIRCLE((float)(infile->seekBytes - seekBytesSyncWord) / (float)media_data_totalBytes, seek_circle_12x12);
 			LCD_FRAME_BUFFER_Transmit_Music(LCD_DMA_TRANSMIT_NOBLOCKING);
 			continue;
 		}

 		if(draw_part_item == 1 && time > prevTime)
 		{
 			if(SpiLcdHandle.State != HAL_SPI_STATE_READY){
 				continue;
 			}
 			draw_part_item = 2;
 			DRAW_TIME_STR();
 			LCD_FRAME_BUFFER_Transmit_Music(LCD_DMA_TRANSMIT_NOBLOCKING);
 			continue;
 		}

 		if(draw_part_item == 2 && time > prevTime)
 		{
 			if(SpiLcdHandle.State != HAL_SPI_STATE_READY){
 				continue;
 			}
 			draw_part_item = 0;
 			DRAW_REMAIN_TIME_STR();
 			LCD_FRAME_BUFFER_Transmit_Music(LCD_DMA_TRANSMIT_NOBLOCKING);

 			prevTime = time;
 			continue;
 		}
	}

	if((infile->seekBytes < infile->fileSize) && !LCDStatusStruct.waitExitKey){
		ret = RET_PLAY_STOP;
	} else {
		ret = RET_PLAY_NORM;
	}

EXIT_PROCESS:
    memset((void*)SOUND_BUFFER, 0, sizeof(SOUND_BUFFER));
	HAL_I2S_Transmit(&haudio_i2s, (uint16_t*)SOUND_BUFFER, sizeof(SOUND_BUFFER) / sizeof(uint16_t), 100);

//	HAL_NVIC_DisableIRQ(TIM_1SEC_IRQn);

    HAL_Delay(20);

	HAL_I2S_DMAStop(&haudio_i2s);
	DMA_SOUND_IT_DISABLE;
	Delay_us(10);
//	wm8731_set_active(0);

	wm8731_left_headphone_volume_set(121 -121);
//	wm8731_right_headphone_volume_set(121 -121);

//	wm8731_set_dac_soft_mute(1);
//	wm8731_set_power_down_output(1);
//	wm8731_set_power_off(1);
//	init_dac();

	debug.printf("\r\nplay end");

	if(pcf_font.ext_loaded)
	{
		memcpy((void*)&pcf_font, (void*)&pcf_font_bak, sizeof(pcf_font_typedef));
		PCF_RENDER_FUNC_PCF();
	}

	if(hires){
		SystemClock_Config(336, 16);
		USART_Init();

		Tim1SecHandle.Instance = TIM_1SEC;
		HAL_TIM_Base_DeInit(&Tim1SecHandle);
		Tim1SecHandle.Init.Prescaler = 100 - 1;
		Tim1SecHandle.Init.Period = 10000 - 1;
		Tim1SecHandle.Init.RepetitionCounter = (SystemCoreClock / 1000000UL) - 1;
		Tim1SecHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
		Tim1SecHandle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    	HAL_TIM_Base_Init(&Tim1SecHandle);
		HAL_TIM_Base_Start_IT(&Tim1SecHandle);
		__HAL_TIM_CLEAR_IT(&Tim1SecHandle, TIM_IT_UPDATE);
		TIM_1SEC->CNT = 0;
	}


	EXIT_WAV:

	my_fclose(infile);

	LCDStatusStruct.waitExitKey = 0;

	LCD_SetRegion(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);

	return ret;
}


void make_shuffle_table(unsigned int seed){
	  int i, j;
	  uint8_t rnd;
	  static uint8_t rand_table[256];

	  srand(seed);

	  if(!shuffle_play.flag_make_rand){
		  shuffle_play.pRandIdEntry = rand_table;
		  shuffle_play.flag_make_rand = 1;
	  }

	  shuffle_play.pRandIdEntry[1] = rand() % (fat.fileCnt - 1) + 1;

	  for(j = 2;j <= (fat.fileCnt - 1);j++){
	  LOOP:
	    rnd = rand() % (fat.fileCnt - 1) + 1;
	    for(i = 1;i < j;i++){
	      if(shuffle_play.pRandIdEntry[i] == rnd){
	        goto LOOP;
	      }
	    }
	    shuffle_play.pRandIdEntry[j] = rnd;
	  }
}

int PlayMusic(int id)
{
	HAL_TIM_Base_MspInit(&TimHandleDebounce); // reset switch state
	static uint8_t pre_navigation_loop_mode = 0;

	int idNext, idCopy = id;
	unsigned int seed;

	if(music_control.b.navigation_loop_mode == NAV_SHUFFLE_PLAY){
		if(!shuffle_play.mode_changed){
			idNext = id;
			if(++idNext >= fat.fileCnt){
				idNext = 1;
			}
			seed = HAL_GetTick();
			do{
				make_shuffle_table(seed);
				seed += 100;
			}while((id == shuffle_play.pRandIdEntry[idNext]) && (fat.fileCnt > 2));
		}
		if(shuffle_play.play_continuous){
			id = shuffle_play.pRandIdEntry[id];
		}
		shuffle_play.mode_changed = 1;
	}
	shuffle_play.play_continuous = 1;

	if((pre_navigation_loop_mode == NAV_SHUFFLE_PLAY) && (music_control.b.navigation_loop_mode != NAV_SHUFFLE_PLAY)){
		id = idCopy;
	}
	pre_navigation_loop_mode = music_control.b.navigation_loop_mode;

	uint16_t entryPointOffset = getListEntryPoint(id);

	if(fbuf[entryPointOffset + 8] != 0x20){
		char fileTypeStr[4];
		memset(fileTypeStr, '\0', sizeof(fileTypeStr));
		strncpy(fileTypeStr, (char*)&fbuf[entryPointOffset + 8], 3);
		if(strcmp(fileTypeStr, "MP4") == 0 || \
		   strcmp(fileTypeStr, "M4A") == 0 || \
		   strcmp(fileTypeStr, "M4P") == 0)
		{
			return PlayAAC(id);
		} else if(strcmp(fileTypeStr, "MP3") == 0){
			return PlayMP3(id);
		} else if(strcmp(fileTypeStr, "WAV") == 0){
			return PlaySound(id);
		} else if(strcmp(fileTypeStr, "MOV") == 0){
			return PlayMovie(id);
		}
	}

	id = idCopy;

	if((music_control.b.navigation_loop_mode == NAV_INFINITE_PLAY_ENTIRE) || (music_control.b.navigation_loop_mode == NAV_SHUFFLE_PLAY) || id <= fat.fileCnt){
		LCDStatusStruct.waitExitKey = 1;
		return RET_PLAY_NEXT;
	} else {
		LCDStatusStruct.waitExitKey = 0;
		return RET_PLAY_INVALID;
	}

	return 0;
}
