/*
 * image.s
 *
 *  Created on: 2014/04/05
 *      Author: Tonsuke
 */

.section .conf

.balign 2
.global settings_save_128
settings_save_128:
.space 128
.global _sizeof_settings_save_128
.set _sizeof_settings_save_128, . -settings_save_128

.section .misc,"a",%progbits

.balign 2
.global settings_22x22
settings_22x22:
.incbin "settings_22x22.bin"
.global _sizeof_settings_22x22
.set _sizeof_settings_22x22, . -settings_22x22

.balign 2
.global settings_22x22_alpha
settings_22x22_alpha:
.incbin "settings_22x22_alpha.bin"
.global _sizeof_settings_22x22_alpha
.set _sizeof_settings_22x22_alpha, . -settings_22x22_alpha

.balign 2
.global radiobutton_checked_22x22
radiobutton_checked_22x22:
.incbin "radiobutton_checked_22x22.bin"
.global _sizeof_radiobutton_checked_22x22
.set _sizeof_radiobutton_checked_22x22, . -radiobutton_checked_22x22

.balign 2
.global radiobutton_unchecked_22x22
radiobutton_unchecked_22x22:
.incbin "radiobutton_unchecked_22x22.bin"
.global _sizeof_radiobutton_unchecked_22x22
.set _sizeof_radiobutton_unchecked_22x22, . -radiobutton_unchecked_22x22

.balign 2
.global radiobutton_22x22_alpha
radiobutton_22x22_alpha:
.incbin "radiobutton_22x22_alpha.bin"
.global _sizeof_radiobutton_22x22_alpha
.set _sizeof_radiobutton_22x22_alpha, . -radiobutton_22x22_alpha


.balign 2
.global select_22x22
select_22x22:
.incbin "select_22x22.bin"
.global _sizeof_select_22x22
.set _sizeof_select_22x22, . -select_22x22

.balign 2
.global select_22x22_alpha
select_22x22_alpha:
.incbin "select_22x22_alpha.bin"
.global _sizeof_select_22x22_alpha
.set _sizeof_select_22x22_alpha, . -select_22x22_alpha

.balign 2
.global info_22x22
info_22x22:
.incbin "info_22x22.bin"
.global _sizeof_info_22x22
.set _sizeof_info_22x22, . -info_22x22

.balign 2
.global info_22x22_alpha
info_22x22_alpha:
.incbin "info_22x22_alpha.bin"
.global _sizeof_info_22x22_alpha
.set _sizeof_info_22x22_alpha, . -info_22x22_alpha

.balign 2
.global folder_22x22
folder_22x22:
.incbin "folder_22x22.bin"
.global _sizeof_folder_22x22
.set _sizeof_folder_22x22, . -folder_22x22

.balign 2
.global folder_22x22_alpha
folder_22x22_alpha:
.incbin "folder_22x22_alpha.bin"
.global _sizeof_folder_22x22_alpha
.set _sizeof_folder_22x22_alpha, . -folder_22x22_alpha

.balign 2
.global onpu_22x22
onpu_22x22:
.incbin "onpu_22x22.bin"
.global _sizeof_onpu_22x22
.set _sizeof_onpu_22x22, . -onpu_22x22

.balign 2
.global onpu_22x22_alpha
onpu_22x22_alpha:
.incbin "onpu_22x22_alpha.bin"
.global _sizeof_onpu_22x22_alpha
.set _sizeof_onpu_22x22_alpha, . -onpu_22x22_alpha


.balign 2
.global movie_22x22
movie_22x22:
.incbin "movie_22x22.bin"
.global _sizeof_movie_22x22
.set _sizeof_movie_22x22, . -movie_22x22

.balign 2
.global movie_22x22_alpha
movie_22x22_alpha:
.incbin "movie_22x22_alpha.bin"
.global _sizeof_movie_22x22_alpha
.set _sizeof_movie_22x22_alpha, . -movie_22x22_alpha


.balign 2
.global font_22x22
font_22x22:
.incbin "font_22x22.bin"
.global _sizeof_font_22x22
.set _sizeof_font_22x22, . -font_22x22

.balign 2
.global font_22x22_alpha
font_22x22_alpha:
.incbin "font_22x22_alpha.bin"
.global _sizeof_font_22x22_alpha
.set _sizeof_font_22x22_alpha, . -font_22x22_alpha

.balign 2
.global archive_22x22
archive_22x22:
.incbin "archive_22x22.bin"
.global _sizeof_archive_22x22
.set _sizeof_archive_22x22, . -archive_22x22

.balign 2
.global archive_22x22_alpha
archive_22x22_alpha:
.incbin "archive_22x22_alpha.bin"
.global _sizeof_archive_22x22_alpha
.set _sizeof_archive_22x22_alpha, . -archive_22x22_alpha

.balign 2
.global parent_arrow_22x22
parent_arrow_22x22:
.incbin "parent_arrow_22x22.bin"
.global _sizeof_parent_arrow_22x22
.set _sizeof_parent_arrow_22x22, . -parent_arrow_22x22

.balign 2
.global parent_arrow_22x22_alpha
parent_arrow_22x22_alpha:
.incbin "parent_arrow_22x22_alpha.bin"
.global _sizeof_parent_arrow_22x22_alpha
.set _sizeof_parent_arrow_22x22_alpha, . -parent_arrow_22x22_alpha


.section .picture,"a",%progbits
//.section .picture

.balign 2
.global compass_128x128
compass_128x128:
.incbin "compass_128x128.bin"
.global _sizeof_compass_128x128
.set _sizeof_compass_128x128, . -compass_128x128

.global filer_bgimg_160x128
filer_bgimg_160x128:
.incbin "filer_bgimg_160x128.bin"
.global _sizeof_filer_bgimg_160x128
.set _sizeof_filer_bgimg_160x128, . - filer_bgimg_160x128

.global music_bgimg_160x128
music_bgimg_160x128:
.incbin "music_bgimg_160x128.bin"
.global _sizeof_music_bgimg_160x128
.set _sizeof_music_bgimg_160x128, . -music_bgimg_160x128

/*
.section .picture

.balign 2
.global compass_128x128
compass_128x128:
.space 32768
.global _sizeof_compass_128x128
.set _sizeof_compass_128x128, . -compass_128x128

.global filer_bgimg_160x128
filer_bgimg_160x128:
.space 40960
.global _sizeof_filer_bgimg_160x128
.set _sizeof_filer_bgimg_160x128, . - filer_bgimg_160x128

.global music_bgimg_160x128
music_bgimg_160x128:
.space 40960
.global _sizeof_music_bgimg_160x128
.set _sizeof_music_bgimg_160x1288, . -music_bgimg_160x128
*/

.section .rodata

.balign 2
.global internal_flash_pcf_font
internal_flash_pcf_font:
.incbin "mplus_1c_bold_basic_latin_x.pcf"
.global _sizeof_internal_flash_pcf_font
.set _sizeof_internal_flash_pcf_font, . - internal_flash_pcf_font


.balign 2
.global jpeg_22x22
jpeg_22x22:
.incbin "jpeg_22x22.bin"
.global _sizeof_jpeg_22x22
.set _sizeof_jpeg_22x22, . -jpeg_22x22

.balign 2
.global jpeg_22x22_alpha
jpeg_22x22_alpha:
.incbin "jpeg_22x22_alpha.bin"
.global _sizeof_jpeg_22x22_alpha
.set _sizeof_jpeg_22x22_alpha, . -jpeg_22x22_alpha


.balign 2
.global compass_22x22
compass_22x22:
.incbin "compass_22x22.bin"
.global _sizeof_compass_22x22
.set _sizeof_compass_22x22, . -compass_22x22

.balign 2
.global compass_22x22_alpha
compass_22x22_alpha:
.incbin "compass_22x22_alpha.bin"
.global _sizeof_compass_22x22_alpha
.set _sizeof_compass_22x22_alpha, . -compass_22x22_alpha

.balign 2
.global display_22x22
display_22x22:
.incbin "display_22x22.bin"
.global _sizeof_display_22x22
.set _sizeof_display_22x22, . -display_22x22

.balign 2
.global display_22x22_alpha
display_22x22_alpha:
.incbin "display_22x22_alpha.bin"
.global _sizeof_display_22x22_alpha
.set _sizeof_display_22x22_alpha, . -display_22x22_alpha

.balign 2
.global scrollbar_top_6x7
scrollbar_top_6x7:
.incbin "scrollbar_top_6x7.bin"
.global _sizeof_scrollbar_top_6x7
.set _sizeof_scrollbar_top_6x7, . -scrollbar_top_6x7

.balign 2
.global scrollbar_top_6x7_alpha
scrollbar_top_6x7_alpha:
.incbin "scrollbar_top_6x7_alpha.bin"
.global _sizeof_scrollbar_top_6x7_alpha
.set _sizeof_scrollbar_top_6x7_alpha, . -scrollbar_top_6x7_alpha


.balign 2
.global scrollbar_6x98
scrollbar_6x98:
.incbin "scrollbar_6x98.bin"
.global _sizeof_scrollbar_6x98
.set _sizeof_scrollbar_6x98, . -scrollbar_6x98

.balign 2
.global scrollbar_6x98_alpha
scrollbar_6x98_alpha:
.incbin "scrollbar_6x98_alpha.bin"
.global _sizeof_scrollbar_6x98_alpha
.set _sizeof_scrollbar_6x98_alpha, . -scrollbar_6x98_alpha


.balign 2
.global scrollbar_bottom_6x7
scrollbar_bottom_6x7:
.incbin "scrollbar_bottom_6x7.bin"
.global _sizeof_scrollbar_bottom_6x7
.set _sizeof_scrollbar_bottom_6x7, . -scrollbar_bottom_6x7

.balign 2
.global scrollbar_bottom_6x7_alpha
scrollbar_bottom_6x7_alpha:
.incbin "scrollbar_bottom_6x7_alpha.bin"
.global _sizeof_scrollbar_bottom_6x7_alpha
.set _sizeof_scrollbar_bottom_6x7_alpha, . -scrollbar_bottom_6x7_alpha


.balign 2
.global scrollbar_hline_6x1
scrollbar_hline_6x1:
.incbin "scrollbar_hline_6x1.bin"
.global _sizeof_scrollbar_hline_6x1
.set _sizeof_scrollbar_hline_6x1, . -scrollbar_hline_6x1

.balign 2
.global scrollbar_hline_6x1_alpha
scrollbar_hline_6x1_alpha:
.incbin "scrollbar_hline_6x1_alpha.bin"
.global _sizeof_scrollbar_hline_6x1_alpha
.set _sizeof_scrollbar_hline_6x1_alpha, . -scrollbar_hline_6x1_alpha


.balign 2
.global menubar_160x22
menubar_160x22:
.incbin "menubar_160x22.bin"
.global _sizeof_menubar_160x22
.set _sizeof_menubar_160x22, . -menubar_160x22

.balign 2
.global menubar_160x22_alpha
menubar_160x22_alpha:
.incbin "menubar_160x22_alpha.bin"
.global _sizeof_menubar_160x22_alpha
.set _sizeof_menubar_160x22_alpha, . -menubar_160x22_alpha


/*
.balign 2
.global seekbar_130x14
seekbar_130x14:
.incbin "seekbar_130x14.bin"
.global _sizeof_seekbar_130x14
.set _sizeof_seekbar_130x14, . -seekbar_130x14

.balign 2
.global seekbar_130x14_alpha
seekbar_130x14_alpha:
.incbin "seekbar_130x14_alpha.bin"
.global _sizeof_seekbar_130x14_alpha
.set _sizeof_seekbar_130x14_alpha, . -seekbar_130x14_alpha
*/
.balign 2
.global seekbar_120x14
seekbar_120x14:
.incbin "seekbar_120x14.bin"
.global _sizeof_seekbar_120x14
.set _sizeof_seekbar_120x14, . -seekbar_120x14

.balign 2
.global seekbar_120x14_alpha
seekbar_120x14_alpha:
.incbin "seekbar_120x14_alpha.bin"
.global _sizeof_seekbar_120x14_alpha
.set _sizeof_seekbar_120x14_alpha, . -seekbar_120x14_alpha

.balign 2
.global play_icon_12x12
play_icon_12x12:
.incbin "play_icon_12x12.bin"
.global _sizeof_play_icon_12x12
.set _sizeof_play_icon_12x12, . -play_icon_12x12

.balign 2
.global play_icon_12x12_alpha
play_icon_12x12_alpha:
.incbin "play_icon_12x12_alpha.bin"
.global _sizeof_play_icon_12x12_alpha
.set _sizeof_play_icon_12x12_alpha, . -play_icon_12x12_alpha


.balign 2
.global pause_icon_12x12
pause_icon_12x12:
.incbin "pause_icon_12x12.bin"
.global _sizeof_pause_icon_12x12
.set _sizeof_pause_icon_12x12, . -pause_icon_12x12

.balign 2
.global pause_icon_12x12_alpha
pause_icon_12x12_alpha:
.incbin "pause_icon_12x12_alpha.bin"
.global _sizeof_pause_icon_12x12_alpha
.set _sizeof_pause_icon_12x12_alpha, . -pause_icon_12x12_alpha

.global mute_icon_12x12
mute_icon_12x12:
.incbin "mute_icon_12x12.bin"
.global _sizeof_mute_icon_12x12
.set _sizeof_mute_icon_12x12, . -mute_icon_12x12

.balign 2
.global mute_icon_12x12_alpha
mute_icon_12x12_alpha:
.incbin "mute_icon_12x12_alpha.bin"
.global _sizeof_mute_icon_12x12_alpha
.set _sizeof_mute_icon_12x12_alpha, . -mute_icon_12x12_alpha



.balign 2
.global seek_circle_12x12
seek_circle_12x12:
.incbin "seek_circle_12x12.bin"
.global _sizeof_seek_circle_12x12
.set _sizeof_seek_circle_12x12, . -seek_circle_12x12

.balign 2
.global seek_active_circle_12x12
seek_active_circle_12x12:
.incbin "seek_active_circle_12x12.bin"
.global _sizeof_seek_active_circle_12x12
.set _sizeof_seek_active_circle_12x12, . -seek_active_circle_12x12

.balign 2
.global seek_circle_12x12_alpha
seek_circle_12x12_alpha:
.incbin "seek_circle_12x12_alpha.bin"
.global _sizeof_seek_circle_12x12_alpha
.set _sizeof_seek_circle_12x12_alpha, . -seek_circle_12x12_alpha


.balign 2
.global navigation_loop_18x14
navigation_loop_18x14:
.incbin "navigation_loop_18x14.bin"
.global _sizeof_navigation_loop_18x14
.set _sizeof_navigation_loop_18x14, . -navigation_loop_18x14


.balign 2
.global navigation_loop_18x14_alpha
navigation_loop_18x14_alpha:
.incbin "navigation_loop_18x14_alpha.bin"
.global _sizeof_navigation_loop_18x14_alpha
.set _sizeof_navigation_loop_18x14_alpha, . -navigation_loop_18x14_alpha


.balign 2
.global navigation_infinite_entire_loop_18x14
navigation_infinite_entire_loop_18x14:
.incbin "navigation_infinite_entire_loop_18x14.bin"
.global _sizeof_navigation_infinite_entire_loop_18x14
.set _sizeof_navigation_infinite_entire_loop_18x14, . -navigation_infinite_entire_loop_18x14

.balign 2
.global navigation_infinite_entire_loop_18x14_alpha
navigation_infinite_entire_loop_18x14_alpha:
.incbin "navigation_infinite_entire_loop_18x14_alpha.bin"
.global _sizeof_navigation_infinite_entire_loop_18x14_alpha
.set _sizeof_navigation_infinite_entire_loop_18x14_alpha, . -navigation_infinite_entire_loop_18x14_alpha

.balign 2
.global navigation_bar_18x14
navigation_bar_18x14:
.incbin "navigation_bar_18x14.bin"
.global _sizeof_navigation_bar_18x14
.set _sizeof_navigation_bar_18x14, . -navigation_bar_18x14

.balign 2
.global navigation_bar_18x14_alpha
navigation_bar_18x14_alpha:
.incbin "navigation_bar_18x14_alpha.bin"
.global _sizeof_navigation_bar_18x14_alpha
.set _sizeof_navigation_bar_18x14_alpha, . -navigation_bar_18x14_alpha

.balign 2
.global navigation_infinite_one_loop_18x14
navigation_infinite_one_loop_18x14:
.incbin "navigation_infinite_one_loop_18x14.bin"
.global _sizeof_navigation_infinite_one_loop_18x14
.set _sizeof_navigation_infinite_one_loop_18x14, . -navigation_infinite_one_loop_18x14

.balign 2
.global navigation_infinite_one_loop_18x14_alpha
navigation_infinite_one_loop_18x14_alpha:
.incbin "navigation_infinite_one_loop_18x14_alpha.bin"
.global _sizeof_navigation_infinite_one_loop_18x14_alpha
.set _sizeof_navigation_infinite_one_loop_18x14_alpha, . -navigation_infinite_one_loop_18x14_alpha

.balign 2
.global navigation_shuffle_18x14
navigation_shuffle_18x14:
.incbin "navigation_shuffle_18x14.bin"
.global _sizeof_navigation_shuffle_18x14
.set _sizeof_navigation_shuffle_18x14, . -navigation_shuffle_18x14

.balign 2
.global navigation_shuffle_18x14_alpha
navigation_shuffle_18x14_alpha:
.incbin "navigation_shuffle_18x14_alpha.bin"
.global _sizeof_navigation_shuffle_18x14_alpha
.set _sizeof_navigation_shuffle_18x14_alpha, . -navigation_shuffle_18x14_alpha


.balign 2
.global bass_base_24x14
bass_base_24x14:
.incbin "bass_base_24x14.bin"
.global _sizeof_bass_base_24x14
.set _sizeof_bass_base_24x14, . -bass_base_24x14

.balign 2
.global bass_base_24x14_alpha
bass_base_24x14_alpha:
.incbin "bass_base_24x14_alpha.bin"
.global _sizeof_bass_base_24x14_alpha
.set _sizeof_bass_base_24x14_alpha, . -bass_base_24x14_alpha

.balign 2
.global bass_level1_24x14
bass_level1_24x14:
.incbin "bass_level1_24x14.bin"
.global _sizeof_bass_level1_24x14
.set _sizeof_bass_level1_24x14, . -bass_level1_24x14

.balign 2
.global bass_level1_24x14_alpha
bass_level1_24x14_alpha:
.incbin "bass_level1_24x14_alpha.bin"
.global _sizeof_bass_level1_24x14_alpha
.set _sizeof_bass_level1_24x14_alpha, . -bass_level1_24x14_alpha

.balign 2
.global bass_level2_24x14
bass_level2_24x14:
.incbin "bass_level2_24x14.bin"
.global _sizeof_bass_level2_24x14
.set _sizeof_bass_level2_24x14, . -bass_level2_24x14

.balign 2
.global bass_level2_24x14_alpha
bass_level2_24x14_alpha:
.incbin "bass_level2_24x14_alpha.bin"
.global _sizeof_bass_level2_24x14_alpha
.set _sizeof_bass_level2_24x14_alpha, . -bass_level2_24x14_alpha

.balign 2
.global bass_level3_24x14
bass_level3_24x14:
.incbin "bass_level3_24x14.bin"
.global _sizeof_bass_level3_24x14
.set _sizeof_bass_level3_24x14, . -bass_level3_24x14

.balign 2
.global bass_level3_24x14_alpha
bass_level3_24x14_alpha:
.incbin "bass_level3_24x14_alpha.bin"
.global _sizeof_bass_level3_24x14_alpha
.set _sizeof_bass_level3_24x14_alpha, . -bass_level3_24x14_alpha
