
#include "stm32f4xx_hal_conf.h"
#include "HMC5883L.h"
#include "main.h"
#include "lcd.h"
#include "icon.h"
#include "pcf_font.h"
#include "usart.h"
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

extern I2C_HandleTypeDef I2cHandle;
extern uint16_t frame_buffer[160 * 128];

typedef struct {
	  int16_t XAxis;
	  int16_t ZAxis;
	  int16_t YAxis;
}compass_axis_typedef;

void HMC5883L_Calib(int16_t *calX, int16_t *calY)
{
	compass_axis_typedef raw;
	uint8_t pData[13];

	pcf_typedef pcf;
	pcf.dst_gram_addr = (uint32_t)frame_buffer;
	pcf.pixelFormat = PCF_PIXEL_FORMAT_RGB565;
	pcf.size = 12;
	pcf.color = WHITE;
	pcf.colorShadow = BLACK;
	pcf.alphaSoftBlending = 0;
	pcf.enableShadow = 0;
	pcf_font.metrics.hSpacing = 2;

	memset((void*)frame_buffer, 0, FRAME_BUFFER_SIZE);
	LCDPutIcon(3, 0, 22, 22, compass_22x22, compass_22x22_alpha);
	LCD_GotoXY(28, 5);
	LCDPutString("Compass Calibration\n", &pcf);

	pcf.color = GREEN;
	LCDPutString("1/2 Place me on a flat\ndesk.\nthen push right button.\n", &pcf);

	LCD_FRAME_BUFFER_Transmit(LCD_DMA_TRANSMIT_COMPBLOCKING);

	while(LCDStatusStruct.waitExitKey != PLAY_NEXT){
		HAL_Delay(10);
	}
	LCDStatusStruct.waitExitKey = 1;

	pcf.color = YELLOW;
	LCDPutString("2/2 Rotate me 360degree\nthen push right button\nto complete.\n", &pcf);
	LCD_FRAME_BUFFER_Transmit(LCD_DMA_TRANSMIT_COMPBLOCKING);

	HAL_StatusTypeDef errorState;
	int16_t minX = 1000, maxX = -1000;
	int16_t minY = 1000, maxY = -1000;

	while(LCDStatusStruct.waitExitKey != PLAY_NEXT){
		if((errorState = HAL_I2C_Mem_Read(&I2cHandle, I2C_ADDRESS_HMC5883L, 0x3, I2C_MEMADD_SIZE_8BIT, pData, 6, 10000)) != HAL_OK)
		{
			debug.printf("\r\nI2C READ ERROR:%d", errorState);
		}

		raw.XAxis = (int16_t)((pData[0] << 8) | pData[1]);
		raw.ZAxis = (int16_t)((pData[2] << 8) | pData[3]);
		raw.YAxis = (int16_t)((pData[4] << 8) | pData[5]);

		if(raw.XAxis > maxX){
			maxX = raw.XAxis;
		}
		if(raw.XAxis < minX){
			minX = raw.XAxis;
		}

		if(raw.YAxis > maxY){
			maxY = raw.YAxis;
		}
		if(raw.YAxis < minY){
			minY = raw.YAxis;
		}

		 debug.printf("\r\n%d,%d", raw.XAxis, raw.YAxis);
	}
	LCDStatusStruct.waitExitKey = 1;

	debug.printf("\r\nminX:%d minY:%d", minX, minY);
	debug.printf("\r\nmaxX:%d maxY:%d", maxX, maxY);

	*calX = ((abs(maxX) + abs(minX)) / 2) + minX;
	*calY = ((abs(maxY) + abs(minY)) / 2) + minY;

	debug.printf("\r\ncalX:%d calY:%d", *calX, *calY);

	uint8_t flashBuff[16384];
	uint16_t *p16 = (uint16_t*)flashBuff;
	FLASH_EraseInitTypeDef FlashErase;
	uint32_t SectorError;
	uint32_t flash_addr;

	int i;

	__disable_irq();

	memcpy((void*)flashBuff, (void*)FLASH_BASE, 16384);
	memcpy((void*)&flashBuff[636], (void*)calX, sizeof(calX));
	memcpy((void*)&flashBuff[638], (void*)calY, sizeof(calY));

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

	pcf.color = WHITE;
	LCDPutString("Calibration complete", &pcf);
	__enable_irq();

	LCD_FRAME_BUFFER_Transmit(LCD_DMA_TRANSMIT_COMPBLOCKING);

	HAL_Delay(1000);
}

void HMC5883L()
{
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
	  pcf.size = 16;
	  pcf.color = WHITE;
	  pcf.colorShadow = BLACK;
	  pcf.alphaSoftBlending = 0;
	  pcf.enableShadow = 0;
	  pcf_font.metrics.hSpacing = 2;

	  HAL_StatusTypeDef errorState;

	  uint8_t pData[13];

	  memset((void*)pData, '\0', sizeof(pData));

	  // 8sample avg 30Hz
	  pData[0] = 0x74;
	  if((errorState = HAL_I2C_Mem_Write(&I2cHandle, I2C_ADDRESS_HMC5883L, 0x0, I2C_MEMADD_SIZE_8BIT, pData, 1, 10000)) != HAL_OK)
	  {
		  debug.printf("\r\nI2C WRITE ERROR:%d", errorState);
	  }

	  // continuas
	  pData[0] = 0x00;
	  if((errorState = HAL_I2C_Mem_Write(&I2cHandle, I2C_ADDRESS_HMC5883L, 0x2, I2C_MEMADD_SIZE_8BIT, pData, 1, 10000)) != HAL_OK)
	  {
		  debug.printf("\r\nI2C WRITE ERROR:%d", errorState);
	  }

	  int i;
	  for(i = 0;i <= 12;i++){
		  if((errorState = HAL_I2C_Mem_Read(&I2cHandle, I2C_ADDRESS_HMC5883L, i, I2C_MEMADD_SIZE_8BIT, pData, 1, 10000)) != HAL_OK)
		  {
			  debug.printf("\r\nI2C READ ERROR:%d", errorState);
		  }

		  debug.printf("\r\npData[%d]:%02x", i, pData[0]);
	  }

	  compass_axis_typedef raw, scaled;
	  float m_Scale = 0.92f, heading, declinationAngle, headingDegrees;

	  debug.printf("\r\n\n");

	  char headingStr[5];

	  LCDStatusStruct.waitExitKey = 1;

	  int16_t calX = 0, calY = 0;
	  uint8_t *flash = (uint8_t*)FLASH_BASE;

	  memcpy((void*)&calX, (void*)&flash[636], sizeof(calX));
	  memcpy((void*)&calY, (void*)&flash[638], sizeof(calY));

	  debug.printf("\r\ncalX:%d calY:%d", calX, calY);

	  while(LCDStatusStruct.waitExitKey){
		  if(LCDStatusStruct.waitExitKey == PLAY_SW_HOLD_RIGHT){
			  LCDStatusStruct.waitExitKey = 1;
			  HMC5883L_Calib(&calX, &calY);
		  }

		  if((errorState = HAL_I2C_Mem_Read(&I2cHandle, I2C_ADDRESS_HMC5883L, 0x3, I2C_MEMADD_SIZE_8BIT, pData, 6, 10000)) != HAL_OK)
		  {
			  debug.printf("\r\nI2C READ ERROR:%d", errorState);
		  }

		  raw.XAxis = (int16_t)((pData[0] << 8) | pData[1]) - calX;
		  raw.ZAxis = (int16_t)((pData[2] << 8) | pData[3]);
		  raw.YAxis = (int16_t)((pData[4] << 8) | pData[5]) - calY;

		  scaled.XAxis = raw.XAxis * m_Scale;
		  scaled.ZAxis = raw.ZAxis * m_Scale;
		  scaled.YAxis = raw.YAxis * m_Scale;

		  // Calculate heading when the magnetometer is level, then correct for signs of axis.
		  heading = atan2f(scaled.YAxis, scaled.XAxis) - M_PI / 2;

		  // Once you have your heading, you must then add your 'Declination Angle', which is the 'Error' of the magnetic field in your location.
		  // Find yours here: http://www.magnetic-declination.com/
		  // Mine is: 2&#65533; 37' W, which is 2.617 Degrees, or (which we need) 0.0456752665 radians, I will use 0.0457
		  // If you cannot find your Declination, comment out these two lines, your compass will be slightly off.
		  //  float declinationAngle = 0.0457;

		  //  弊店所在地における磁気偏角は 、7°18 '(西偏) =127.4 mrad
		  //  磁気偏角については http://magnetic-declination.com/とhttp://vldb.gsi.go.jp/sokuchi/geomag/menu_01/index.htmlを参照してください。
		  declinationAngle = 0;//0.12682f;

		  //  磁気偏角が東偏の場合 -= declinationAngle、西偏の場合 += declinationAngleを使ってください。
		  heading += declinationAngle;

		  // Correct for when signs are reversed.
		  if(heading < 0)
		    heading += 2 * M_PI;

		  // Check for wrap due to addition of declination.
		  if(heading > 2 * M_PI)
		    heading -= 2 * M_PI;

		  // Convert radians to degrees for readability.
		  headingDegrees = heading * 180.0f / M_PI;

	//	  debug.printf("\r\nHDG:%03d", (int)headingDegrees);

	//	  debug.printf("\r\nraw.XAxis:%d", raw.XAxis);
	//	  debug.printf("\r\nraw.ZAxis:%d", raw.ZAxis);
	//	  debug.printf("\r\nraw.YAxis:%d", raw.YAxis);
	//	  debug.printf("\r\nF:%d", sqrtf(raw.XAxis ^ 2 + raw.ZAxis ^ 2 + raw.YAxis ^ 2));

	//	 debug.printf("\r\n%d,%d", raw.XAxis, raw.YAxis);


		  LCDRotateImage(79, 63, 128, 128, compass_128x128, colorc[BLACK], (int)headingDegrees, 1);
		  LCD_DrawSquare(51, 63, 58, 1, WHITE);
		  LCD_DrawSquare(80, 35, 1, 58, WHITE);

//		  LCDPutIcon(51, 35, 58, 58, compass_cross_58x58, compass_cross_58x58_alpha);

		  sprintf(headingStr, "%03d", (int)headingDegrees);
		  if(headingStr[0] == '0'){
			  headingStr[0] = ' ';
			  if(headingStr[1] == '0'){
				  headingStr[1] = ' ';
			  }
		  }
		  LCD_GotoXY(124, 110);
		  LCDPutString(headingStr, &pcf);

		  headingStr[0] = 0xB0;
		  headingStr[1] = '\0';
		  LCD_GotoXY(124 + 28, 110);
		  LCDPutString(headingStr, &pcf);


		  LCD_FRAME_BUFFER_Transmit(LCD_DMA_TRANSMIT_NOBLOCKING);
	  }

	  if(pcf_font.ext_loaded)
	  {
		  memcpy((void*)&pcf_font, (void*)&pcf_font_bak, sizeof(pcf_font_typedef));
		  PCF_RENDER_FUNC_PCF();
	  }
}
