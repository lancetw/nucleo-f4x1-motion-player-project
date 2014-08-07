/**
  ******************************************************************************
  * @file    GPIO/GPIO_IOToggle/Inc/main.h 
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    26-February-2014
  * @brief   Header for main.c module
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx_nucleo.h"

#include "picojpeg.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

extern int8_t current_mode; // 0:Filer 1:Music

#define DJPEG_PLAY        (1 << 2)
#define DJPEG_ARROW_LEFT  (1 << 1)
#define DJPEG_ARROW_RIGHT (1 << 0)

/* Push Switch Definition */
#define SW_PUSH_ENTER_CLK_ENABLE()		__GPIOC_CLK_ENABLE()
#define SW_PUSH_LEFT_CLK_ENABLE()		__GPIOB_CLK_ENABLE()
#define SW_PUSH_RIGHT_CLK_ENABLE()		__GPIOC_CLK_ENABLE()
#define SW_PUSH_UP_CLK_ENABLE()			__GPIOC_CLK_ENABLE()
#define SW_PUSH_DOWN_CLK_ENABLE()		__GPIOC_CLK_ENABLE()

#define SW_PUSH_ENTER_PIN				GPIO_PIN_4
#define SW_PUSH_ENTER_PORT				GPIOC

#define SW_PUSH_LEFT_PIN				GPIO_PIN_2
#define SW_PUSH_LEFT_PORT				GPIOB

#define SW_PUSH_RIGHT_PIN				GPIO_PIN_0
#define SW_PUSH_RIGHT_PORT				GPIOC

#define SW_PUSH_UP_PIN					GPIO_PIN_7
#define SW_PUSH_UP_PORT					GPIOC

#define SW_PUSH_DOWN_PIN				GPIO_PIN_1
#define SW_PUSH_DOWN_PORT				GPIOC

#define SW_HOLD_CNT_VAL					2

#define TIM_DEBOUNCE					TIM5
#define TIM_DEBOUNCE_CLK_ENABLE()		__TIM5_CLK_ENABLE()
#define TIM_DEBOUNCE_IRQn				TIM5_IRQn
#define TIM_DEBOUNCE_IRQHandler			TIM5_IRQHandler
#define TIM_DEBOUNCE_MICROSECOND        40000 // x1us

#define TIM_PROGRESS_BAR				TIM9
#define TIM_PROGRESS_BAR_CLK_ENABLE()	__TIM9_CLK_ENABLE()
#define TIM_PROGRESS_BAR_IRQn			TIM1_BRK_TIM9_IRQn
#define TIM_PROGRESS_BAR_IRQHandler		TIM1_BRK_TIM9_IRQHandler

#define TIM_LCD_PWM						TIM2
#define TIM_LCD_PWM_CLK_ENABLE()		__TIM2_CLK_ENABLE()
#define GPIO_LCD_PWM_CLK_ENABLE()		__GPIOA_CLK_ENABLE()
#define GPIO_PIN_CHANNEL2              	GPIO_PIN_1

#define LCD_PWM_PERIOD					4200

extern	TIM_HandleTypeDef TimProgressBar;

#define TIM_DURATION					TIM3
#define TIM_DURATION_CLK_ENABLE()		__TIM3_CLK_ENABLE()

#define TIM_SW_HOLD						TIM4
#define TIM_SW_HOLD_CLK_ENABLE() 		__TIM4_CLK_ENABLE()
#define TIM_SW_HOLD_IRQn				TIM4_IRQn
#define TIM_SW_HOLD_IRQHandler			TIM4_IRQHandler
#define TIM_SW_HOLD_INTERVAL	  		2500 // x100us

#define SW_MODE_FILER					0
#define SW_MODE_PLAYER					1

#define TIM_1SEC						TIM1
#define TIM_1SEC_CLK_ENABLE()			__TIM1_CLK_ENABLE()
#define TIM_1SEC_IRQn					TIM1_UP_TIM10_IRQn
#define TIM_1SEC_IRQHandler				TIM1_UP_TIM10_IRQHandler

extern TIM_HandleTypeDef Tim1SecHandle;
extern int time;


#define BITPOS_TO_NUMBER(x)				__CLZ(__RBIT(x))
#define SET_SW_EDGE_RISING(x)		do{\
											EXTI->FTSR &= ~(x);\
											EXTI->RTSR |= (x);\
										}while(0)

#define SET_SW_EDGE_FALLING(x)		do{\
											EXTI->RTSR &= ~(x);\
											EXTI->FTSR |= (x);\
										}while(0)


/* User can use this section to tailor I2Cx/I2Cx instance used and associated
   resources */
/* Definition for I2Cx clock resources */
#define I2Cx                             I2C3
#define I2Cx_CLK_ENABLE()                __I2C3_CLK_ENABLE()
#define I2Cx_SDA_GPIO_CLK_ENABLE()       __GPIOB_CLK_ENABLE()
#define I2Cx_SCL_GPIO_CLK_ENABLE()       __GPIOA_CLK_ENABLE()

#define I2Cx_FORCE_RESET()               __I2C3_FORCE_RESET()
#define I2Cx_RELEASE_RESET()             __I2C3_RELEASE_RESET()

/* Definition for I2Cx Pins */
#define I2Cx_SCL_PIN                    GPIO_PIN_8
#define I2Cx_SCL_GPIO_PORT              GPIOA
#define I2Cx_SCL_AF                     GPIO_AF4_I2C3
#define I2Cx_SDA_PIN                    GPIO_PIN_4
#define I2Cx_SDA_GPIO_PORT              GPIOB
#define I2Cx_SDA_AF                     GPIO_AF9_I2C3


/* Definition for USARTx clock resources */
#define USARTx                           USART2
#define USARTx_CLK_ENABLE()              __USART2_CLK_ENABLE();
#define USARTx_RX_GPIO_CLK_ENABLE()      __GPIOA_CLK_ENABLE()
#define USARTx_TX_GPIO_CLK_ENABLE()      __GPIOA_CLK_ENABLE()

#define USARTx_FORCE_RESET()             __USART2_FORCE_RESET()
#define USARTx_RELEASE_RESET()           __USART2_RELEASE_RESET()

/* Definition for USARTx Pins */
#define USARTx_TX_PIN                    GPIO_PIN_2
#define USARTx_TX_GPIO_PORT              GPIOA
#define USARTx_TX_AF                     GPIO_AF7_USART2
#define USARTx_RX_PIN                    GPIO_PIN_3
#define USARTx_RX_GPIO_PORT              GPIOA
#define USARTx_RX_AF                     GPIO_AF7_USART2

#define USARTx_IRQn						 USART2_IRQn
#define USARTx_IRQHandler           	 USART2_IRQHandler


/* WM8731 supply */
#define WM8731_SUPPLY_GPIO_CLK_ENABLE()     __GPIOC_CLK_ENABLE()
#define WM8731_SUPPLY_GPIO_PORT				GPIOC
#define WM8731_SUPPLY_PIN					GPIO_PIN_15

/* Definition for SPI_LCD clock resources */
#define SPI_LCD                             SPI1
#define DMA_SPI_LCD_CLK_ENABLE()       		__DMA2_CLK_ENABLE()
#define SPI_LCD_CLK_ENABLE()                __SPI1_CLK_ENABLE()
#define SPI_LCD_SCK_GPIO_CLK_ENABLE()       __GPIOB_CLK_ENABLE()
#define SPI_LCD_MISO_GPIO_CLK_ENABLE()      __GPIOA_CLK_ENABLE()
#define SPI_LCD_MOSI_GPIO_CLK_ENABLE()      __GPIOA_CLK_ENABLE()
#define SPI_LCD_NSS_GPIO_CLK_ENABLE()       __GPIOA_CLK_ENABLE()

#define SPI_LCD_RS_GPIO_CLK_ENABLE()        __GPIOC_CLK_ENABLE()

#define SPI_LCD_RESET_GPIO_CLK_ENABLE()     __GPIOC_CLK_ENABLE()

#define SPI_LCD_FORCE_RESET()               __SPI1_FORCE_RESET()
#define SPI_LCD_RELEASE_RESET()             __SPI1_RELEASE_RESET()

#define LCD_BKPWM_GPIO_CLK_ENABLE()			__GPIOA_CLK_ENABLE()
#define LCD_BKPWM_PIN						GPIO_PIN_1
#define LCD_BKPWM_GPIO_PORT					GPIOA

/* Definition for SPI_LCD Pins */
#define SPI_LCD_SCK_PIN                     GPIO_PIN_3
#define SPI_LCD_SCK_GPIO_PORT               GPIOB
#define SPI_LCD_SCK_AF                      GPIO_AF5_SPI1
#define SPI_LCD_MISO_PIN                    GPIO_PIN_6
#define SPI_LCD_MISO_GPIO_PORT              GPIOA
#define SPI_LCD_MISO_AF                     GPIO_AF5_SPI1
#define SPI_LCD_MOSI_PIN                    GPIO_PIN_7
#define SPI_LCD_MOSI_GPIO_PORT              GPIOA
#define SPI_LCD_MOSI_AF                     GPIO_AF5_SPI1
#define SPI_LCD_NSS_PIN                     GPIO_PIN_15
#define SPI_LCD_NSS_GPIO_PORT               GPIOA
#define SPI_LCD_NSS_AF                      GPIO_AF5_SPI1

#define SPI_LCD_RS_PIN            			GPIO_PIN_5
#define SPI_LCD_RS_GPIO_PORT                GPIOC

#define SPI_LCD_RESET_PIN					GPIO_PIN_14
#define SPI_LCD_RESET_GPIO_PORT				GPIOC

#define SPI_LCD_TX_DMA_STREAM				DMA2_Stream5
#define SPI_LCD_TX_DMA_CHANNEL				DMA_CHANNEL_3

#define SPI_LCD_DMA_TX_IRQn					DMA2_Stream5_IRQn
#define SPI_LCD_DMA_TX_IRQHandler           DMA2_Stream5_IRQHandler

extern SPI_HandleTypeDef SpiLcdHandle;

/* SDIO configuration defines */
#define SDIO_CLK_ENABLE()               __SDIO_CLK_ENABLE()
#define SDIO_D0_GPIO_CLK_ENABLE()		__GPIOC_CLK_ENABLE()
#define SDIO_D1_GPIO_CLK_ENABLE()		__GPIOC_CLK_ENABLE()
#define SDIO_D2_GPIO_CLK_ENABLE()		__GPIOC_CLK_ENABLE()
#define SDIO_D3_GPIO_CLK_ENABLE()		__GPIOC_CLK_ENABLE()
#define SDIO_CK_GPIO_CLK_ENABLE()		__GPIOC_CLK_ENABLE()
#define SDIO_CMD_GPIO_CLK_ENABLE()		__GPIOD_CLK_ENABLE()
#define SDIO_CARD_DETECT_GPIO_CLK_ENABLE() __GPIOB_CLK_ENABLE()


/* Definition for SDIO Pins */
#define SDIO_D0_PIN                     GPIO_PIN_8
#define SDIO_D0_GPIO_PORT               GPIOC
#define SDIO_D0_AF                      GPIO_AF12_SDIO
#define SDIO_D1_PIN                     GPIO_PIN_9
#define SDIO_D1_GPIO_PORT               GPIOC
#define SDIO_D1_AF                      GPIO_AF12_SDIO
#define SDIO_D2_PIN                     GPIO_PIN_10
#define SDIO_D2_GPIO_PORT               GPIOC
#define SDIO_D2_AF                      GPIO_AF12_SDIO
#define SDIO_D3_PIN                     GPIO_PIN_11
#define SDIO_D3_GPIO_PORT               GPIOC
#define SDIO_D3_AF                      GPIO_AF12_SDIO
#define SDIO_CK_PIN                     GPIO_PIN_12
#define SDIO_CK_GPIO_PORT               GPIOC
#define SDIO_CK_AF                      GPIO_AF12_SDIO
#define SDIO_CMD_PIN                    GPIO_PIN_2
#define SDIO_CMD_GPIO_PORT              GPIOD
#define SDIO_CMD_AF                     GPIO_AF12_SDIO
#define SDIO_CARD_DETECT_PIN			GPIO_PIN_14
#define SDIO_CARD_DETECT_GPIO_PORT		GPIOB


#define SDIO_DMAx_CLK_ENABLE()        __DMA2_CLK_ENABLE()
#define SDIO_DMAx_STREAM              DMA2_Stream6
#define SDIO_DMAx_CHANNEL             DMA_CHANNEL_4
#define SDIO_DMAx_IRQ                 SDIO_IRQn
#define SDIO_DMAx_PERIPH_DATA_SIZE    DMA_PDATAALIGN_WORD
#define SDIO_DMAx_MEM_DATA_SIZE       DMA_MDATAALIGN_WORD

/* I2S peripheral configuration defines */
#define AUDIO_I2Sx                          SPI2
#define AUDIO_I2Sx_CLK_ENABLE()             __SPI2_CLK_ENABLE()
#define AUDIO_I2Sx_SCK_SD_WS_AF             GPIO_AF5_SPI2
#define AUDIO_I2Sx_SCK_SD_WS_CLK_ENABLE()   __GPIOB_CLK_ENABLE()
#define AUDIO_I2Sx_MCK_CLK_ENABLE()         __GPIOC_CLK_ENABLE()
#define AUDIO_I2Sx_WS_PIN                   GPIO_PIN_12
#define AUDIO_I2Sx_SCK_PIN                  GPIO_PIN_13
#define AUDIO_I2Sx_SD_PIN                   GPIO_PIN_15
#define AUDIO_I2Sx_MCK_PIN                  GPIO_PIN_6
#define AUDIO_I2Sx_SCK_SD_WS_GPIO_PORT      GPIOB
#define AUDIO_I2Sx_MCK_GPIO_PORT            GPIOC

/* I2S DMA Stream definitions */
#define AUDIO_I2Sx_DMAx_CLK_ENABLE()        __DMA1_CLK_ENABLE()
#define AUDIO_I2Sx_DMAx_STREAM              DMA1_Stream4
#define AUDIO_I2Sx_DMAx_CHANNEL             DMA_CHANNEL_0
#define AUDIO_I2Sx_DMAx_IRQ                 DMA1_Stream4_IRQn
#define AUDIO_I2Sx_DMAx_PERIPH_DATA_SIZE    DMA_PDATAALIGN_HALFWORD
#define AUDIO_I2Sx_DMAx_MEM_DATA_SIZE       DMA_MDATAALIGN_HALFWORD //DMA_MDATAALIGN_HALFWORD
#define DMA_MAX_SZE                         0xFFFF

#define AUDIO_I2Sx_DMAx_IRQHandler          DMA1_Stream4_IRQHandler

#define DMA_SOUND_IT_ENABLE					HAL_NVIC_EnableIRQ(AUDIO_I2Sx_DMAx_IRQ)
#define DMA_SOUND_IT_DISABLE				HAL_NVIC_DisableIRQ(AUDIO_I2Sx_DMAx_IRQ)


/*------------------------------------------------------------------------------
             CONFIGURATION: Audio Driver Configuration parameters
------------------------------------------------------------------------------*/
/* Select the interrupt preemption priority for the DMA interrupt */
#define AUDIO_IRQ_PREPRIO           0   /* Select the preemption priority level(0 is the highest) */

#define AUDIODATA_SIZE              2   /* 16-bits audio data size */

/* Audio status definition */
#define AUDIO_OK         0x00
#define AUDIO_ERROR      0x01
#define AUDIO_TIMEOUT    0x02

#define DMA_MAX(x)           (((x) <= DMA_MAX_SZE)? (x):DMA_MAX_SZE)


/* Size of Trasmission buffer */
#define TXBUFFERSIZE                      (COUNTOF(aTxBuffer) - 1)
/* Size of Reception buffer */
#define RXBUFFERSIZE                      TXBUFFERSIZE/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


/* --- TIM1 SR Register ---*/
/* Alias word address of TIM1 SR UIF bit */
#define TIM1_SR_OFFSET         (TIM1_BASE - PERIPH_BASE + 0x10)
#define TIM1_SR_UIF_BitNumber  0x00
#define TIM1_SR_UIF_BB         (*(__IO uint32_t *)(PERIPH_BB_BASE + (TIM1_SR_OFFSET * 32) + (TIM1_SR_UIF_BitNumber * 4)))

/* --- TIM3 CR1 Register ---*/
/* Alias word address of TIM3 CR1 CEN bit */
#define TIM3_CR1_OFFSET         (TIM3_BASE - PERIPH_BASE + 0x00)
#define TIM3_CR1_CEN_BitNumber  0x00
#define TIM3_CR1_CEN_BB         (*(__IO uint32_t *)(PERIPH_BB_BASE + (TIM3_CR1_OFFSET * 32) + (TIM3_CR1_CEN_BitNumber * 4)))

/* --- TIM3 DIER Register ---*/
/* Alias word address of TIM3 DIER UIE bit */
#define TIM3_DIER_OFFSET         (TIM3_BASE - PERIPH_BASE + 0x0C)
#define TIM3_DIER_UIE_BitNumber  0x00
#define TIM3_DIER_UIE_BB         (*(__IO uint32_t *)(PERIPH_BB_BASE + (TIM3_DIER_OFFSET * 32) + (TIM3_DIER_UIE_BitNumber * 4)))

/* --- TIM3 SR Register ---*/
/* Alias word address of TIM3 SR UIF bit */
#define TIM3_SR_OFFSET         (TIM3_BASE - PERIPH_BASE + 0x10)
#define TIM3_SR_UIF_BitNumber  0x00
#define TIM3_SR_UIF_BB	       (*(__IO uint32_t *)(PERIPH_BB_BASE + (TIM3_SR_OFFSET << 5) + (TIM3_SR_UIF_BitNumber << 2)))


extern void SystemClock_Config(uint32_t pll_N, uint32_t pll_M);
extern uint8_t BSP_AUDIO_OUT_Init(uint16_t OutputDevice, uint8_t Volume, uint8_t bitPerSample, uint32_t AudioFreq);
extern void init_dac();
extern int picojpeg(int id, int play_mode);
extern uint8_t *pjpeg_load_from_file2(uint8_t *x, uint8_t *y, int *comps, pjpeg_scan_type_t *pScan_type, int reduce);
extern void Delay_us(int us);

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
