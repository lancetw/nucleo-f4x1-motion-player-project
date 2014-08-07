/*
 * icon.h
 *
 *  Created on: 2014/05/22
 *      Author: masayuki
 */

#ifndef ICON_H_
#define ICON_H_

//#include "stm32f4xx_hal_conf.h"
#include <stdint.h>
#include <stddef.h>

typedef struct{
	int x, y, width, height;
	uint16_t p[12 * 12];
}posision_icon_buf_typedef;

typedef struct{
	int x, y, width, height;
	uint16_t p[12 * 12];
}play_pause_icon_buf_typedef;

typedef struct{
	int x, y, width, height;
	uint16_t p[32 * 32];
}navigation_buf_typedef;


typedef struct{
	int x, y, width, height;
	uint16_t p[13 * 13 * 10];
}timeElapsed_buf_typedef;

typedef struct{
	int x, y, width, height;
	uint16_t p[13 * 13 * 10];
}timeRemain_buf_typedef;

typedef struct{
	uint8_t x, y, width, height, squareSize, spacing;
	uint16_t bar[8][8][3 * 3];
}fft_analyzer_typedef;

typedef struct{
	int x, y, width, height;
	uint16_t p[18 * 14];
}navigation_loop_typedef;

typedef struct{
	int x, y, width, height;
	uint16_t p[24 * 18];
}bass_boost_typedef;

typedef struct{
	int x, y, width, height;
	uint16_t p[24 * 18];
}reverb_effect_typedef;

typedef struct{
	int x, y, width, height;
	uint16_t p[24 * 18];
}vocal_cancel_typedef;


typedef struct{
	posision_icon_buf_typedef posision;
	play_pause_icon_buf_typedef play_pause;
	fft_analyzer_typedef fft_analyzer_left;
	navigation_loop_typedef navigation_loop;
}drawBuff_typedef;


extern void Update_Navigation_Loop_Icon(drawBuff_typedef *db, uint8_t index);


extern const uint16_t compass_128x128[]; extern const size_t _sizeof_compass_128x128[];

extern const uint16_t compass_cross_58x58[]; extern const size_t _sizeof_compass_cross_58x58[];
extern const uint8_t compass_cross_58x58_alpha[]; extern const size_t _sizeof_compass_cross_58x58_alpha[];


extern const uint16_t filer_bgimg_160x128[]; extern const size_t _sizeof_filer_bgimg_160x128[];

extern const uint16_t music_bgimg_160x128[]; extern const size_t _sizeof_music_bgimg_160x128[];


extern const uint16_t radiobutton_checked_22x22[],_sizeof_radiobutton_checked_22x22[];
extern const uint16_t radiobutton_unchecked_22x22[],_sizeof_radiobutton_unchecked_22x22[];
extern const uint8_t radiobutton_22x22_alpha[],_sizeof_radiobutton_22x22_alpha[];

extern const uint16_t info_22x22[],_sizeof_info_22x22[];
extern const uint8_t info_22x22_alpha[],_sizeof_info_22x22_alpha[];


extern const uint16_t select_22x22[],_sizeof_select_22x22[];
extern const uint8_t select_22x22_alpha[],_sizeof_select_22x22_alpha[];

extern const uint16_t onpu_22x22[]; extern const size_t _sizeof_onpu_22x22[];
extern const uint8_t onpu_22x22_alpha[]; extern const size_t _sizeof_onpu_22x22_alpha[];

extern const uint16_t folder_22x22[]; extern const size_t _sizeof_folder_22x22[];
extern const uint8_t folder_22x22_alpha[]; extern const size_t _sizeof_folder_22x22_alpha[];

extern const uint16_t jpeg_22x22[]; extern const size_t _sizeof_jpeg_22x22[];
extern const uint8_t jpeg_22x22_alpha[]; extern const size_t _sizeof_jpeg_22x22_alpha[];

extern const uint16_t compass_22x22[]; extern const size_t _sizeof_compass_22x22[];
extern const uint8_t compass_22x22_alpha[]; extern const size_t _sizeof_compass_22x22_alpha[];

extern const uint16_t settings_22x22[]; extern const size_t _sizeof_settings_22x22[];
extern const uint8_t settings_22x22_alpha[]; extern const size_t _sizeof_settings_22x22_alpha[];

static const uint16_t movie_22x22[]; extern const size_t _sizeof_movie_22x22[];
static const uint8_t movie_22x22_alpha[]; extern const size_t _sizeof_movie_22x22_alpha[];

static const uint16_t font_22x22[]; extern const size_t _sizeof_font_22x22[];
static const uint8_t font_22x22_alpha[]; extern const size_t _sizeof_font_22x22_alpha[];

static const uint16_t archive_22x22[]; extern const size_t _sizeof_archive_22x22[];
static const uint8_t archive_22x22_alpha[]; extern const size_t _sizeof_archive_22x22_alpha[];

extern const uint16_t parent_arrow_22x22[]; extern const size_t _sizeof_parent_arrow_22x22[];
extern const uint8_t parent_arrow_22x22_alpha[]; extern const size_t _sizeof_parent_arrow_22x22_alpha[];

extern const uint16_t display_22x22[]; extern const size_t _sizeof_display_22x22[];
extern const uint8_t display_22x22_alpha[]; extern const size_t _sizeof_display_22x22_alpha[];

static const uint16_t scrollbar_top_6x7[]; extern const size_t _sizeof_scrollbar_top_6x7[];
static const uint8_t scrollbar_top_6x7_alpha[]; extern const size_t _sizeof_scrollbar_top_6x7_alpha[];

static const uint16_t scrollbar_6x98[]; extern const size_t _sizeof_scrollbar_6x98[];
static const uint8_t scrollbar_6x98_alpha[]; extern const size_t _sizeof_scrollbar_6x98_alpha[];

static const uint16_t scrollbar_bottom_6x7[]; extern const size_t _sizeof_scrollbar_bottom_6x7[];
static const uint8_t scrollbar_bottom_6x7_alpha[]; extern const size_t _sizeof_scrollbar_bottom_6x7_alpha[];

static const uint16_t scrollbar_hline_6x1[]; extern const size_t _sizeof_scrollbar_hline_6x1[];
static const uint8_t scrollbar_hline_6x1_alpha[]; extern const size_t _sizeof_scrollbar_hline_6x1_alpha[];

extern const uint16_t menubar_160x22[]; extern const size_t _sizeof_menubar_160x22[];
extern const uint8_t menubar_160x22_alpha[]; extern const size_t _sizeof_menubar_160x22_alpha[];

extern const uint16_t seekbar_130x14[]; extern const size_t _sizeof_seekbar_130x14[];
extern const uint8_t seekbar_130x14_alpha[]; extern const size_t _sizeof_seekbar_130x14_alpha[];

extern const uint16_t seekbar_120x14[]; extern const size_t _sizeof_seekbar_120x14[];
extern const uint8_t seekbar_120x14_alpha[]; extern const size_t _sizeof_seekbar_120x14_alpha[];

extern const uint16_t play_icon_12x12[]; extern const size_t _sizeof_play_icon_12x12[];
extern const uint8_t play_icon_12x12_alpha[]; extern const size_t _sizeof_play_icon_12x12_alpha[];

extern const uint16_t pause_icon_12x12[]; extern const size_t _sizeof_pause_icon_12x12[];
extern const uint8_t pause_icon_12x12_alpha[]; extern const size_t _sizeof_pause_icon_12x12_alpha[];

extern const uint16_t mute_icon_12x12[]; extern const size_t _sizeof_mute_icon_12x12[];
extern const uint8_t mute_icon_12x12_alpha[]; extern const size_t _sizeof_mute_icon_12x12_alpha[];

extern const uint16_t seek_circle_12x12[]; extern const size_t _sizeof_seek_circle_12x12[];
extern const uint16_t seek_active_circle_12x12[]; extern const size_t _sizeof_seek_active_circle_12x12[];
extern const uint8_t seek_circle_12x12_alpha[]; extern const size_t _sizeof_seek_circle_12x12_alpha[];

extern const uint16_t navigation_loop_18x14[]; extern const size_t _sizeof_navigation_loop_18x14[];
extern const uint8_t navigation_loop_18x14_alpha[]; extern const size_t _sizeof_navigation_loop_18x14_alpha[];

extern const uint16_t navigation_infinite_entire_loop_18x14[]; extern const size_t _sizeof_navigation_infinite_entire_loop_18x14[];
extern const uint8_t navigation_infinite_entire_loop_18x14_alpha[]; extern const size_t _sizeof_navigation_infinite_entire_loop_18x14_alpha[];


extern const uint16_t navigation_bar_18x14[]; extern const size_t _sizeof_navigation_bar_18x14[];
extern const uint8_t navigation_bar_18x14_alpha[]; extern const size_t _sizeof_navigation_bar_18x14_alpha[];

extern const uint16_t navigation_infinite_one_loop_18x14[]; extern const size_t _sizeof_navigation_infinite_one_loop_18x14[];
extern const uint8_t navigation_infinite_one_loop_18x14_alpha[]; extern const size_t _sizeof_navigation_infinite_one_loop_18x14_alpha[];

extern const uint16_t navigation_shuffle_18x14[]; extern const size_t _sizeof_navigation_shuffle_18x14[];
extern const uint8_t navigation_shuffle_18x14_alpha[]; extern const size_t _sizeof_navigation_shuffle_18x14_alpha[];


extern const uint16_t progress_circular_bar_16x16x12[];//,_sizeof_progress_circular_bar_16x16x12[];



#endif /* ICON_H_ */
