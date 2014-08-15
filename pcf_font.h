/*
 * pcf_font.h
 *
 *  Created on: 2012/03/12
 *      Author: murakamimasayuki
 */

#ifndef PCF_FONT_H_
#define PCF_FONT_H_

#include "stm32f4xx_hal.h"
#include "fat.h"


#define PCF_PROPERTIES			(1<<0)
#define PCF_ACCELERATORS		(1<<1)
#define PCF_METRICS				(1<<2)
#define PCF_BITMAPS				(1<<3)
#define PCF_INK_METRICS			(1<<4)
#define	PCF_BDF_ENCODINGS		(1<<5)
#define PCF_SWIDTHS				(1<<6)
#define PCF_GLYPH_NAMES			(1<<7)
#define PCF_BDF_ACCELERATORS	(1<<8)

typedef struct toc_entry {
	uint32_t type;
	uint32_t format;
	uint32_t size;
	uint32_t offset;
} toc_entry;

typedef struct metric_data_typedef {
	int8_t left_sided_bearing, \
			right_sided_bearing, \
			character_width, \
			character_ascent, \
			character_descent;
}metric_data_typedef;

typedef struct encode_info_typedef {
	uint16_t min_char_or_byte2, \
			 max_char_or_byte2, \
			 min_byte1, \
			 max_byte1, \
			 default_char;
}encode_info_typedef;

typedef struct metrics_table_typedef {
	uint32_t size, offset;
	MY_FILE fp;
}metrics_table_typedef;

typedef struct bitmap_table_typedef {
	uint32_t size, offset;
	MY_FILE fp_offset, fp_bitmap;
}bitmap_table_typedef;

typedef struct encode_table_typedef {
	uint32_t size, offset, glyphindeces;
	MY_FILE fp;
}encode_table_typedef;


typedef struct pcf_typedef{
	uint32_t dst_gram_addr;
	int pixelFormat, alphaSoftBlending, enableShadow;
	int color, colorT, colorShadow, size;
} pcf_typedef;


typedef struct{
	union{
		uint16_t d16;
		struct {
			uint16_t B : 5;
			uint16_t G : 6;
			uint16_t R : 5;
		};
	}color;
}pixel_fmt_rgb565_typedef;

typedef struct{
	union{
		uint16_t d16;
		struct {
			uint16_t B : 4;
			uint16_t G : 4;
			uint16_t R : 4;
			uint16_t A : 4;
		};
	}color;
}pixel_fmt_argb4444_typedef;



typedef struct {
	int offsetX, offsetY, enable;
	int color;
}text_shade_typedef;

typedef struct {
	int startPosX, startPosY, \
		borderCols, spaceBetweenLines;
	pcf_typedef pcf;
	int color;
	text_shade_typedef shade;
}text_conf_typedef;

typedef struct cache_typedef {
	void *start_address;
	int glyph_count;
}cache_typedef;

typedef struct metrics {
	uint8_t hSpacing;
}metrics_typedef;

#define PCF_METRICS_DEFAULT_HSPACING 2

#define C_FONT_UNDEF_CODE       0x0080


typedef struct {
	uint32_t table_count;
	metrics_table_typedef metrics_tbl;
	bitmap_table_typedef bitmap_tbl;
	encode_table_typedef enc_tbl;
	encode_info_typedef enc_info;
	cache_typedef cache;
	metrics_typedef metrics;
	int8_t isOK, ext_loaded;
}pcf_font_typedef;

extern pcf_font_typedef pcf_font;


#define PCF_PIXEL_FORMAT_RGB565    0
#define PCF_PIXEL_FORMAT_ARGB4444  1


#define PCF_RENDER_FUNC_C_PCF()	do{LCD_FUNC.putChar = C_PCFPutChar; \
								   LCD_FUNC.putWideChar = C_PCFPutChar; \
								   LCD_FUNC.getCharLength = C_PCFGetCharPixelLength;}while(0)

#define PCF_RENDER_FUNC_PCF()	do{LCD_FUNC.putChar = PCFPutChar; \
								   LCD_FUNC.putWideChar = PCFPutChar; \
								   LCD_FUNC.getCharLength = PCFGetCharPixelLength;}while(0)

extern const char internal_flash_pcf_font[];
extern char _sizeof_internal_flash_pcf_font[];


extern int PCFFontInit(int id);
extern void PCFPutChar(uint16_t code, void *cp);
extern void PCFPutString(const uint16_t *uni_str, int n, pcf_typedef *pcf);
extern void PCFPutString16px(const uint16_t *uni_str, int n, pcf_typedef *pcf);
extern uint16_t PCFGetCharPixelLength(uint16_t code, uint16_t font_width);

extern int C_PCFFontInit(uint32_t fileAddr, size_t fileSize);
extern void C_PCFPutChar(uint16_t code, void *cp);
extern void C_PCFPutChar16px(uint16_t code, void *cp);
extern void C_PCFPutString(const uint16_t *uni_str, int n, pcf_typedef *pcf);
extern void C_PCFPutString16px(const uint16_t *uni_str, int n, pcf_typedef *pcf);
extern uint16_t C_PCFGetCharPixelLength(uint16_t code, uint16_t font_width);




#endif /* PCF_FONT_H_ */
