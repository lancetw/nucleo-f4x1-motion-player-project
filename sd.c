/*
 * sd.c
 *
 *  Created on: 2011/02/22
 *      Author: masayuki
 */

#include "sd.h"
//#include "lcd.h"
#include "main.h"
#include "usart.h"
#include "fat.h"
#include <stdio.h>
#include <string.h>

void SDIO_ClearFlag(uint32_t SDIO_FLAG)
{
  SDIO->ICR = SDIO_FLAG;
}

void SDIO_ClockCmd(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  *(__IO uint32_t *) CLKCR_CLKEN_BB = (uint32_t)NewState;
}


#define SD_DEBUG
//#undef SD_DEBUG

//#define SDIO_DMA_ENABLE 1

volatile card_info_typedef cardInfo;

SD_HandleTypeDef hsd;
HAL_SD_CardInfoTypedef SDCardInfo;


/*
inline void SD_LowLevel_DMA_RxConfig(uint32_t *BufferDST)
{
  DMA_InitTypeDef DMA_InitStructure;

  DMA_ClearFlag(DMA2_Stream3, DMA_FLAG_FEIF3 | DMA_FLAG_DMEIF3 | DMA_FLAG_TEIF3 | DMA_FLAG_HTIF3 | DMA_FLAG_TCIF3);

  // !< DMA2 Channel4 disable
  DMA_Cmd(DMA2_Stream3, DISABLE);

  DMA_DeInit(DMA2_Stream3);

  // !< DMA2 Channel4 Config
  DMA_InitStructure.DMA_Channel = DMA_Channel_4;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&SDIO->FIFO;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)BufferDST;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = 0;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_INC4;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_INC4;
  DMA_Init(DMA2_Stream3, &DMA_InitStructure);

  DMA_FlowControllerConfig(DMA2_Stream3, DMA_FlowCtrl_Peripheral);

  // !< DMA2 Channel4 enable
  DMA_Cmd(DMA2_Stream3, ENABLE);
}
*/

uint32_t SDSetDPSM(uint32_t dLen, uint32_t dBlkSize, uint32_t trDir, uint32_t trMode, uint32_t timeout, void *buf)
{
	uint32_t ret, sta, *pbuf = (uint32_t*)buf;
	SDIO_DataInitTypeDef SDIO_DataInitStructure;

	// DPSM DPSM WAIT_R
	SDIO_DataInitStructure.DataTimeOut = timeout;
	SDIO_DataInitStructure.DataLength = dLen;
	SDIO_DataInitStructure.DataBlockSize = dBlkSize;
	SDIO_DataInitStructure.TransferDir = trDir;
	SDIO_DataInitStructure.TransferMode = trMode;
	SDIO_DataInitStructure.DPSM = SDIO_DPSM_ENABLE;
	SDIO_DataConfig(hsd.Instance, &SDIO_DataInitStructure);

	do{
		while(STA_RXDAVL_BB_FLAG){
			*pbuf++ = SDIO->FIFO;
		}
	}while(!((sta = SDIO->STA) & SDIO_BLOCK_READ_STATUS_MASK));

#ifdef SD_DEBUG
	if(!(sta & SDIO_FLAG_DATAEND)){
		if(sta & SDIO_FLAG_RXOVERR){
			USARTPutString("\r\n*RXOVERR");
			ret = SDIO_FLAG_RXOVERR;
		}
		if(sta & SDIO_FLAG_STBITERR){
			USARTPutString("\r\n*STBITERR");
			ret = SDIO_FLAG_STBITERR;
		}
		if(sta & SDIO_FLAG_DATAEND){
			USARTPutString("\r\n*DATAEND");
			ret = SDIO_FLAG_DATAEND;
		}
		if(sta & SDIO_FLAG_DTIMEOUT){
			USARTPutString("\r\n*DTIMEOUT");
			ret = SDIO_FLAG_DTIMEOUT;
		}
		if(sta & SDIO_FLAG_DCRCFAIL){
			USARTPutString("\r\n*DCRCFAIL");
			ret = SDIO_FLAG_DCRCFAIL;
		}
	}
#endif

	SDIO_ClearFlag(SDIO_FLAG_STBITERR | SDIO_FLAG_DBCKEND | SDIO_FLAG_DATAEND | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_DCRCFAIL);

	return ret;
}


uint32_t SDSendCMD(int cmdIdx, uint32_t arg, uint32_t resType, uint32_t *resbuf)
{
	uint32_t sta;
	uint32_t ret = resType ? SDIO_FLAG_CMDREND : SDIO_FLAG_CMDSENT;
	SDIO_CmdInitTypeDef SDIO_CmdInitStructure;

	SDIO_CmdInitStructure.Argument = arg;
	SDIO_CmdInitStructure.CmdIndex = cmdIdx;
	SDIO_CmdInitStructure.Response = resType;
	SDIO_CmdInitStructure.WaitForInterrupt = SDIO_WAIT_NO;
	SDIO_CmdInitStructure.CPSM = SDIO_CPSM_ENABLE;
	SDIO_SendCommand(hsd.Instance, &SDIO_CmdInitStructure);

	do{
		sta = SDIO->STA;
		if(sta & SDIO_FLAG_CTIMEOUT){
			ret = SDIO_FLAG_CTIMEOUT;
			USARTPutString("\r\n*TIMEOUT");
			break;
		}
		if(sta & SDIO_FLAG_CCRCFAIL){
			ret = SDIO_FLAG_CCRCFAIL;
			USARTPutString("\r\n*CRCFAIL");
			break;
		}
	}while(!(sta & ret));

	if(resType != SDIO_RESPONSE_NO){
		*(resbuf + 0) = SDIO_GetResponse(SDIO_RESP1);
		*(resbuf + 1) = SDIO_GetResponse(SDIO_RESP2);
		*(resbuf + 2) = SDIO_GetResponse(SDIO_RESP3);
		*(resbuf + 3)   = SDIO_GetResponse(SDIO_RESP4);
	}

	SDIO_ClearFlag(SDIO_FLAG_CMDSENT | SDIO_FLAG_CMDREND | SDIO_FLAG_CTIMEOUT | SDIO_FLAG_CCRCFAIL);

	return ret;
}

/*
void DMA2_Stream6_IRQHandler(void){
	USARTPutString("\r\nDMA2_Stream6_IRQ");
}
*/

/*
void SDIO_IRQHandler(void){
	USARTPutString("\r\nSDIO_IRQ");
}
*/

const IWDG_HandleTypeDef iwdg = { \
	.Instance = IWDG
};


inline uint32_t SDBlockRead(void *buf, uint32_t blockAddress)
{
	__disable_irq();

	if(!cardInfo.ready)
	{
		__HAL_IWDG_START(&iwdg);
		while(1);
		return 0;
	}

	uint32_t sta, timeout = 500000, *pbuf = (uint32_t*)buf;

	// CMD17 Single Block Read
	SDIO->ICR = 0xffffffff;
	SDIO->ARG = cardInfo.csdVer ? blockAddress : (blockAddress << 9);
	SDIO->CMD = SDIO_CPSM_ENABLE   | SDIO_WAIT_NO | \
				SDIO_RESPONSE_SHORT | 17;
	while(!((sta = SDIO->STA) & SDIO_CMD_STATUS_MASK));


#ifdef SD_DEBUG
	if(!(sta & SDIO_FLAG_CMDREND)){
		USARTPutString("\r\nCMD17:");
		if(sta & SDIO_FLAG_CTIMEOUT){
			USARTPutString("\r\nCTIMEOUT");
		}
		if(sta & SDIO_FLAG_CCRCFAIL){
			USARTPutString("\r\nCCRCFAIL");
		}
		if(sta & SDIO_FLAG_STBITERR){
			USARTPutString("\r\nSTBITERR");
		}
	}
#endif

	if(cardInfo.mmc){
		volatile uint32_t delay = 500;
		while(--delay){
			__NOP();
		};
	}

#ifdef SDIO_DMA_ENABLE
	SD_LowLevel_DMA_RxConfig((uint32_t *)buf);

	SDIO->ICR = 0xffffffff;
	SDIO->DTIMER = 1000000;
	SDIO->DLEN = 512;
	SDIO->DCTRL = SDIO_DATABLOCK_SIZE_512B | SDIO_TRANSFER_DIR_TO_SDIO | \
			      SDIO_TRANSFER_MODE_BLOCK | SDIO_DCTRL_DTEN | SDIO_DCTRL_DMAEN;

	while(!(DMA2->LISR & 0x08000000) && timeout--);
	sta = SDIO->STA;
#else
	SDIO->ICR = 0xffffffff;
	SDIO->DTIMER = 0xffffffff;
	SDIO->DLEN = 512;
	SDIO->DCTRL = SDIO_DATABLOCK_SIZE_512B | SDIO_TRANSFER_DIR_TO_SDIO | \
			      SDIO_TRANSFER_MODE_BLOCK | SDIO_DCTRL_DTEN;

	do{
		while(STA_RXFIFOHF_BB_FLAG){
			pbuf[0] = SDIO->FIFO;
			pbuf[1] = SDIO->FIFO;
			pbuf[2] = SDIO->FIFO;
			pbuf[3] = SDIO->FIFO;
			pbuf[4] = SDIO->FIFO;
			pbuf[5] = SDIO->FIFO;
			pbuf[6] = SDIO->FIFO;
			pbuf[7] = SDIO->FIFO;
			pbuf += 8;
		}
	}while(!((sta = SDIO->STA) & SDIO_BLOCK_READ_STATUS_MASK) && timeout--);

#endif
	if(!(sta & (SDIO_FLAG_DATAEND | SDIO_FLAG_DBCKEND))){
#ifdef SD_DEBUG
		USARTPutString("\r\nBLOCK*");
		if(sta & SDIO_FLAG_DCRCFAIL){
			USARTPutString("DCRCFAIL");
		}
		if(sta & SDIO_FLAG_DTIMEOUT){
			USARTPutString("DTIMEOUT");
		}
		if(timeout <= 0){
			USARTPutString("STIMEOUT");
		}
		if(sta & SDIO_FLAG_RXOVERR){
			USARTPutString("RXOVERR");
		}
		if(sta & SDIO_FLAG_STBITERR){
			USARTPutString("STBITERR");
		}
#endif
		// CMD12 Stop Multi Block Read
		SDIO->ICR = 0xffffffff;
		SDIO->ARG = 0;
		SDIO->CMD = SDIO_CPSM_ENABLE    | SDIO_WAIT_NO | \
					SDIO_RESPONSE_SHORT | 12;
		while(!((sta = SDIO->STA) & SDIO_CMD_STATUS_MASK));

#ifdef SD_DEBUG
		if(!(sta & SDIO_FLAG_CMDREND)){
			USARTPutString("\r\nCMD12:");
			if(sta & SDIO_FLAG_CTIMEOUT){
				USARTPutString("CTIMEOUT");
			}
			if(sta & SDIO_FLAG_CCRCFAIL){
				USARTPutString("CCRCFAIL");
			}
		}
#endif
	}
	__enable_irq();
	return 0;
}

inline uint32_t SDMultiBlockRead(void *buf, uint32_t blockAddress, uint32_t count)
{
	__disable_irq();

	if(!cardInfo.ready)
	{
		__HAL_IWDG_START(&iwdg);
		while(1);
		return 0;
	}


	uint32_t sta, timeout = 100000, *pbuf = (uint32_t*)buf;

	// CMD18 Multi Block Read
	SDIO->ICR = 0xffffffff;
	SDIO->ARG = cardInfo.csdVer ? blockAddress : (blockAddress << 9);
	SDIO->CMD = SDIO_CPSM_ENABLE    | SDIO_WAIT_NO | \
				SDIO_RESPONSE_SHORT | 18;
	while(!((sta = SDIO->STA) & SDIO_CMD_STATUS_MASK));

#ifdef SD_DEBUG
	if(!(sta & SDIO_FLAG_CMDREND)){
		USARTPutString("\r\nCMD18:");
		if(sta & SDIO_FLAG_CTIMEOUT){
			USARTPutString("CTIMEOUT");
		}
		if(sta & SDIO_FLAG_CCRCFAIL){
			USARTPutString("CCRCFAIL");
		}
	}
#endif

#ifdef SDIO_DMA_ENABLE
	SD_LowLevel_DMA_RxConfig((uint32_t *)buf);

	SDIO->ICR = 0xffffffff;
	SDIO->DTIMER = 1000000;
	SDIO->DLEN = count << 9;
	SDIO->DCTRL = SDIO_DATABLOCK_SIZE_512B | SDIO_TRANSFER_DIR_TO_SDIO | \
			      SDIO_TRANSFER_MODE_BLOCK | SDIO_DCTRL_DTEN | SDIO_DCTRL_DMAEN;

	while(!(DMA2->LISR & 0x08000000) && timeout--);
	sta = SDIO->STA;
#else

	if(cardInfo.mmc){
		volatile uint32_t delay = 500;
		while(--delay){
			__NOP();
		};
	}

	SDIO->DTIMER = 1000000;
	SDIO->DLEN = count << 9;
	SDIO->ICR = 0xffffffff;
	SDIO->DCTRL = SDIO_DATABLOCK_SIZE_512B | SDIO_TRANSFER_DIR_TO_SDIO | \
			      SDIO_TRANSFER_MODE_BLOCK | SDIO_DCTRL_DTEN;

	do{
		while(STA_RXFIFOHF_BB_FLAG){
			pbuf[0] = SDIO->FIFO;
			pbuf[1] = SDIO->FIFO;
			pbuf[2] = SDIO->FIFO;
			pbuf[3] = SDIO->FIFO;
			pbuf[4] = SDIO->FIFO;
			pbuf[5] = SDIO->FIFO;
			pbuf[6] = SDIO->FIFO;
			pbuf[7] = SDIO->FIFO;
			pbuf += 8;
		}
	}while(!((sta = SDIO->STA) & SDIO_BLOCK_READ_STATUS_MASK) && timeout--);

#endif
#ifdef SD_DEBUG
	if(!(sta & SDIO_FLAG_DATAEND)){
		USARTPutString("\r\nMULTI*");
		if(sta & SDIO_FLAG_DCRCFAIL){
			USARTPutString("DCRCFAIL");
		}
		if(sta & SDIO_FLAG_DTIMEOUT){
			USARTPutString("DTIMEOUT");
		}
		if(timeout <= 0){
			USARTPutString("STIMEOUT");
		}
		if(sta & SDIO_FLAG_RXOVERR){
			USARTPutString("RXOVERR");
		}
		if(sta & SDIO_FLAG_STBITERR){
			USARTPutString("STBITERR");
		}
	}
#endif
	// CMD12 Stop Multi Block Read
	SDIO->ICR = 0xffffffff;
	SDIO->ARG = 0;
	SDIO->CMD = SDIO_CPSM_ENABLE    | SDIO_WAIT_NO | \
				SDIO_RESPONSE_SHORT | 12;
	while(!((sta = SDIO->STA) & SDIO_CMD_STATUS_MASK));

#ifdef SD_DEBUG
	if(!(sta & SDIO_FLAG_CMDREND)){
		USARTPutString("\r\nCMD12:");
		if(sta & SDIO_FLAG_CTIMEOUT){
			USARTPutString("CTIMEOUT");
		}
		if(sta & SDIO_FLAG_CCRCFAIL){
			USARTPutString("CCRCFAIL");
		}
	}
#endif

	__enable_irq();
	return 0;
}

inline uint32_t SDMultiBlockWrite(void *buf, uint32_t blockAddress, uint32_t count)
{
	__disable_irq();

	uint32_t *pbuf = (uint32_t*)buf, ret = count * 512;
	uint32_t resbuf[4], res, sta;
	volatile int delay;

	res = SDSendCMD(55, cardInfo.rca, SDIO_RESPONSE_SHORT, resbuf);

	res = SDSendCMD(23, count, SDIO_RESPONSE_SHORT, resbuf);

	res = SDSendCMD(25, cardInfo.csdVer ? blockAddress : (blockAddress << 9), SDIO_RESPONSE_SHORT, resbuf);
	delay = 2000;
	while(delay--){};

//	int fifo_cnt = 0;
	SDIO->DLEN = count * 512;
	SDIO->DTIMER = 0x0fffffff;
	SDIO->ICR = 0xffffffff;
	SDIO->DCTRL = SDIO_DATABLOCK_SIZE_512B | SDIO_TRANSFER_DIR_TO_CARD | \
			      SDIO_TRANSFER_MODE_BLOCK | SDIO_DCTRL_DTEN;
	do{
		while((SDIO->STA & SDIO_FLAG_TXFIFOHE)){
//			if(++fifo_cnt <= (count * 16)){
				SDIO->FIFO = pbuf[0];
				SDIO->FIFO = pbuf[1];
				SDIO->FIFO = pbuf[2];
				SDIO->FIFO = pbuf[3];
				SDIO->FIFO = pbuf[4];
				SDIO->FIFO = pbuf[5];
				SDIO->FIFO = pbuf[6];
				SDIO->FIFO = pbuf[7];
				pbuf += 8;
//			} else {
//				SDIO->FIFO = 0;
//				SDIO->FIFO = 0;
//				SDIO->FIFO = 0;
//				SDIO->FIFO = 0;
//				SDIO->FIFO = 0;
//				SDIO->FIFO = 0;
//				SDIO->FIFO = 0;
//				SDIO->FIFO = 0;
//			}
		}
 	 }while(!((sta = SDIO->STA) & (SDIO_FLAG_TXUNDERR | SDIO_FLAG_DCRCFAIL | SDIO_FLAG_DATAEND | SDIO_FLAG_DTIMEOUT | SDIO_FLAG_STBITERR)));

	if(!(sta & SDIO_FLAG_DATAEND)){
		if(sta & SDIO_FLAG_TXUNDERR){
			USARTPutString("\r\nSDIO_FLAG_TXUNDERR");
		}
		if(sta & SDIO_FLAG_DCRCFAIL){
			USARTPutString("\r\nSDIO_FLAG_DCRCFAIL");
			debug.printf(" SDIO->DCOUNT:%d", SDIO->DCOUNT);
		}
		if(sta & SDIO_FLAG_DTIMEOUT){
			USARTPutString("\r\nSDIO_FLAG_DTIMEOUT");
		}
		if(sta & SDIO_FLAG_STBITERR){
			USARTPutString("\r\nSDIO_FLAG_STBITERR");
		}
	}
	while(SDIO->STA & SDIO_FLAG_TXACT){};

STOP:
	res = SDSendCMD(12, 0, SDIO_RESPONSE_SHORT, resbuf);

	resbuf[0] = 0;
	do{
		res = SDSendCMD(13, cardInfo.rca, SDIO_RESPONSE_SHORT, resbuf);
	}while(!(resbuf[0] & 0x00000100));

	__enable_irq();
	return 0;
}


int SD_Switch_BusWidth(int width)
{
	uint32_t res, resbuf[4];
	SDIO_InitTypeDef SDIO_InitStructure;

	if(width != 1 && width != 4){
		width = 1;
	}

	if(width == 4 && cardInfo.busWidth != 0x5){
		width = 1;
	}

	// CMD55
	USARTPutString("\r\n\nCMD55");
	res = SDSendCMD(55, 0x00000000 | cardInfo.rca, SDIO_RESPONSE_SHORT, resbuf);

	debug.printf("\r\nsta:0x%08x", res);
	debug.printf("\r\nres:0x%08x", resbuf[0]);

	// ACMD6
	USARTPutString("\r\n\nACMD6");
	res = SDSendCMD(6, width == 1 ? 0x000000000 : 0x000000002, SDIO_RESPONSE_SHORT, resbuf);

	debug.printf("\r\nsta:0x%08x", res);
	debug.printf("\r\nres:0x%08x", resbuf[0]);

	SDIO_InitStructure.ClockDiv = 0; // SDIO_CK frequency = SDIOCLK / [CLKDIV + 2] = 48MHz / (0 + 2) = 24MHz
	SDIO_InitStructure.ClockEdge = SDIO_CLOCK_EDGE_RISING;
	SDIO_InitStructure.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
	SDIO_InitStructure.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_ENABLE;
	SDIO_InitStructure.BusWide = width == 1 ? SDIO_BUS_WIDE_1B : SDIO_BUS_WIDE_4B;
	SDIO_InitStructure.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
	SDIO_Init(hsd.Instance, SDIO_InitStructure);

	return res;
}

int SDInit(void){
	uint8_t tbuf[512];
	uint16_t ccc;
	int i, hcsTry = 0, retry = 0, retry2 = 0;
	uint32_t res, resbuf[4];

	hsd.Instance = SDIO;

	cardInfo.ready = 0;

	SDIO_InitTypeDef SDIO_InitStructure;

	HAL_SD_MspInit(&hsd);

	if(HAL_GPIO_ReadPin(SDIO_CARD_DETECT_GPIO_PORT, SDIO_CARD_DETECT_PIN) == GPIO_PIN_SET){
		return -1;
	}

	// SDIO初期設定
SD_INITIALIZE:
	USARTPutString("\r\n***SD Card Debug***");

	SDIO_InitStructure.ClockDiv = 255;
	SDIO_InitStructure.ClockEdge = SDIO_CLOCK_EDGE_RISING;
	SDIO_InitStructure.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
	SDIO_InitStructure.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
	SDIO_InitStructure.BusWide = SDIO_BUS_WIDE_1B;
	SDIO_InitStructure.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
	SDIO_Init(hsd.Instance, SDIO_InitStructure);

	SDIO_PowerState_ON(hsd.Instance);
	SDIO_ClockCmd(ENABLE);

	HAL_Delay(10);

	// CMD0
	USARTPutString("\r\n\nCMD0");
	res = SDSendCMD(0, 0x00000000, SDIO_RESPONSE_NO, resbuf);
	debug.printf("\r\nsta:0x%08x", res);

	HAL_Delay(10);

	// CMD8
	USARTPutString("\r\n\nCMD8");
	res = SDSendCMD(8, 0x000001AA, SDIO_RESPONSE_SHORT, resbuf);
	debug.printf("\r\nsta:0x%08x", res);
	debug.printf("\r\nres:0x%08x", resbuf[0]);


	if(res == SDIO_FLAG_CTIMEOUT){
		USARTPutString("\r\nnot SD card");

		do{
			// CMD1
			USARTPutString("\r\n\nCMD1");
			res = SDSendCMD(1, 0x40FF8000, SDIO_RESPONSE_SHORT, resbuf);
			debug.printf("\r\nsta:0x%08x", res);
			debug.printf("\r\nres:0x%08x", resbuf[0]);

			if(res == SDIO_FLAG_CTIMEOUT) goto SD_INIT;
		}while(!(resbuf[0] & _BV(31)));

		// CMD2
		USARTPutString("\r\n\nCMD2");
		res = SDSendCMD(2, 0x00000000, SDIO_RESPONSE_LONG, resbuf);
		debug.printf("\r\nsta:0x%08x", res);
		debug.printf("\r\nres:0x%08x", resbuf[0]);
		debug.printf("\r\nres:0x%08x", resbuf[1]);
		debug.printf("\r\nres:0x%08x", resbuf[2]);
		debug.printf("\r\nres:0x%08x", resbuf[3]);

		cardInfo.rca = 0x00010000;

		// CMD3
		USARTPutString("\r\n\nCMD3");
		res = SDSendCMD(3, 0x00000000 | cardInfo.rca, SDIO_RESPONSE_SHORT, resbuf);
		debug.printf("\r\nsta:0x%08x", res);
		debug.printf("\r\nres:0x%08x", resbuf[0]);


		// CMD9 SEND_CSD
		USARTPutString("\r\n\nCMD9");
		res = SDSendCMD(9, 0x00000000 | cardInfo.rca, SDIO_RESPONSE_LONG, resbuf);
		debug.printf("\r\nsta:0x%08x", res);
		debug.printf("\r\nres:0x%08x", resbuf[0]);
		debug.printf("\r\nres:0x%08x", resbuf[1]);
		debug.printf("\r\nres:0x%08x", resbuf[2]);
		debug.printf("\r\nres:0x%08x", resbuf[3]);


		// CMD7
		USARTPutString("\r\n\nCMD7");
		res = SDSendCMD(7, 0x00000000 | cardInfo.rca, SDIO_RESPONSE_SHORT, resbuf);
		debug.printf("\r\nsta:0x%08x", res);

		debug.printf("\r\nres:0x%08x", resbuf[0]);

		// CMD6 powerclass
		USARTPutString("\r\n\nCMD6");
		res = SDSendCMD(6, 0x03bb0400, SDIO_RESPONSE_SHORT, resbuf);
		debug.printf("\r\nsta:0x%08x", res);

		USARTPutString("\r\n\nCMD6"); // widebus 4bit
		res = SDSendCMD(6, 0x03b70100, SDIO_RESPONSE_SHORT, resbuf);
		debug.printf("\r\nsta:0x%08x", res);


		HAL_Delay(10);

		SDIO_InitStructure.ClockDiv = 0; // SDIO_CK frequency = SDIOCLK / [CLKDIV + 2] = 48MHz / (0 + 2) = 24MHz
		SDIO_InitStructure.ClockEdge = SDIO_CLOCK_EDGE_RISING;
		SDIO_InitStructure.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
		SDIO_InitStructure.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_ENABLE;
		SDIO_InitStructure.BusWide = SDIO_BUS_WIDE_4B;
		SDIO_InitStructure.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
		SDIO_Init(hsd.Instance, SDIO_InitStructure);

		// CMD8
		USARTPutString("\r\n\nCMD8");
		res = SDSendCMD(8, 0x00000000, SDIO_RESPONSE_SHORT, resbuf);
		debug.printf("\r\nsta:0x%08x", res);


		// DPSM DPSM WAIT_R for ACMD13
		SDSetDPSM(512, SDIO_DATABLOCK_SIZE_512B, SDIO_TRANSFER_DIR_TO_SDIO, SDIO_TRANSFER_MODE_BLOCK, 1000000, tbuf);
/*
		for(i = 0;i < 512;i++){
			if((i % 16) == 0) USARTPutString("\r\n");
			debug.printf("%02x ", tbuf[i]);
		}
		debug.printf("\r\nCard Type:%02x", tbuf[196]);
		debug.printf("\r\nHS_TIMING:%02x", tbuf[185]);
		debug.printf("\r\nBUSWIDTH:%02x", tbuf[183]);
		debug.printf("\r\nTIMEOUT:%02x", tbuf[241]);
*/
		// CMD16
		USARTPutString("\r\n\nCMD16");
		res = SDSendCMD(16, 0x00000200, SDIO_RESPONSE_SHORT, resbuf);

		debug.printf("\r\nsta:0x%08x", res);
		debug.printf("\r\nres:0x%08x", resbuf[0]);

		cardInfo.csdVer = CSD_VER_1XX;
		cardInfo.mmc = 1;
		cardInfo.ready = 1;
		/*
		int n;
		for(n = 0;n < 1000;n++){
		SDBlockRead(tbuf, 0);
		for(i = 0;i < 512;i++){
			if((i % 16) == 0) USARTPutString("\r\n");
			debug.printf("%02x ", tbuf[i]);

		}
		}
		while(1);
		*/
		return 0;
	}
SD_INIT:

	do{
		// CMD55
		USARTPutString("\r\n\nCMD55");
		res = SDSendCMD(55, 0x00000000, SDIO_RESPONSE_SHORT, resbuf);
		debug.printf("\r\nsta:0x%08x", res);
		debug.printf("\r\nres:0x%08x", resbuf[0]);


		// ACMD41
		if(!hcsTry){
			USARTPutString("\r\n\nACMD41");
			res = SDSendCMD(41, 0x00ff8000 | _BV(30), SDIO_RESPONSE_SHORT, resbuf);
			debug.printf("\r\nsta:0x%08x", res);
			debug.printf("\r\nres:0x%08x", resbuf[0]);

			if(res == SDIO_FLAG_CTIMEOUT){
				hcsTry = 1;
			}
		}else{
			USARTPutString("\r\n\nACMD41");
			res = SDSendCMD(41, 0x00ff8000, SDIO_RESPONSE_SHORT, resbuf);
			debug.printf("\r\nsta:0x%08x", res);
			debug.printf("\r\nres:0x%08x", resbuf[0]);
		}
		if(++retry > 50){
			USARTPutString("\r\n\nCard initialization failed!!");
			return -1;
			retry = 0;
			if(++retry2 >= 3){
				return -1;
			}
			goto SD_INITIALIZE;
		}
	}while(!(resbuf[0] & _BV(31)));

	if(resbuf[0] & _BV(30)){
		cardInfo.csdVer = CSD_VER_2XX;
	}else{
		cardInfo.csdVer = CSD_VER_1XX;
	}


	// CMD2
	USARTPutString("\r\n\nCMD2");
	res = SDSendCMD(2, 0x00000000, SDIO_RESPONSE_LONG, resbuf);
	debug.printf("\r\nsta:0x%08x", res);
	debug.printf("\r\nres:0x%08x", resbuf[0]);
	debug.printf("\r\nres:0x%08x", resbuf[1]);
	debug.printf("\r\nres:0x%08x", resbuf[2]);
	debug.printf("\r\nres:0x%08x", resbuf[3]);


	// CMD3
	USARTPutString("\r\n\nCMD3");
	res = SDSendCMD(3, 0x00000000, SDIO_RESPONSE_SHORT, resbuf);
	debug.printf("\r\nsta:0x%08x", res);
	debug.printf("\r\nres:0x%08x", resbuf[0]);


	cardInfo.rca = resbuf[0] & 0xffff0000;

	// CMD9 SEND_CSD
	USARTPutString("\r\n\nCMD9");
	res = SDSendCMD(9, 0x00000000 | cardInfo.rca, SDIO_RESPONSE_LONG, resbuf);
	debug.printf("\r\nsta:0x%08x", res);
	debug.printf("\r\nres:0x%08x", resbuf[0]);
	debug.printf("\r\nres:0x%08x", resbuf[1]);
	debug.printf("\r\nres:0x%08x", resbuf[2]);
	debug.printf("\r\nres:0x%08x", resbuf[3]);


	// TRAN_SPEED
	cardInfo.tranSpeed  = tranUnit[resbuf[0] & 0x00000007];
	cardInfo.tranSpeed *= timeVal[(resbuf[0] & 0x00000078) >> 3];

	// MAX_CLOCK_FREQUENCY
	ccc = (resbuf[1] >> 20) & 0x0fff;
	cardInfo.maxClkFreq = (ccc & _BV(10)) ? 50:20;

	if(cardInfo.csdVer == CSD_VER_1XX){
		cardInfo.c_size  = (resbuf[1] & 0x000003ff) << 2;
		cardInfo.c_size |= (resbuf[2] >> 30) & 0x3;
//		cardInfo.c_size_mult = (resbuf[2] & 0x00070000) >> 16;
		cardInfo.c_size_mult = (resbuf[2] & 0x00038000) >> 15;
		cardInfo.read_bl_len = (resbuf[1] & 0x000f0000) >> 16;
		cardInfo.totalBlocks = ((cardInfo.c_size + 1) << (cardInfo.c_size_mult + cardInfo.read_bl_len - 7));
	} else if(cardInfo.csdVer == CSD_VER_2XX){
		cardInfo.c_size  = (resbuf[1] & 0x0000003f) << 16;
		cardInfo.c_size |= (resbuf[2] & 0xffff0000) >> 16;
		cardInfo.totalBlocks = (cardInfo.c_size + 1) << 10;
	}


	// CMD7
	USARTPutString("\r\n\nCMD7");
	res = SDSendCMD(7, 0x00000000 | cardInfo.rca, SDIO_RESPONSE_SHORT, resbuf);
	debug.printf("\r\nsta:0x%08x", res);
	debug.printf("\r\nres:0x%08x", resbuf[0]);


	SDIO_InitStructure.ClockDiv = 0; // SDIO_CK frequency = SDIOCLK / [CLKDIV + 2] = 48MHz / (0 + 2) = 24MHz
	SDIO_InitStructure.ClockEdge = SDIO_CLOCK_EDGE_RISING;
	SDIO_InitStructure.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
	SDIO_InitStructure.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_ENABLE;
	SDIO_InitStructure.BusWide = SDIO_BUS_WIDE_1B;
	SDIO_InitStructure.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
	SDIO_Init(hsd.Instance, SDIO_InitStructure);


	// CMD16
	USARTPutString("\r\n\nCMD16");
	res = SDSendCMD(16, 0x00000200, SDIO_RESPONSE_SHORT, resbuf);
	debug.printf("\r\nsta:0x%08x", res);
	debug.printf("\r\nres:0x%08x", resbuf[0]);


	// CMD55
	USARTPutString("\r\n\nCMD55");
	res = SDSendCMD(55, 0x00000000 | cardInfo.rca, SDIO_RESPONSE_SHORT, resbuf);
	debug.printf("\r\nsta:0x%08x", res);
	debug.printf("\r\nres:0x%08x", resbuf[0]);


	// ACMD13 SD_STATUS
	USARTPutString("\r\n\nACMD13");
	res = SDSendCMD(13, 0x00000000, SDIO_RESPONSE_SHORT, resbuf);
	debug.printf("\r\nsta:0x%08x", res);
	debug.printf("\r\nres:0x%08x", resbuf[0]);

	// DPSM DPSM WAIT_R for ACMD13
	SDSetDPSM(64, SDIO_DATABLOCK_SIZE_64B, SDIO_TRANSFER_DIR_TO_SDIO, SDIO_TRANSFER_MODE_BLOCK, 1000000, tbuf);

	// Speed Class Rating
	cardInfo.speedClass = tbuf[8] * 2;
	cardInfo.speedClass = cardInfo.speedClass < 8 ? cardInfo.speedClass : 10;

	// ACMD51
	// CMD55
	USARTPutString("\r\n\nCMD55");
	res = SDSendCMD(55, 0x00000000 | cardInfo.rca, SDIO_RESPONSE_SHORT, resbuf);
	debug.printf("\r\nsta:0x%08x", res);
	debug.printf("\r\nres:0x%08x", resbuf[0]);


	// ACMD51 SEND_SCR
	USARTPutString("\r\n\nACMD51");
	res = SDSendCMD(51, 0x00000000, SDIO_RESPONSE_SHORT, resbuf);
	debug.printf("\r\nsta:0x%08x", res);
	debug.printf("\r\nres:0x%08x", resbuf[0]);


	// DPSM DPSM WAIT_R for ACMD51
	SDSetDPSM(8, SDIO_DATABLOCK_SIZE_8B, SDIO_TRANSFER_DIR_TO_SDIO, SDIO_TRANSFER_MODE_BLOCK, 1000000, tbuf);

	cardInfo.specVer = tbuf[0] & 0x0f;
	cardInfo.busWidth = tbuf[1] & 0x0f;

	cardInfo.mmc = 0;

	SD_Switch_BusWidth(4);

	cardInfo.ready = 1;
/*
		// CMD6
		USARTPutString("\r\n\nCMD6");
		res = SDSendCMD(6, 0x00FFFFFF, SDIO_RESPONSE_SHORT, resbuf);
	//	res = SDSendCMD(6, 0x00000001, SDIO_RESPONSE_SHORT, resbuf);

		debug.printf("\r\nsta:0x%08x", res);
		debug.printf("\r\nres:0x%08x", resbuf[0]);

		// DPSM DPSM WAIT_R for CMD6
		SDSetDPSM(64, SDIO_DataBlockSize_64b, SDIO_TRANSFER_DIR_TO_SDIO, SDIO_TRANSFER_MODE_BLOCK, 1000000, tbuf);

		debug.printf("\r\nHS support:%02x", tbuf[63-50]);
		debug.printf("\r\nswitch status:%02x", tbuf[63-47]);
		debug.printf("\r\nbusy check:%02x", tbuf[63-34]);
		debug.printf("\r\ninfo:%02x", tbuf[63-50]);


		for(i = 0;i < 64;i++){
			if((i % 16) == 0) USARTPutString("\r\n");
			debug.printf("%02x", tbuf[i]);

		}

		// CMD6
		USARTPutString("\r\n\nCMD6");
//		res = SDSendCMD(6, 0x80FFFFF1, SDIO_RESPONSE_SHORT, resbuf);
		res = SDSendCMD(6, 0x80000001, SDIO_RESPONSE_SHORT, resbuf);

		debug.printf("\r\nsta:0x%08x", res);
		debug.printf("\r\nres:0x%08x", resbuf[0]);

		// DPSM DPSM WAIT_R for CMD6
		SDSetDPSM(64, SDIO_DataBlockSize_64b, SDIO_TRANSFER_DIR_TO_SDIO, SDIO_TRANSFER_MODE_BLOCK, 100000, tbuf);

		HAL_Delay(100);

		SDIO_InitStructure.ClockDiv = 0; // SDIO_CK frequency = SDIOCLK / [CLKDIV + 2] = 48MHz / (0 + 2) = 24MHz
		SDIO_InitStructure.SDIO_ClockEdge = SDIO_ClockEdge_Rising;
		SDIO_InitStructure.SDIO_ClockBypass = SDIO_ClockBypass_Enable;
		SDIO_InitStructure.SDIO_ClockPowerSave = SDIO_ClockPowerSave_Enable;
		SDIO_InitStructure.SDIO_BusWide = SDIO_BusWide_4b;
		SDIO_InitStructure.SDIO_HardwareFlowControl = SDIO_HardwareFlowControl_Disable;
		SDIO_Init(&SDIO_InitStructure);

		debug.printf("\r\nswitch status:%02x", tbuf[63-47]);
		debug.printf("\r\nbusy check:%02x", tbuf[63-34]);
		debug.printf("\r\ninfo:%02x", tbuf[63-50]);

		for(i = 0;i < 64;i++){
			if((i % 16) == 0) USARTPutString("\r\n");
			debug.printf("%02x", tbuf[i]);

		}

		// CMD6
		USARTPutString("\r\n\nCMD6");
		res = SDSendCMD(6, 0x00FFFFFF, SDIO_RESPONSE_SHORT, resbuf);
	//	res = SDSendCMD(6, 0x80000001, SDIO_RESPONSE_SHORT, resbuf);

		debug.printf("\r\nsta:0x%08x", res);
		debug.printf("\r\nres:0x%08x", resbuf[0]);


		// DPSM DPSM WAIT_R for CMD6
		SDSetDPSM(64, SDIO_DataBlockSize_64b, SDIO_TRANSFER_DIR_TO_SDIO, SDIO_TRANSFER_MODE_BLOCK, 100000, tbuf);

		debug.printf("\r\nswitch status:%02x", tbuf[63-47]);
		debug.printf("\r\nbusy check:%02x", tbuf[63-34]);
		debug.printf("\r\ninfo:%02x", tbuf[63-50]);


		for(i = 0;i < 64;i++){
			if((i % 16) == 0) USARTPutString("\r\n");
			debug.printf("%02x", tbuf[i]);

		}
		*/

	/*
	memset(tbuf, '\0', sizeof(tbuf));
	SDMultiBlockRead(tbuf, 0, 1);

	USARTPutString("\r\nMBR");
	for(i = 0;i < 512;i++){
		if((i % 16) == 0) USARTPutString("\r\n");
		debug.printf("%02x ", tbuf[i]);

	}
*/
	debug.printf("\r\n\nSpec Version:%s", &specVer[cardInfo.specVer][0]);
	debug.printf("\r\nHigh Capacity:%s", cardInfo.csdVer ? "Yes" : "No");
	char s[10];
	if(cardInfo.speedClass){
		SPRINTF(s, "CLASS%d", cardInfo.speedClass);
	} else {
		strcpy(s, "N/A");
	}
	debug.printf("\r\nSpeed Class:%s", s);
	debug.printf("\r\nSupported Bus Widths:%s", &busWidth[cardInfo.busWidth][0]);
	debug.printf("\r\nMax Transfer Speed Per Bus:%dMbit/s", cardInfo.tranSpeed / 1000);
	debug.printf("\r\nMax Clock Frequency:%dMHz", cardInfo.maxClkFreq);
	debug.printf("\r\nTotal Blocks:%d", cardInfo.totalBlocks);
	debug.printf("\r\nccc:%04x", ccc);
//	debug.printf("\r\nCard Capacity:%0.2fGB", (float)cardInfo.totalBlocks / 1000000000.0f * 512.0f);


	return 0;
}

