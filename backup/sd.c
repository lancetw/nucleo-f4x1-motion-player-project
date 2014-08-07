/*
 * sd.c
 *
 *  Created on: 2014/05/06
 *      Author: masayuki
 */

#include "sd.h"

SD_HandleTypeDef hsd;
HAL_SD_CardInfoTypedef SDCardInfo;


HAL_SD_ErrorTypedef SD_Init()
{
	hsd.Instance = SDIO;
	return HAL_SD_Init(&hsd, &SDCardInfo);
}

