/*
 * fft.c
 *
 *  Created on: 2013/02/23
 *      Author: Tonsuke
 */

#include "fft.h"

#include "lcd.h"

#include "settings.h"


void FFT_Init(FFT_Struct_Typedef *FFT)
{
	FFT->status = arm_cfft_radix4_init_q31(&FFT->S, FFT->length, FFT->ifftFlag, FFT->bitReverseFlag);

//	if(settings_group.music_conf.b.prehalve){
//		FFT->rightShift = 21;
//	} else {
//		FFT->rightShift = 22;
//	}
	if(FFT->bitPerSample == 16) {
		FFT->rightShift = 25;
	} else {
		FFT->rightShift = 24;
	}
}

void FFT_Sample(FFT_Struct_Typedef *FFT, uint32_t *pSrc)
{
	int idx = 0, i;
	uint32_t u32;

	if(FFT->bitPerSample == 16){
		for(i = 0;i < FFT->samples;i += (FFT->samples / FFT->length)){

			FFT->left_inbuf[idx] = (*(q31_t*)&pSrc[i] << 16) & 0xffff0000; // Real Left
			FFT->left_inbuf[idx + 1] = 0; // Imag

			FFT->left_inbuf[idx] += *(q31_t*)&pSrc[i] & 0xffff0000; // Real Right
			FFT->left_inbuf[idx + 1] = 0; // Imag
	 		idx += 2;
		 }
	} else {
		for(i = 0;i < FFT->samples;i += (FFT->samples / FFT->length)){

			u32 = pSrc[i];
			u32 = __REV(u32);
			u32 = __REV16(u32);

			FFT->left_inbuf[idx] = (q31_t)u32; // Real Left
			FFT->left_inbuf[idx + 1] = 0; // Imag

	 		idx += 2;
		 }
	}
}
/*
void FFT_Display_Left(FFT_Struct_Typedef *FFT, drawBuff_typedef *drawBuff, uint16_t color)
{
	int power, i, j, barType = 0, colorBar = 0;
	fft_analyzer_typedef fftDrawBuff;

	arm_cfft_radix4_q31(&FFT->S, FFT->left_inbuf);
	arm_cmplx_mag_q31(FFT->left_inbuf, FFT->outbuf, FFT->length >> 1);

	memcpy(&fftDrawBuff, &drawBuff->fft_analyzer_left, sizeof(fft_analyzer_typedef));

	switch(barType){
	case 0: // Solid
		for(i = 0;i < FFT->length >> 1;i++){
			power = __USAT(FFT->outbuf[i] >> FFT->rightShift, 3);
			for(j = 0;j < power;j++){
				fftDrawBuff.p[i + (FFT->length - 1 - j) * FFT->length] = color_bar[colorBar][j];
			}
		}
		break;
	case 1: // V Split
		for(i = 0;i < FFT->length >> 1;i += 2){
			power = __USAT(FFT->outbuf[i] >> FFT->rightShift, 5);
			for(j = 0;j < power;j++){
				fftDrawBuff.p[i + (31 - j) * 32] = color_bar[colorBar][j];
			}
		}
		break;
	case 2: // H Split
		for(i = 0;i < FFT->length >> 1;i += 2){
			power = FFT->outbuf[i] >> FFT->rightShift;
			power += FFT->outbuf[i + 1] >> FFT->rightShift;
			power =  __USAT(power >>= 1, 5);

			for(j = 0;j < power;j += 2){
				fftDrawBuff.p[i + (31 - j) * 32] = color_bar[colorBar][j];
				fftDrawBuff.p[i + (31 - j) * 32 + 1] = color_bar[colorBar][j];
			}
		}
		break;
	case 3: // Wide
		for(i = 0;i < FFT->length >> 1;i += 2){
			power = FFT->outbuf[i] >> FFT->rightShift;
			power += FFT->outbuf[i + 1] >> FFT->rightShift;
			power =  __USAT(power >>= 1, 5);

			for(j = 0;j < power;j++){
				fftDrawBuff.p[i + (31 - j) * 32] = color_bar[colorBar][j];
				fftDrawBuff.p[i + (31 - j) * 32 + 1] = color_bar[colorBar][j];
			}
		}
		break;
	default:
		break;
	}

	LCDPutBuffToBgImg(fftDrawBuff.x, fftDrawBuff.y, \
			fftDrawBuff.width, fftDrawBuff.height, fftDrawBuff.p);
}
*/
#define FRAME_BUFFER_SIZE (160 * 128 * sizeof(uint16_t))

__attribute__( ( always_inline ) ) static __INLINE void set_image_data(uint16_t *p_img, uint16_t width, uint16_t x, uint16_t y, uint16_t data)
{
	if((y * width + x) >= FRAME_BUFFER_SIZE){
		return;
	}
	p_img[y * width + x] = data;
}

__attribute__( ( always_inline ) ) static __INLINE uint16_t get_image_data(const uint16_t *p_img, uint16_t width, uint16_t x, uint16_t y)
{
	return (p_img[y * width + x]);
}


void LCD_DrawSquare(int x, int y, int width, int height, colors color)
{
	int i, j;
	extern uint16_t frame_buffer[160 * 128];

	for(j = 0;j < height;j++){
		for(i = 0;i < width;i++){
			set_image_data(frame_buffer, LCD_WIDTH, x + i, y + j, colorc[color]);
		}
	}
}

void LCD_DrawSquare_DB(int x, int y, int width, int height, colors color)
{
	int i, j;

	for(j = 0;j < height;j++){
		for(i = 0;i < width;i++){
			LCD_SetGramAddr(x + i, y + j);
			LCD_CMD(0x002c);
			LCD_DATA(colorc[color]);
		}
	}
}


void LCD_DrawHideSquare(int x, int y, int width, int height)
{
	int i, j;
	extern uint16_t frame_buffer[160 * 128];

	for(j = 0;j < height;j++){
		for(i = 0;i < width;i++){
			set_image_data(frame_buffer, LCD_WIDTH, x + i, y + j, get_image_data(music_bgimg_160x128, LCD_WIDTH, x + i, y + j));
		}
	}
}

__attribute__( ( always_inline ) ) static __INLINE void framebuffer_to_square(int x, int y, int width, int height, int col, int row, fft_analyzer_typedef *fft)
{
	int i, j;
	extern uint16_t frame_buffer[160 * 128];

	for(j = 0;j < height;j++){
		for(i = 0;i < width;i++){
			fft->bar[col][row][j * width + i] = get_image_data(frame_buffer, LCD_WIDTH, x + i, y + j);
		}
	}
}


void LCD_StoreFrameBufferToFFTSquares(drawBuff_typedef *drawBuff)
{
	int i, j;
	fft_analyzer_typedef *fft = (fft_analyzer_typedef*)&drawBuff->fft_analyzer_left;

	for(j = 0;j < 8;j++){
		for(i = 0;i < 8;i++){
			framebuffer_to_square(j * (fft->squareSize + fft->spacing) + fft->x, \
					fft->y - i * (fft->squareSize + fft->spacing), fft->squareSize, fft->squareSize, i, j, fft);
		}
	}
}

__attribute__( ( always_inline ) ) static __INLINE void square_to_framebuffer(int x, int y, int width, int height, int col, int row, fft_analyzer_typedef *fft)
{
	int i, j;
	extern uint16_t frame_buffer[160 * 128];

	for(j = 0;j < height;j++){
		for(i = 0;i < width;i++){
			set_image_data(frame_buffer, LCD_WIDTH, x + i, y + j, fft->bar[col][row][j * width + i]);
		}
	}
}

/*
void LCD_PutFFTSquaresToFrameBuffer(drawBuff_typedef *drawBuff)
{
	int i, j;
	fft_analyzer_typedef *fft = (fft_analyzer_typedef*)&drawBuff->fft_analyzer_left;

	for(j = 0;j < 8;j++){
		for(i = 0;i < 5;i++){
			square_to_framebuffer(i * (fft->squareSize + fft->spacing) + fft->x, \
					fft->y - j * (fft->squareSize + fft->spacing), fft->squareSize, fft->squareSize, i, j, fft);
		}
	}
}
*/

const static colors fft_colors[6] = {WHITE, SKYBLUE, YELLOW, GREEN, RED, BLACK};

void FFT_Display(FFT_Struct_Typedef *FFT, drawBuff_typedef *drawBuff, uint8_t erase)
{
	int power, i, j;
	fft_analyzer_typedef *fft = (fft_analyzer_typedef*)&drawBuff->fft_analyzer_left;
	extern uint16_t frame_buffer[160 * 128];

	if(erase == 1)
	{
		for(j = 0;j < FFT->length >> 1;j++){
			for(i = 0;i < 8;i++){
					square_to_framebuffer(j * (fft->squareSize + fft->spacing) + fft->x, \
							fft->y - i * (fft->squareSize + fft->spacing), fft->squareSize, fft->squareSize, i, j, fft);
			}
		}
		return;
	}

	arm_cfft_radix4_q31(&FFT->S, FFT->left_inbuf);
	arm_cmplx_mag_q31(FFT->left_inbuf, FFT->outbuf, FFT->length >> 1);

	for(j = 0;j < FFT->length >> 1;j++){
		power = __USAT(FFT->outbuf[j] >> FFT->rightShift, 3);
		for(i = 0;i < 8;i++){
			if(power-- > 0){
				LCD_DrawSquare(j * (fft->squareSize + fft->spacing) + fft->x, \
						fft->y - i * (fft->squareSize + fft->spacing), fft->squareSize, fft->squareSize, fft_colors[settings_group.music_conf.b.fft_bar_color_idx]);
			} else {
				square_to_framebuffer(j * (fft->squareSize + fft->spacing) + fft->x, \
						fft->y - i * (fft->squareSize + fft->spacing), fft->squareSize, fft->squareSize, i, j, fft);
			}
		}
	}
}


/*
void FFT_Display_Right(FFT_Struct_Typedef *FFT, drawBuff_typedef *drawBuff, uint16_t color)
{
	int power, i, j;
	fft_analyzer_typedef fftDrawBuff;
	extern settings_group_typedef settings_group;

	arm_cfft_radix4_q31(&FFT->S, FFT->right_inbuf);
	arm_cmplx_mag_q31(FFT->right_inbuf, FFT->outbuf, FFT->length >> 1);

	memcpy(&fftDrawBuff, &drawBuff->fft_analyzer_right, sizeof(fft_analyzer_typedef));

	switch(settings_group.music_conf.b.fft_bar_type){
	case 0: // Solid
		for(i = 0;i < FFT->length >> 1;i++){
			power = __USAT(FFT->outbuf[i] >> FFT->rightShift, 5);
			for(j = 0;j < power;j++){
				fftDrawBuff.p[i + (31 - j) * 32] = color_bar[settings_group.music_conf.b.fft_bar_color_idx][j];
			}
		}
		break;
	case 1: // V Split
		for(i = 0;i < FFT->length >> 1;i += 2){
			power = __USAT(FFT->outbuf[i] >> FFT->rightShift, 5);
			for(j = 0;j < power;j++){
				fftDrawBuff.p[i + (31 - j) * 32] = color_bar[settings_group.music_conf.b.fft_bar_color_idx][j];
			}
		}
		break;
	case 2: // H Split
		for(i = 0;i < FFT->length >> 1;i += 2){
			power = FFT->outbuf[i] >> FFT->rightShift;
			power += FFT->outbuf[i + 1] >> FFT->rightShift;
			power =  __USAT(power >>= 1, 5);

			for(j = 0;j < power;j += 2){
				fftDrawBuff.p[i + (31 - j) * 32] = color_bar[settings_group.music_conf.b.fft_bar_color_idx][j];
				fftDrawBuff.p[i + (31 - j) * 32 + 1] = color_bar[settings_group.music_conf.b.fft_bar_color_idx][j];
			}
		}
		break;
	case 3: // Wide
		for(i = 0;i < FFT->length >> 1;i += 2){
			power = FFT->outbuf[i] >> FFT->rightShift;
			power += FFT->outbuf[i + 1] >> FFT->rightShift;
			power =  __USAT(power >>= 1, 5);

			for(j = 0;j < power;j++){
				fftDrawBuff.p[i + (31 - j) * 32] = color_bar[settings_group.music_conf.b.fft_bar_color_idx][j];
				fftDrawBuff.p[i + (31 - j) * 32 + 1] = color_bar[settings_group.music_conf.b.fft_bar_color_idx][j];
			}
		}
		break;
	default:
		break;
	}

	LCDPutBuffToBgImg(fftDrawBuff.x, fftDrawBuff.y, \
			fftDrawBuff.width, fftDrawBuff.height, fftDrawBuff.p);
}
*/
