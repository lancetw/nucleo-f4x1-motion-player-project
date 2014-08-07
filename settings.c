/*
 * settings.c
 *
 *  Created on: 2013/03/18
 *      Author: Tonsuke
 */


#include "settings.h"
#include <stdio.h>


#include "usart.h"
#include "fat.h"
#include "lcd.h"
#include "pcf_font.h"
#include "sd.h"
#include "main.h"

char settings_mode;
unsigned char settings_root_list_fileCnt;

settings_group_typedef settings_group;


settings_list_typedef *settings_p, *settings_parent;
settings_stack_typedef settings_stack;

settings_item_typedef settings_item_card;
settings_item_typedef settings_item_cpu;
settings_item_typedef settings_item_usart;
settings_item_typedef settings_item_font;
settings_item_typedef settings_item_sort;
settings_item_typedef settings_item_photo_frame_td;
settings_item_typedef settings_item_brightness;
settings_item_typedef settings_item_sleeptime;
settings_item_typedef settings_item_fft;
settings_item_typedef settings_item_fft_bar_type;
settings_item_typedef settings_item_fft_color_type;
settings_item_typedef settings_item_musicinfo;
settings_item_typedef settings_item_prehalve;

const icon_ptr_typedef music_icon[] = {{onpu_22x22, onpu_22x22_alpha}};
const icon_ptr_typedef photo_icon[] = {{jpeg_22x22, jpeg_22x22_alpha}};
const icon_ptr_typedef display_icon[] = {{display_22x22, display_22x22_alpha}};
const icon_ptr_typedef info_icon[] = {{info_22x22, info_22x22_alpha}};

int my_sprintf(char *a, const char *b, ...)
{
	return 0;
}


const settings_list_typedef settings_root_list[] = {
		{"..", NULL, 0, NULL, },
		{"Info", info_icon, 1, NEXT_LIST(settings_about_motionplayer_list), SETTING_ABOUT_MOTIONPLAYER},
		{"Display", display_icon, 3, NEXT_LIST(settings_display_list)},
		{"Photo Frame", photo_icon, 2, NEXT_LIST(settings_photo_list)},
		{"Music", music_icon, 3, NEXT_LIST(settings_music_list)},
};

const settings_list_typedef settings_about_motionplayer_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_root_list)},
};

const settings_list_typedef settings_music_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_root_list)},
		{"FFT Settings", NULL, 3, NEXT_LIST(settings_fft_list)},
		{"Music Info", NULL, 3, NEXT_LIST(settings_musicinfo_list), NULL, SETTING_TYPE_ITEM, &settings_item_musicinfo},
};

const settings_list_typedef settings_fft_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_music_list)},
		{"Analyzer", NULL, 3, NEXT_LIST(settings_fft_display_list), NULL, SETTING_TYPE_ITEM, &settings_item_fft},
		{"Bar Color", NULL, 7, NEXT_LIST(settings_fft_bar_color_list), NULL, SETTING_TYPE_ITEM, &settings_item_fft_color_type},
};

const settings_list_typedef settings_fft_display_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_fft_list)},
		{"ON", NULL, 0, NULL, NULL, 0, &settings_item_fft},
		{"OFF", NULL, 0, NULL, NULL, 0, &settings_item_fft},
};

const settings_list_typedef settings_fft_bar_type_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_fft_list)},
		{"Solid", NULL, 0, NULL, NULL, 0, &settings_item_fft_bar_type},
		{"V Split", NULL, 0, NULL, NULL, 0, &settings_item_fft_bar_type},
		{"H Split", NULL, 0, NULL, NULL, 0, &settings_item_fft_bar_type},
		{"Wide", NULL, 0, NULL, NULL, 0, &settings_item_fft_bar_type},
};

const settings_list_typedef settings_fft_bar_color_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_fft_list)},
		{"White", NULL, 0, NULL, NULL, 0, &settings_item_fft_color_type},
		{"Skyblue", NULL, 0, NULL, NULL, 0, &settings_item_fft_color_type},
		{"Yellow", NULL, 0, NULL, NULL, 0, &settings_item_fft_color_type},
		{"Green", NULL, 0, NULL, NULL, 0, &settings_item_fft_color_type},
		{"Red", NULL, 0, NULL, NULL, 0, &settings_item_fft_color_type},
		{"Black", NULL, 0, NULL, NULL, 0, &settings_item_fft_color_type},
//		{"Colorful", NULL, 0, NULL, NULL, 0, &settings_item_fft_color_type},
};

const settings_list_typedef settings_musicinfo_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_music_list)},
		{"ON", NULL, 0, NULL, NULL, 0, &settings_item_musicinfo},
		{"OFF", NULL, 0, NULL, NULL, 0, &settings_item_musicinfo},
};

const settings_list_typedef settings_prehalve_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_music_list)},
		{"Enable", NULL, 0, NULL, NULL, 0, &settings_item_prehalve},
		{"Disable", NULL, 0, NULL, NULL, 0, &settings_item_prehalve},
};

const settings_list_typedef settings_display_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_root_list)},
		{"Brightness", NULL, 5, NEXT_LIST(settings_brightness_list), NULL, SETTING_TYPE_ITEM, &settings_item_brightness},
		{"Time To Sleep", NULL, 5, NEXT_LIST(settings_sleeptime_list), NULL, SETTING_TYPE_ITEM, &settings_item_sleeptime},
};

const settings_list_typedef settings_brightness_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_display_list)},
		{"25%", NULL, 0, NULL, NULL, 0, &settings_item_brightness},
		{"50%", NULL, 0, NULL, NULL, 0, &settings_item_brightness},
		{"75%", NULL, 0, NULL, NULL, 0, &settings_item_brightness},
		{"100%", NULL, 0, NULL, NULL, 0, &settings_item_brightness},
};

const settings_list_typedef settings_sleeptime_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_display_list)},
		{"Always ON", NULL, 0, NULL, NULL, 0, &settings_item_sleeptime},
		{"15s", NULL, 0, NULL, NULL, 0, &settings_item_sleeptime},
		{"30s", NULL, 0, NULL, NULL, 0, &settings_item_sleeptime},
		{"60s", NULL, 0, NULL, NULL, 0, &settings_item_sleeptime},
};

const settings_list_typedef settings_photo_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_root_list)},
		{"Play Interval", NULL, 5, NEXT_LIST(settings_photo_frame_td_list), NULL, SETTING_TYPE_ITEM, &settings_item_photo_frame_td},
};


const settings_list_typedef settings_photo_frame_td_list[] = {
		{"..", NULL, 0, NEXT_LIST(settings_photo_list)},
		{"1s", NULL, 0, NULL, NULL, 0, &settings_item_photo_frame_td},
		{"3s", NULL, 0, NULL, NULL, 0, &settings_item_photo_frame_td},
		{"5s", NULL, 0, NULL, NULL, 0, &settings_item_photo_frame_td},
		{"10s", NULL, 0, NULL, NULL, 0, &settings_item_photo_frame_td},
};

int validate_saved_val(int val, int default_val, const unsigned int tbl[], size_t tbl_size)
{
	int i, coincide = 0;

	for(i = 0;i < tbl_size;i++){
		if(val == tbl[i]){
			coincide = 1;
			break;
		}
	}

	if(!coincide){
		return default_val;
	}

	return val;
}

int selected_id(int val, const unsigned int tbl[], size_t tbl_size)
{
	int i, ret, coincide  = 0;

	for(i = 0;i < tbl_size;i++){
		if(val == tbl[i]){
			coincide = 1;
			ret = i;
			break;
		}
	}

	if(!coincide){
		return 0;
	}

	return ret;
}

void SETTINGS_Init()
{
	settings_mode = 0;
	settings_root_list_fileCnt = sizeof(settings_root_list) / sizeof(settings_root_list[0]);

	/* Load settings */
	memcpy((void*)&settings_group, (void*)settings_save_128, sizeof(settings_group));

	/* FFT Item */
	static const unsigned int fft_tbl[] = {1, 0};
#define FFT_TABLE_ITEMS (sizeof(fft_tbl) / sizeof(fft_tbl[0]))
	settings_group.music_conf.b.fft = validate_saved_val(settings_group.music_conf.b.fft, 0, fft_tbl, FFT_TABLE_ITEMS);
	settings_item_fft.selected_id = selected_id(settings_group.music_conf.b.fft, fft_tbl, FFT_TABLE_ITEMS);
	settings_item_fft.item_count = FFT_TABLE_ITEMS;
	settings_item_fft.item_array = fft_tbl;
	settings_item_fft.func = SETTINGS_FFT;

	/* FFT Bar Type Item */
	static const unsigned int fft_bar_type_tbl[] = {0, 1, 2, 3}; // 0:Solid 1:V Split 2:H Split 3:Wide
#define FFT_BAR_TYPE_TABLE_ITEMS (sizeof(fft_bar_type_tbl) / sizeof(fft_bar_type_tbl[0]))
	settings_group.music_conf.b.fft_bar_type = validate_saved_val(settings_group.music_conf.b.fft_bar_type, 0, fft_bar_type_tbl, FFT_BAR_TYPE_TABLE_ITEMS);
	settings_item_fft_bar_type.selected_id = selected_id(settings_group.music_conf.b.fft_bar_type, fft_bar_type_tbl, FFT_BAR_TYPE_TABLE_ITEMS);
	settings_item_fft_bar_type.item_count = FFT_BAR_TYPE_TABLE_ITEMS;
	settings_item_fft_bar_type.item_array = fft_bar_type_tbl;
	settings_item_fft_bar_type.func = SETTINGS_FFT_BAR_TYPE;

	/* FFT Bar Color Item */
	static const unsigned int fft_bar_color_tbl[] = {0, 1, 2, 3, 4, 5}; // 0:White 1:Skyblue 2:Yellow 3:Green 4:Red 5:Black
#define FFT_BAR_COLOR_TABLE_ITEMS (sizeof(fft_bar_color_tbl) / sizeof(fft_bar_color_tbl[0]))
	settings_group.music_conf.b.fft_bar_color_idx = validate_saved_val(settings_group.music_conf.b.fft_bar_color_idx, 0, fft_bar_color_tbl, FFT_BAR_COLOR_TABLE_ITEMS);
	settings_item_fft_color_type.selected_id = selected_id(settings_group.music_conf.b.fft_bar_color_idx, fft_bar_color_tbl, FFT_BAR_COLOR_TABLE_ITEMS);
	settings_item_fft_color_type.item_count = FFT_BAR_COLOR_TABLE_ITEMS;
	settings_item_fft_color_type.item_array = fft_bar_color_tbl;
	settings_item_fft_color_type.func = SETTINGS_FFT_BAR_COLOR;

	/* Music Info Item */
	static const unsigned int musicinfo_tbl[] = {1, 0};
#define MUSICINFO_TABLE_ITEMS (sizeof(musicinfo_tbl) / sizeof(musicinfo_tbl[0]))
	settings_group.music_conf.b.musicinfo = validate_saved_val(settings_group.music_conf.b.musicinfo, 0, musicinfo_tbl, MUSICINFO_TABLE_ITEMS);
	settings_item_musicinfo.selected_id = selected_id(settings_group.music_conf.b.musicinfo, musicinfo_tbl, MUSICINFO_TABLE_ITEMS);
	settings_item_musicinfo.item_count = MUSICINFO_TABLE_ITEMS;
	settings_item_musicinfo.item_array = musicinfo_tbl;
	settings_item_musicinfo.func = SETTINGS_MUSICINFO;

	/* Display Brightness Item */
	static const unsigned int brightness_tbl[] = {25, 50, 75, 100};
#define BRIGHTNESS_TABLE_ITEMS (sizeof(brightness_tbl) / sizeof(brightness_tbl[0]))
	settings_group.disp_conf.brightness = validate_saved_val(settings_group.disp_conf.brightness, 100, brightness_tbl, BRIGHTNESS_TABLE_ITEMS);
	settings_item_brightness.selected_id = selected_id(settings_group.disp_conf.brightness, brightness_tbl, BRIGHTNESS_TABLE_ITEMS);
	settings_item_brightness.item_count = BRIGHTNESS_TABLE_ITEMS;
	settings_item_brightness.item_array = brightness_tbl;
	settings_item_brightness.func = SETTINGS_DISPLAY_BRIGHTNESS;

	/* Display Sleep Time Item */
	static const unsigned int sleeptime_tbl[] = {0, 15, 30, 60};
#define SLEEPTIME_TABLE_ITEMS (sizeof(sleeptime_tbl) / sizeof(sleeptime_tbl[0]))
	settings_group.disp_conf.time2sleep = validate_saved_val(settings_group.disp_conf.time2sleep, 15, sleeptime_tbl, SLEEPTIME_TABLE_ITEMS);
	settings_item_sleeptime.selected_id = selected_id(settings_group.disp_conf.time2sleep, sleeptime_tbl, SLEEPTIME_TABLE_ITEMS);
	settings_item_sleeptime.item_count = SLEEPTIME_TABLE_ITEMS;
	settings_item_sleeptime.item_array = sleeptime_tbl;
	settings_item_sleeptime.func = SETTINGS_DISPLAY_SLEEP;

	/* Photo Frame Time Duration Item */
	static const unsigned int photo_frame_td_tbl[] = {1, 3, 5, 10};
#define PHOTO_FRAME_TD_TABLE_ITEMS (sizeof(photo_frame_td_tbl) / sizeof(photo_frame_td_tbl[0]))
	settings_group.filer_conf.photo_frame_td = validate_saved_val(settings_group.filer_conf.photo_frame_td, 3, photo_frame_td_tbl, PHOTO_FRAME_TD_TABLE_ITEMS);
	settings_item_photo_frame_td.selected_id = selected_id(settings_group.filer_conf.photo_frame_td, photo_frame_td_tbl, PHOTO_FRAME_TD_TABLE_ITEMS);
	settings_item_photo_frame_td.item_count = PHOTO_FRAME_TD_TABLE_ITEMS;
	settings_item_photo_frame_td.item_array = photo_frame_td_tbl;
	settings_item_photo_frame_td.func = SETTINGS_PHOTO_FRAME_TD;

}

void SETTINGS_Save()
{
	uint8_t flashBuff[16384];
	uint16_t *p16 = (uint16_t*)flashBuff;
	FLASH_EraseInitTypeDef FlashErase;
	uint32_t SectorError;
	uint32_t flash_addr;

	int i;

	__disable_irq();

	memcpy((void*)flashBuff, (void*)FLASH_BASE, 16384);
	memcpy((void*)&flashBuff[512], (void*)&settings_group, sizeof(settings_group));

	Delay_us(10000);

	HAL_FLASH_Unlock();
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
	FlashErase.TypeErase = TYPEERASE_SECTORS;
	FlashErase.Sector = FLASH_SECTOR_0;
	FlashErase.NbSectors = 1;
	FlashErase.VoltageRange = VOLTAGE_RANGE_3;
	if(HAL_FLASHEx_Erase(&FlashErase, &SectorError) != HAL_OK){
		debug.printf("\r\nErase Failed:%08x", SectorError);
	}

	flash_addr = FLASH_BASE;
	for(i = 0;i < (16384 / sizeof(uint16_t));i++){
		HAL_FLASH_Program(TYPEPROGRAM_HALFWORD, flash_addr, p16[i]);
		flash_addr += sizeof(uint16_t);
	}

	HAL_FLASH_Lock();

	__enable_irq();

	debug.printf("\r\nsettings saved");
}


void *SETTING_ABOUT_MOTIONPLAYER(void *arg)
{
	char s[30];

	pcf_typedef pcf;
	pcf.dst_gram_addr = (uint32_t)frame_buffer;
	pcf.pixelFormat = PCF_PIXEL_FORMAT_RGB565;
	pcf.size = 12;
	pcf.color = WHITE;
	pcf.colorShadow = BLACK;
	pcf.alphaSoftBlending = 1;
	pcf.enableShadow = 1;
	pcf_font.metrics.hSpacing = 2;


	LCD_GotoXY(2, 45);
	SPRINTF(s, "FW Ver: %d.%d (%s)\n", VERSION_MAJOR, VERSION_MINOR, __DATE__);
	LCDPutString(s, &pcf);

	LCD_GotoXY(2, cly);
	uint32_t mcu_revision = *(uint32_t*)0xE0042000 & 0xffff0000;
	switch(mcu_revision){
	case 0x10000000:
		LCDPutString("MCU Rev: A\n", &pcf);
		break;
	case 0x20000000:
		LCDPutString("MCU Rev: B\n", &pcf);
		break;
	case 0x10010000:
		LCDPutString("MCU Rev: Z\n", &pcf);
		break;
	default:
		LCDPutString("MCU Rev: Unkown\n", &pcf);
		break;
	}

	LCD_GotoXY(2, cly);
	LCDPutString("Site: nucleof401player.\n", &pcf);

	LCD_GotoXY(2, cly);
	LCDPutString("wiki.fc2.com\n", &pcf);

	LCD_GotoXY(2, cly);
	LCDPutString("Author: Tonsuke", &pcf);

	LCD_FRAME_BUFFER_Transmit(LCD_DMA_TRANSMIT_BLOCKING);

	return NULL;
}



void *SETTINGS_PHOTO_FRAME_TD(void *arg)
{
	settings_item_typedef *photo_frame_td_item = (settings_item_typedef*)arg;

	settings_group.filer_conf.photo_frame_td = photo_frame_td_item->item_array[photo_frame_td_item->selected_id];

	SETTINGS_Save();

	return NULL;
}


void *SETTINGS_DISPLAY_BRIGHTNESS(void *arg)
{
	settings_item_typedef *brightness_item = (settings_item_typedef*)arg;

	settings_group.disp_conf.brightness = brightness_item->item_array[brightness_item->selected_id];


	LCDSetPWMValue(settings_group.disp_conf.brightness);

	debug.printf("\r\nbrightness:%d", settings_group.disp_conf.brightness);

	SETTINGS_Save();

	return NULL;
}

void *SETTINGS_FFT(void *arg)
{
	settings_item_typedef *fft_item = (settings_item_typedef*)arg;

	settings_group.music_conf.b.fft = fft_item->item_array[fft_item->selected_id];

	SETTINGS_Save();

	return NULL;
}

void *SETTINGS_FFT_BAR_TYPE(void *arg)
{
	settings_item_typedef *fft_bar_type_item = (settings_item_typedef*)arg;

	settings_group.music_conf.b.fft_bar_type = fft_bar_type_item->item_array[fft_bar_type_item->selected_id];

	SETTINGS_Save();

	return NULL;
}

void *SETTINGS_FFT_BAR_COLOR(void *arg)
{
	settings_item_typedef *fft_bar_color_item = (settings_item_typedef*)arg;

	settings_group.music_conf.b.fft_bar_color_idx = fft_bar_color_item->item_array[fft_bar_color_item->selected_id];

	SETTINGS_Save();

	return NULL;
}

void *SETTINGS_MUSICINFO(void *arg)
{
	settings_item_typedef *musicinfo_item = (settings_item_typedef*)arg;

	settings_group.music_conf.b.musicinfo = musicinfo_item->item_array[musicinfo_item->selected_id];

	SETTINGS_Save();

	return NULL;
}


void *SETTINGS_DISPLAY_SLEEP(void *arg)
{
	settings_item_typedef *sleep_item = (settings_item_typedef*)arg;

	settings_group.disp_conf.time2sleep = sleep_item->item_array[sleep_item->selected_id];

	SETTINGS_Save();

	debug.printf("\r\nsleep time:%d", settings_group.disp_conf.time2sleep);

	return NULL;
}
