//------------------------------------------------------------------------------
// jpg2tga.c
// JPEG to TGA file conversion example program.
// Public domain, Rich Geldreich <richgel99@gmail.com>
// Last updated Nov. 26, 2010
//------------------------------------------------------------------------------
#include "picojpeg.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "fat.h"
#include "usart.h"
#include "lcd.h"
#include "icon.h"
#include "main.h"

#include "settings.h"

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//typedef unsigned char uint8_t;
//typedef unsigned int uint;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
MY_FILE *g_pInFile;
uint32_t g_pInFileSize;
uint32_t g_pInFileOfs;
//------------------------------------------------------------------------------
unsigned char pjpeg_need_bytes_callback(unsigned char* pBuf, unsigned char buf_size, unsigned char *pBytes_actually_read, void *pCallback_data)
{
   uint n;
   pCallback_data;
   
   n = min(g_pInFileSize - g_pInFileOfs, buf_size);
   if (n && (my_fread(pBuf, 1, n, g_pInFile) != n))
      return PJPG_STREAM_READ_ERROR;
   *pBytes_actually_read = (unsigned char)(n);
   g_pInFileOfs += n;
   return 0;
}
//------------------------------------------------------------------------------
// Loads JPEG image from specified file. Returns NULL on failure.
// On success, the malloc()'d image's width/height is written to *x and *y, and
// the number of components (1 or 3) is written to *comps.
// pScan_type can be NULL, if not it'll be set to the image's pjpeg_scan_type_t.
// Not thread safe.
// If reduce is non-zero, the image will be more quickly decoded at approximately
// 1/8 resolution (the actual returned resolution will depend on the JPEG 
// subsampling factor).
uint8_t *pjpeg_load_from_file(int *x, int *y, int *comps, pjpeg_scan_type_t *pScan_type, int reduce, int play_mode)
{
   pjpeg_image_info_t image_info;
   int mcu_x = 0;
   int mcu_y = 0;
   uint row_pitch;
   uint8_t *pImage;
   uint8_t status;
   uint decoded_width, decoded_height;
   uint row_blocks_per_mcu, col_blocks_per_mcu;
   extern uint16_t frame_buffer[160 * 128];


   *x = 0;
   *y = 0;
   *comps = 0;
   if (pScan_type) *pScan_type = PJPG_GRAYSCALE;

//   g_pInFile = my_fopen(id);
//  if (!g_pInFile)
//      return NULL;

   g_pInFileOfs = 0;

//   my_fseek(g_pInFile, 0, SEEK_END);
//   g_ng_pInFileSize = ftell(g_pInFile);
   g_pInFileSize = g_pInFile->fileSize;
   my_fseek(g_pInFile, 0, SEEK_SET);
      
   status = pjpeg_decode_init(&image_info, pjpeg_need_bytes_callback, NULL, (unsigned char)reduce);
         
   if (status)
   {
      debug.printf("\r\npjpeg_decode_init() failed with status %u", status);
      if (status == PJPG_UNSUPPORTED_MODE)
      {
         debug.printf("\r\nProgressive JPEG files are not supported.");
      }

      my_fclose(g_pInFile);
      return NULL;
   }
   
   if (pScan_type)
      *pScan_type = image_info.m_scanType;

   const char* p = "?";

   switch (*pScan_type)
   {
      case PJPG_GRAYSCALE: p = "GRAYSCALE"; break;
      case PJPG_YH1V1: p = "H1V1"; break;
      case PJPG_YH2V1: p = "H2V1"; break;
      case PJPG_YH1V2: p = "H1V2"; break;
      case PJPG_YH2V2: p = "H2V2"; break;
   }
   debug.printf("\r\nScan type: %s", p);
   debug.printf("\r\nimage_info.m_MCUSPerRow:%d", image_info.m_MCUSPerRow);
   debug.printf("\r\nimage_info.m_MCUSPerCol:%d", image_info.m_MCUSPerCol);
   debug.printf("\r\nimage_info.m_MCUWidth:%d", image_info.m_MCUWidth);
   debug.printf("\r\nimage_info.m_MCUHeight:%d", image_info.m_MCUHeight);

   // In reduce mode output 1 pixel per 8x8 block.
   decoded_width = reduce ? (image_info.m_MCUSPerRow * image_info.m_MCUWidth) / 8 : image_info.m_width;
   decoded_height = reduce ? (image_info.m_MCUSPerCol * image_info.m_MCUHeight) / 8 : image_info.m_height;

   debug.printf("\r\n\nWidth: %i, Height: %i", decoded_width, decoded_height);

   uint down_scale = 1, width, height, t = 0;

   if((decoded_width > LCD_WIDTH) || (decoded_height > LCD_HEIGHT))
   {
	   reduce = 1;

	   g_pInFileOfs = 0;

	   my_fseek(g_pInFile, 0, SEEK_END);
	//   g_ng_pInFileSize = ftell(g_pInFile);
	   g_pInFileSize = g_pInFile->fileSize;
	   my_fseek(g_pInFile, 0, SEEK_SET);

	   status = pjpeg_decode_init(&image_info, pjpeg_need_bytes_callback, NULL, (unsigned char)reduce);

	   if (status)
	   {
	      debug.printf("\r\npjpeg_decode_init() failed with status %u", status);
	      if (status == PJPG_UNSUPPORTED_MODE)
	      {
	         debug.printf("\r\nProgressive JPEG files are not supported.");
	      }

	      my_fclose(g_pInFile);
	      return NULL;
	   }

	   if (pScan_type)
	      *pScan_type = image_info.m_scanType;

	   const char* p = "?";

	   switch (*pScan_type)
	   {
	      case PJPG_GRAYSCALE: p = "GRAYSCALE"; break;
	      case PJPG_YH1V1: p = "H1V1"; break;
	      case PJPG_YH2V1: p = "H2V1"; break;
	      case PJPG_YH1V2: p = "H1V2"; break;
	      case PJPG_YH2V2: p = "H2V2"; break;
	   }
	   debug.printf("\r\nScan type: %s", p);
	   debug.printf("\r\nimage_info.m_MCUSPerRow:%d", image_info.m_MCUSPerRow);
	   debug.printf("\r\nimage_info.m_MCUSPerCol:%d", image_info.m_MCUSPerCol);
	   debug.printf("\r\nimage_info.m_MCUWidth:%d", image_info.m_MCUWidth);
	   debug.printf("\r\nimage_info.m_MCUHeight:%d", image_info.m_MCUHeight);

	   // In reduce mode output 1 pixel per 8x8 block.
	   decoded_width = reduce ? (image_info.m_MCUSPerRow * image_info.m_MCUWidth) / 8 : image_info.m_width;
	   decoded_height = reduce ? (image_info.m_MCUSPerCol * image_info.m_MCUHeight) / 8 : image_info.m_height;


		width = decoded_width;
		height = decoded_height;

		while(1){
			if(width <= LCD_WIDTH && height <= LCD_HEIGHT){
				break;
			}
			down_scale *= 2;
			width = decoded_width / down_scale;
			height = decoded_height / down_scale;
		}

		debug.printf("\r\ndown_scale:%d", down_scale);

//		if((decoded_width > LCD_WIDTH) || (decoded_height > LCD_HEIGHT))
//		{
//			my_fclose(g_pInFile);
//			return NULL;
//		}
   }

//   row_pitch = decoded_width * image_info.m_comps;
   row_pitch = (decoded_width / down_scale) * 2;
//   row_pitch = LCD_WIDTH * 2;

//   pImage = (uint8_t *)malloc(row_pitch * decoded_height);
   pImage = (uint8_t *)frame_buffer;
   if (!pImage)
   {
      my_fclose(g_pInFile);
      return NULL;
   }

   row_blocks_per_mcu = image_info.m_MCUWidth >> 3;
   col_blocks_per_mcu = image_info.m_MCUHeight >> 3;
   
   debug.printf("\r\nimage_info.m_MCUWidth:%d", image_info.m_MCUWidth);
   debug.printf("\r\nimage_info.m_MCUHeight:%d", image_info.m_MCUHeight);

   memset((void*)frame_buffer, 0, FRAME_BUFFER_SIZE);
   if(play_mode != DJPEG_PLAY){
	   LCD_FRAME_BUFFER_Transmit(LCD_DMA_TRANSMIT_BLOCKING);
	   DMA_ProgressBar_Start();
   }

   for ( ; ; )
   {
      int y, x;
      uint8_t *pDst_row;

      status = pjpeg_decode_mcu();
      
//      debug.printf("\r\nmcu_x:%d mcu_y:%d", mcu_x, mcu_y);

      if (status)
      {
         if (status != PJPG_NO_MORE_BLOCKS)
         {
            debug.printf("\r\npjpeg_decode_mcu() failed with status %u", status);

//            free(pImage);
            my_fclose(g_pInFile);
            return NULL;
         }

         break;
      }

      if (mcu_y >= image_info.m_MCUSPerCol)
      {
//         free(pImage);
         my_fclose(g_pInFile);
         return NULL;
      }

      if (reduce)
      {
         // In reduce mode, only the first pixel of each 8x8 block is valid.
//         pDst_row = pImage + mcu_y * col_blocks_per_mcu * row_pitch + mcu_x * row_blocks_per_mcu * image_info.m_comps;
          pDst_row = pImage + (mcu_y * col_blocks_per_mcu) / down_scale * row_pitch + (mcu_x * row_blocks_per_mcu) / down_scale * 2;
         if (image_info.m_scanType == PJPG_GRAYSCALE)
         {
            *pDst_row = image_info.m_pMCUBufR[0];
         }
         else
         {
            uint y, x;
            for (y = 0; y < col_blocks_per_mcu; y++)
            {
               uint src_ofs = (y * 128U);
               for (x = 0; x < row_blocks_per_mcu; x++)
               {
//                  pDst_row[0] = image_info.m_pMCUBufR[src_ofs];
//                  pDst_row[1] = image_info.m_pMCUBufG[src_ofs];
//                  pDst_row[2] = image_info.m_pMCUBufB[src_ofs];
//                  pDst_row += 3;
            	  *(uint16_t*)pDst_row = ( (image_info.m_pMCUBufR[src_ofs] >> 3) << 11 ) | ( (image_info.m_pMCUBufG[src_ofs]  >> 2) << 5 ) | (image_info.m_pMCUBufB[src_ofs] >> 3);
            	  pDst_row += 2;
                  src_ofs += 64;
                  t++;
               }

//               pDst_row += row_pitch - 3 * row_blocks_per_mcu;
               pDst_row += row_pitch - 2 * row_blocks_per_mcu;
            }
         }
      }
      else
      {
         // Copy MCU's pixel blocks into the destination bitmap.
//         pDst_row = pImage + (mcu_y * image_info.m_MCUHeight) * row_pitch + (mcu_x * image_info.m_MCUWidth * image_info.m_comps);
          pDst_row = pImage + (mcu_y * image_info.m_MCUHeight) * row_pitch + (mcu_x * image_info.m_MCUWidth * 2);

         for (y = 0; y < image_info.m_MCUHeight; y += 8)
         {
            const int by_limit = min(8, image_info.m_height - (mcu_y * image_info.m_MCUHeight + y));

            for (x = 0; x < image_info.m_MCUWidth; x += 8)
            {
//               uint8_t *pDst_block = pDst_row + x * image_info.m_comps;
                uint8_t *pDst_block = pDst_row + x * 2;

               // Compute source byte offset of the block in the decoder's MCU buffer.
               uint src_ofs = (x * 8U) + (y * 16U);
               const uint8_t *pSrcR = image_info.m_pMCUBufR + src_ofs;
               const uint8_t *pSrcG = image_info.m_pMCUBufG + src_ofs;
               const uint8_t *pSrcB = image_info.m_pMCUBufB + src_ofs;

               const int bx_limit = min(8, image_info.m_width - (mcu_x * image_info.m_MCUWidth + x));

               if (image_info.m_scanType == PJPG_GRAYSCALE)
               {
                  int bx, by;
                  for (by = 0; by < by_limit; by++)
                  {
                     uint8_t *pDst = pDst_block;

                     for (bx = 0; bx < bx_limit; bx++)
                        *pDst++ = *pSrcR++;

                     pSrcR += (8 - bx_limit);

                     pDst_block += row_pitch;
                  }
               }
               else
               {
                  int bx, by;
                  for (by = 0; by < by_limit; by++)
                  {
                     uint8_t *pDst = pDst_block;

                     for (bx = 0; bx < bx_limit; bx++)
                     {
//                        pDst[0] = *pSrcR++;
//                        pDst[1] = *pSrcG++;
//                        pDst[2] = *pSrcB++;
//                        pDst += 3;
//                    	debug.printf("\r\nx:%d y:%d", ((pDst - pImage) >> 1) % decoded_width, ((pDst - pImage) >> 1) / decoded_width);

                    	*(uint16_t*)pDst = ( (*pSrcR++ >> 3) << 11 ) | ( (*pSrcG++  >> 2) << 5 ) | (*pSrcB++ >> 3);
                    	pDst += 2;
                    	t++;

//                    	extern SPI_HandleTypeDef SpiLcdHandle;
//                    	while(SpiLcdHandle.State != HAL_SPI_STATE_READY);
//                    	LCD_SetGramAddr(0, 0);
//                    	LCD_CMD(0x002c);
//                    	LCD_FRAME_BUFFER_Transmit();
                     }

                     pSrcR += (8 - bx_limit);
                     pSrcG += (8 - bx_limit);
                     pSrcB += (8 - bx_limit);

                     pDst_block += row_pitch;
                  }
               }
            }

            pDst_row += (row_pitch * 8);
         }
      }

      mcu_x++;
      if (mcu_x == image_info.m_MCUSPerRow)
      {
         mcu_x = 0;
         mcu_y++;
      }
   }

   my_fclose(g_pInFile);

   debug.printf("\r\nt:%d", t);

   if(down_scale > 1){
	   decoded_width = width;
	   decoded_height = height;
   }
   *x = decoded_width;
   *y = decoded_height;
   *comps = image_info.m_comps;

   DMA_ProgressBar_Stop();

//   if(play_mode == DJPEG_PLAY){
//	   LCD_Clear(colorc[RED]);
 //  }

   if(decoded_height != LCD_HEIGHT || decoded_width != LCD_WIDTH)
   {
	   uint max_span;
	   int16_t zoom, width, height, offsetX, offsetY;
	   max_span = max(decoded_width, decoded_height);
	   if(max_span == decoded_height)
	   {
		   zoom = (LCD_HEIGHT * 100) / max_span;
		   height = LCD_HEIGHT;
		   width = decoded_width * (float)LCD_HEIGHT / (float)max_span;
	   } else {
		   zoom = (LCD_WIDTH * 100) / max_span;
		   width = LCD_WIDTH;
		   height = decoded_height * (float)LCD_WIDTH / (float)max_span;
	   }

	   debug.printf("\r\nwidth:%d height:%d", width, height);

	   offsetX = width > LCD_WIDTH ? 0 : (LCD_WIDTH - width) / 2;
	   offsetY = height > LCD_HEIGHT ? 0 : (LCD_HEIGHT - height) / 2;

	   LCD_DrawSquare_DB(0, 0, LCD_WIDTH, offsetY, BLACK);
	   LCD_DrawSquare_DB(0, offsetY, (LCD_WIDTH - width) / 2, height, BLACK);
	   LCD_DrawSquare_DB(offsetX + width, offsetY, (LCD_WIDTH - width) / 2 + 1, height, BLACK);
	   LCD_DrawSquare_DB(0, offsetY + height, LCD_WIDTH, LCD_HEIGHT - (offsetY + height), BLACK);

	   LCDZoomImage(offsetX, offsetY, decoded_width, decoded_height, frame_buffer, colorc[BLACK], zoom, 1);
	   LCD_SetGramAddr(0, 0);
	   LCD_CMD(0x002c);

   } else {
	   LCD_SetGramAddr(0, 0);
	   LCD_CMD(0x002c);
	   LCD_FRAME_BUFFER_Transmit(LCD_DMA_TRANSMIT_BLOCKING);
   }

   return pImage;
}
//------------------------------------------------------------------------------
typedef struct image_compare_results_tag
{
   double max_err;
   double mean;
   double mean_squared;
   double root_mean_squared;
   double peak_snr;
} image_compare_results;

//static void get_pixel(int* pDst, const uint8_t *pSrc, int luma_only, int num_comps)
//{
//   int r, g, b;
//   if (num_comps == 1)
//   {
//      r = g = b = pSrc[0];
//   }
//   else if (luma_only)
//   {
//      const int YR = 19595, YG = 38470, YB = 7471;
//      r = g = b = (pSrc[0] * YR + pSrc[1] * YG + pSrc[2] * YB + 32768) / 65536;
//   }
//   else
//   {
//      r = pSrc[0]; g = pSrc[1]; b = pSrc[2];
//   }
//   pDst[0] = r; pDst[1] = g; pDst[2] = b;
//}

//// Compute image error metrics.
//static void image_compare(image_compare_results *pResults, int width, int height, const uint8_t *pComp_image, int comp_image_comps, const uint8_t *pUncomp_image_data, int uncomp_comps, int luma_only)
//{
//   double hist[256];
//   double sum = 0.0f, sum2 = 0.0f;
//   double total_values;
//   const uint first_channel = 0, num_channels = 3;
//   int x, y;
//   uint i;
//
//   memset(hist, 0, sizeof(hist));
//
//   for (y = 0; y < height; y++)
//   {
//      for (x = 0; x < width; x++)
//      {
//         uint c;
//         int a[3];
//         int b[3];
//
//         get_pixel(a, pComp_image + (y * width + x) * comp_image_comps, luma_only, comp_image_comps);
//         get_pixel(b, pUncomp_image_data + (y * width + x) * uncomp_comps, luma_only, uncomp_comps);
//
//         for (c = 0; c < num_channels; c++)
//            hist[labs(a[first_channel + c] - b[first_channel + c])]++;
//      }
//   }
//
//   pResults->max_err = 0;
//
//   for (i = 0; i < 256; i++)
//   {
//      double x;
//      if (!hist[i])
//         continue;
//      if (i > pResults->max_err)
//         pResults->max_err = i;
//      x = i * hist[i];
//      sum += x;
//      sum2 += i * x;
//   }
//
//   // See http://bmrc.berkeley.edu/courseware/cs294/fall97/assignment/psnr.html
//   total_values = width * height;
//
//   pResults->mean = sum / total_values;
//   pResults->mean_squared = sum2 / total_values;
//
//   pResults->root_mean_squared = sqrt(pResults->mean_squared);
//
//   if (!pResults->root_mean_squared)
//      pResults->peak_snr = 1e+10f;
//   else
//      pResults->peak_snr = log10(255.0f / pResults->root_mean_squared) * 20.0f;
//}
//------------------------------------------------------------------------------
int picojpeg(int id, int play_mode)
{
   int width, height, comps;
   pjpeg_scan_type_t scan_type;
   const char* p = "?";
   uint8_t *pImage;
   int reduce = 0;
   MY_FILE *infile;

   debug.printf("\r\nReduce during decoding: %u", reduce);

   infile = my_fopen(id);
   if (!infile)
   {
	   debug.printf("\r\nfopen error - picojpeg");
	   LCDStatusStruct.waitExitKey = 0;
	   return -1;
   }

   g_pInFile = infile;

   pImage = pjpeg_load_from_file(&width, &height, &comps, &scan_type, reduce, play_mode);

   DMA_ProgressBar_Stop();

   if (!pImage)
   {
      debug.printf("\r\nFailed loading source image!");
      LCDStatusStruct.waitExitKey = 0;
      return -1;
   }

   debug.printf("\r\nWidth: %i, Height: %i, Comps: %i", width, height, comps);
   
   switch (scan_type)
   {
      case PJPG_GRAYSCALE: p = "GRAYSCALE"; break;
      case PJPG_YH1V1: p = "H1V1"; break;
      case PJPG_YH2V1: p = "H2V1"; break;
      case PJPG_YH1V2: p = "H1V2"; break;
      case PJPG_YH2V2: p = "H2V2"; break;
   }
   debug.printf("\r\nScan type: %s", p);

   if(play_mode == DJPEG_PLAY){
	   uint32_t timingdelay = HAL_GetTick() + settings_group.filer_conf.photo_frame_td * 1000;
	   while(HAL_GetTick() < timingdelay)
	   {
			switch(LCDStatusStruct.waitExitKey)
			{
				case PLAY_NEXT:
					LCDStatusStruct.waitExitKey = 1;
					return DJPEG_ARROW_RIGHT;
				case PLAY_PREV:
					LCDStatusStruct.waitExitKey = 1;
					return DJPEG_ARROW_LEFT;
				case PLAY_PAUSE:
					goto EXIT_PLAY_MODE;
				default:
					break;

			}
			if(LCDStatusStruct.waitExitKey == 0){
				return EXIT_SUCCESS;
			}
	   }
	   return DJPEG_PLAY;
   }


   uint8_t *ptr;

   uint16_t flashBuff[12288];
   FLASH_EraseInitTypeDef FlashErase;
   uint32_t SectorError;
   uint32_t flash_addr;
   HAL_StatusTypeDef flash_status;

   int i;

EXIT_PLAY_MODE:

   LCDStatusStruct.waitExitKey = 1;
   while(LCDStatusStruct.waitExitKey)
   {
		switch(LCDStatusStruct.waitExitKey)
		{
			case PLAY_NEXT:
				LCDStatusStruct.waitExitKey = 1;
				return DJPEG_ARROW_RIGHT;
			case PLAY_PREV:
				LCDStatusStruct.waitExitKey = 1;
				return DJPEG_ARROW_LEFT;
			case PLAY_PAUSE:
				LCDStatusStruct.waitExitKey = 1;
				return DJPEG_PLAY;
			case PLAY_SW_HOLD_LEFT:
				if(width != LCD_WIDTH || height != LCD_HEIGHT){
					LCDStatusStruct.waitExitKey = 1;
					continue;
				}

				debug.printf("\r\nsaving filer image...");

				ptr = (uint8_t*)music_bgimg_160x128;
				memcpy((void*)flashBuff, (void*)&ptr[0], 16384);

				HAL_FLASH_Unlock();
				__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
				FlashErase.TypeErase = TYPEERASE_SECTORS;
				FlashErase.Sector = FLASH_SECTOR_3;
				FlashErase.NbSectors = 1;
				FlashErase.VoltageRange = VOLTAGE_RANGE_3;
				if(HAL_FLASHEx_Erase(&FlashErase, &SectorError) != HAL_OK){
					debug.printf("\r\nErase Failed:%08x", SectorError);
				}

				flash_addr = (FLASH_BASE + (16 * 3) * 1024);
				for(i = 0;i < (16384 / sizeof(uint16_t));i++){
					flash_status = HAL_FLASH_Program(TYPEPROGRAM_HALFWORD, flash_addr, flashBuff[i]);
					flash_addr += sizeof(uint16_t);
				}

				ptr = (uint8_t*)music_bgimg_160x128;
				memcpy((void*)flashBuff, (void*)&ptr[16384], 24576);


				__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
				FlashErase.TypeErase = TYPEERASE_SECTORS;
				FlashErase.Sector = FLASH_SECTOR_4;
				FlashErase.NbSectors = 1;
				FlashErase.VoltageRange = VOLTAGE_RANGE_3;
				if(HAL_FLASHEx_Erase(&FlashErase, &SectorError) != HAL_OK){
					debug.printf("\r\nErase Failed:%08x", SectorError);
				}

				flash_addr = (FLASH_BASE + (16 * 4) * 1024);
				for(i = 0;i < (24576 / sizeof(uint16_t));i++){
					flash_status = HAL_FLASH_Program(TYPEPROGRAM_HALFWORD, flash_addr, frame_buffer[16384 / sizeof(uint16_t) + i]);
					flash_addr += sizeof(uint16_t);
				}


				flash_addr = (FLASH_BASE + (16 * 4 + 24) * 1024);
				for(i = 0;i < (16384 / sizeof(uint16_t));i++){
					flash_status = HAL_FLASH_Program(TYPEPROGRAM_HALFWORD, flash_addr, filer_bgimg_160x128[i]);
					flash_addr += sizeof(uint16_t);
				}

				flash_addr = (FLASH_BASE + (16 * 4 + 24 + 16) * 1024);
				for(i = 0;i < (24576 / sizeof(uint16_t));i++){
					flash_status = HAL_FLASH_Program(TYPEPROGRAM_HALFWORD, flash_addr, flashBuff[i]);
					flash_addr += sizeof(uint16_t);
				}

				__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
				FlashErase.TypeErase = TYPEERASE_SECTORS;
				FlashErase.Sector = FLASH_SECTOR_3;
				FlashErase.NbSectors = 1;
				FlashErase.VoltageRange = VOLTAGE_RANGE_3;
				if(HAL_FLASHEx_Erase(&FlashErase, &SectorError) != HAL_OK){
					debug.printf("\r\nErase Failed:%08x", SectorError);
				}

				flash_addr = (FLASH_BASE + (16 * 3) * 1024);
				for(i = 0;i < (16384 / sizeof(uint16_t));i++){
					flash_status = HAL_FLASH_Program(TYPEPROGRAM_HALFWORD, flash_addr, frame_buffer[i]);
					flash_addr += sizeof(uint16_t);
				}

				debug.printf("\r\ncomplete");

				memcpy((void*)frame_buffer, (void*)filer_bgimg_160x128, FRAME_BUFFER_SIZE);

				LCDXor();
				HAL_Delay(150);
				LCDXor();
				HAL_Delay(150);
				LCDXor();
				HAL_Delay(150);
				LCDXor();
				HAL_Delay(150);
				LCDXor();
				HAL_Delay(150);
				LCDXor();

				HAL_FLASH_Lock();

				LCDStatusStruct.waitExitKey = 1;
				continue;
			case PLAY_SW_HOLD_RIGHT:
				if(width != LCD_WIDTH || height != LCD_HEIGHT){
					LCDStatusStruct.waitExitKey = 1;
					continue;
				}

				debug.printf("\r\nsaving music image...");

				ptr = (uint8_t*)filer_bgimg_160x128;
				memcpy((void*)flashBuff, (void*)&ptr[16384], 24576);

				HAL_FLASH_Unlock();
				__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
				FlashErase.TypeErase = TYPEERASE_SECTORS;
				FlashErase.Sector = FLASH_SECTOR_4;
				FlashErase.NbSectors = 1;
				FlashErase.VoltageRange = VOLTAGE_RANGE_3;

				if(HAL_FLASHEx_Erase(&FlashErase, &SectorError) != HAL_OK){
					debug.printf("\r\nErase Failed:%08x", SectorError);
				}

				flash_addr = (FLASH_BASE + (16 * 4) * 1024);
				for(i = 0;i < (24576 / sizeof(uint16_t));i++){
					flash_status = HAL_FLASH_Program(TYPEPROGRAM_HALFWORD, flash_addr, flashBuff[i]);
					flash_addr += sizeof(uint16_t);
				}
				flash_addr = (FLASH_BASE + (16 * 4 + 24) * 1024);
				for(i = 0;i < (FRAME_BUFFER_SIZE / sizeof(uint16_t));i++){
					flash_status = HAL_FLASH_Program(TYPEPROGRAM_HALFWORD, flash_addr, frame_buffer[i]);
					flash_addr += sizeof(uint16_t);
				}

				debug.printf("\r\ncomplete");

				memcpy((void*)frame_buffer, (void*)music_bgimg_160x128, FRAME_BUFFER_SIZE);

				LCDXor();
				HAL_Delay(150);
				LCDXor();
				HAL_Delay(150);
				LCDXor();
				HAL_Delay(150);
				LCDXor();
				HAL_Delay(150);
				LCDXor();
				HAL_Delay(150);
				LCDXor();

				HAL_FLASH_Lock();

				LCDStatusStruct.waitExitKey = 1;
				continue;
			default:
				break;

		}
		HAL_Delay(10);
   }

   return EXIT_SUCCESS;
}
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Loads JPEG image from specified file. Returns NULL on failure.
// On success, the malloc()'d image's width/height is written to *x and *y, and
// the number of components (1 or 3) is written to *comps.
// pScan_type can be NULL, if not it'll be set to the image's pjpeg_scan_type_t.
// Not thread safe.
// If reduce is non-zero, the image will be more quickly decoded at approximately
// 1/8 resolution (the actual returned resolution will depend on the JPEG
// subsampling factor).
uint8_t *pjpeg_load_from_file2(uint8_t *x, uint8_t *y, int *comps, pjpeg_scan_type_t *pScan_type, int reduce)
{
   pjpeg_image_info_t image_info;
   int mcu_x = 0;
   int mcu_y = 0;
   uint row_pitch;
   uint8_t *pImage;
   uint8_t status;
   uint decoded_width, decoded_height;
   uint row_blocks_per_mcu, col_blocks_per_mcu;
   uint orgSeekBytes;
   extern uint16_t frame_buffer[160 * 128];

   *x = 0;
   *y = 0;
   *comps = 0;
   if (pScan_type) *pScan_type = PJPG_GRAYSCALE;

   orgSeekBytes = g_pInFile->seekBytes;
   g_pInFileOfs = 0;

   g_pInFileSize = g_pInFile->fileSize;

   status = pjpeg_decode_init(&image_info, pjpeg_need_bytes_callback, NULL, (unsigned char)reduce);

   if (status)
   {
      debug.printf("\r\npjpeg_decode_init() failed with status %u", status);
      if (status == PJPG_UNSUPPORTED_MODE)
      {
         debug.printf("\r\nProgressive JPEG files are not supported.");
      }

      return NULL;
   }

   if (pScan_type)
      *pScan_type = image_info.m_scanType;


   // In reduce mode output 1 pixel per 8x8 block.
   decoded_width = reduce ? (image_info.m_MCUSPerRow * image_info.m_MCUWidth) / 8 : image_info.m_width;
   decoded_height = reduce ? (image_info.m_MCUSPerCol * image_info.m_MCUHeight) / 8 : image_info.m_height;


   if((decoded_width > LCD_WIDTH) || (decoded_height > LCD_HEIGHT))
   {
	   reduce = 1;

	   g_pInFileOfs = 0;

	   g_pInFileSize = g_pInFile->fileSize;
	   my_fseek(g_pInFile, orgSeekBytes, SEEK_SET);

	   status = pjpeg_decode_init(&image_info, pjpeg_need_bytes_callback, NULL, (unsigned char)reduce);

	   if (status)
	   {
	      debug.printf("\r\npjpeg_decode_init() failed with status %u", status);
	      if (status == PJPG_UNSUPPORTED_MODE)
	      {
	         debug.printf("\r\nProgressive JPEG files are not supported.");
	      }

	      return NULL;
	   }

	   if (pScan_type)
	      *pScan_type = image_info.m_scanType;

	   // In reduce mode output 1 pixel per 8x8 block.
	   decoded_width = reduce ? (image_info.m_MCUSPerRow * image_info.m_MCUWidth) / 8 : image_info.m_width;
	   decoded_height = reduce ? (image_info.m_MCUSPerCol * image_info.m_MCUHeight) / 8 : image_info.m_height;

	   if((decoded_width > LCD_WIDTH) || (decoded_height > LCD_HEIGHT))
	   {
		   return NULL;
	   }
   }

//   row_pitch = decoded_width * image_info.m_comps;
   row_pitch = decoded_width * 2;

   pImage = (uint8_t *)frame_buffer;
   if (!pImage)
   {
      return NULL;
   }

   row_blocks_per_mcu = image_info.m_MCUWidth >> 3;
   col_blocks_per_mcu = image_info.m_MCUHeight >> 3;

   for ( ; ; )
   {
      int y, x;
      uint8_t *pDst_row;

      status = pjpeg_decode_mcu();

//      debug.printf("\r\nmcu_x:%d mcu_y:%d", mcu_x, mcu_y);

      if (status)
      {
         if (status != PJPG_NO_MORE_BLOCKS)
         {
            debug.printf("\r\npjpeg_decode_mcu() failed with status %u", status);

            my_fclose(g_pInFile);
            return NULL;
         }

         break;
      }

      if (mcu_y >= image_info.m_MCUSPerCol)
      {
         return NULL;
      }

      if (reduce)
      {
         // In reduce mode, only the first pixel of each 8x8 block is valid.
//         pDst_row = pImage + mcu_y * col_blocks_per_mcu * row_pitch + mcu_x * row_blocks_per_mcu * image_info.m_comps;
          pDst_row = pImage + mcu_y * col_blocks_per_mcu * row_pitch + mcu_x * row_blocks_per_mcu * 2;
         if (image_info.m_scanType == PJPG_GRAYSCALE)
         {
            *pDst_row = image_info.m_pMCUBufR[0];
         }
         else
         {
            uint y, x;
            for (y = 0; y < col_blocks_per_mcu; y++)
            {
               uint src_ofs = (y * 128U);
               for (x = 0; x < row_blocks_per_mcu; x++)
               {
//                  pDst_row[0] = image_info.m_pMCUBufR[src_ofs];
//                  pDst_row[1] = image_info.m_pMCUBufG[src_ofs];
//                  pDst_row[2] = image_info.m_pMCUBufB[src_ofs];
//                  pDst_row += 3;
            	  *(uint16_t*)pDst_row = ( (image_info.m_pMCUBufR[src_ofs] >> 3) << 11 ) | ( (image_info.m_pMCUBufG[src_ofs]  >> 2) << 5 ) | (image_info.m_pMCUBufB[src_ofs] >> 3);

            	  pDst_row += 2;
                  src_ofs += 64;
               }

//               pDst_row += row_pitch - 3 * row_blocks_per_mcu;
               pDst_row += row_pitch - 2 * row_blocks_per_mcu;
            }
         }
      }
      else
      {
         // Copy MCU's pixel blocks into the destination bitmap.
//         pDst_row = pImage + (mcu_y * image_info.m_MCUHeight) * row_pitch + (mcu_x * image_info.m_MCUWidth * image_info.m_comps);
          pDst_row = pImage + (mcu_y * image_info.m_MCUHeight) * row_pitch + (mcu_x * image_info.m_MCUWidth * 2);

         for (y = 0; y < image_info.m_MCUHeight; y += 8)
         {
            const int by_limit = min(8, image_info.m_height - (mcu_y * image_info.m_MCUHeight + y));

            for (x = 0; x < image_info.m_MCUWidth; x += 8)
            {
//               uint8_t *pDst_block = pDst_row + x * image_info.m_comps;
                uint8_t *pDst_block = pDst_row + x * 2;

               // Compute source byte offset of the block in the decoder's MCU buffer.
               uint src_ofs = (x * 8U) + (y * 16U);
               const uint8_t *pSrcR = image_info.m_pMCUBufR + src_ofs;
               const uint8_t *pSrcG = image_info.m_pMCUBufG + src_ofs;
               const uint8_t *pSrcB = image_info.m_pMCUBufB + src_ofs;

               const int bx_limit = min(8, image_info.m_width - (mcu_x * image_info.m_MCUWidth + x));

               if (image_info.m_scanType == PJPG_GRAYSCALE)
               {
                  int bx, by;
                  for (by = 0; by < by_limit; by++)
                  {
                     uint8_t *pDst = pDst_block;

                     for (bx = 0; bx < bx_limit; bx++)
                        *pDst++ = *pSrcR++;

                     pSrcR += (8 - bx_limit);

                     pDst_block += row_pitch;
                  }
               }
               else
               {
                  int bx, by;
                  for (by = 0; by < by_limit; by++)
                  {
                     uint8_t *pDst = pDst_block;

                     for (bx = 0; bx < bx_limit; bx++)
                     {
                    	*(uint16_t*)pDst = ( (*pSrcR++ >> 3) << 11 ) | ( (*pSrcG++  >> 2) << 5 ) | (*pSrcB++ >> 3);
                    	pDst += 2;
                     }

                     pSrcR += (8 - bx_limit);
                     pSrcG += (8 - bx_limit);
                     pSrcB += (8 - bx_limit);

                     pDst_block += row_pitch;
                  }
               }
            }

            pDst_row += (row_pitch * 8);
         }
      }

      mcu_x++;
      if (mcu_x == image_info.m_MCUSPerRow)
      {
         mcu_x = 0;
         mcu_y++;
      }
   }

   debug.printf("\r\ndecoded_width:%d ndecoded_height:%d", decoded_width, decoded_height);

   uint max_span;
   uint16_t zoom;
   max_span = max(decoded_width, decoded_height);
   zoom = (ART_WORK_SIZE * 100) / max_span;

   *x = decoded_width;
   *y = decoded_height;

   if(max_span == decoded_width){
	   *y *= (float)ART_WORK_SIZE / (float)(decoded_width);
	   *x *= (float)ART_WORK_SIZE / (float)(decoded_width);
   } else {
	   *x *= (float)ART_WORK_SIZE / (float)(decoded_height);
	   *y *= (float)ART_WORK_SIZE / (float)(decoded_height);
   }

   extern uint16_t cursorRAM[LCD_WIDTH * 13];
   LCDZoomImageToBuff(*x, *y, cursorRAM, decoded_width, decoded_height, frame_buffer, colorc[BLACK], zoom, 1);


   *comps = image_info.m_comps;

   return pImage;
}
