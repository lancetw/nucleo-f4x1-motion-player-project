/*
 * sd.h
 *
 *  Created on: 2014/05/06
 *      Author: masayuki
 */

#ifndef SD_H_
#define SD_H_

#include "stm32f4xx_hal.h"

#define	SDBlockRead(buf, addr) do{__disable_irq();\
								  HAL_SD_ReadBlocks(&hsd, (uint32_t*)buf, (uint64_t)(addr) << 9, 512, 1);\
								  __enable_irq();}while(0)
#define SDMultiBlockRead(buf, addr, block_num) do{__disable_irq();\
												  HAL_SD_ReadBlocks(&hsd, (uint32_t*)buf, (uint64_t)(addr) << 9, 512, block_num);\
												  __enable_irq();}while(0)

//#define	SDBlockRead(buf, addr) do{HAL_SD_ReadBlocks_DMA(&hsd, (uint32_t*)buf, (uint64_t)(addr) << 9, 512, 1);\
//								  HAL_SD_CheckReadOperation(&hsd, 500000);}while(0)
//
//#define SDMultiBlockRead(buf, addr, block_num) do{HAL_SD_ReadBlocks_DMA(&hsd, (uint32_t*)buf, (uint64_t)(addr) << 9, 512, block_num);\
//												  HAL_SD_CheckReadOperation(&hsd, 500000);}while(0)
//

#endif /* SD_H_ */
