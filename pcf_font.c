/*
 * pcf_font.c
 *
 *  Created on: 2012/03/12
 *      Author: murakamimasayuki
 */

#include "pcf_font.h"
#include "lcd.h"
#include "fat.h"
#include "cfile.h"
#include "usart.h"
#include <stdlib.h>
#include <string.h>


pcf_font_typedef pcf_font;

//static const int bit_count_table[] = {0, 1, 1, 2};
static const int bit_count_table[] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};

#define DEBUG
#undef DEBUG

#ifdef DEBUG
static const char type[][21] = {
		"PCF_PROPERTIES",
		"PCF_ACCELERATORS",
		"PCF_METRICS",
		"PCF_BITMAPS",
		"PCF_INK_METRICS",
		"PCF_BDF_ENCODINGS",
		"PCF_SWIDTHS",
		"PCF_GLYPH_NAMES",
		"PCF_BDF_ACCELERATORS",
};
#endif

static const float color_tone_table_4bit[] = {
		0,
		0.0625,
		0.125,
		0.1875,
		0.25,
		0.3125,
		0.375,
		0.4375,
		0.5,
		0.5625,
		0.625,
		0.6875,
		0.75,
		0.8125,
		0.875,
		0.9372,
		1.0
};

static const float color_tone_table_3bit[] = {
		0,
		0.111,
		0.222,
		0.333,
		0.444,
		0.555,
		0.666,
		0.777,
		0.888,
		1.0
};

static void PCFDrawPixel(int x, int y, int offsetY, uint8_t glyph_samples[48][16], pcf_typedef *pcf);


int PCFFontInit(int id)
{
	uint8_t buf[512];
	MY_FILE *fp = '\0';
	toc_entry toc;
	int i;

	fp = my_fopen(id);

	if(!fp || id == -1){
		my_fclose((void*)fp);
		pcf_font.isOK = 0;
		return -1;
	}

	my_fread(buf, 1, 4, fp);

	if(strncmp((char*)buf, "\1fcp" , 4) != 0){
		debug.printf("\r\nNot PCF File");
		my_fclose((void*)fp);
		pcf_font.isOK = 0;
		return -1;
	}

	my_fread(buf, 1, sizeof(uint32_t), fp);
	memcpy((void*)&pcf_font.table_count, buf, sizeof(uint32_t));

#ifdef DEBUG
	debug.printf("\r\ntable_count:%d", pcf_font.table_count);
#endif

	for(i = 0;i < pcf_font.table_count;i++){
		my_fread(buf, 1, sizeof(toc_entry), fp);
		memcpy((void*)&toc, buf, sizeof(toc_entry));

#ifdef DEBUG
		debug.printf("\r\n\nEntry#%d", i);

		int type_idx = 0;
		do{
			if((toc.type >> type_idx) & 1){
				debug.printf("\r\n%s", (char*)&type[type_idx][0]);
				break;
			}
		}while(++type_idx < 9);

		debug.printf("\r\ntype:%d", toc.type);
		debug.printf("\r\nformat:%d", toc.format);
		debug.printf("\r\nsize:%d", toc.size);
		debug.printf("\r\noffset:%d", toc.offset);
#endif

		switch(toc.type){
		case PCF_METRICS:
			pcf_font.metrics_tbl.size = toc.size;
			pcf_font.metrics_tbl.offset = toc.offset;
			break;
		case PCF_BITMAPS:
			pcf_font.bitmap_tbl.size = toc.size;
			pcf_font.bitmap_tbl.offset = toc.offset;
			break;
		case PCF_BDF_ENCODINGS:
			pcf_font.enc_tbl.size = toc.size;
			pcf_font.enc_tbl.offset = toc.offset;
			break;
		default:
			break;
		}

	}

#ifdef DEBUG
	debug.printf("\r\npcf_font.metrics_tbl.size:%d", pcf_font.metrics_tbl.size);
	debug.printf("\r\npcf_font.metrics.offset:%d", pcf_font.metrics_tbl.offset);

	debug.printf("\r\npcf_font.bitmap_tbl.size:%d", pcf_font.bitmap_tbl.size);
	debug.printf("\r\npcf_font.bitmap_tbl.offset:%d", pcf_font.bitmap_tbl.offset);

	debug.printf("\r\npcf_font.enc_tbl.size:%d", pcf_font.enc_tbl.size);
	debug.printf("\r\npcf_font.enc_tbl.offset:%d", pcf_font.enc_tbl.offset);
#endif

	// Collect Metrics Information
	my_fseek(fp, pcf_font.metrics_tbl.offset + 6, SEEK_SET); // jump to metrics table. skip format(4bytes), metrics_count(2bytes)
	memcpy((void*)&pcf_font.metrics_tbl.fp, (void*)fp, sizeof(MY_FILE)); // copy file pointer to the metrics table.

	// Collect Encoding Information
	my_fseek(fp, pcf_font.enc_tbl.offset + 4, SEEK_SET); // jump to encoding table. skip format(4bytes)

	my_fread(buf, 1, sizeof(uint16_t), fp);
//	pcf_font.enc_info.min_char_or_byte2 = conv_b2l((void*)buf, sizeof(uint16_t));
	pcf_font.enc_info.min_char_or_byte2 = __REV16(*(uint16_t*)buf);


	my_fread(buf, 1, sizeof(uint16_t), fp);
//	pcf_font.enc_info.max_char_or_byte2 = conv_b2l((void*)buf, sizeof(uint16_t));
	pcf_font.enc_info.max_char_or_byte2 = __REV16(*(uint16_t*)buf);

	my_fread(buf, 1, sizeof(uint16_t), fp);
//	pcf_font.enc_info.min_byte1 = conv_b2l((void*)buf, sizeof(uint16_t));
	pcf_font.enc_info.min_byte1 = __REV16(*(uint16_t*)buf);

	my_fread(buf, 1, sizeof(uint16_t), fp);
//	pcf_font.enc_info.max_byte1 = conv_b2l((void*)buf, sizeof(uint16_t));
	pcf_font.enc_info.max_byte1 = __REV16(*(uint16_t*)buf);

	my_fread(buf, 1, sizeof(uint16_t), fp);
//	pcf_font.enc_info.default_char = conv_b2l((void*)buf, sizeof(uint16_t));
	pcf_font.enc_info.default_char = __REV16(*(uint16_t*)buf);

#ifdef DEBUG
	debug.printf("\r\nmin_char_or_byte2:%d", pcf_font.enc_info.min_char_or_byte2);
	debug.printf("\r\nmax_char_or_byte2:%d", pcf_font.enc_info.max_char_or_byte2);
	debug.printf("\r\nmin_byte1:%d", pcf_font.enc_info.min_byte1);
	debug.printf("\r\nmax_byte1:%d", pcf_font.enc_info.max_byte1);
	debug.printf("\r\ndefault_char:%d", pcf_font.enc_info.default_char);
#endif

	pcf_font.enc_tbl.glyphindeces = (pcf_font.enc_info.max_char_or_byte2 - pcf_font.enc_info.min_char_or_byte2 + 1) * (pcf_font.enc_info.max_byte1 - pcf_font.enc_info.min_byte1 + 1);

#ifdef DEBUG
	debug.printf("\r\nglyphindeces:%d", pcf_font.enc_tbl.glyphindeces);
#endif

	memcpy((void*)&pcf_font.enc_tbl.fp, (void*)fp, sizeof(MY_FILE)); // copy file pointer to the encode table.

	// Collect Bitmap information
//	my_fseek(fp, pcf_font.bitmap_tbl.offset + 8, SEEK_SET); // jump to bitmap_table, skip format(4bytes), glyph_count(4bytes)
	my_fseek(fp, pcf_font.bitmap_tbl.offset, SEEK_SET); // jump to bitmap_table, skip format(4bytes), glyph_count(4bytes)
	my_fread(buf, 1, sizeof(uint32_t), fp);
//	debug.printf("\r\nformat***%08x", conv_b2l((void*)buf, sizeof(uint32_t)));
#ifdef DEBUG
	debug.printf("\r\nformat***%08x", __REV(*(uint32_t*)buf));
#endif
	my_fread(buf, 1, sizeof(uint32_t), fp);


	memcpy((void*)&pcf_font.bitmap_tbl.fp_offset, (void*)fp, sizeof(MY_FILE)); // copy file pointer to the bitmap offset.

	uint32_t glyph_count;
	my_fseek(fp, pcf_font.bitmap_tbl.offset + 4, SEEK_SET); // jump to bitmap_table, skip format(4bytes)
	my_fread(buf, 1, sizeof(uint32_t), fp);
//	glyph_count = conv_b2l((void*)buf, sizeof(uint32_t));
	glyph_count = __REV(*(uint32_t*)buf);
	my_fseek(fp, glyph_count * sizeof(uint32_t) + sizeof(uint32_t) * 4, SEEK_CUR); // skip glyph_count * 4, bitmapSize(uint32_t * 4)
	memcpy((void*)&pcf_font.bitmap_tbl.fp_bitmap, (void*)fp, sizeof(MY_FILE)); // copy file pointer to the bitmap data.

#ifdef DEBUG
	debug.printf("\r\nglyph_count:%d", glyph_count);
#endif

	my_fclose(fp);
	pcf_font.isOK = 1;
	pcf_font.metrics.hSpacing = PCF_METRICS_DEFAULT_HSPACING;

	return 0;
}

static void PCFDrawPixel(int x, int y, int offsetY, uint8_t glyph_samples[48][16], pcf_typedef *pcf)
{
	int i, j, tmp;
	float alpha_ratio = 0.0f;
	uint16_t *p_u16;

	pixel_fmt_rgb565_typedef pixel_rgb565_fg, pixel_rgb565_bg;

	for(i = 0;i < pcf->size;i++){ // オーバーサンプリングしたデータと背景データを合成して描画
		for(j = 0;j < pcf->size;j++){

			if((x + j) < 0 || (x + j) >= LCD_WIDTH){
				continue;
			}

			tmp = i + y + offsetY;

			if(tmp < 0 || tmp >= LCD_HEIGHT){
				continue;
			}

			// サブピクセルのドット合計を求める
			if(pcf->size == 12){
				alpha_ratio = color_tone_table_4bit[glyph_samples[i * 4 + 0][j] + \
				                                    glyph_samples[i * 4 + 1][j] + \
				                                    glyph_samples[i * 4 + 2][j] + \
				                                    glyph_samples[i * 4 + 3][j]]; // アルファ率を取得
			} else {
				alpha_ratio = color_tone_table_3bit[glyph_samples[i * 3 + 0][j] + \
				                                    glyph_samples[i * 3 + 1][j] + \
				                                    glyph_samples[i * 3 + 2][j]]; // アルファ率を取得
			}

			if(alpha_ratio == 0.0f){
				continue;
			}

			p_u16 = (uint16_t*)(pcf->dst_gram_addr + tmp * LCD_WIDTH * sizeof(uint16_t) + (x + j) * sizeof(uint16_t));
//			LCD_SetGramAddr(x + j, tmp);
//			LCD_CMD(0x002C);

			// Foreground Image
			pixel_rgb565_fg.color.d16 = colorc[pcf->color];
			pixel_rgb565_fg.color.R *= alpha_ratio;
			pixel_rgb565_fg.color.G *= alpha_ratio;
			pixel_rgb565_fg.color.B *= alpha_ratio;

			if(pcf->alphaSoftBlending){
				// Background Image
				pixel_rgb565_bg.color.d16 = *p_u16;
//				pixel_rgb565_bg.color.d16 = 0;
				pixel_rgb565_bg.color.R *= (1.0f - alpha_ratio);
				pixel_rgb565_bg.color.G *= (1.0f - alpha_ratio);
				pixel_rgb565_bg.color.B *= (1.0f - alpha_ratio);

				// Add colors
				pixel_rgb565_fg.color.R += pixel_rgb565_bg.color.R;
				pixel_rgb565_fg.color.G += pixel_rgb565_bg.color.G;
				pixel_rgb565_fg.color.B += pixel_rgb565_bg.color.B;
			}
			*p_u16 = pixel_rgb565_fg.color.d16;
//			LCD_DATA(pixel_rgb565_fg.color.d16);
		}
	}
}


void PCFPutChar(uint16_t code, void *cp)
{
	uint64_t bitmap_data_64;
	uint32_t tmp, *p_u32;
	int i, j, offsetY, double_size = 0;
	uint16_t glyph_index;

	uint8_t buf[512], glyph_samples[48][16];
	uint8_t enc1, enc2;

	MY_FILE fpTmp;
	pcf_typedef *pcf = (pcf_typedef*)cp;

	// Get the glyph_index from UTF16 code.
	enc1 = (code >> 8) & 0xff;
	enc2 = code & 0xff;

	float metrics_scale = 0.25f;
	uint8_t sample_mask = 0xf, sample_shift = 4, y_offset = 2;

	if(enc1 > pcf_font.enc_info.max_byte1 || enc1 < pcf_font.enc_info.min_byte1){
		return;
	}

	if(enc2 > pcf_font.enc_info.max_char_or_byte2 || enc2 < pcf_font.enc_info.min_char_or_byte2){
		return;
	}

	switch(pcf->size){
	case 16:
		metrics_scale = 0.333f;
		sample_mask = 0x7;
		sample_shift = 3;
		y_offset = 5;
		break;
	case 12:
	default:
		pcf->size = 12;
		metrics_scale = 0.25f;
		sample_mask = 0xf;
		sample_shift = 4;
		y_offset = 2;
		break;
	}

	// グリフの位置を計算
	tmp = 	((enc1 - pcf_font.enc_info.min_byte1) * \
			(pcf_font.enc_info.max_char_or_byte2 - pcf_font.enc_info.min_char_or_byte2 + 1) + \
			enc2 - pcf_font.enc_info.min_char_or_byte2) * sizeof(uint16_t);

	memcpy((void*)&fpTmp, (void*)&pcf_font.enc_tbl.fp, sizeof(MY_FILE));
	my_fseek(&fpTmp, tmp, SEEK_CUR);
	my_fread(buf, 1, sizeof(uint16_t), &fpTmp);
	//glyph_index = conv_b2l((void*)buf, sizeof(uint16_t));
	glyph_index = __REV16(*(uint16_t*)buf);

//	debug.printf("\r\n\nGlyph Index");
//	debug.printf("\r\nglyph index:0x%02x", glyph_index);
	if(glyph_index == 0xFFFF) {
		return;
//		code = 0x25A1; // 0x25A1;
	}

	// Get the glyph mtric data
	metric_data_typedef metric;
	memcpy((void*)&fpTmp, (void*)&pcf_font.metrics_tbl.fp, sizeof(MY_FILE)); //
	my_fseek(&fpTmp, glyph_index * sizeof(metric_data_typedef), SEEK_CUR);
	my_fread((void*)&metric, 1, sizeof(metric_data_typedef), &fpTmp);

	*(uint32_t*)&metric.left_sided_bearing ^= 0x80808080;
//	metric.left_sided_bearing  ^= (1 << 7);
//	metric.right_sided_bearing ^= (1 << 7);
//	metric.character_width     ^= (1 << 7);
//	metric.character_ascent    ^= (1 << 7);
	metric.character_descent   ^= (1 << 7);
/*
	debug.printf("\r\n\nMetric data code:%04x", code);
	debug.printf("\r\nmetric.left_sided_bearing:%d", metric.left_sided_bearing);
	debug.printf("\r\nmetric.right_sided_bearing:%d", metric.right_sided_bearing);
	debug.printf("\r\nmetric.character_width:%d", metric.character_width);
	debug.printf("\r\nmetric.character_ascent:%d", metric.character_ascent);
	debug.printf("\r\nmetric.character_descent:%d", metric.character_descent);
*/
	// Get the bitmap data offset
	uint32_t bitmap_offset;
	memcpy((void*)&fpTmp, (void*)&pcf_font.bitmap_tbl.fp_offset, sizeof(MY_FILE));
	my_fseek(&fpTmp, glyph_index * sizeof(int32_t), SEEK_CUR); // グリフインデックスが指すビットマップオフセットまでファイルポインタを移動
	my_fread(buf, 1, sizeof(int32_t), &fpTmp);
//	bitmap_offset = conv_b2l((void*)buf, sizeof(int32_t)); // ビットマップオフセット
	bitmap_offset = __REV(*(uint32_t*)buf);

//	debug.printf("\r\n\nBitmap data offset");
//	debug.printf("\r\nglyph offset:0x%04x", bitmap_offset);

	if(metric.right_sided_bearing - metric.left_sided_bearing > 32){ // 右ベアリング - 左ベアリングが32pxを超えていたらダブルサイズ有効
		double_size = 1;
	}

	memcpy((void*)&fpTmp, (void*)&pcf_font.bitmap_tbl.fp_bitmap, sizeof(MY_FILE)); // ビットマップデータまでファイルポインタを移動
	my_fseek(&fpTmp, bitmap_offset, SEEK_CUR);

	// バッファにビットマップデータをキャッシュする
	my_fread((void*)buf, 1, ((metric.character_ascent + metric.character_descent) * \
                             sizeof(uint32_t)) * (double_size == 0 ? 1 : 2), &fpTmp); 	// ビットマップデータが32bitを超える場合は2倍

	memset((void*)glyph_samples, 0, sizeof(glyph_samples));

	p_u32 = (uint32_t*)buf;
	if(!double_size){
		for(i = 0;i < metric.character_ascent + metric.character_descent;i++){
			bitmap_data_64 = __REV(*p_u32++);
			bitmap_data_64 <<= 16; // 48bit - 32bit = 16bit
			for(j = 1;j <= pcf->size;j++){
				glyph_samples[i][pcf->size - j] = bit_count_table[bitmap_data_64 & sample_mask]; // 4bitずつサンプルする
				bitmap_data_64 >>= sample_shift;
			}
		}
	} else {
		for(i = 0;i < metric.character_ascent + metric.character_descent;i++){
			bitmap_data_64 = __REV(*p_u32++);
			bitmap_data_64 <<= 32;
			bitmap_data_64 |= __REV(*p_u32++);
			bitmap_data_64 >>= 16; // 64bit - 48bit = 16bit  余白を右に詰める
			for(j = 1;j <= pcf->size;j++){
				glyph_samples[i][pcf->size - j] = bit_count_table[bitmap_data_64 & sample_mask]; // 4bitずつサンプルする
				bitmap_data_64 >>= sample_shift;
			}
		}
	}

	offsetY = pcf->size - 1 - (int)(((float)(metric.character_ascent) * metrics_scale) + 0.5f); // グリフの高さ揃えパラメータ

	if(!pcf->enableShadow){
		PCFDrawPixel(clx, cly, offsetY, glyph_samples, pcf);
	} else {
		pcf->colorT = pcf->color;
		pcf->color = pcf->colorShadow;

		PCFDrawPixel(clx - 1, cly - 1, offsetY, glyph_samples, pcf);
		PCFDrawPixel(clx + 0, cly - 1, offsetY, glyph_samples, pcf);
		PCFDrawPixel(clx + 1, cly - 1, offsetY, glyph_samples, pcf);

		PCFDrawPixel(clx - 1, cly + 0, offsetY, glyph_samples, pcf);
//		PCFDrawPixel(clx + 0, cly + 0, offsetY, glyph_samples, pcf);
		PCFDrawPixel(clx + 1, cly + 0, offsetY, glyph_samples, pcf);

		PCFDrawPixel(clx - 1, cly + 1, offsetY, glyph_samples, pcf);
		PCFDrawPixel(clx + 0, cly + 1, offsetY, glyph_samples, pcf);
		PCFDrawPixel(clx + 1, cly + 1, offsetY, glyph_samples, pcf);

		pcf->color = pcf->colorT;
		PCFDrawPixel(clx, cly, offsetY, glyph_samples, pcf);
	}

	clx += ((metric.right_sided_bearing - metric.left_sided_bearing) * metrics_scale) + pcf_font.metrics.hSpacing;

	if(code == 0x20 || code == 0x3000){
		clx += y_offset;
	}
}


void PCFPutString(const uint16_t *uni_str, int n, pcf_typedef *pcf)
{
//	int offset = 0;
	while(n-- > 0){
		PCFPutChar(*uni_str++, pcf);
	}
}

uint16_t PCFGetCharPixelLength(uint16_t code, uint16_t font_width)
{
	uint32_t temp;
	uint16_t len;
	uint8_t buf[512], enc1, enc2;

	MY_FILE fpTmp;

	// Get the glyph_index from UTF16 code.
	uint16_t glyph_index;
	enc1 = (uint8_t)(code >> 8);
	enc2 = (uint8_t)code;

	if(enc1 > pcf_font.enc_info.max_byte1 || enc1 < pcf_font.enc_info.min_byte1){
		return 0;
	}

	if(enc2 > pcf_font.enc_info.max_char_or_byte2 || enc2 < pcf_font.enc_info.min_char_or_byte2){
		return 0;
	}

	// グリフの位置を計算
	temp = 	((enc1 - pcf_font.enc_info.min_byte1) * \
			(pcf_font.enc_info.max_char_or_byte2 - pcf_font.enc_info.min_char_or_byte2 + 1) + \
			enc2 - pcf_font.enc_info.min_char_or_byte2) * sizeof(uint16_t);

	memcpy((void*)&fpTmp, (void*)&pcf_font.enc_tbl.fp, sizeof(MY_FILE));
	my_fseek(&fpTmp, temp, SEEK_CUR);
	my_fread(buf, 1, sizeof(uint16_t), &fpTmp);
//	glyph_index = conv_b2l((void*)buf, sizeof(uint16_t));
	glyph_index = __REV16(*(uint16_t*)buf);

	if(glyph_index == 0xFFFF) {
		return 0;
	}

	// Get the glyph mtric data
	metric_data_typedef metric;
	memcpy((void*)&fpTmp, (void*)&pcf_font.metrics_tbl.fp, sizeof(MY_FILE)); //
	my_fseek(&fpTmp, glyph_index * sizeof(metric_data_typedef), SEEK_CUR);
	my_fread((void*)&metric, 1, sizeof(metric_data_typedef), &fpTmp);

	// xor 0x80 each metric(subtract 0x80)
	*((uint32_t*)&metric.left_sided_bearing) ^= 0x80808080;
//	metric.left_sided_bearing  ^= (1 << 7);
//	metric.right_sided_bearing ^= (1 << 7);
//	metric.character_width     ^= (1 << 7);
//	metric.character_ascent    ^= (1 << 7);
	metric.character_descent   ^= (1 << 7);

	len = (metric.right_sided_bearing - metric.left_sided_bearing) * (font_width < 16 ? 0.25f : 0.333f) + pcf_font.metrics.hSpacing;

	if(code == 0x20 || code == 0x3000){
		len += font_width < 16 ? 2 : 5;
	}

	return len;
}


int C_PCFFontInit(uint32_t fileAddr, size_t fileSize)
{
	uint8_t buf[512];
	C_FILE *fp = '\0';
	toc_entry toc;
	int i;
#ifdef DEBUG
	int type_idx;
#endif
	fp = c_fopen(fileAddr, fileSize);

	if(fp == NULL){
		debug.printf("\r\nc_fopen error");
		pcf_font.isOK = 0;
		return -1;
	}

	c_fread(buf, 1, 4, fp);

	if(strncmp((char*)buf, "\1fcp" , 4) != 0){
		debug.printf("\r\nC_PCFFontInit:Not PCF File");
		c_fclose((void*)fp);
		pcf_font.isOK = 0;
		return -1;
	}

	c_fread(buf, 1, sizeof(uint32_t), fp);
	memcpy((void*)&pcf_font.table_count, buf, sizeof(uint32_t));

#ifdef DEBUG
	debug.printf("\r\ntable_count:%d", pcf_font.table_count);
#endif

	for(i = 0;i < pcf_font.table_count;i++){
		c_fread(buf, 1, sizeof(toc_entry), fp);
		memcpy((void*)&toc, buf, sizeof(toc_entry));
#ifdef DEBUG
		debug.printf("\r\n\nEntry#%d", i);
		type_idx = 0;
		do{
			if((toc.type >> type_idx) & 1){
				debug.printf("\r\n%s", (char*)&type[type_idx][0]);
				break;
			}
		}while(++type_idx < 9);
		debug.printf("\r\ntype:%d", toc.type);
		debug.printf("\r\nformat:%d", toc.format);
		debug.printf("\r\nsize:%d", toc.size);
		debug.printf("\r\noffset:%d", toc.offset);
#endif

		switch(toc.type){
            case PCF_METRICS:
                pcf_font.metrics_tbl.size = toc.size;
                pcf_font.metrics_tbl.offset = toc.offset;
                break;
            case PCF_BITMAPS:
                pcf_font.bitmap_tbl.size = toc.size;
                pcf_font.bitmap_tbl.offset = toc.offset;
                break;
            case PCF_BDF_ENCODINGS:
                pcf_font.enc_tbl.size = toc.size;
                pcf_font.enc_tbl.offset = toc.offset;
                break;
            default:
                break;
		}

	}

#ifdef DEBUG
	debug.printf("\r\npcf_font.metrics_tbl.size:%d", pcf_font.metrics_tbl.size);
	debug.printf("\r\npcf_font.metrics.offset:%d", pcf_font.metrics_tbl.offset);

	debug.printf("\r\npcf_font.bitmap_tbl.size:%d", pcf_font.bitmap_tbl.size);
	debug.printf("\r\npcf_font.bitmap_tbl.offset:%d", pcf_font.bitmap_tbl.offset);

	debug.printf("\r\npcf_font.enc_tbl.size:%d", pcf_font.enc_tbl.size);
	debug.printf("\r\npcf_font.enc_tbl.offset:%d", pcf_font.enc_tbl.offset);
#endif

	// Collect Metrics Information
	c_fseek(fp, pcf_font.metrics_tbl.offset + 6, SEEK_SET); // jump to metrics table. skip format(4bytes), metrics_count(2bytes)
	memcpy((void*)&pcf_font.metrics_tbl.fp, (void*)fp, sizeof(C_FILE)); // copy file pointer to the metrics table.

	// Collect Encoding Information
	c_fseek(fp, pcf_font.enc_tbl.offset + 4, SEEK_SET); // jump to encoding table. skip format(4bytes)

	c_fread(buf, 1, sizeof(uint16_t), fp);
    //	pcf_font.enc_info.min_char_or_byte2 = conv_b2l((void*)buf, sizeof(uint16_t));
	pcf_font.enc_info.min_char_or_byte2 = __REV16(*(uint16_t*)buf);

	c_fread(buf, 1, sizeof(uint16_t), fp);
    //	pcf_font.enc_info.max_char_or_byte2 = conv_b2l((void*)buf, sizeof(uint16_t));
	pcf_font.enc_info.max_char_or_byte2 = __REV16(*(uint16_t*)buf);

	c_fread(buf, 1, sizeof(uint16_t), fp);
    //	pcf_font.enc_info.min_byte1 = conv_b2l((void*)buf, sizeof(uint16_t));
	pcf_font.enc_info.min_byte1 = __REV16(*(uint16_t*)buf);

	c_fread(buf, 1, sizeof(uint16_t), fp);
    //	pcf_font.enc_info.max_byte1 = conv_b2l((void*)buf, sizeof(uint16_t));
	pcf_font.enc_info.max_byte1 = __REV16(*(uint16_t*)buf);

	c_fread(buf, 1, sizeof(uint16_t), fp);
    //	pcf_font.enc_info.default_char = conv_b2l((void*)buf, sizeof(uint16_t));
	pcf_font.enc_info.default_char = __REV16(*(uint16_t*)buf);

#ifdef DEBUG
	debug.printf("\r\nmin_char_or_byte2:%d", pcf_font.enc_info.min_char_or_byte2);
	debug.printf("\r\nmax_char_or_byte2:%d", pcf_font.enc_info.max_char_or_byte2);
	debug.printf("\r\nmin_byte1:%d", pcf_font.enc_info.min_byte1);
	debug.printf("\r\nmax_byte1:%d", pcf_font.enc_info.max_byte1);
	debug.printf("\r\ndefault_char:%d", pcf_font.enc_info.default_char);
#endif
	pcf_font.enc_tbl.glyphindeces = (pcf_font.enc_info.max_char_or_byte2 - pcf_font.enc_info.min_char_or_byte2 + 1) * (pcf_font.enc_info.max_byte1 - pcf_font.enc_info.min_byte1 + 1);

#ifdef DEBUG
	debug.printf("\r\nglyphindeces:%d", pcf_font.enc_tbl.glyphindeces);
#endif

	memcpy((void*)&pcf_font.enc_tbl.fp, (void*)fp, sizeof(C_FILE)); // copy file pointer to the encode table.

	// Collect Bitmap information
    //	c_fseek(fp, pcf_font.bitmap_tbl.offset + 8, SEEK_SET); // jump to bitmap_table, skip format(4bytes), glyph_count(4bytes)
	c_fseek(fp, pcf_font.bitmap_tbl.offset, SEEK_SET); // jump to bitmap_table, skip format(4bytes), glyph_count(4bytes)
	c_fread(buf, 1, sizeof(uint32_t), fp);
    //	debug.printf("\r\nformat***%08x", conv_b2l((void*)buf, sizeof(uint32_t)));
#ifdef DEBUG
	debug.printf("\r\nformat***%08x", __REV(*(uint32_t*)buf));
#endif
	c_fread(buf, 1, sizeof(uint32_t), fp);


	memcpy((void*)&pcf_font.bitmap_tbl.fp_offset, (void*)fp, sizeof(C_FILE)); // copy file pointer to the bitmap offset.

	uint32_t glyph_count;
	c_fseek(fp, pcf_font.bitmap_tbl.offset + 4, SEEK_SET); // jump to bitmap_table, skip format(4bytes)
	c_fread(buf, 1, sizeof(uint32_t), fp);
    //	glyph_count = conv_b2l((void*)buf, sizeof(uint32_t));
	glyph_count = __REV(*(uint32_t*)buf);
	c_fseek(fp, glyph_count * sizeof(uint32_t) + sizeof(uint32_t) * 4, SEEK_CUR); // skip glyph_count * 4, bitmapSize(uint32_t * 4)
	memcpy((void*)&pcf_font.bitmap_tbl.fp_bitmap, (void*)fp, sizeof(C_FILE)); // copy file pointer to the bitmap data.
#ifdef DEBUG
	debug.printf("\r\nglyph_count:%d", glyph_count);
#endif
	c_fclose(fp);
	pcf_font.isOK = 1;
	pcf_font.metrics.hSpacing = PCF_METRICS_DEFAULT_HSPACING;

	return 0;
}



void C_PCFPutChar(uint16_t code, void *cp)
{
	uint64_t bitmap_data_64;
	uint32_t tmp, *p_u32;
	int i, j, offsetY, double_size = 0;
	uint16_t glyph_index;

	uint8_t buf[512], glyph_samples[48][16];
	uint8_t enc1, enc2;

	C_FILE fpTmp;
	pcf_typedef *pcf = (pcf_typedef*)cp;

	// Get the glyph_index from UTF16 code.
	enc1 = (code >> 8) & 0xff;
	enc2 = code & 0xff;

	float metrics_scale = 0.25f;
	uint8_t sample_mask = 0xf, sample_shift = 4, y_offset = 2;


	if(enc1 > pcf_font.enc_info.max_byte1 || enc1 < pcf_font.enc_info.min_byte1){
		return;
	}

	if(enc2 > pcf_font.enc_info.max_char_or_byte2 || enc2 < pcf_font.enc_info.min_char_or_byte2){
		return;
	}

	switch(pcf->size){
	case 16:
		metrics_scale = 0.333f;
		sample_mask = 0x7;
		sample_shift = 3;
		y_offset = 5;
		break;
	case 12:
	default:
		pcf->size = 12;
		metrics_scale = 0.25f;
		sample_mask = 0xf;
		sample_shift = 4;
		y_offset = 2;
		break;
	}

	// グリフの位置を計算
	tmp = 	((enc1 - pcf_font.enc_info.min_byte1) * \
             (pcf_font.enc_info.max_char_or_byte2 - pcf_font.enc_info.min_char_or_byte2 + 1) + \
             enc2 - pcf_font.enc_info.min_char_or_byte2) * sizeof(uint16_t);

	memcpy((void*)&fpTmp, (void*)&pcf_font.enc_tbl.fp, sizeof(C_FILE));
	c_fseek(&fpTmp, tmp, SEEK_CUR);
	c_fread(buf, 1, sizeof(uint16_t), &fpTmp);
	//glyph_index = conv_b2l((void*)buf, sizeof(uint16_t));
	glyph_index = __REV16(*(uint16_t*)buf);

    //	debug.printf("\r\n\nGlyph Index");
    //	debug.printf("\r\nglyph index:0x%02x", glyph_index);
	if(glyph_index == 0xFFFF) {
		return;
        //		code = 0x25A1; // 0x25A1;
	}

	// Get the glyph mtric data
	metric_data_typedef metric;
	memcpy((void*)&fpTmp, (void*)&pcf_font.metrics_tbl.fp, sizeof(C_FILE)); //
	c_fseek(&fpTmp, glyph_index * sizeof(metric_data_typedef), SEEK_CUR);
	c_fread((void*)&metric, 1, sizeof(metric_data_typedef), &fpTmp);

	*(uint32_t*)&metric.left_sided_bearing ^= 0x80808080;
    //	metric.left_sided_bearing  ^= (1 << 7);
    //	metric.right_sided_bearing ^= (1 << 7);
    //	metric.character_width     ^= (1 << 7);
    //	metric.character_ascent    ^= (1 << 7);
	metric.character_descent   ^= (1 << 7);
    /*
     debug.printf("\r\n\nMetric data code:%04x", code);
     debug.printf("\r\nmetric.left_sided_bearing:%d", metric.left_sided_bearing);
     debug.printf("\r\nmetric.right_sided_bearing:%d", metric.right_sided_bearing);
     debug.printf("\r\nmetric.character_width:%d", metric.character_width);
     debug.printf("\r\nmetric.character_ascent:%d", metric.character_ascent);
     debug.printf("\r\nmetric.character_descent:%d", metric.character_descent);
     */
	// Get the bitmap data offset
	uint32_t bitmap_offset;
	memcpy((void*)&fpTmp, (void*)&pcf_font.bitmap_tbl.fp_offset, sizeof(C_FILE));
	c_fseek(&fpTmp, glyph_index * sizeof(int32_t), SEEK_CUR); // グリフインデックスが指すビットマップオフセットまでファイルポインタを移動
	c_fread(buf, 1, sizeof(int32_t), &fpTmp);
    //	bitmap_offset = conv_b2l((void*)buf, sizeof(int32_t)); // ビットマップオフセット
	bitmap_offset = __REV(*(uint32_t*)buf);

    //	debug.printf("\r\n\nBitmap data offset");
    //	debug.printf("\r\nglyph offset:0x%04x", bitmap_offset);

	if(metric.right_sided_bearing - metric.left_sided_bearing > 32){ // 右ベアリング - 左ベアリングが32pxを超えていたらダブルサイズ有効
		double_size = 1;
	}

	memcpy((void*)&fpTmp, (void*)&pcf_font.bitmap_tbl.fp_bitmap, sizeof(C_FILE)); // ビットマップデータまでファイルポインタを移動
	c_fseek(&fpTmp, bitmap_offset, SEEK_CUR);

	// バッファにビットマップデータをキャッシュする
	c_fread((void*)buf, 1, ((metric.character_ascent + metric.character_descent) * \
                             sizeof(uint32_t)) * (double_size == 0 ? 1 : 2), &fpTmp); 	// ビットマップデータが32bitを超える場合は2倍

	memset((void*)glyph_samples, 0, sizeof(glyph_samples));

	p_u32 = (uint32_t*)buf;
	if(!double_size){
		for(i = 0;i < metric.character_ascent + metric.character_descent;i++){
			bitmap_data_64 = __REV(*p_u32++);
			bitmap_data_64 <<= 16; // 48bit - 32bit = 16bit
			for(j = 1;j <= pcf->size;j++){
				glyph_samples[i][pcf->size - j] = bit_count_table[bitmap_data_64 & sample_mask]; // 4bitずつサンプルする
				bitmap_data_64 >>= sample_shift;
			}
		}
	} else {
		for(i = 0;i < metric.character_ascent + metric.character_descent;i++){
			bitmap_data_64 = __REV(*p_u32++);
			bitmap_data_64 <<= 32;
			bitmap_data_64 |= __REV(*p_u32++);
			bitmap_data_64 >>= 16; // 64bit - 48bit = 16bit  余白を右に詰める
			for(j = 1;j <= pcf->size;j++){
				glyph_samples[i][pcf->size - j] = bit_count_table[bitmap_data_64 & sample_mask]; // 4bitずつサンプルする
				bitmap_data_64 >>= sample_shift;
			}
		}
	}

	offsetY = pcf->size - 1 - (int)(((float)metric.character_ascent * metrics_scale) + 0.5f); // グリフの高さ揃えパラメータ

	if(!pcf->enableShadow){
		PCFDrawPixel(clx, cly, offsetY, glyph_samples, pcf);
	} else {
		pcf->colorT = pcf->color;
		pcf->color = pcf->colorShadow;

		PCFDrawPixel(clx - 1, cly - 1, offsetY, glyph_samples, pcf);
		PCFDrawPixel(clx + 0, cly - 1, offsetY, glyph_samples, pcf);
		PCFDrawPixel(clx + 1, cly - 1, offsetY, glyph_samples, pcf);

		PCFDrawPixel(clx - 1, cly + 0, offsetY, glyph_samples, pcf);
//		PCFDrawPixel(clx + 0, cly + 0, offsetY, glyph_samples, pcf);
		PCFDrawPixel(clx + 1, cly + 0, offsetY, glyph_samples, pcf);

		PCFDrawPixel(clx - 1, cly + 1, offsetY, glyph_samples, pcf);
		PCFDrawPixel(clx + 0, cly + 1, offsetY, glyph_samples, pcf);
		PCFDrawPixel(clx + 1, cly + 1, offsetY, glyph_samples, pcf);

		pcf->color = pcf->colorT;
		PCFDrawPixel(clx, cly, offsetY, glyph_samples, pcf);
	}

	clx += ((metric.right_sided_bearing - metric.left_sided_bearing) * metrics_scale) + pcf_font.metrics.hSpacing;

	if(code == 0x20 || code == 0x3000){
		clx += y_offset;
	}
}





void C_PCFPutString(const uint16_t *uni_str, int n, pcf_typedef *pcf)
{
    //	int offset = 0;
	while(n-- > 0){
		C_PCFPutChar(*uni_str++, pcf);
	}
}


uint16_t C_PCFGetCharPixelLength(uint16_t code, uint16_t font_width)
{
	uint32_t temp;
	uint16_t len;
	uint8_t buf[512], enc1, enc2;

	C_FILE fpTmp;

	// Get the glyph_index from UTF16 code.
	uint16_t glyph_index;
	enc1 = (uint8_t)(code >> 8);
	enc2 = (uint8_t)code;

	if(enc1 > pcf_font.enc_info.max_byte1 || enc1 < pcf_font.enc_info.min_byte1){
		return 0;
	}

	if(enc2 > pcf_font.enc_info.max_char_or_byte2 || enc2 < pcf_font.enc_info.min_char_or_byte2){
		return 0;
	}

	// グリフの位置を計算
	temp = 	((enc1 - pcf_font.enc_info.min_byte1) * \
			(pcf_font.enc_info.max_char_or_byte2 - pcf_font.enc_info.min_char_or_byte2 + 1) + \
			enc2 - pcf_font.enc_info.min_char_or_byte2) * sizeof(uint16_t);

	memcpy((void*)&fpTmp, (void*)&pcf_font.enc_tbl.fp, sizeof(C_FILE));
	c_fseek(&fpTmp, temp, SEEK_CUR);
	c_fread(buf, 1, sizeof(uint16_t), &fpTmp);
//	glyph_index = conv_b2l((void*)buf, sizeof(uint16_t));
	glyph_index = __REV16(*(uint16_t*)buf);

	if(glyph_index == 0xFFFF) {
		return 0;
	}

	// Get the glyph mtric data
	metric_data_typedef metric;
	memcpy((void*)&fpTmp, (void*)&pcf_font.metrics_tbl.fp, sizeof(C_FILE)); //
	c_fseek(&fpTmp, glyph_index * sizeof(metric_data_typedef), SEEK_CUR);
	c_fread((void*)&metric, 1, sizeof(metric_data_typedef), &fpTmp);

	// xor 0x80 each metric(subtract 0x80)
	*((uint32_t*)&metric.left_sided_bearing) ^= 0x80808080;
//	metric.left_sided_bearing  ^= (1 << 7);
//	metric.right_sided_bearing ^= (1 << 7);
//	metric.character_width     ^= (1 << 7);
//	metric.character_ascent    ^= (1 << 7);
	metric.character_descent   ^= (1 << 7);

	len = (metric.right_sided_bearing - metric.left_sided_bearing) * (font_width < 16 ? 0.25f : 0.333f) + pcf_font.metrics.hSpacing;

	if(code == 0x20 || code == 0x3000){
		len += font_width < 16 ? 2 : 5;
	}

	return len;
}

