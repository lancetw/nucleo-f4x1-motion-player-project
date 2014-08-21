/*
 * mp3.c
 *
 *  Created on: 2012/03/25
 *      Author: Tonsuke
 */


/* ***** BEGIN LICENSE BLOCK *****
 * Version: RCSL 1.0/RPSL 1.0
 *
 * Portions Copyright (c) 1995-2002 RealNetworks, Inc. All Rights Reserved.
 *
 * The contents of this file, and the files included with this file, are
 * subject to the current version of the RealNetworks Public Source License
 * Version 1.0 (the "RPSL") available at
 * http://www.helixcommunity.org/content/rpsl unless you have licensed
 * the file under the RealNetworks Community Source License Version 1.0
 * (the "RCSL") available at http://www.helixcommunity.org/content/rcsl,
 * in which case the RCSL will apply. You may also obtain the license terms
 * directly from RealNetworks.  You may not use this file except in
 * compliance with the RPSL or, if you have a valid RCSL with RealNetworks
 * applicable to this file, the RCSL.  Please see the applicable RPSL or
 * RCSL for the rights, obligations and limitations governing use of the
 * contents of the file.
 *
 * This file is part of the Helix DNA Technology. RealNetworks is the
 * developer of the Original Code and owns the copyrights in the portions
 * it created.
 *
 * This file, and the files included with this file, is distributed and made
 * available on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND REALNETWORKS HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 *
 * Technology Compatibility Kit Test Suite(s) Location:
 *    http://www.helixcommunity.org/content/tck
 *
 * Contributor(s):
 *
 * ***** END LICENSE BLOCK ***** */

/**************************************************************************************
 * Fixed-point MP3 decoder
 * Jon Recker (jrecker@real.com), Ken Cooke (kenc@real.com)
 * June 2003
 *
 * main.c - command-line test app that uses C interface to MP3 decoder
 **************************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "mp3dec.h"
#include "mp3common.h"
#include "mp3.h"
#include "fat.h"
#include "lcd.h"
#include "pcf_font.h"
#include "sound.h"
#include "usart.h"
#include "icon.h"

#include "main.h"
#include "wm8731.h"
#include "settings.h"

//#include "board_config.h"

//#include "mjpeg.h"

//#include "arm_math.h"
#include "fft.h"

//#include "fx.h"


#define READBUF_SIZE		(1024 * 5)	/* feel free to change this, but keep big enough for >= one frame at high bitrates */
#define MAX_ARM_FRAMES		100
#define ARMULATE_MUL_FACT	1

typedef struct {
	char head_char[4];
	uint32_t flags;
	uint32_t frames;
}xing_struct_typedef;

typedef struct {
	char id_str[3];
	uint8_t verU16[2]; // must cast to (uint16_t*)
	uint8_t flag;
	uint8_t p_size[4];
}id3tag_head_struct_typedef;

typedef struct {
	char frameID_str[4];
	uint8_t p_size[4];
	uint8_t flagsU16[2];
}id3v34tag_frame_struct_typedef;

typedef struct {
	char frameID_str[3];
	uint8_t p_size[3];
}id3v2tag_frame_struct_typedef;

typedef struct{
	uint8_t encType;
	uint8_t str_p[100];
	uint16_t flags;
	uint8_t bom[2];
	size_t size;
}id3_nameTag_struct_typedef;

id3_nameTag_struct_typedef *_id3_title, *_id3_album, *_id3_artist;

MY_FILE *_file_artwork;

static uint32_t b2l(void* val, size_t t){
	uint32_t ret = 0;
	size_t tc = t;

	for(;t > 0;t--){
		ret |= *(uint8_t*)(val + tc - t) << 8 * (t - 1);
	}

	return ret;
}


uint32_t unSynchSize(void *p_size){
	uint32_t frameSize;

	frameSize  = ((uint32_t)*(uint8_t*)p_size << 21);
	frameSize |= ((uint32_t)*(uint8_t*)(p_size + 1) << 14);
	frameSize |= ((uint32_t)*(uint8_t*)(p_size + 2) << 7);
	frameSize |= ((uint32_t)*(uint8_t*)(p_size + 3));

	return frameSize;
}

static int ID3_2v34(MY_FILE *fp, uint32_t entireTagSize, uint8_t ver)
{
	uint32_t frameSize;
	int tagPoint;
	id3v34tag_frame_struct_typedef id3tag_frame;
	id3_nameTag_struct_typedef *nameTag;
	uint16_t flags;
	char frameID_str[5];
	uint8_t *str_p;

	tagPoint = sizeof(id3tag_head_struct_typedef);

	debug.printf("\r\nentireTagSize:%d", entireTagSize);

	while(tagPoint < entireTagSize){
		my_fread((void*)&id3tag_frame, 1, sizeof(id3v34tag_frame_struct_typedef), fp);
		frameID_str[4] = '\0';
		strncpy(frameID_str, id3tag_frame.frameID_str, 4);
		if(frameID_str[0] < 0x20 || frameID_str[0] > 0x80){ // not ascii then break
			break;
		}

		debug.printf("\r\n\nframeID_str:%s", frameID_str);
		if(ver == 4){
			frameSize = unSynchSize((void*)&id3tag_frame.p_size);
		} else {
			frameSize = b2l((void*)&id3tag_frame.p_size, sizeof(id3tag_frame.p_size));
		}
		debug.printf("\r\nsize:%d", frameSize);

		flags = *(uint16_t*)&id3tag_frame.flagsU16;
		debug.printf("\r\nflags:%d", flags);

		tagPoint += sizeof(id3v34tag_frame_struct_typedef) + frameSize;

		nameTag = '\0';
		if(strncmp(frameID_str, "TIT2", 4) == 0){
			nameTag = _id3_title;
		} else if(strncmp(frameID_str, "TALB", 4) == 0){
			nameTag = _id3_album;
		} else if(strncmp(frameID_str, "TPE1", 4) == 0){
			nameTag = _id3_artist;
		} else if(strncmp(frameID_str, "APIC", 4) == 0){
			memcpy((void*)_file_artwork, (void*)fp, sizeof(MY_FILE));
			return 0;
		} else {
			my_fseek(fp, frameSize, SEEK_CUR);
			continue;
		}

		if(nameTag != '\0'){
			my_fread((void*)&nameTag->encType, 1, 1, fp);
			frameSize -= sizeof(nameTag->encType);

			if(nameTag->encType){
				my_fread((void*)nameTag->bom, 1, 2, fp);
				frameSize -= sizeof(nameTag->bom);
			}

			str_p = malloc(frameSize);
			my_fread(str_p, 1, frameSize, fp);

			nameTag->flags = flags;
			nameTag->size = (frameSize < sizeof(nameTag->str_p) - 2 ? frameSize : sizeof(nameTag->str_p) - 2);

			memcpy(nameTag->str_p, str_p, nameTag->size);
			nameTag->str_p[nameTag->size] = '\0';
			nameTag->str_p[nameTag->size + 1] = '\0';

			free((void*)str_p);
		}
	}

	return 0;
}

static int ID3_2v2(MY_FILE *fp, uint32_t entireTagSize)
{
	uint32_t frameSize;
	int tagPoint;
	id3v2tag_frame_struct_typedef id3tag_frame;
	id3_nameTag_struct_typedef *nameTag;
	char frameID_str[4];
	uint8_t *str_p;

	tagPoint = sizeof(id3tag_head_struct_typedef);

	while(tagPoint < entireTagSize){
		my_fread((void*)&id3tag_frame, 1, sizeof(id3v2tag_frame_struct_typedef), fp);
		frameID_str[3] = '\0';
		strncpy(frameID_str, id3tag_frame.frameID_str, 3);
		if(frameID_str[0] < 0x20 || frameID_str[0] > 0x80){ // not ascii then break
			break;
		}

		debug.printf("\r\n\nframeID_str:%s", frameID_str);

		frameSize = b2l((void*)&id3tag_frame.p_size, sizeof(id3tag_frame.p_size));
		debug.printf("\r\nsize:%d", frameSize);

		tagPoint += sizeof(id3v2tag_frame_struct_typedef) + frameSize;

		nameTag = '\0';
		if(strncmp(frameID_str, "TT2", 3) == 0){
			nameTag = _id3_title;
		} else if(strncmp(frameID_str, "TAL", 3) == 0){
			nameTag = _id3_album;
		} else if(strncmp(frameID_str, "TP1", 3) == 0){
			nameTag = _id3_artist;
		} else if(strncmp(frameID_str, "PIC", 3) == 0){
			memcpy((void*)_file_artwork, (void*)fp, sizeof(MY_FILE));
			return 0;
		} else {
			my_fseek(fp, frameSize, SEEK_CUR);
			continue;
		}

		if(nameTag != '\0'){
			my_fread((void*)&nameTag->encType, 1, 1, fp);
			frameSize -= sizeof(nameTag->encType);

			if(nameTag->encType){
				my_fread((void*)nameTag->bom, 1, 2, fp);
				frameSize -= sizeof(nameTag->bom);
			}

			str_p = malloc(frameSize);
			my_fread(str_p, 1, frameSize, fp);

			nameTag->size = (frameSize < sizeof(nameTag->str_p) - 2 ? frameSize : sizeof(nameTag->str_p) - 2);

			memcpy(nameTag->str_p, str_p, nameTag->size);
			nameTag->str_p[nameTag->size] = '\0';
			nameTag->str_p[nameTag->size + 1] = '\0';

			free((void*)str_p);
		}
	}

	return 0;
}

void eliminateEndSpace(uint8_t data[], uint8_t len)
{
	int i, chrCnt = 0;

	for(i = len;i > 0;i--){
		if(data[i - 1] != '\0' && data[i - 1] != ' '){
			data[i] ='\0';
			break;
		}
	}

	for(i = 0;i < len;i++){
		if(data[i] != '\0') chrCnt++;
		else break;
	}

	if(chrCnt == 0 || chrCnt == len){
		data[0] = ' ';
	}
}


int ID3_1v01(MY_FILE *fp){
	char tag[4];

	my_fseek(fp, -127, SEEK_END);
	my_fread((uint8_t*)tag, 1, 3, fp);
	if(strncmp(tag, "TAG", 3) == 0){
		debug.printf("\r\nID3v1.x");
		my_fread(_id3_title->str_p, 1, 30, fp);
		my_fread(_id3_artist->str_p, 1, 30, fp);
		my_fread(_id3_album->str_p, 1, 30, fp);

		_id3_title->encType = _id3_artist->encType = _id3_album->encType = 0;

		eliminateEndSpace(_id3_title->str_p, 30);
		eliminateEndSpace(_id3_artist->str_p, 30);
		eliminateEndSpace(_id3_album->str_p, 30);
	}

	return 0;
}

int id3tag(MY_FILE *fp){
	volatile id3tag_head_struct_typedef id3tag;
	uint16_t ver;
	size_t size;

	my_fread((void*)&id3tag, 1, sizeof(id3tag_head_struct_typedef), fp);
	if(strncmp((char*)id3tag.id_str, "ID3", 3) == 0){
		ver = *(uint16_t*)&id3tag.verU16;
		size = unSynchSize((void*)id3tag.p_size);

		debug.printf("\r\nid3tag.version:%d", ver);
		debug.printf("\r\nsize:%d", size);

		switch(ver){
		case 2:
			ID3_2v2(fp, size);
			break;
		case 3:
		case 4:
			ID3_2v34(fp, size, ver);
			break;
		default:
			break;
		}
		my_fseek(fp, size + sizeof(id3tag_head_struct_typedef), SEEK_SET);
	} else {
		ID3_1v01(fp);
		my_fseek(fp, 0, SEEK_SET);
	}

	return 0;
}
void UTF16BigToLittle(uint8_t *str_p, size_t size)
{
	int i;
	uint8_t t_char;

	for(i = 0;i < size;i += sizeof(uint16_t)){
		t_char = str_p[i];
		str_p[i] = str_p[i + 1];
		str_p[i + 1] = t_char;
	}
}

uint16_t MP3PutString(uint16_t startPosX, uint16_t endPosX, uint8_t lineCnt, id3_nameTag_struct_typedef *id3_nameTag, pcf_typedef *pcf)
{
	uint16_t len = 0;

	if(!id3_nameTag->encType){
		len = LCDPutStringSJISN(startPosX, endPosX, lineCnt, id3_nameTag->str_p, pcf);
	} else { // UTF-16
		if(id3_nameTag->bom[0] == 0xfe && id3_nameTag->bom[1] == 0xff){ // UTF-16BE
			UTF16BigToLittle(id3_nameTag->str_p, id3_nameTag->size); // big to little
		}
		len = LCDPutStringLFN(startPosX, endPosX, lineCnt, id3_nameTag->str_p, pcf);
	}
	return len;
}

uint16_t MP3GetStringPixelLength(id3_nameTag_struct_typedef *id3_nameTag, uint16_t font_width)
{
	uint16_t len = 0;

	if(!id3_nameTag->encType){
		len = LCDGetStringSJISPixelLength(id3_nameTag->str_p, font_width);
	} else { // UTF-16
		if(id3_nameTag->bom[0] == 0xfe && id3_nameTag->bom[1] == 0xff){ // UTF-16BE
			UTF16BigToLittle(id3_nameTag->str_p, id3_nameTag->size); // big to little
		}
		len = LCDGetStringLFNPixelLength(id3_nameTag->str_p, font_width);
	}

	return len;
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

int MP3FindXingWord(unsigned char *buf, int nBytes)
{
	int i;

	for (i = 0; i < nBytes - 3; i++) {
		if ((buf[i+0] == 'X') && (buf[i+1] == 'i') && (buf[i+2] == 'n') && (buf[i+3] == 'g'))
			return i;
	}

	return -1;
}

int MP3FindInfoWord(unsigned char *buf, int nBytes)
{
	int i;

	for (i = 0; i < nBytes - 3; i++) {
		if ((buf[i+0] == 'I') && (buf[i+1] == 'n') && (buf[i+2] == 'f') && (buf[i+3] == 'o'))
			return i;
	}

	return -1;
}

#include "mp3/coder.h"

#define SAFE_FREE(x)	{if (x)	free((void*)x);	(x) = 0;}	/* helper macro */

MP3DecInfo *MP3AllocateBuffers(void)
{
	MP3DecInfo *mp3DecInfo;
	FrameHeader *fh;
	SideInfo *si;
	ScaleFactorInfo *sfi;
	HuffmanInfo *hi;
	DequantInfo *di;
	IMDCTInfo *mi;
	SubbandInfo *sbi;

	uint8_t *ptr = (uint8_t*)cursorRAM;
	mp3DecInfo = (MP3DecInfo *)ptr;
	memset(mp3DecInfo, 0, sizeof(MP3DecInfo));

	fh =  (FrameHeader *)     &ptr[sizeof(MP3DecInfo)];//malloc(sizeof(FrameHeader));
	si =  (SideInfo *)        &ptr[sizeof(MP3DecInfo) + sizeof(FrameHeader)];//malloc(sizeof(SideInfo));
	sfi = (ScaleFactorInfo *) &ptr[sizeof(MP3DecInfo) + sizeof(FrameHeader) + sizeof(SideInfo)];//malloc(sizeof(ScaleFactorInfo));
	di =  (DequantInfo *)     &ptr[sizeof(MP3DecInfo) + sizeof(FrameHeader) + sizeof(SideInfo) + sizeof(ScaleFactorInfo)];//malloc(sizeof(DequantInfo));

	hi =  (HuffmanInfo *)     malloc(sizeof(HuffmanInfo));
	sbi = (SubbandInfo *)     malloc(sizeof(SubbandInfo));
	mi =  (IMDCTInfo *)       malloc(sizeof(IMDCTInfo));

	mp3DecInfo->FrameHeaderPS =     (void *)fh;
	mp3DecInfo->SideInfoPS =        (void *)si;
	mp3DecInfo->ScaleFactorInfoPS = (void *)sfi;
	mp3DecInfo->HuffmanInfoPS =     (void *)hi;
	mp3DecInfo->DequantInfoPS =     (void *)di;
	mp3DecInfo->IMDCTInfoPS =       (void *)mi;
	mp3DecInfo->SubbandInfoPS =     (void *)sbi;

	/* important to do this - DSP primitives assume a bunch of state variables are 0 on first use */
	memset(fh, 0,  sizeof(FrameHeader));
	memset(si, 0, sizeof(SideInfo));
	memset(sfi, 0, sizeof(ScaleFactorInfo));
	memset(hi, 0, sizeof(HuffmanInfo));
	memset(di, 0, sizeof(DequantInfo));
	memset(mi, 0, sizeof(IMDCTInfo));
	memset(sbi, 0, sizeof(SubbandInfo));

	return mp3DecInfo;
}

void MP3FreeBuffers(MP3DecInfo *mp3DecInfo)
{
	if (!mp3DecInfo)
		return;

//	SAFE_FREE(mp3DecInfo->FrameHeaderPS);
//	SAFE_FREE(mp3DecInfo->SideInfoPS);
//	SAFE_FREE(mp3DecInfo->ScaleFactorInfoPS);
	SAFE_FREE(mp3DecInfo->HuffmanInfoPS);
//	SAFE_FREE(mp3DecInfo->DequantInfoPS);
	SAFE_FREE(mp3DecInfo->IMDCTInfoPS);
	SAFE_FREE(mp3DecInfo->SubbandInfoPS);

//	SAFE_FREE(mp3DecInfo);
}

int PlayMP3(int id)
{
	short *outbuf;
	int curX = 0, prevX = 0, ret = 0;

    uint8_t buf[512], hasXing = 0, hasInfo = 0;
    uint32_t seekBytes, count = 0, seekBytesSyncWord = 0;
    int xingSeekBytes = 0, infoSeekBytes = 0, header_offset;
    float duration;
    MP3FrameInfo info;
    MY_FILE file_artwork;
    _file_artwork = &file_artwork;
    _file_artwork->clusterOrg = 0;

	extern LCDStatusStruct_typedef LCDStatusStruct;

	MY_FILE *infile = '\0';

	infile = my_fopen(id);
	if(infile == '\0'){
		LCDStatusStruct.waitExitKey = 0;
		ret = -1;
		goto EXIT_PROCESS;
	}

	MP3FrameInfo mp3FrameInfo;
	HMP3Decoder hMP3Decoder = NULL;

	memset(&mp3FrameInfo, '\0', sizeof(mp3FrameInfo));


	unsigned char *readPtr;

	char timeStr[20];

	int media_data_totalBytes;
	int bytesLeft, nRead, err, offset, eofReached;


	id3_nameTag_struct_typedef title, album, artist;
	_id3_title = &title, _id3_album = &album, _id3_artist = &artist;
	short xTag = 5, yTag = 33, disp_limit, strLen = 0, yPos;

	_id3_title->str_p[0] = _id3_album->str_p[0] = _id3_artist->str_p[0] = '\0';

	id3tag(infile);


	typedef union
	{
		struct
		{
			uint8_t readBuf[READBUF_SIZE];
			uint8_t SOUND_BUFFER[9216];
		};
	}shared_memory_typedef;

	shared_memory_typedef *sm = (shared_memory_typedef*)frame_buffer;


	uint8_t art_work_width, art_work_height;
	ret = LCDArtWork(_file_artwork, &art_work_width, &art_work_height);
	LCDFadeIn(2, (void*)music_bgimg_160x128);
	memcpy((void*)frame_buffer, (void*)music_bgimg_160x128, sizeof(frame_buffer));

	if(ret){
		LCDPutIcon(ART_WORK_X, ART_WORK_Y + (ART_WORK_SIZE - art_work_height) / 2, art_work_width, art_work_height, cursorRAM, 0);
		LCDDrawReflection(ART_WORK_X, ART_WORK_Y + (ART_WORK_SIZE - art_work_height) / 2, 1, art_work_width, art_work_height, cursorRAM);
	}
	ret = 0;

	if ( (hMP3Decoder = MP3AllocateBuffers()) == 0 ){
		LCDStatusStruct.waitExitKey = 0;
		ret = -1;
		goto END_MP3;
	}


	if(_id3_title->str_p[0] != '\0'){
		debug.printf("\r\n\ntitle found\r\nsize:%d\r\nencType:%d", _id3_title->size, _id3_title->encType);
		debug.printf("\r\n%04x", *(uint32_t*)_id3_title->str_p);
		debug.printf("\r\n%04x", *(uint32_t*)&_id3_title->str_p[4]);
	}
	if(_id3_album->str_p[0] != '\0'){
		debug.printf("\r\n\nalbum found\r\nsize:%d\r\nencType:%d", _id3_album->size, _id3_album->encType);
		debug.printf("\r\n%04x", *(uint32_t*)_id3_album->str_p);
	}
	if(_id3_artist->str_p[0] != '\0'){
		debug.printf("\r\n\nartist found\r\nsize:%d\r\nencType:%d", _id3_artist->size, _id3_artist->encType);
		debug.printf("\r\n%04x", *(uint32_t*)_id3_artist->str_p);
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

	if(!_id3_album->str_p[0] && !_id3_artist->str_p[0]){
		yTag += 20;
	} else if(!_id3_album->str_p[0] || !_id3_artist->str_p[0]){
		yTag += 10;
	}

	if(_id3_title->str_p[0] != 0){
		strLen = MP3GetStringPixelLength(_id3_title, pcf.size);
		if((xTag + strLen) < LCD_WIDTH){
			disp_limit = xTag + strLen;
		} else {
			disp_limit = LCD_WIDTH - 16;
			yTag -= 8;
		}

		debug.printf("\r\ndisp_limit TITLE:%d", disp_limit);

		strLen = MP3GetStringPixelLength(_id3_album, pcf.size);
		if((xTag + strLen) > (LCD_WIDTH - 20)){
			yTag -= 6;
		}
		LCD_GotoXY(xTag, yTag);
		yPos = MP3PutString(xTag, disp_limit , 2, _id3_title, &pcf);
		yTag += 24 + yPos;
	} else {
		uint8_t strNameLFN[80];
		if(setLFNname(strNameLFN, id, LFN_WITHOUT_EXTENSION, sizeof(strNameLFN))){
			strLen = LCDGetStringLFNPixelLength(strNameLFN, pcf.size);
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

	if(_id3_album->str_p[0] != 0){
		LCD_GotoXY(xTag, yTag);
		yPos = MP3PutString(xTag, disp_limit - 1, _id3_artist->str_p[0] != 0 ? 1 : 2, _id3_album, &pcf);
		yTag += 20 + yPos;
	}
	if(_id3_artist->str_p[0] != 0){
		LCD_GotoXY(xTag, yTag);
		MP3PutString(xTag, disp_limit - 1, _id3_album->str_p[0] != 0 ? 1 : 2, _id3_artist, &pcf);
	}


	bytesLeft = 0;
	eofReached = 0;
	readPtr = sm->readBuf;
	nRead = 0;

	seekBytes = infile->seekBytes;

	while(!eofReached){
		/* somewhat arbitrary trigger to refill buffer - should always be enough for a full frame */
		if (bytesLeft < 2*MAINBUF_SIZE && !eofReached) {
			nRead = FillReadBuffer(sm->readBuf, readPtr, READBUF_SIZE, bytesLeft, infile);
			bytesLeft += nRead;
			readPtr = sm->readBuf;
			if (nRead == 0){
				eofReached = 1;
				break;
			}
		}

		if((header_offset = (MP3FindXingWord(readPtr, bytesLeft))) != -1){
			xingSeekBytes = header_offset + seekBytes;
			hasXing = 1;
		}

		if((header_offset = (MP3FindInfoWord(readPtr, bytesLeft))) != -1){
			infoSeekBytes = header_offset + seekBytes;
			hasInfo = 1;
		}

		/* find start of next MP3 frame - assume EOF if no sync found */
		offset = MP3FindSyncWord(readPtr, bytesLeft);
		if (offset < 0) {
			bytesLeft = 0;
			readPtr = sm->readBuf;
		    seekBytes = infile->seekBytes;
			continue;
		} else {
			my_fseek(infile, seekBytes + offset, SEEK_SET);
			my_fread(buf, 1, 4, infile);

			if(UnpackFrameHeader((MP3DecInfo*)hMP3Decoder, buf) != -1){
				MP3GetLastFrameInfo(hMP3Decoder, &info);
			    if(info.bitrate){
			    	if(!count){
				    	memcpy(&mp3FrameInfo, &info, sizeof(MP3FrameInfo));
				    	debug.printf("\r\n\nmp3FrameInfo");
				    	debug.printf("\r\nbitrate:%d", mp3FrameInfo.bitrate);
				    	debug.printf("\r\nnChans:%d", mp3FrameInfo.nChans);
				    	debug.printf("\r\nsamprate:%d", mp3FrameInfo.samprate);
				    	debug.printf("\r\nbitsPerSample:%d", mp3FrameInfo.bitsPerSample);
				    	debug.printf("\r\noutputSamps:%d", mp3FrameInfo.outputSamps);
				    	debug.printf("\r\nlayer:%d", mp3FrameInfo.layer);
				    	debug.printf("\r\nversion:%d", mp3FrameInfo.version);
				    	seekBytesSyncWord = seekBytes + offset;
			    	}
			    	if(++count >= 2){
			    		break;
			    	}
			    }
			}
			bytesLeft = 0;
			readPtr = sm->readBuf;
		    seekBytes = infile->seekBytes;
			continue;
		}
	}
	debug.printf("\r\nseekBytesSyncWord:%d", seekBytesSyncWord);
	media_data_totalBytes = infile->fileSize - seekBytesSyncWord;
	debug.printf("\r\nmedia_data_totalBytes:%d", media_data_totalBytes);

	if(hasXing){
		debug.printf("\r\nXing");
	}
	if(hasInfo){
		debug.printf("\r\nInfo");
	}

	if(hasInfo){
		hasXing = 1;
		xingSeekBytes = infoSeekBytes;
	}

	if(hasXing){
		debug.printf("\r\nxingSeekBytes:%d", xingSeekBytes);
		xing_struct_typedef xing_struct;
		my_fseek(infile, xingSeekBytes, SEEK_SET);
		my_fread((uint8_t*)&xing_struct, 1, 12, infile);
		if((strncmp(xing_struct.head_char, "Xing", 4) != 0) && (strncmp(xing_struct.head_char, "Info", 4) != 0)){
			goto CALC_CBR_DURATION;
		}
		debug.printf("\r\ncalc VBR duration");
		xing_struct.flags = b2l(&xing_struct.flags, sizeof(xing_struct.flags));
		if(!(xing_struct.flags & 0x0001)){ // has frames?
			goto CALC_CBR_DURATION;
		}
		xing_struct.frames = b2l(&xing_struct.frames, sizeof(xing_struct.frames));
		duration = (float)xing_struct.frames * ((float)(mp3FrameInfo.outputSamps / mp3FrameInfo.nChans) / (float)mp3FrameInfo.samprate) + 0.5f;
		debug.printf("\r\n%s", setStrSec(timeStr, duration));
	} else {
		CALC_CBR_DURATION:
		duration = (float)media_data_totalBytes / (float)mp3FrameInfo.bitrate * 8.0f + 0.5f;
		debug.printf("\r\n%s", setStrSec(timeStr, duration));
	}

	pcf.size = 12;
	pcf.color = WHITE;
	pcf.colorShadow = GRAY;

	char s[20], s1[10];
	SPRINTF(s, "%d/%d", id, fat.fileCnt - 1);
	LCD_GotoXY(5, MUSIC_INFO_POS_Y + 1);
	if(settings_group.music_conf.b.musicinfo){
		strcat(s, "  MP3 ");
		SPRINTF(s1, "%dk ", mp3FrameInfo.bitrate / 1000);
		strcat(s, s1);
		SPRINTF(s1, "%dkHz", (int)(mp3FrameInfo.samprate / 1000));
		strcat(s, s1);
	}
	memcpy((void*)frame_buffer, (void*)music_bgimg_160x128, sizeof(sm->readBuf));
	LCDPutString(s, &pcf);

	debug.printf("\r\nsampleRate:%d", mp3FrameInfo.samprate);

	my_fseek(infile, seekBytesSyncWord, SEEK_SET);


	dac_intr.buff = sm->SOUND_BUFFER;
    dac_intr.bufferSize = (MAX_NCHAN * MAX_NGRAN * MAX_NSAMP) * sizeof(int16_t) * mp3FrameInfo.nChans;

	FFT_Struct_Typedef FFT;
	FFT.bitPerSample = 16;
	FFT.ifftFlag = 0;
	FFT.bitReverseFlag = 1;
	FFT.length = 16;
	FFT.samples = (dac_intr.bufferSize / (sizeof(int16_t) * mp3FrameInfo.nChans) / 2) / 1;
	if(mp3FrameInfo.nChans < 2){
		FFT.samples >>= 1;
	}
	FFT_Init(&FFT);

	bytesLeft = 0;
	eofReached = 0;
	readPtr = sm->readBuf;
	nRead = 0;

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

	time = 0;
	DRAW_TIME_STR();
	DRAW_REMAIN_TIME_STR();
	LCD_FRAME_BUFFER_Transmit(LCD_DMA_TRANSMIT_NOBLOCKING);

	HAL_Delay(30);

	LCD_SetRegion(0, 90, LCD_WIDTH - 1, LCD_HEIGHT - 1);


	__HAL_TIM_CLEAR_FLAG(&Tim1SecHandle, TIM_FLAG_UPDATE);
	__HAL_TIM_CLEAR_IT(&Tim1SecHandle, TIM_IT_UPDATE);
	TIM_1SEC->CNT = 0;

    memset((void*)sm->SOUND_BUFFER, 0, sizeof(sm->SOUND_BUFFER));
    BSP_AUDIO_OUT_Init(0, 0, mp3FrameInfo.bitsPerSample, mp3FrameInfo.nChans >= 2 ? mp3FrameInfo.samprate : mp3FrameInfo.samprate / 2);
	//wm8731_left_headphone_volume_set(121 + vol);
	debug.printf("\r\nhaudio_i2s.State:%d", haudio_i2s.State);
	//    HAL_StatusTypeDef errorState;
	HAL_I2S_Transmit_DMA(&haudio_i2s, (uint16_t*)sm->SOUND_BUFFER, DMA_MAX(dac_intr.bufferSize / ( AUDIODATA_SIZE )));

	HAL_TIM_Base_Start_IT(&Tim1SecHandle);
	HAL_NVIC_EnableIRQ(TIM_1SEC_IRQn);

	LCDStatusStruct.waitExitKey = 1;

	uint32_t outflag = 0, outcount = 0;

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
 				Update_Navigation_Loop_Icon(drawBuff, (music_control.b.navigation_loop_mode = ++music_control.b.navigation_loop_mode % 5));
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
 					my_fseek(infile, media_data_totalBytes / media_data_denom, SEEK_CUR);

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

		if(!outflag && ++outcount >= 10){
			outflag = 1;
			if(!music_control.b.mute){
				HAL_I2S_DMAPause(&haudio_i2s);
				Delay_us(3);
				wm8731_left_headphone_volume_set(121 + vol);
				HAL_I2S_DMAResume(&haudio_i2s);
			}
		}


		/* somewhat arbitrary trigger to refill buffer - should always be enough for a full frame */
		if (bytesLeft < 2*MAINBUF_SIZE && !eofReached) {
			nRead = FillReadBuffer(sm->readBuf, readPtr, READBUF_SIZE, bytesLeft, infile);
			bytesLeft += nRead;
			readPtr = sm->readBuf;
			if (nRead == 0){
				eofReached = 1;
				break;
			}
		}

		/* find start of next MP3 frame - assume EOF if no sync found */
		offset = MP3FindSyncWord(readPtr, bytesLeft);
		if (offset < 0) {
			debug.printf("\r\nno sync found");
			bytesLeft = 0;
			readPtr = sm->readBuf;
			continue;
		}
		readPtr += offset;
		bytesLeft -= offset;

 		/* decode one MP3 frame - if offset < 0 then bytesLeft was less than a full frame */
		err = MP3Decode(hMP3Decoder, &readPtr, &bytesLeft, outbuf, 0);

		if (err) {
			noerror_cnt = 0;
			/* error occurred */
			switch (err) {
			case ERR_MP3_INDATA_UNDERFLOW:
				debug.printf("\r\nERR_MP3_INDATA_UNDERFLOW");
				break;
			case ERR_MP3_MAINDATA_UNDERFLOW:
				/* do nothing - next call to decode will provide more mainData */
				debug.printf("\r\nERR_MP3_MAINDATA_UNDERFLOW");
				break;
			case ERR_MP3_FREE_BITRATE_SYNC:
				debug.printf("\r\nERR_MP3_FREE_BITRATE_SYNC");
				break;
			case ERR_MP3_INVALID_FRAMEHEADER:
			case ERR_MP3_INVALID_HUFFCODES:
				debug.printf("\r\nERR_MP3_INVALID_(FRAMEHEADER | HUFFCODES)");
				bytesLeft = 0;
				readPtr = sm->readBuf;
				continue;
			default:
				debug.printf("\r\nerr:%d", err);
				break;
			}
		} else if(position_changed){
			if(++noerror_cnt >= 30){
				position_changed = 0;
				noerror_cnt = 0;
				if(!music_control.b.mute){
					HAL_I2S_DMAPause(&haudio_i2s);
					Delay_us(3);
					wm8731_left_headphone_volume_set(121 + vol);
					HAL_I2S_DMAResume(&haudio_i2s);
				}
			}
		}

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

	my_fclose(infile);

END_MP3:
//	HAL_NVIC_DisableIRQ(TIM_1SEC_IRQn);

	MP3FreeBuffers(hMP3Decoder);

	LCDStatusStruct.waitExitKey = 0;

	if(pcf_font.ext_loaded)
	{
		memcpy((void*)&pcf_font, (void*)&pcf_font_bak, sizeof(pcf_font_typedef));
		PCF_RENDER_FUNC_PCF();
	}

	LCD_SetRegion(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
	memcpy((void*)frame_buffer, (void*)music_bgimg_160x128, sizeof(frame_buffer));

	return ret;
}
