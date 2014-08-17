/*
 * aac.c
 *
 *  Created on: 2012/03/27
 *      Author: Tonsuke
 */


#include "aac.h"
#include "pcf_font.h"
#include "wm8731.h"

/* ***** BEGIN LICENSE BLOCK *****
 * Source last modified: $Id: main.c,v 1.4 2005/07/05 21:08:13 ehyche Exp $
 *
 * Portions Copyright (c) 1995-2005 RealNetworks, Inc. All Rights Reserved.
 *
 * The contents of this file, and the files included with this file,
 * are subject to the current version of the RealNetworks Public
 * Source License (the "RPSL") available at
 * http://www.helixcommunity.org/content/rpsl unless you have licensed
 * the file under the current version of the RealNetworks Community
 * Source License (the "RCSL") available at
 * http://www.helixcommunity.org/content/rcsl, in which case the RCSL
 * will apply. You may also obtain the license terms directly from
 * RealNetworks.  You may not use this file except in compliance with
 * the RPSL or, if you have a valid RCSL with RealNetworks applicable
 * to this file, the RCSL.  Please see the applicable RPSL or RCSL for
 * the rights, obligations and limitations governing use of the
 * contents of the file.
 *
 * This file is part of the Helix DNA Technology. RealNetworks is the
 * developer of the Original Code and owns the copyrights in the
 * portions it created.
 *
 * This file, and the files included with this file, is distributed
 * and made available on an 'AS IS' basis, WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, AND REALNETWORKS HEREBY DISCLAIMS
 * ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET
 * ENJOYMENT OR NON-INFRINGEMENT.
 *
 * Technology Compatibility Kit Test Suite(s) Location:
 *    http://www.helixcommunity.org/content/tck
 *
 * Contributor(s):
 *
 * ***** END LICENSE BLOCK ***** */

/**************************************************************************************
 * Fixed-point HE-AAC decoder
 * Jon Recker (jrecker@real.com)
 * February 2005
 *
 * main.c - sample command-line test wrapper for decoder
 **************************************************************************************/

#include <stdio.h>
#include <string.h>

#include "aacdec.h"
#include "aaccommon.h"

#include "fat.h"
#include "usart.h"
#include "sound.h"
#include "movie.h"
#include "lcd.h"
#include "icon.h"

#include "main.h"
//#include "bgimage.h"
//#include "board_config.h"

#include "settings.h"

//#include "arm_math.h"
#include "fft.h"

#include <ctype.h>		/* to declare isprint() */


#define READBUF_SIZE	(2 * AAC_MAINBUF_SIZE * AAC_MAX_NCHANS)	/* pick something big enough to hold a bunch of frames */
//#define READBUF_SIZE	(AAC_MAINBUF_SIZE * AAC_MAX_NCHANS)	/* pick something big enough to hold a bunch of frames */


#ifdef AAC_ENABLE_SBR
#define SBR_MUL		2
#else
#define SBR_MUL		1
#endif

#define TAG_MAX_CNT 100

typedef struct {
	int numEntry;
	uint32_t mdat_size;
	MY_FILE fp_stco, fp;
}aac_stco_Typedef;

aac_stco_Typedef *_aac_stco_struct;

volatile int8_t aac_sound_hdlr_found, aac_video_hdlr_found;

uint8_t *_nameTag, *_artistTag, *_albumTag;
media_info_typedef *_media_info;
MY_FILE *_file_covr;

static inline uint32_t getAtomSize(void* atom){
/*
	ret = *(uint8_t*)(atom) << 24;
	ret |= *(uint8_t*)(atom + 1) << 16;
	ret |= *(uint8_t*)(atom + 2) << 8;
	ret |= *(uint8_t*)(atom + 3);
*/
	return __REV(*(uint32_t*)atom);
}


int collectMediaData(MY_FILE *fp, uint32_t parentAtomSize, uint32_t child)
{
	uint8_t atombuf[512];
	int atomSize, totalAtomSize = 0, index, is, contentLength;
	volatile MY_FILE fp_tmp;

	do{
		memset(atombuf, '\0', sizeof(atombuf));
		my_fread(atombuf, 1, 8, fp);
		atomSize = getAtomSize(atombuf);

		debug.printf("\r\n");
		for(is = child;is > 0;is--){
			debug.printf(" ");
		}
		debug.printf("%s %d", (char*)&atombuf[4], atomSize);
		if(atomSize == 0){
			return -1;
		}

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
		case COVR: // art work image
//			my_fseek(fp, 16, SEEK_CUR);
			memcpy((void*)_file_covr, (void*)fp, sizeof(MY_FILE));
			break;
		case CNAM:
			my_fread(atombuf, 1, 16, fp);
			contentLength = getAtomSize(atombuf) - 16;
			contentLength = contentLength < TAG_MAX_CNT ? contentLength : TAG_MAX_CNT;
			my_fread(_nameTag, 1, contentLength, fp);
			_nameTag[contentLength] = '\0';
			break;
		case CART:
			my_fread(atombuf, 1, 16, fp);
			contentLength = getAtomSize(atombuf) - 16;
			contentLength = contentLength < TAG_MAX_CNT ? contentLength : TAG_MAX_CNT;
			my_fread(_artistTag, 1, contentLength, fp);
			_artistTag[contentLength] = '\0';
			break;
		case CALB:
			my_fread(atombuf, 1, 16, fp);
			contentLength = getAtomSize(atombuf) - 16;
			contentLength = contentLength < TAG_MAX_CNT ? contentLength : TAG_MAX_CNT;
			my_fread(_albumTag, 1, contentLength, fp);
			_albumTag[contentLength] = '\0';
			break;
		case META:
			my_fseek((MY_FILE*)&fp_tmp, 4, SEEK_CUR);
			totalAtomSize -= 4;
			atomSize -= 4;
			break;
		case MDAT:
			_aac_stco_struct->mdat_size = atomSize;
			break;
		case MDHD:
			my_fseek(fp, 12, SEEK_CUR); // skip ver/flag  creationtime modificationtime
			my_fread(atombuf, 1, 4, fp); // time scale
			_media_info->sound.timeScale = getAtomSize(atombuf);

			my_fread(atombuf, 1, 4, fp); // duration
			_media_info->sound.duration = getAtomSize(atombuf);
			break;
		case HDLR:
			my_fseek(fp, 4, SEEK_CUR); // skip flag ver
			my_fread(atombuf, 1, 4, fp); // Component type
			my_fread(atombuf, 1, 4, fp); // Component subtype
			if(!strncmp((char*)atombuf, (const char*)"soun", 4)){
				aac_sound_hdlr_found = 1;
			}
			if(!strncmp((char*)atombuf, (const char*)"vide", 4)){
				aac_video_hdlr_found = 1;
			}
			break;
		case STSD:
			if(!aac_sound_hdlr_found || aac_video_hdlr_found){
				break;
			}
			my_fseek(fp, 8, SEEK_CUR); // skip Reserved(6bytes)/Data Reference Index
			my_fread(atombuf, 1, 4, fp); // next atom size
			my_fread(atombuf, 1, sizeof(sound_format), fp);
			memcpy((void*)&_media_info->format, (void*)atombuf, sizeof(sound_format));
			_media_info->format.version = (uint16_t)b2l((void*)&_media_info->format.version, sizeof(uint16_t));
			_media_info->format.revision = (uint16_t)b2l((void*)&_media_info->format.revision, sizeof(uint16_t));
			_media_info->format.vendor = (uint16_t)b2l((void*)&_media_info->format.vendor, sizeof(uint16_t));
			_media_info->format.numChannel = (uint16_t)b2l((void*)&_media_info->format.numChannel, sizeof(uint16_t));
			_media_info->format.sampleSize = (uint16_t)b2l((void*)&_media_info->format.sampleSize, sizeof(uint16_t));
			_media_info->format.complesionID = (uint16_t)b2l((void*)&_media_info->format.complesionID, sizeof(uint16_t));
			_media_info->format.packetSize = (uint16_t)b2l((void*)&_media_info->format.packetSize, sizeof(uint16_t));
			_media_info->format.sampleRate = (uint16_t)b2l((void*)&_media_info->format.sampleRate, sizeof(uint16_t));
			my_fread(atombuf, 1, 4, fp); // etds atom size
			uint32_t etds_size = (uint32_t)b2l((void*)&atombuf[0], sizeof(uint32_t));
			my_fread(atombuf, 1, 512, fp);

			_media_info->bitrate.maxBitrate = 0;
			_media_info->bitrate.avgBitrate = 0;

			int i;
			for(i = 0;i < etds_size - 4 - 1;i++){
				if(atombuf[i] == 0x40 && atombuf[i + 1] == 0x15){
					_media_info->bitrate.maxBitrate = (uint32_t)b2l((void*)&atombuf[i + 5], sizeof(uint32_t));
					_media_info->bitrate.avgBitrate = (uint32_t)b2l((void*)&atombuf[i + 9], sizeof(uint32_t));
					break;
				}
			}
			break;
		case STCO:
			if(!aac_sound_hdlr_found || aac_video_hdlr_found){
				break;
			}
			my_fseek(fp, 4, SEEK_CUR); // skip flag ver
			my_fread(atombuf, 1, 4, fp); // numEntry
			_aac_stco_struct->numEntry = getAtomSize(atombuf);
			memcpy((void*)&_aac_stco_struct->fp_stco, (void*)fp, sizeof(MY_FILE));
			break;
		default:
			break;
		}

		memcpy((void*)fp, (void*)&fp_tmp, sizeof(MY_FILE));

		if(atomHasChild[index]){
			if(collectMediaData(fp, atomSize - 8, child + 1) != 0){ // Re entrant
				return -1;
			}
			memcpy((void*)fp, (void*)&fp_tmp, sizeof(MY_FILE));
		}

NEXT:
		my_fseek(fp, atomSize - 8, SEEK_CUR);
		totalAtomSize += atomSize;
//		debug.printf("\r\n***parentAtomSize:%d totalAtomSize:%d", parentAtomSize, totalAtomSize);
	}while(parentAtomSize > (totalAtomSize + 8));

	return 0;
}



static int FillReadBuffer(unsigned char *readBuf, unsigned char *readPtr, int bufSize, int bytesLeft, MY_FILE *infile)
{
	int nRead;

	/* move last, small chunk from end of buffer to start, then fill with new data */
	memmove(readBuf, readPtr, bytesLeft);
	nRead = my_fread(readBuf + bytesLeft, 1, bufSize - bytesLeft, infile);

	/* zero-pad to avoid finding false sync word after last frame (from old data in readBuf) */
	if (nRead < bufSize - bytesLeft)
		memset(readBuf + bytesLeft + nRead, 0, bufSize - bytesLeft - nRead);

	return nRead;
}

int AACFindSyncSingleChannelElement(unsigned char *buf, int nBytes)
{
	int i;

	/* find byte-aligned syncword (12 bits = 0xFFF) */
	for (i = 0; i < nBytes - 1; i++) {
		if ( (buf[i+0] == 0x21) && ((buf[i+1] & 0xF0) == 0x00) )
			return i;
	}
//	for (i = 0; i < nBytes; i++) {
//		if ( (buf[i] == 0x21) )
//			return i;
//	}


	return -1;
}

void display_tag_info(int id)
{

	short xTag = 5, yTag = 33, disp_limit, strLen = 0, yPos;

	if(!_albumTag[0] && !_artistTag[0]){
		yTag += 20;
	} else if(!_albumTag[0] || !_artistTag[0]){
		yTag += 10;
	}

	pcf_typedef pcf;
	pcf.dst_gram_addr = (uint32_t)frame_buffer;
	pcf.pixelFormat = PCF_PIXEL_FORMAT_RGB565;
	pcf.size = 16;
	pcf.color = WHITE;
	pcf.colorShadow = BLACK;
	pcf.alphaSoftBlending = 1;
	pcf.enableShadow = 1;
	pcf_font.metrics.hSpacing = 2;

	if(_nameTag[0] != 0){
		strLen = LCDGetStringUTF8PixelLength(_nameTag, 16);
		if((xTag + strLen) < LCD_WIDTH){
			disp_limit = xTag + strLen;
		} else {
			disp_limit = LCD_WIDTH - 16;
			yTag -= 8;
		}

		strLen = LCDGetStringUTF8PixelLength(_albumTag, 12);
		if((xTag + strLen) > (LCD_WIDTH - 20)){
			yTag -= 6;
		}
		LCD_GotoXY(xTag, yTag);
		yPos = LCDPutStringUTF8(xTag, disp_limit, 2, _nameTag, &pcf);
		yTag += 24 + yPos;
	} else {
		uint8_t strNameLFN[80];
		if(setLFNname(strNameLFN, id, LFN_WITHOUT_EXTENSION, sizeof(strNameLFN))){
			strLen = LCDGetStringLFNPixelLength(strNameLFN, 16);
			if((xTag + strLen) < LCD_WIDTH){
				disp_limit = LCD_WIDTH - 16;
			} else {
				disp_limit = LCD_WIDTH - 16;
				yTag -= 10;
			}
			LCD_GotoXY(xTag, yTag);
			LCDPutStringLFN(xTag, disp_limit - 1, 2, strNameLFN, &pcf);
		} else {
			char strNameSFN[9];
			memset(strNameSFN, '\0', sizeof(strNameSFN));
			setSFNname(strNameSFN, id);
			LCD_GotoXY(xTag, yTag);
			LCDPutString(strNameSFN, &pcf);
		}
		yTag += 24;
	}

	disp_limit = LCD_WIDTH - 12;

	pcf.size = 12;

	if(_albumTag[0] != 0){
		LCD_GotoXY(xTag, yTag);
		yPos = LCDPutStringUTF8(xTag, disp_limit - 1, _artistTag[0] != 0 ? 1 : 2, _albumTag, &pcf);
		yTag += 20 + yPos;
	}
	if(_artistTag[0] != 0){
		LCD_GotoXY(xTag, yTag);
		LCDPutStringUTF8(xTag, disp_limit - 1, _albumTag[0] != 0 ? 1 : 2, _artistTag, &pcf);
	}

	pcf.size = 12;
	pcf.color = WHITE;
	pcf.colorShadow = GRAY;

	char s[20], s1[10];
	SPRINTF(s, "%d/%d", id, fat.fileCnt - 1);
	LCD_GotoXY(5, MUSIC_INFO_POS_Y + 1);
	if(settings_group.music_conf.b.musicinfo){
		strcat(s, "  AAC ");
		SPRINTF(s1, "%dk ", (int)(_media_info->bitrate.avgBitrate / 1000));
		strcat(s, s1);
		SPRINTF(s1, "%dkHz", (int)(_media_info->format.sampleRate / 1000));
		strcat(s, s1);
	}
	LCDPutString(s, &pcf);

}

int PlayAAC(int id)
{
	int duration = 0, seekBytesSyncWord, media_data_totalBytes = 0;
	int bytesLeft, nRead, err, eofReached;
	int curX = 0, prevX = 0, ret = 0;
	unsigned char *readPtr;
	short *outbuf;

	typedef union
	{
		struct
		{
			uint8_t readBuf[READBUF_SIZE];
			uint8_t SOUND_BUFFER[8192];
		};
	}shared_memory_typedef;

	shared_memory_typedef *sm = (shared_memory_typedef*)frame_buffer;

	HAACDecoder *hAACDecoder;
	AACFrameInfo aacFrameInfo;
	char timeStr[16];

	media_info_typedef media_info;
	_media_info = &media_info;

	uint8_t aac_stco_buf[4];
	aac_stco_Typedef aac_stco_struct;
	_aac_stco_struct = &aac_stco_struct;

	MY_FILE *infile = '\0', infilecp;
	MY_FILE file_covr;
	_file_covr = &file_covr;
	_file_covr->clusterOrg = 0;

	uint8_t *pu8 = (uint8_t*)&cursorRAM[ART_WORK_SIZE * ART_WORK_SIZE];
	_nameTag   = &pu8[0];
	_artistTag = &pu8[TAG_MAX_CNT];
	_albumTag  = &pu8[TAG_MAX_CNT * 2];

	_nameTag[0] = _artistTag[0] = _albumTag[0] = 0;

	extern LCDStatusStruct_typedef LCDStatusStruct;

	/* open input file */
	infile = my_fopen(id);
	if(infile == '\0'){
		LCDStatusStruct.waitExitKey = 0;
		return -1;
	}

	memcpy((void*)&infilecp, (void*)infile, sizeof(MY_FILE));

	debug.printf("\r\n\n*** Atom Info ***");

	hAACDecoder = (HAACDecoder *)AACInitDecoder();

	if (!hAACDecoder) {
		debug.printf("\r\n*** Error initializing decoder ***");
		my_fclose(infile);
		AACFreeDecoder(hAACDecoder);
		LCDStatusStruct.waitExitKey = 0;
		return -1;
	}


	uint8_t atombuf[8];
	my_fread(atombuf, 1, 8, infile);
	seekBytesSyncWord = 0;
	if(strncmp((char*)&atombuf[4], "ftyp", 4) == 0){
		aac_sound_hdlr_found = 0, aac_video_hdlr_found = 0;

		my_fseek(infile, 0, SEEK_SET);
		if(collectMediaData(infile, infile->fileSize, 0) != 0){ // チャネル数、サンプルレート、デュレーション、メタデータ等を取得する
			debug.printf("\r\nCouldn't collect media data");
			my_fclose(infile);
			AACFreeDecoder(hAACDecoder);
			LCDStatusStruct.waitExitKey = 0;
			return -1;
		}

		if (!aac_sound_hdlr_found || aac_video_hdlr_found) {
			debug.printf("\r\nNot supported media containts");
			my_fclose(infile);
			AACFreeDecoder(hAACDecoder);
			LCDStatusStruct.waitExitKey = 0;
			return -1;
		}

		memcpy((void*)infile, (void*)&infilecp, sizeof(MY_FILE));

		memcpy((void*)&_aac_stco_struct->fp, (void*)&_aac_stco_struct->fp_stco, sizeof(MY_FILE));
//		my_fread(aac_stco_buf, 4, &aac_stco_struct.fp);
//		my_fseek(infile, getAtomSize(aac_stco_buf), SEEK_SET);

//		do{
			my_fread(aac_stco_buf, 1, 4, (MY_FILE*)&_aac_stco_struct->fp);
			my_fseek(infile, getAtomSize(aac_stco_buf), SEEK_SET);
//			my_fread(aac_stco_buf, 1, 1, infile);
//		}while(aac_stco_buf[0] != 0x20 && aac_stco_buf[0] != 0x21);
//		my_fseek(infile, -1, SEEK_CUR);

		debug.printf("\r\n\ntimeScale:%d", media_info.sound.timeScale);
		debug.printf("\r\nduration:%d", media_info.sound.duration);
//		debug.printf("\r\n\nmedia_info.format.version:%d",media_info.format.version);
//		debug.printf("\r\nmedia_info.format.revision:%d", media_info.format.revision);
//		debug.printf("\r\nmedia_info.format.vendor:%d", media_info.format.vendor);
		debug.printf("\r\nnumChannel:%d", media_info.format.numChannel);
		debug.printf("\r\nsampleSize:%d", media_info.format.sampleSize);
//		debug.printf("\r\nmedia_info.format.complesionID:%d", media_info.format.complesionID);
//		debug.printf("\r\nmedia_info.format.packetSize:%d", media_info.format.packetSize);
		debug.printf("\r\nsampleRate:%d", media_info.format.sampleRate);

		duration = (int)((float)media_info.sound.duration / (float)media_info.sound.timeScale + 0.5);
		seekBytesSyncWord = infile->seekBytes;
//		media_data_totalBytes = infile->fileSize - seekBytesSyncWord;
		media_data_totalBytes = _aac_stco_struct->mdat_size;

		aacFrameInfo.nChans = media_info.format.numChannel;
//		aacFrameInfo.sampRateCore = media_info.format.sampleRate;
		aacFrameInfo.sampRateCore = media_info.sound.timeScale;
		aacFrameInfo.profile = 0;
		AACSetRawBlockParams(hAACDecoder, 0, &aacFrameInfo);
	} else {
		debug.printf("\r\nnot encapseled data");
		my_fseek(infile, 0, SEEK_SET);
	}

	uint8_t art_work_width, art_work_height;
	ret = LCDArtWork(_file_covr, &art_work_width, &art_work_height);
	LCDFadeIn(2, music_bgimg_160x128);
	memcpy((void*)frame_buffer, (void*)music_bgimg_160x128, sizeof(frame_buffer));

	if(ret){
		LCDPutIcon(ART_WORK_X, ART_WORK_Y + (ART_WORK_SIZE - art_work_height) / 2, art_work_width, art_work_height, cursorRAM, 0);
		LCDDrawReflection(ART_WORK_X, ART_WORK_Y + (ART_WORK_SIZE - art_work_height) / 2, 1, art_work_width, art_work_height, cursorRAM);
	}
	ret = 0;

	display_tag_info(id);

	dac_intr.buff = sm->SOUND_BUFFER;
    dac_intr.bufferSize = (AAC_MAX_NCHANS * AAC_MAX_NSAMPS * SBR_MUL) * sizeof(int16_t) * aacFrameInfo.nChans;

	FFT_Struct_Typedef FFT;
	FFT.bitPerSample = 16;
	FFT.ifftFlag = 0;
	FFT.bitReverseFlag = 1;
	FFT.length = 16;
	FFT.samples = (dac_intr.bufferSize / (sizeof(int16_t) * aacFrameInfo.nChans) / 2) / 1;
	if(aacFrameInfo.nChans < 2){
		FFT.samples >>= 1;
	}
	FFT_Init(&FFT);

	drawBuff_typedef *drawBuff;
	drawBuff = (drawBuff_typedef*)cursorRAM;

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


	pcf_font_typedef pcf_font_bak;
	if(pcf_font.ext_loaded)
	{
		memcpy((void*)&pcf_font_bak, (void*)&pcf_font, sizeof(pcf_font_typedef));
		/* internal flash pcf font */
		C_PCFFontInit((uint32_t)internal_flash_pcf_font, (size_t)_sizeof_internal_flash_pcf_font);
		PCF_RENDER_FUNC_C_PCF();
	}

	pcf_typedef pcf;
	pcf.dst_gram_addr = (uint32_t)frame_buffer;
	pcf.pixelFormat = PCF_PIXEL_FORMAT_RGB565;
	pcf.size = 12;
	pcf.color = WHITE;
	pcf.colorShadow = GRAY;
	pcf.alphaSoftBlending = 1;
	pcf.enableShadow = 1;
	pcf_font.metrics.hSpacing = 3;


	int swHoldCnt, diff, noerror_cnt = 0;
	uint8_t draw_part_item = 0, media_data_denom, position_changed = 0;

	extern I2S_HandleTypeDef haudio_i2s;
	extern int8_t vol;
	int8_t play_pause = 0;
	uint8_t buf[512];
	int16_t findSyncWord;

	time = 0;
	DRAW_TIME_STR();
	DRAW_REMAIN_TIME_STR();
	LCD_FRAME_BUFFER_Transmit(LCD_DMA_TRANSMIT_NOBLOCKING);

	HAL_Delay(30);

	LCD_SetRegion(0, 90, LCD_WIDTH - 1, LCD_HEIGHT - 1);

	bytesLeft = 0;
	eofReached = 0;
	readPtr = sm->readBuf;
	nRead = 0;

	debug.printf("\r\nnumEntry:%d", _aac_stco_struct->numEntry);

	memset(sm->SOUND_BUFFER, 0, sizeof(sm->SOUND_BUFFER));

    BSP_AUDIO_OUT_Init(0, 0, 16, aacFrameInfo.nChans >= 2 ? aacFrameInfo.sampRateCore : aacFrameInfo.sampRateCore / 2);
//	wm8731_left_headphone_volume_set(121 + vol);
//	debug.printf("\r\nhaudio_i2s.State:%d", haudio_i2s.State);
	//    HAL_StatusTypeDef errorState;
	HAL_I2S_Transmit_DMA(&haudio_i2s, (uint16_t*)sm->SOUND_BUFFER, DMA_MAX(dac_intr.bufferSize / ( AUDIODATA_SIZE )));

	__HAL_TIM_CLEAR_FLAG(&Tim1SecHandle, TIM_FLAG_UPDATE);
	__HAL_TIM_CLEAR_IT(&Tim1SecHandle, TIM_IT_UPDATE);
	TIM_1SEC->CNT = 0;

	HAL_TIM_Base_Start_IT(&Tim1SecHandle);
	HAL_NVIC_EnableIRQ(TIM_1SEC_IRQn);
	LCDStatusStruct.waitExitKey = 1;

	uint32_t outflag = 0, count = 0;

	while(!eofReached && LCDStatusStruct.waitExitKey && ((infile->seekBytes - seekBytesSyncWord) < media_data_totalBytes)){

		if(SOUND_DMA_HALF_TRANS_BB){ // Half
 			SOUND_DMA_CLEAR_HALF_TRANS_BB = 1;
 			outbuf = (short*)dac_intr.buff;
 		} else if(SOUND_DMA_FULL_TRANS_BB){ // Full
 			SOUND_DMA_CLEAR_FULL_TRANS_BB = 1;
 			outbuf = (short*)&dac_intr.buff[dac_intr.bufferSize >> 1];
 		} else {
 			switch(LCDStatusStruct.waitExitKey)
 			{
 			case VOL_UP:
 				HAL_I2S_DMAPause(&haudio_i2s);
 				Delay_us(5);
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
 				Delay_us(5);
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
 				Delay_us(5);
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
 	 	 	 		LCD_FRAME_BUFFER_Transmit_Music(LCD_DMA_TRANSMIT_BLOCKING);
 	 				LCDStatusStruct.waitExitKey = 1;
 				} else {
 					if(music_control.b.mute){
 	 	 				DRAW_MUTE_ICON();
 					} else {
 						DRAW_PLAY_ICON();
 					}
 	 	 	 		LCD_FRAME_BUFFER_Transmit_Music(LCD_DMA_TRANSMIT_BLOCKING);
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
 				Delay_us(5);
 				wm8731_left_headphone_volume_set(0);
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
 						my_fseek(infile, -media_data_totalBytes / media_data_denom, SEEK_CUR);
 					}

 					time = duration * (float)(infile->seekBytes - seekBytesSyncWord) / (float)media_data_totalBytes;
 		 			DRAW_TIME_STR();
 		 			DRAW_REMAIN_TIME_STR();

 					DRAW_SEEK_CIRCLE((float)(infile->seekBytes - seekBytesSyncWord) / (float)media_data_totalBytes, seek_active_circle_12x12);
 		 	 		LCD_FRAME_BUFFER_Transmit_Music(LCD_DMA_TRANSMIT_BLOCKING);

 		 	 		HAL_Delay(100);
 		 	 		media_data_denom = 100 / (++swHoldCnt / SW_HOLD_CNT_VAL);
 				}
				while(my_fread(buf, 1, 512, infile)){
					if((findSyncWord = AACFindSyncSingleChannelElement(buf, 512)) != -1){
						my_fseek(infile, -512 + findSyncWord, SEEK_CUR);

						bytesLeft = 0;
	 					eofReached = 0;
	 					readPtr = sm->readBuf;
	 					err = 0;
	 					noerror_cnt = 0;

						break;
					}
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
 				LCDStatusStruct.waitExitKey = 1;
 				break;
 			case PLAY_SW_HOLD_RIGHT:
 				HAL_I2S_DMAPause(&haudio_i2s);
				Delay_us(5);
 				wm8731_left_headphone_volume_set(0);
	 		 	FFT_Display(&FFT, drawBuff, 1); // erase fft
	 		 	DRAW_PAUSE_ICON();
	 		 	media_data_denom = 100;
 				swHoldCnt = SW_HOLD_CNT_VAL;
 				while(HAL_GPIO_ReadPin(SW_PUSH_RIGHT_PORT, SW_PUSH_RIGHT_PIN) == GPIO_PIN_RESET){
 					if((infile->seekBytes + media_data_totalBytes / media_data_denom) > media_data_totalBytes){
 						continue;
 					}
 					my_fseek(infile, media_data_totalBytes / media_data_denom, SEEK_CUR);

 					time = duration * (float)(infile->seekBytes - seekBytesSyncWord) / (float)media_data_totalBytes;
 		 			DRAW_TIME_STR();
 		 			DRAW_REMAIN_TIME_STR();

 					DRAW_SEEK_CIRCLE((float)(infile->seekBytes - seekBytesSyncWord) / (float)media_data_totalBytes, seek_active_circle_12x12);
 		 	 		LCD_FRAME_BUFFER_Transmit_Music(LCD_DMA_TRANSMIT_BLOCKING);

 		 	 		HAL_Delay(100);
 		 	 		media_data_denom = 100 / (++swHoldCnt / SW_HOLD_CNT_VAL);
 				}

				while(my_fread(buf, 1, 512, infile)){
					if((findSyncWord = AACFindSyncSingleChannelElement(buf, 512)) != -1){
						my_fseek(infile, -512 + findSyncWord, SEEK_CUR);

						bytesLeft = 0;
	 					eofReached = 0;
	 					readPtr = sm->readBuf;
	 					err = 0;
	 					noerror_cnt = 0;

						break;
					}
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
				Delay_us(5);
				wm8731_left_headphone_volume_set(121 + vol);
				HAL_I2S_DMAResume(&haudio_i2s);
			}
		}

		do{
			/* somewhat arbitrary trigger to refill buffer - should always be enough for a full frame */
			if (bytesLeft < AAC_MAX_NCHANS * AAC_MAINBUF_SIZE && !eofReached) {
				nRead = FillReadBuffer(sm->readBuf, readPtr, READBUF_SIZE, bytesLeft, infile);
				bytesLeft += nRead;
				readPtr = sm->readBuf;
				if (nRead == 0){
					eofReached = 1;
					debug.printf("\r\neofReached");
					break;
				}
			}

			/* decode one AAC frame */
	 		err = AACDecode(hAACDecoder, &readPtr, &bytesLeft, outbuf);

			if (err) {
				debug.printf("\r\nerr:%d", err);
				// error occurred
				switch (err) {
				case ERR_AAC_INDATA_UNDERFLOW:
					debug.printf("\r\nERR_AAC_INDATA_UNDERFLOW");
					// need to provide more data on next call to AACDecode() (if possible)
					if (eofReached || bytesLeft == READBUF_SIZE){
						debug.printf("\r\noutOfData");
					}
					break;
				default:
					break;
				}

					while(my_fread(buf, 1, 512, infile)){
						if((findSyncWord = AACFindSyncSingleChannelElement(buf, 512)) != -1){
							my_fseek(infile, -512 + findSyncWord, SEEK_CUR);

							bytesLeft = 0;
		 					eofReached = 0;
		 					readPtr = sm->readBuf;
		 					err = 0;
		 		 			outbuf = (short*)dac_intr.buff;

							break;
						}
					}
				noerror_cnt = 0;
				continue;
			} else if(position_changed){
				if(++noerror_cnt >= 30){
					position_changed = 0;
					noerror_cnt = 0;
					if(!music_control.b.mute){
						HAL_I2S_DMAPause(&haudio_i2s);
						Delay_us(5);
						wm8731_left_headphone_volume_set(121 + vol);
						HAL_I2S_DMAResume(&haudio_i2s);
					}
				}
			}
		}while(position_changed == 1);

		if(settings_group.music_conf.b.fft){
			/* sample audio data for FFT calcuration */
	 		FFT_Sample(&FFT, (uint32_t*)outbuf);

	 		/* FFT analyzer left */
	 		FFT_Display(&FFT, drawBuff, 0);
		}

 		LCD_FRAME_BUFFER_Transmit_Music(LCD_DMA_TRANSMIT_COMPBLOCKING);
 		if(draw_part_item == 0)
 		{
 			draw_part_item = 1;
 			DRAW_SEEK_CIRCLE((float)(infile->seekBytes - seekBytesSyncWord) / (float)media_data_totalBytes, seek_circle_12x12);
 			continue;
 		}

 		if(draw_part_item == 1)
 		{
 			draw_part_item = 2;
 			DRAW_TIME_STR();
 			continue;
 		}

 		if(draw_part_item == 2)
 		{
 			draw_part_item = 0;
 			DRAW_REMAIN_TIME_STR();
 			continue;
 		}
	}


	if(!eofReached && !LCDStatusStruct.waitExitKey){
		ret = RET_PLAY_STOP;
	} else {
		ret = RET_PLAY_NORM;
	}

	if (err != ERR_AAC_NONE && err != ERR_AAC_INDATA_UNDERFLOW){
		debug.printf("\r\nError - %d", err);
	}

EXIT_PROCESS:
	memset((void*)sm->SOUND_BUFFER, 0, sizeof(sm->SOUND_BUFFER));
	HAL_I2S_Transmit(&haudio_i2s, (uint16_t*)sm->SOUND_BUFFER, sizeof(sm->SOUND_BUFFER) / sizeof(uint16_t), 100);

    HAL_Delay(20);

	HAL_I2S_DMAStop(&haudio_i2s);
	Delay_us(10);
//	wm8731_set_active(0);

	wm8731_left_headphone_volume_set(121 -121);
//	wm8731_right_headphone_volume_set(121 -121);

//	wm8731_set_dac_soft_mute(1);
//	wm8731_set_power_down_output(1);
//	wm8731_set_power_off(1);
//	init_dac();

	debug.printf("\r\nplay end");
END_AAC:
//	HAL_NVIC_DisableIRQ(TIM_1SEC_IRQn);

	AACFreeDecoder(hAACDecoder);

	LCDStatusStruct.waitExitKey = 0;

	/* close files */
	my_fclose(infile);

	if(pcf_font.ext_loaded)
	{
		memcpy((void*)&pcf_font, (void*)&pcf_font_bak, sizeof(pcf_font_typedef));
		PCF_RENDER_FUNC_PCF();
	}

	LCD_SetRegion(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
	memcpy((void*)frame_buffer, (void*)music_bgimg_160x128, sizeof(frame_buffer));

	return ret;
}

