/**
  ******************************************************************************
  * @file    GPIO/GPIO_IOToggle/Src/stm32f4xx_it.c 
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    26-February-2014
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"
#include "usart.h"
#include "lcd.h"
#include "sd.h"
#include "icon.h"
#include "settings.h"
#include <stdlib.h>
/** @addtogroup STM32F4xx_HAL_Examples
  * @{
  */

/** @addtogroup GPIO_IOToggle
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

void prvGetRegistersFromStack( uint32_t *pulFaultStackAddress )
{
/* These are volatile to try and prevent the compiler/linker optimising them
away as the variables never actually get used.  If the debugger won't show the
values of the variables, make them global my moving their declaration outside
of this function. */
volatile uint32_t r0;
volatile uint32_t r1;
volatile uint32_t r2;
volatile uint32_t r3;
volatile uint32_t r12;
volatile uint32_t lr; /* Link register. */
volatile uint32_t pc; /* Program counter. */
volatile uint32_t psr;/* Program status register. */

    r0 = pulFaultStackAddress[ 0 ];
    r1 = pulFaultStackAddress[ 1 ];
    r2 = pulFaultStackAddress[ 2 ];
    r3 = pulFaultStackAddress[ 3 ];

    r12 = pulFaultStackAddress[ 4 ];
    lr = pulFaultStackAddress[ 5 ];
    pc = pulFaultStackAddress[ 6 ];
    psr = pulFaultStackAddress[ 7 ];

    /* When the following line is hit, the variables contain the register values. */
    for( ;; );
}

/* The fault handler implementation calls a function called
prvGetRegistersFromStack(). */
__attribute__( ( naked ) )
void HardFault_Handler(void)
{
    __asm volatile
    (
        " tst lr, #4                                                \n"
        " ite eq                                                    \n"
        " mrseq r0, msp                                             \n"
        " mrsne r0, psp                                             \n"
        " ldr r1, [r0, #24]                                         \n"
        " ldr r2, handler2_address_const                            \n"
        " bx r2                                                     \n"
        " handler2_address_const: .word prvGetRegistersFromStack    \n"
    );
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
  HAL_IncTick();
}

/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

void TIM1_UP_TIM10_IRQHandler(void)
{
	if(TIM1_SR_UIF_BB)
	{
		TIM1_SR_UIF_BB = 0;
		++time;

		if((settings_group.disp_conf.time2sleep == 0) || (sleep_time.flags.enable == 0)){
			return;
		}

		++sleep_time.curTime;

		if((abs(sleep_time.curTime - sleep_time.prevTime) > (settings_group.disp_conf.time2sleep / 2)) && !sleep_time.flags.dimLight){ // Dim Light
			LCDSetPWMValue(settings_group.disp_conf.brightness * 0.3f);
			sleep_time.flags.dimLight = 1;
		}

		if(abs(sleep_time.curTime - sleep_time.prevTime) > settings_group.disp_conf.time2sleep && sleep_time.flags.dimLight){ // Sleep Enable
			sleep_time.flags.stop_mode = 1;
		}

	}
}

void TIM_PROGRESS_BAR_IRQHandler(void)
{
	static uint8_t idx = 0;
	__HAL_TIM_CLEAR_IT(&TimProgressBar, TIM_IT_UPDATE);

	LCD_CMD(0x002c);
	SPI_LCD_NSS_PIN_ASSERT;
	SPI_LCD_RS_PIN_DEASSERT;
	HAL_SPI_Transmit_DMA(&SpiLcdHandle, (uint8_t*)&progress_circular_bar_16x16x12[idx++ * 16 * 16], (16 * 16 * 2) / sizeof(uint16_t));
	if(idx >= 11)
	{
		idx = 0;
	}
}

void USARTx_IRQHandler(void)
{
	HAL_UART_IRQHandler(&UartHandle);
}

/**
  * @brief  This function handles DMA Tx interrupt request.
  * @param  None
  * @retval None
  */
void SPI_LCD_DMA_TX_IRQHandler(void)
{
  HAL_DMA_IRQHandler(SpiLcdHandle.hdmatx);
}

void SDIO_IRQHandler(void)
{
	HAL_SD_IRQHandler(&hsd);
}

void EXTI0_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(SW_PUSH_RIGHT_PIN);
}

void EXTI1_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(SW_PUSH_DOWN_PIN);
}

void EXTI2_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(SW_PUSH_LEFT_PIN);
}

void EXTI4_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(SW_PUSH_ENTER_PIN);
}

void EXTI9_5_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(SW_PUSH_UP_PIN);
}

void EXTI15_10_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(SDIO_CARD_DETECT_PIN);
}



const GPIO_TypeDef* port_maps[] =
{
		SW_PUSH_RIGHT_PORT, // EXTI0
		SW_PUSH_DOWN_PORT,  // EXTI1
		SW_PUSH_LEFT_PORT,  // EXTI2
		NULL,
		SW_PUSH_ENTER_PORT, // EXTI4
		NULL,
		NULL,
		SW_PUSH_UP_PORT,    // EXTI7
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		SDIO_CARD_DETECT_GPIO_PORT // EXTI14
};

uint16_t sw_pushed, sw_pressing =  0, sw_touched = 0;

void TIM_DEBOUNCE_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&TimHandleDebounce);
}

void TIM_SW_HOLD_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&TimHandleSwHold);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (sw_pressing != GPIO_Pin) { // switch pressed (Falling Edge Detect)

		sw_pushed = GPIO_Pin;
		TIM_DEBOUNCE->CNT = 0; // clear debounce timer
		HAL_TIM_Base_Start_IT(&TimHandleDebounce); // start debounce

	} else {					// switch released (Rising Edge Detect)
		if(!sw_touched && current_mode == SW_MODE_PLAYER){
			switch (sw_pressing) {
			case SW_PUSH_ENTER_PIN:
				break;
			case SW_PUSH_LEFT_PIN:
//				debug.printf("\r\nprev");
				LCDStatusStruct.waitExitKey = PLAY_PREV;
				break;
			case SW_PUSH_RIGHT_PIN:
//				debug.printf("\r\nnext");
				LCDStatusStruct.waitExitKey = PLAY_NEXT;
				break;
			case SW_PUSH_UP_PIN:
				debug.printf("\r\nexit");
				LCDStatusStruct.waitExitKey = 0;
				break;
			case SW_PUSH_DOWN_PIN:
				break;
			default:
				break;
			}
		}

		// switch Rising Edge to Falling Edge
		SET_SW_EDGE_FALLING(sw_pressing);

		sw_pressing = 0;
		sw_touched = 0;
	}
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

	if (htim->Instance == TIM_SW_HOLD) { // switch hold detection
		HAL_TIM_Base_Stop_IT(htim);
		if(current_mode == SW_MODE_PLAYER){
			switch (sw_pressing) {
			case SW_PUSH_ENTER_PIN:
				break;
			case SW_PUSH_LEFT_PIN:
	//			debug.printf("\r\nLeft");
				LCDStatusStruct.waitExitKey = PLAY_SW_HOLD_LEFT;
				break;
			case SW_PUSH_RIGHT_PIN:
	//			debug.printf("\r\nRight");
				LCDStatusStruct.waitExitKey = PLAY_SW_HOLD_RIGHT;
				break;
			default:
				break;
			}
		}

		// switch Rising Edge to Falling Edge
		SET_SW_EDGE_FALLING(sw_pressing);

		sw_pressing = 0;
		sw_touched = 0;
		return;
	}

	if (htim->Instance == TIM_DEBOUNCE) { // after tim debounce millisecond past
		HAL_TIM_Base_Stop_IT(htim);

		if (HAL_GPIO_ReadPin((GPIO_TypeDef*)port_maps[BITPOS_TO_NUMBER(sw_pushed)], sw_pushed) != GPIO_PIN_RESET) { // switch not pushed then ignore push
			if(sw_pushed == SDIO_CARD_DETECT_PIN){
				debug.printf("\r\n*CARD PULLED");

				C_PCFFontInit((uint32_t)internal_flash_pcf_font, (size_t)_sizeof_internal_flash_pcf_font);
				LCD_FUNC.putChar = C_PCFPutChar;
				LCD_FUNC.putWideChar = C_PCFPutChar;
				LCD_FUNC.getCharLength = C_PCFGetCharPixelLength;
				pcf_font.ext_loaded = 0;

				cardInfo.ready = 0;
				initFat();
				LCDPrintFileList();
			}
			return;
		}

		sleep_time.prevTime = sleep_time.curTime;

		if(sleep_time.flags.wakeup)
		{
			sleep_time.flags.wakeup = 0;
			return;
		}

		if(sleep_time.flags.dimLight)
		{
			LCDSetPWMValue(settings_group.disp_conf.brightness);
			sleep_time.flags.dimLight = 0;
		}


		if(sw_pushed == SDIO_CARD_DETECT_PIN){
			debug.printf("\r\nCARD INSERTED");

			HAL_SD_ErrorTypedef errorState = SDInit();
			debug.printf("\r\nSD_Init:%d", errorState);

			settings_mode = 0;
			int  ret = initFat();
			  if(ret == 0){
				  const char fat_init_succeeded_str[] = "\r\nFat initialization succeeded.";
				  debug.printf(fat_init_succeeded_str);
				  if(fat.fsType == FS_TYPE_FAT16){
					  debug.printf("\r\nFileSystem:FAT16");
				  } else if (fat.fsType == FS_TYPE_FAT32){
					  debug.printf("\r\nFileSystem:FAT32");
				  }
				  debug.printf("\r\nCluster Size:%dKB", (fat.sectorsPerCluster * 512) / 1024);
			  } else {
				  const char fat_init_failed_str[] = "\r\nFat initialization failed.";
				  debug.printf("%s: %d", fat_init_failed_str, ret);
			  }

			  if(PCFFontInit(getIdByName("FONT.PCF")) != -1){
				  debug.printf("\r\nexternal font loaded.");
				  LCD_FUNC.putChar = PCFPutChar;
				  LCD_FUNC.putWideChar = PCFPutChar;
				  LCD_FUNC.getCharLength = PCFGetCharPixelLength;
				  pcf_font.ext_loaded = 1;
			  } else {
				  debug.printf("\r\nexternal font load failed.");
				  // internal flash pcf font
				  C_PCFFontInit((uint32_t)internal_flash_pcf_font, (size_t)_sizeof_internal_flash_pcf_font);
				  debug.printf("\r\ninternal flash font loaded.");
				  LCD_FUNC.putChar = C_PCFPutChar;
				  LCD_FUNC.putWideChar = C_PCFPutChar;
				  LCD_FUNC.getCharLength = C_PCFGetCharPixelLength;
				  pcf_font.ext_loaded = 0;
			  }

			  LCDFadeIn(1, filer_bgimg_160x128);

			  LCDPrintFileList();

			LCDStatusStruct.waitExitKey = 0;
			return;
		}


		if (current_mode == SW_MODE_FILER) { // Filer Mode
			switch (sw_pushed) {
			case SW_PUSH_ENTER_PIN:
				LCDCursorEnter();
				break;
			case SW_PUSH_LEFT_PIN:
				LCDPutCursorBar(cursor.pos);
				LCDStoreCursorBar(0);
				cursor.pos = 0, cursor.pageIdx = 0;
				LCDCursorEnter();
				break;
			case SW_PUSH_RIGHT_PIN:
				LCDCursorEnter();
				break;
			case SW_PUSH_UP_PIN:
				LCDCursorUp();
				__HAL_TIM_CLEAR_FLAG(&TimHandleSwHold, TIM_FLAG_UPDATE);
				TimHandleSwHold.Instance->CNT = 0;
				HAL_TIM_Base_Start(&TimHandleSwHold);
				while(HAL_GPIO_ReadPin((GPIO_TypeDef*)port_maps[BITPOS_TO_NUMBER(sw_pushed)], sw_pushed) == GPIO_PIN_RESET){
					if(__HAL_TIM_GET_FLAG(&TimHandleSwHold, TIM_FLAG_UPDATE) == SET){
						__HAL_TIM_CLEAR_FLAG(&TimHandleSwHold, TIM_FLAG_UPDATE);
						do{
							sleep_time.prevTime = sleep_time.curTime;
							LCDCursorUp();
							HAL_Delay(80);
						}while(HAL_GPIO_ReadPin((GPIO_TypeDef*)port_maps[BITPOS_TO_NUMBER(sw_pushed)], sw_pushed) == GPIO_PIN_RESET);
					}
				};
				HAL_TIM_Base_Stop(&TimHandleSwHold);
				break;
			case SW_PUSH_DOWN_PIN:
				LCDCursorDown();
				__HAL_TIM_CLEAR_FLAG(&TimHandleSwHold, TIM_FLAG_UPDATE);
				TimHandleSwHold.Instance->CNT = 0;
				HAL_TIM_Base_Start(&TimHandleSwHold);
				while(HAL_GPIO_ReadPin((GPIO_TypeDef*)port_maps[BITPOS_TO_NUMBER(sw_pushed)], sw_pushed) == GPIO_PIN_RESET){
					if(__HAL_TIM_GET_FLAG(&TimHandleSwHold, TIM_FLAG_UPDATE) == SET){
						__HAL_TIM_CLEAR_FLAG(&TimHandleSwHold, TIM_FLAG_UPDATE);
						do{
							sleep_time.prevTime = sleep_time.curTime;
							LCDCursorDown();
							HAL_Delay(80);
						}while(HAL_GPIO_ReadPin((GPIO_TypeDef*)port_maps[BITPOS_TO_NUMBER(sw_pushed)], sw_pushed) == GPIO_PIN_RESET);
					}
				};
				HAL_TIM_Base_Stop(&TimHandleSwHold);
				break;
			default:
				break;
			}
		} else if (current_mode == SW_MODE_PLAYER) { // Play Mode
			switch (sw_pushed) {
			case SW_PUSH_ENTER_PIN:
				break;
			case SW_PUSH_LEFT_PIN:
				if (sw_pressing == SW_PUSH_ENTER_PIN) {
					LCDStatusStruct.waitExitKey = VOL_MUTE;
					sw_touched = 1;
					return;
				}
				HAL_TIM_Base_Start_IT(&TimHandleSwHold);
				break;
			case SW_PUSH_RIGHT_PIN:
				if (sw_pressing == SW_PUSH_ENTER_PIN) {
					LCDStatusStruct.waitExitKey = PLAY_LOOP_MODE; // Change Play Loop Mode
					sw_touched = 1;
					return;
				}
				HAL_TIM_Base_Start_IT(&TimHandleSwHold);
				break;
			case SW_PUSH_UP_PIN:
				if (sw_pressing == SW_PUSH_ENTER_PIN) {
//					debug.printf("\r\nL+UP");
					LCDStatusStruct.waitExitKey = VOL_UP; // Volume +1db
					sw_touched = 1;
					return;
				}
				break;
			case SW_PUSH_DOWN_PIN:
				if (sw_pressing == SW_PUSH_ENTER_PIN) {
//					debug.printf("\r\nL+DOWN");
					LCDStatusStruct.waitExitKey = VOL_DOWN; // Volume -1db
					sw_touched = 1;
					return;
				}
				LCDStatusStruct.waitExitKey = PLAY_PAUSE; // Play Pause
				break;
			default:
				break;
			}
			// switch Falling Edge to Rising Edge: to detect switch release
			SET_SW_EDGE_RISING(sw_pushed);

			sw_pressing = sw_pushed;
		}
	}
}

void AUDIO_I2Sx_DMAx_IRQHandler(void)
{
	HAL_DMA_IRQHandler(haudio_i2s.hdmatx);
}


/**
  * @}
  */ 

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
