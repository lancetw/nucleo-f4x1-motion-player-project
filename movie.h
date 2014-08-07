/*
 * mjpeg.h
 *
 *  Created on: 2011/07/10
 *      Author: Tonsuke
 */

#ifndef MOVIE_H_
#define MOVIE_H_

#include "stm32f4xx_hal_conf.h"
#include <stddef.h>
#include "fat.h"


MY_FILE fp_global;

struct {
	int buf_type;
	uint32_t frame_size;
}jpeg_read;


#define CMARK 0xA9

static uint8_t atomHasChild[] = {0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0 ,0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0};

#define ATOM_ITEMS (sizeof(atomHasChild) / sizeof(atomHasChild[0]))

static const uint8_t atomTypeString[ATOM_ITEMS][5] =
{
	"ftyp", // -
	"wide", // -
	"mdat", // -
	"moov", // +
	"mvhd", // -
	"trak", // +
	"tkhd", // -
	"tapt", // +
	"clef", // -
	"prof", // -
	"enof", // -
	"edts", // +
	"elst", // -
	"mdia", // +
	"mdhd", // -
	"hdlr", // -
	"minf", // +
	"vmhd", // -
	"smhd", // -
	"dinf", // +
	"dref", // -
	"stbl", // +
	"stsd", // -
	"stts", // -
	"stsc", // -
	"stsz", // -
	"stco", // -
	"udta", // +
	"free", // -
	"skip", // -
	"meta", // +
	"load", // -
	"iods", // -
	"ilst", // +
	"keys", // -
	"data", // -
	"trkn", // +
	"disk", // +
	"cpil", // +
	"pgap", // +
	"tmpo", // +
	"gnre", // +
	"covr", // -
	{CMARK, 'n', 'a', 'm', '\0'}, // -
	{CMARK, 'A', 'R', 'T', '\0'}, // -
	{CMARK, 'a', 'l', 'b', '\0'}, // -
	{CMARK, 'g', 'e', 'n', '\0'}, // -
	{CMARK, 'd', 'a', 'y', '\0'}, // -
	{CMARK, 't', 'o', 'o', '\0'}, // -
	{CMARK, 'w', 'r', 't', '\0'}, // -
	"----", // -
};

enum AtomEnum {
	FTYP, // -
	WIDE, // -
	MDAT, // -
	MOOV, // +
	MVHD, // -
	TRAK, // +
	TKHD, // -
	TAPT, // +
	CLEF, // -
	PROF, // -
	ENOF, // -
	EDTS, // +
	ELST, // -
	MDIA, // +
	MDHD, // -
	HDLR, // -
	MINF, // +
	VMHD, // -
	SMHD, // -
	DINF, // +
	DREF, // -
	STBL, // +
	STSD, // -
	STTS, // -
	STSC, // -
	STSZ, // -
	STCO, // -
	UDTA, // +
	FREE, // -
	SKIP, // -
	META, // +
	LOAD, // -
	IODS, // -
	ILST, // +
	KEYS, // -
	DATA, // -
	TRKN, // +
	DISK, // +
	CPIL, // +
	PGAP, // +
	TMPO, // +
	GNRE, // +
	COVR, // -
	CNAM, // -
	CART, // -
	CALB, // -
	CGEN, // -
	CDAY, // -
	CTOO, // -
	CWRT, // -
	NONE, // -
};

volatile struct {
	int8_t id, \
		   done, \
		   resynch, \
		   draw_icon;
	uint32_t paused_chunk, \
			 resynch_entry;
} mjpeg_touch;


struct {
	uint32_t *firstChunk, *prevChunk, *prevSamples, *samples, *totalSamples, *videoStcoCount;
	MY_FILE *fp_video_stsc, *fp_video_stsz, *fp_video_stco, *fp_frame;
} pv_src;

struct {
	uint32_t *firstChunk, *prevChunk, *prevSamples, *samples, *soundStcoCount;
	MY_FILE *fp_sound_stsc, *fp_sound_stsz, *fp_sound_stco;
} ps_src;

struct{
	uint32_t numEntry;
	MY_FILE fp;
} sound_stts;

struct{
	uint32_t numEntry;
	MY_FILE fp;
} sound_stsc;

struct{
	uint32_t sampleSize,
	         numEntry;
	MY_FILE fp;
} sound_stsz;


struct{
	uint32_t numEntry;
	MY_FILE fp;
} sound_stco;

struct{
	uint32_t numEntry;
	MY_FILE fp;
} video_stts;

struct{
	uint32_t numEntry;
	MY_FILE fp;
} video_stsc;

struct{
	uint32_t sampleSize,
	         numEntry;
	MY_FILE fp;
} video_stsz;

struct{
	uint32_t numEntry;
	MY_FILE fp;
} video_stco;


typedef struct samples_struct{
	uint32_t numEntry;
	MY_FILE *fp;
} samples_struct;

typedef struct sound_flag{
	uint32_t process, complete;
} sound_flag;

typedef struct video_flag{
	uint32_t process, complete;
} video_flag;

typedef struct sound_format{
	char audioFmtString[4];
	uint8_t reserved[10];
	uint16_t version,
			 revision,
	 	 	 vendor,
	 	 	 numChannel,
	 	 	 sampleSize,
	 	 	 complesionID,
	 	 	 packetSize,
	 	 	 sampleRate,
			 reserved1;
} sound_format;

typedef struct esds_format{
	char esdsString[4];
//	uint8_t reserved[22];
	uint32_t maxBitrate, avgBitrate;
} esds_format;


typedef struct media_sound{
	sound_flag flag;
	sound_format format;
	uint32_t timeScale,
	         duration;
} media_sound;

typedef struct media_video{
	video_flag flag;
	uint32_t timeScale,
	         duration;
	int16_t width, height, frameRate, startPosX, startPosY;
	char videoFmtString[5], videoCmpString[15], playJpeg;
} media_video;

volatile struct{
	media_sound sound;
	media_video video;
} media;

typedef struct {
	int output_scanline, frame_size, rasters, buf_size;
	uint16_t *p_raster;
} raw_video_typedef;


#define DRAW_MOV_TIME_STR() do{LCD_GotoXY(TIME_X, TIME_Y); \
							   LCDPutString(setStrSec(timeStr, time), &pcf);}while(0)

#define DRAW_MOV_REMAIN_TIME_STR() do{LCD_GotoXY(abs(time - duration) > (99 * 60 + 99) ? TIME_REMAIN_X - 6 : TIME_REMAIN_X, TIME_REMAIN_Y); \
									  LCDPutString(setStrSec(timeStr, time - duration), &pcf);}while(0)


#define PERIPH_GET_BB(periph_offset, bitnumber)  (*(volatile uint32_t *)(PERIPH_BB_BASE + (periph_offset - PERIPH_BASE) << 5) + (bitnumber << 2)))

extern uint32_t b2l(void* val, size_t t);
//inline uint32_t getAtomSize(void* atom);
extern int PlayMotionJpeg(int id);
extern int PlayMovie(int id);



#endif /* MOVIE_H_ */
