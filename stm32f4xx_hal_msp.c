/**
  ******************************************************************************
  * @file    UART/UART_TwoBoards_ComPolling/Src/stm32f4xx_hal_msp.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    26-February-2014
  * @brief   HAL MSP module.    
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
#include "main.h"
#include "usart.h"
#include "sd.h"
#include "lcd.h"

/** @addtogroup STM32F4xx_HAL_Examples
  * @{
  */

/** @defgroup UART_TwoBoards_ComPolling
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup HAL_MSP_Private_Functions
  * @{
  */

void HAL_MspInit(void)
{
	GPIO_InitTypeDef  GPIO_InitStruct;


	LCD_BKPWM_GPIO_CLK_ENABLE();
	GPIO_InitStruct.Pin = LCD_BKPWM_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Alternate = 0;
	HAL_GPIO_Init(LCD_BKPWM_GPIO_PORT, &GPIO_InitStruct);
	HAL_GPIO_WritePin(LCD_BKPWM_GPIO_PORT, LCD_BKPWM_PIN, GPIO_PIN_SET);


	AUDIO_I2Sx_SCK_SD_WS_CLK_ENABLE();
	AUDIO_I2Sx_MCK_CLK_ENABLE();

	GPIO_InitStruct.Pin = AUDIO_I2Sx_SCK_PIN | AUDIO_I2Sx_SD_PIN | AUDIO_I2Sx_WS_PIN;
	HAL_GPIO_Init(AUDIO_I2Sx_SCK_SD_WS_GPIO_PORT, &GPIO_InitStruct);

	HAL_GPIO_WritePin(AUDIO_I2Sx_SCK_SD_WS_GPIO_PORT, AUDIO_I2Sx_SCK_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(AUDIO_I2Sx_SCK_SD_WS_GPIO_PORT, AUDIO_I2Sx_SD_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(AUDIO_I2Sx_SCK_SD_WS_GPIO_PORT, AUDIO_I2Sx_WS_PIN, GPIO_PIN_SET);

	GPIO_InitStruct.Pin = AUDIO_I2Sx_MCK_PIN;
	HAL_GPIO_Init(AUDIO_I2Sx_MCK_GPIO_PORT, &GPIO_InitStruct);

	HAL_GPIO_WritePin(AUDIO_I2Sx_MCK_GPIO_PORT, AUDIO_I2Sx_MCK_PIN, GPIO_PIN_SET);


	I2Cx_SCL_GPIO_CLK_ENABLE();
	I2Cx_SDA_GPIO_CLK_ENABLE();

	GPIO_InitStruct.Pin = I2Cx_SDA_PIN;
	HAL_GPIO_Init(I2Cx_SDA_GPIO_PORT, &GPIO_InitStruct);

	HAL_GPIO_WritePin(I2Cx_SDA_GPIO_PORT, I2Cx_SDA_PIN, GPIO_PIN_SET);

	GPIO_InitStruct.Pin = I2Cx_SCL_PIN;
	HAL_GPIO_Init(I2Cx_SCL_GPIO_PORT, &GPIO_InitStruct);

	HAL_GPIO_WritePin(I2Cx_SCL_GPIO_PORT, I2Cx_SCL_PIN, GPIO_PIN_SET);


	SDIO_D0_GPIO_CLK_ENABLE();
	SDIO_D1_GPIO_CLK_ENABLE();
	SDIO_D2_GPIO_CLK_ENABLE();
	SDIO_D3_GPIO_CLK_ENABLE();
	SDIO_CK_GPIO_CLK_ENABLE();
	SDIO_CMD_GPIO_CLK_ENABLE();
	SDIO_CARD_DETECT_GPIO_CLK_ENABLE();

	GPIO_InitStruct.Pin = SDIO_D0_PIN | SDIO_D1_PIN | SDIO_D2_PIN | SDIO_D3_PIN | SDIO_CK_PIN;
	HAL_GPIO_Init(SDIO_D0_GPIO_PORT, &GPIO_InitStruct);

	HAL_GPIO_WritePin(SDIO_D0_GPIO_PORT, SDIO_D0_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(SDIO_D0_GPIO_PORT, SDIO_D1_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(SDIO_D0_GPIO_PORT, SDIO_D2_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(SDIO_D0_GPIO_PORT, SDIO_D3_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(SDIO_D0_GPIO_PORT, SDIO_CK_PIN, GPIO_PIN_SET);

	/* SDIO_CMD GPIO pin configuration  */
	GPIO_InitStruct.Pin = SDIO_CMD_PIN;
	HAL_GPIO_Init(SDIO_CMD_GPIO_PORT, &GPIO_InitStruct);

	HAL_GPIO_WritePin(SDIO_CMD_GPIO_PORT, SDIO_CMD_PIN, GPIO_PIN_SET);

	/* SDIO CARD DETECT */
	GPIO_InitStruct.Pin = SDIO_CARD_DETECT_PIN;
	HAL_GPIO_Init(SDIO_CARD_DETECT_GPIO_PORT, &GPIO_InitStruct);

	HAL_GPIO_WritePin(SDIO_CARD_DETECT_GPIO_PORT, SDIO_CARD_DETECT_PIN, GPIO_PIN_SET);


	SPI_LCD_SCK_GPIO_CLK_ENABLE();
	SPI_LCD_MISO_GPIO_CLK_ENABLE();
	SPI_LCD_MOSI_GPIO_CLK_ENABLE();
	SPI_LCD_NSS_GPIO_CLK_ENABLE();
	SPI_LCD_RS_GPIO_CLK_ENABLE();
	SPI_LCD_RESET_GPIO_CLK_ENABLE();

//	GPIO_InitStruct.Pin = SPI_LCD_SCK_PIN;
//	HAL_GPIO_Init(SPI_LCD_SCK_GPIO_PORT, &GPIO_InitStruct);
//	HAL_GPIO_WritePin(SPI_LCD_SCK_GPIO_PORT, SPI_LCD_SCK_PIN, GPIO_PIN_RESET);
//
//	GPIO_InitStruct.Pin = SPI_LCD_MISO_PIN;
//	HAL_GPIO_Init(SPI_LCD_MISO_GPIO_PORT, &GPIO_InitStruct);
//	HAL_GPIO_WritePin(SPI_LCD_MISO_GPIO_PORT, SPI_LCD_MISO_PIN, GPIO_PIN_RESET);
//
//	GPIO_InitStruct.Pin = SPI_LCD_MOSI_PIN;
//	HAL_GPIO_Init(SPI_LCD_MOSI_GPIO_PORT, &GPIO_InitStruct);
//	HAL_GPIO_WritePin(SPI_LCD_MOSI_GPIO_PORT, SPI_LCD_MOSI_PIN, GPIO_PIN_RESET);
//
//	GPIO_InitStruct.Pin = SPI_LCD_NSS_PIN;
//	HAL_GPIO_Init(SPI_LCD_NSS_GPIO_PORT, &GPIO_InitStruct);
//	HAL_GPIO_WritePin(SPI_LCD_NSS_GPIO_PORT, SPI_LCD_NSS_PIN, GPIO_PIN_RESET);
//
//	GPIO_InitStruct.Pin = SPI_LCD_RS_PIN;
//	HAL_GPIO_Init(SPI_LCD_RS_GPIO_PORT, &GPIO_InitStruct);
//	HAL_GPIO_WritePin(SPI_LCD_RS_GPIO_PORT, SPI_LCD_RS_PIN, GPIO_PIN_RESET);
//
//	GPIO_InitStruct.Pin = SPI_LCD_RESET_PIN;
//	HAL_GPIO_Init(SPI_LCD_RESET_GPIO_PORT, &GPIO_InitStruct);
//	HAL_GPIO_WritePin(SPI_LCD_RESET_GPIO_PORT, SPI_LCD_RESET_PIN, GPIO_PIN_RESET);
//
//	Delay_us(1000);

	GPIO_InitStruct.Pin = SPI_LCD_SCK_PIN;
	HAL_GPIO_Init(SPI_LCD_SCK_GPIO_PORT, &GPIO_InitStruct);
	HAL_GPIO_WritePin(SPI_LCD_SCK_GPIO_PORT, SPI_LCD_SCK_PIN, GPIO_PIN_SET);

	GPIO_InitStruct.Pin = SPI_LCD_MISO_PIN;
	HAL_GPIO_Init(SPI_LCD_MISO_GPIO_PORT, &GPIO_InitStruct);
	HAL_GPIO_WritePin(SPI_LCD_MISO_GPIO_PORT, SPI_LCD_MISO_PIN, GPIO_PIN_SET);

	GPIO_InitStruct.Pin = SPI_LCD_MOSI_PIN;
	HAL_GPIO_Init(SPI_LCD_MOSI_GPIO_PORT, &GPIO_InitStruct);
	HAL_GPIO_WritePin(SPI_LCD_MOSI_GPIO_PORT, SPI_LCD_MOSI_PIN, GPIO_PIN_SET);

	GPIO_InitStruct.Pin = SPI_LCD_NSS_PIN;
	HAL_GPIO_Init(SPI_LCD_NSS_GPIO_PORT, &GPIO_InitStruct);
	HAL_GPIO_WritePin(SPI_LCD_NSS_GPIO_PORT, SPI_LCD_NSS_PIN, GPIO_PIN_SET);

	GPIO_InitStruct.Pin = SPI_LCD_RS_PIN;
	HAL_GPIO_Init(SPI_LCD_RS_GPIO_PORT, &GPIO_InitStruct);
	HAL_GPIO_WritePin(SPI_LCD_RS_GPIO_PORT, SPI_LCD_RS_PIN, GPIO_PIN_SET);

	GPIO_InitStruct.Pin = SPI_LCD_RESET_PIN;
	HAL_GPIO_Init(SPI_LCD_RESET_GPIO_PORT, &GPIO_InitStruct);
	HAL_GPIO_WritePin(SPI_LCD_RESET_GPIO_PORT, SPI_LCD_RESET_PIN, GPIO_PIN_SET);

	Delay_us(1000);

	WM8731_SUPPLY_GPIO_CLK_ENABLE();
	GPIO_InitStruct.Pin = WM8731_SUPPLY_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	GPIO_InitStruct.Alternate = 0;
	HAL_GPIO_Init(WM8731_SUPPLY_GPIO_PORT, &GPIO_InitStruct);
	HAL_GPIO_WritePin(WM8731_SUPPLY_GPIO_PORT, WM8731_SUPPLY_PIN, GPIO_PIN_SET);

	Delay_us(3000);
}

/**
  * @brief I2C MSP Initialization
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration
  *           - DMA configuration for transmission request by peripheral
  *           - NVIC configuration for DMA interrupt request enable
  * @param hi2c: I2C handle pointer
  * @retval None
  */
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  /*##-1- Enable GPIO Clocks #################################################*/
  /* Enable GPIO TX/RX clock */
  I2Cx_SCL_GPIO_CLK_ENABLE();
  I2Cx_SDA_GPIO_CLK_ENABLE();

  /*##-2- Configure peripheral GPIO ##########################################*/
  /* I2C TX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = I2Cx_SCL_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
  GPIO_InitStruct.Alternate = I2Cx_SCL_AF;
  HAL_GPIO_Init(I2Cx_SCL_GPIO_PORT, &GPIO_InitStruct);

  /* I2C RX GPIO pin configuration  */
  GPIO_InitStruct.Pin = I2Cx_SDA_PIN;
  GPIO_InitStruct.Alternate = I2Cx_SDA_AF;
  HAL_GPIO_Init(I2Cx_SDA_GPIO_PORT, &GPIO_InitStruct);

  /*##-3- Enable I2C peripheral Clock ########################################*/
  /* Enable I2C1 clock */
  I2Cx_CLK_ENABLE();
}

/**
  * @brief I2C MSP De-Initialization
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO, DMA and NVIC configuration to their default state
  * @param hi2c: I2C handle pointer
  * @retval None
  */
void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{
  /*##-1- Reset peripherals ##################################################*/
  I2Cx_FORCE_RESET();
  I2Cx_RELEASE_RESET();

  /*##-2- Disable peripherals and GPIO Clocks ################################*/
  /* Configure I2C Tx as alternate function  */
  HAL_GPIO_DeInit(I2Cx_SCL_GPIO_PORT, I2Cx_SCL_PIN);
  /* Configure I2C Rx as alternate function  */
  HAL_GPIO_DeInit(I2Cx_SDA_GPIO_PORT, I2Cx_SDA_PIN);
}

/**
  * @brief UART MSP Initialization 
  *        This function configures the hardware resources used in this example: 
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration  
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{  
  GPIO_InitTypeDef  GPIO_InitStruct;
  
  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO TX/RX clock */
  USARTx_TX_GPIO_CLK_ENABLE();
  USARTx_RX_GPIO_CLK_ENABLE();
  /* Enable USART2 clock */
  USARTx_CLK_ENABLE(); 
  
  /*##-2- Configure peripheral GPIO ##########################################*/  
  /* UART TX GPIO pin configuration  */
  GPIO_InitStruct.Pin       = USARTx_TX_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
  GPIO_InitStruct.Alternate = USARTx_TX_AF;
  
  HAL_GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStruct);
    
  /* UART RX GPIO pin configuration  */
  GPIO_InitStruct.Pin = USARTx_RX_PIN;
  GPIO_InitStruct.Alternate = USARTx_RX_AF;
    
  HAL_GPIO_Init(USARTx_RX_GPIO_PORT, &GPIO_InitStruct);

  /*##-4- Configure the NVIC for USART #########################################*/
  /* NVIC configuration for USART2 interrupt */
  HAL_NVIC_SetPriority(USARTx_IRQn, 14, 0);
//  HAL_NVIC_EnableIRQ(USARTx_IRQn);
}

/**
  * @brief UART MSP De-Initialization 
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO configuration to their default state
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
  /*##-1- Reset peripherals ##################################################*/
  USARTx_FORCE_RESET();
  USARTx_RELEASE_RESET();

  /*##-2- Disable peripherals and GPIO Clocks #################################*/
  /* Configure UART Tx as alternate function  */
  HAL_GPIO_DeInit(USARTx_TX_GPIO_PORT, USARTx_TX_PIN);
  /* Configure UART Rx as alternate function  */
  HAL_GPIO_DeInit(USARTx_RX_GPIO_PORT, USARTx_RX_PIN);
}


/**
  * @brief SPI MSP Initialization
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration
  * @param hspi: SPI handle pointer
  * @retval None
  */
void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
  static DMA_HandleTypeDef hdma_tx;

  GPIO_InitTypeDef  GPIO_InitStruct;

  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO clock */
  SPI_LCD_SCK_GPIO_CLK_ENABLE();
  SPI_LCD_MISO_GPIO_CLK_ENABLE();
  SPI_LCD_MOSI_GPIO_CLK_ENABLE();
  SPI_LCD_NSS_GPIO_CLK_ENABLE();
  SPI_LCD_RS_GPIO_CLK_ENABLE();
  SPI_LCD_RESET_GPIO_CLK_ENABLE();
  /* Enable SPI LCD clock */
  SPI_LCD_CLK_ENABLE();
  /* Enable SPI LCD DMA clock */
  DMA_SPI_LCD_CLK_ENABLE();

  /*##-2- Configure peripheral GPIO ##########################################*/
  /* SPI SCK GPIO pin configuration  */
  GPIO_InitStruct.Pin       = SPI_LCD_SCK_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
  GPIO_InitStruct.Alternate = SPI_LCD_SCK_AF;

  HAL_GPIO_Init(SPI_LCD_SCK_GPIO_PORT, &GPIO_InitStruct);

  /* SPI MISO GPIO pin configuration  */
//  GPIO_InitStruct.Pin = SPI_LCD_MISO_PIN;
//  GPIO_InitStruct.Pull      = GPIO_PULLDOWN;
//  GPIO_InitStruct.Alternate = SPI_LCD_MISO_AF;

//  HAL_GPIO_Init(SPI_LCD_MISO_GPIO_PORT, &GPIO_InitStruct);

  /* SPI MOSI GPIO pin configuration  */
  GPIO_InitStruct.Pin = SPI_LCD_MOSI_PIN;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Alternate = SPI_LCD_MOSI_AF;

  HAL_GPIO_Init(SPI_LCD_MOSI_GPIO_PORT, &GPIO_InitStruct);

  /* SPI NSS GPIO pin configuration  */
  GPIO_InitStruct.Pin = SPI_LCD_NSS_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Alternate = 0;//SPI_LCD_NSS_AF;

  HAL_GPIO_Init(SPI_LCD_NSS_GPIO_PORT, &GPIO_InitStruct);

  /* SPI RS GPIO pin configration */
  GPIO_InitStruct.Pin = SPI_LCD_RS_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Alternate = 0;

  HAL_GPIO_Init(SPI_LCD_RS_GPIO_PORT, &GPIO_InitStruct);

  /* SPI RESET GPIO pin configuration */
  GPIO_InitStruct.Pin = SPI_LCD_RESET_PIN;
  GPIO_InitStruct.Alternate = 0;

  HAL_GPIO_Init(SPI_LCD_RESET_GPIO_PORT, &GPIO_InitStruct);

  /*##-3- Configure the DMA streams ##########################################*/
  /* Configure the DMA handler for Transmission process */
  hdma_tx.Instance                 = SPI_LCD_TX_DMA_STREAM;

  hdma_tx.Init.Channel             = SPI_LCD_TX_DMA_CHANNEL;
  hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
  hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
  hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;
  hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
  hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
  hdma_tx.Init.Mode                = DMA_NORMAL;
  hdma_tx.Init.Priority            = DMA_PRIORITY_LOW;
  hdma_tx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
  hdma_tx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
  hdma_tx.Init.MemBurst            = DMA_MBURST_INC4;
  hdma_tx.Init.PeriphBurst         = DMA_PBURST_INC4;

  HAL_DMA_Init(&hdma_tx);

  /* Associate the initialized DMA handle to the the SPI handle */
  __HAL_LINKDMA(hspi, hdmatx, hdma_tx);

  /*##-4- Configure the NVIC for DMA #########################################*/
  /* NVIC configuration for DMA transfer complete interrupt (SPI3_TX) */
  HAL_NVIC_SetPriority(SPI_LCD_DMA_TX_IRQn, 0, 1);
  HAL_NVIC_EnableIRQ(SPI_LCD_DMA_TX_IRQn);
}


void HAL_SD_MspInit(SD_HandleTypeDef *hsd)
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  /* Enable GPIO clock */
  SDIO_D0_GPIO_CLK_ENABLE();
  SDIO_D1_GPIO_CLK_ENABLE();
  SDIO_D2_GPIO_CLK_ENABLE();
  SDIO_D3_GPIO_CLK_ENABLE();
  SDIO_CK_GPIO_CLK_ENABLE();
  SDIO_CMD_GPIO_CLK_ENABLE();
  SDIO_CARD_DETECT_GPIO_CLK_ENABLE();
  /* Enable SDIO clock */
  SDIO_CLK_ENABLE();


  /*##-2- Configure peripheral GPIO ##########################################*/
  /* SDIO_D0-D3 & SDIO_CK GPIO pin configuration  */
  GPIO_InitStruct.Pin       = SDIO_D0_PIN | SDIO_D1_PIN | SDIO_D2_PIN | SDIO_D3_PIN | SDIO_CK_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
  GPIO_InitStruct.Alternate = SDIO_D0_AF;

  HAL_GPIO_Init(SDIO_D0_GPIO_PORT, &GPIO_InitStruct);

  /* SDIO_CMD GPIO pin configuration  */
  GPIO_InitStruct.Pin       = SDIO_CMD_PIN;

  HAL_GPIO_Init(SDIO_CMD_GPIO_PORT, &GPIO_InitStruct);


  /* SDIO CARD DETECT */
  GPIO_InitStruct.Pin       = SDIO_CARD_DETECT_PIN;
  GPIO_InitStruct.Mode      = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull      = GPIO_PULLUP;
  GPIO_InitStruct.Speed     = GPIO_SPEED_LOW;
  GPIO_InitStruct.Alternate = 0;

  HAL_GPIO_Init(SDIO_CARD_DETECT_GPIO_PORT, &GPIO_InitStruct);

  /* Enable and set EXTI Line15_10 Interrupt to the lowest priority */
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

//  static DMA_HandleTypeDef hdma_sdRx;
//
//  /* Enable the DMA clock */
//  SDIO_DMAx_CLK_ENABLE();
//
//  /* Configure the hdma_sdRx handle parameters */
//  hdma_sdRx.Init.Channel             = SDIO_DMAx_CHANNEL;
//  hdma_sdRx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
//  hdma_sdRx.Init.PeriphInc           = DMA_PINC_DISABLE;
//  hdma_sdRx.Init.MemInc              = DMA_MINC_ENABLE;
//  hdma_sdRx.Init.PeriphDataAlignment = SDIO_DMAx_PERIPH_DATA_SIZE;
//  hdma_sdRx.Init.MemDataAlignment    = SDIO_DMAx_MEM_DATA_SIZE;
//  hdma_sdRx.Init.Mode                = DMA_PFCTRL;
//  hdma_sdRx.Init.Priority            = DMA_PRIORITY_VERY_HIGH;
//  hdma_sdRx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
//  hdma_sdRx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
//  hdma_sdRx.Init.MemBurst            = DMA_MBURST_SINGLE;
//  hdma_sdRx.Init.PeriphBurst         = DMA_PBURST_SINGLE;
//
//  hdma_sdRx.Instance = SDIO_DMAx_STREAM;
//
//  /* Associate the DMA handle */
//  __HAL_LINKDMA(hsd, hdmarx, hdma_sdRx);
//
//  /* Deinitialize the Stream for new transfer */
//  HAL_DMA_DeInit(&hdma_sdRx);
//
//  /* Configure the DMA Stream */
//  HAL_DMA_Init(&hdma_sdRx);
//
//  HAL_NVIC_SetPriority(SDIO_DMAx_IRQ, 0, 0);
//  HAL_NVIC_EnableIRQ(SDIO_DMAx_IRQ);
}

/*
void HAL_SD_DMA_RxCpltCallback(DMA_HandleTypeDef *hdma)
{
	debug.printf("\r\nHAL_SD_DMA_RxCpltCallback");
}

void HAL_SD_DMA_RxErrorCallback(DMA_HandleTypeDef *hdma)
{
	debug.printf("\r\nHAL_SD_DMA_RxErrorCallback:%d", hdma->ErrorCode);
}
*/


void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
	GPIO_InitTypeDef  GPIO_InitStruct;

	if(htim->Instance == TIM_LCD_PWM){
		TIM_LCD_PWM_CLK_ENABLE();

		GPIO_LCD_PWM_CLK_ENABLE();

		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;

		GPIO_InitStruct.Pin = GPIO_PIN_CHANNEL2;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	}
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
	GPIO_InitTypeDef  GPIO_InitStruct;

	if(htim->Instance == TIM_PROGRESS_BAR){
		TIM_PROGRESS_BAR_CLK_ENABLE();

		HAL_NVIC_SetPriority(TIM_PROGRESS_BAR_IRQn, 2, 0);
//		HAL_NVIC_EnableIRQ(TIM_PROGRESS_BAR_IRQn);
	}


	if(htim->Instance == TIM_1SEC){
		TIM_1SEC_CLK_ENABLE();

		HAL_NVIC_SetPriority(TIM_1SEC_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(TIM_1SEC_IRQn);
	}

	if(htim->Instance == TIM_DURATION){
		TIM_DURATION_CLK_ENABLE();
	}

	if(htim->Instance == TIM_SW_HOLD){
		TIM_SW_HOLD_CLK_ENABLE();


		HAL_NVIC_SetPriority(TIM_SW_HOLD_IRQn, 2, 0);
		HAL_NVIC_EnableIRQ(TIM_SW_HOLD_IRQn);
	}

	if(htim->Instance == TIM_DEBOUNCE){
		extern uint16_t sw_pushed, sw_pressing, sw_touched;
		sw_pushed = 0, sw_pressing = 0, sw_touched = 0;

		TIM_DEBOUNCE_CLK_ENABLE();

		HAL_NVIC_SetPriority(TIM_DEBOUNCE_IRQn, 2, 0);
		HAL_NVIC_EnableIRQ(TIM_DEBOUNCE_IRQn);

		HAL_TIM_Base_Stop_IT(htim);

		SW_PUSH_ENTER_CLK_ENABLE();
		SW_PUSH_LEFT_CLK_ENABLE();
		SW_PUSH_RIGHT_CLK_ENABLE();
		SW_PUSH_UP_CLK_ENABLE();
		SW_PUSH_DOWN_CLK_ENABLE();

		GPIO_InitStruct.Pin = SW_PUSH_ENTER_PIN;
		GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
		GPIO_InitStruct.Alternate = 0;
		HAL_GPIO_Init(SW_PUSH_ENTER_PORT, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = SW_PUSH_LEFT_PIN;
		HAL_GPIO_Init(SW_PUSH_LEFT_PORT, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = SW_PUSH_RIGHT_PIN;
		HAL_GPIO_Init(SW_PUSH_RIGHT_PORT, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = SW_PUSH_DOWN_PIN;
		HAL_GPIO_Init(SW_PUSH_DOWN_PORT, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = SW_PUSH_UP_PIN;
		HAL_GPIO_Init(SW_PUSH_UP_PORT, &GPIO_InitStruct);

		/* Enable and set EXTI Line0 Interrupt to the lowest priority */
		HAL_NVIC_SetPriority(EXTI0_IRQn, 2, 0);
		HAL_NVIC_EnableIRQ(EXTI0_IRQn);

		/* Enable and set EXTI Line1 Interrupt to the lowest priority */
		HAL_NVIC_SetPriority(EXTI1_IRQn, 2, 0);
		HAL_NVIC_EnableIRQ(EXTI1_IRQn);

		/* Enable and set EXTI Line2 Interrupt to the lowest priority */
		HAL_NVIC_SetPriority(EXTI2_IRQn, 2, 0);
		HAL_NVIC_EnableIRQ(EXTI2_IRQn);

		/* Enable and set EXTI Line4 Interrupt to the lowest priority */
		HAL_NVIC_SetPriority(EXTI4_IRQn, 2, 0);
		HAL_NVIC_EnableIRQ(EXTI4_IRQn);

		/* Enable and set EXTI Line9_5 Interrupt to the lowest priority */
		HAL_NVIC_SetPriority(EXTI9_5_IRQn, 2, 0);
		HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

		__HAL_TIM_CLEAR_IT(&TimHandleSwHold, TIM_IT_UPDATE);
		TIM_SW_HOLD->CNT = 0;
	}
}

/**
  * @brief SPI MSP De-Initialization
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO configuration to its default state
  * @param hspi: SPI handle pointer
  * @retval None
  */
void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{
  /*##-1- Reset peripherals ##################################################*/
  SPI_LCD_FORCE_RESET();
  SPI_LCD_RELEASE_RESET();

  /*##-2- Disable peripherals and GPIO Clocks ################################*/
  /* Configure SPI SCK as alternate function  */
  HAL_GPIO_DeInit(SPI_LCD_SCK_GPIO_PORT, SPI_LCD_SCK_PIN);
  /* Configure SPI MISO as alternate function  */
  HAL_GPIO_DeInit(SPI_LCD_MISO_GPIO_PORT, SPI_LCD_MISO_PIN);
  /* Configure SPI MOSI as alternate function  */
  HAL_GPIO_DeInit(SPI_LCD_MOSI_GPIO_PORT, SPI_LCD_MOSI_PIN);
}

void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
	if(hspi->Instance == SPI_LCD){
		debug.printf("\r\nHAL_SPI_ErrorCallback:%d", hspi->ErrorCode);
		SPI_LCD_NSS_PIN_DEASSERT;
	}
}

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if(hspi->ErrorCode != HAL_SPI_ERROR_NONE){
		debug.printf("\r\nHAL_SPI_TxCpltCallback:%d", hspi->ErrorCode);
	}

	if(hspi->Instance == SPI_LCD){
		SPI_LCD_NSS_PIN_DEASSERT;
	}
}


void HAL_I2S_MspInit(I2S_HandleTypeDef *hi2s)
{
	  static DMA_HandleTypeDef hdma_i2sTx;
	  GPIO_InitTypeDef  GPIO_InitStruct;

	  if(hi2s->Instance == AUDIO_I2Sx)
	  {

		/* Enable I2S clock */
		AUDIO_I2Sx_CLK_ENABLE();

		/* Enable SCK, SD and WS GPIO clock */
		AUDIO_I2Sx_SCK_SD_WS_CLK_ENABLE();

		/* CODEC_I2S pins configuration: WS, SCK and SD pins */
		GPIO_InitStruct.Pin = AUDIO_I2Sx_SCK_PIN;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
		GPIO_InitStruct.Alternate = AUDIO_I2Sx_SCK_SD_WS_AF;
		HAL_GPIO_Init(AUDIO_I2Sx_SCK_SD_WS_GPIO_PORT, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = AUDIO_I2Sx_SD_PIN;
		HAL_GPIO_Init(AUDIO_I2Sx_SCK_SD_WS_GPIO_PORT, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = AUDIO_I2Sx_WS_PIN;
		HAL_GPIO_Init(AUDIO_I2Sx_SCK_SD_WS_GPIO_PORT, &GPIO_InitStruct);

		/* Enable MCK GPIO clock */
		AUDIO_I2Sx_MCK_CLK_ENABLE();

		/* CODEC_I2S pins configuration: MCK pin */
		GPIO_InitStruct.Pin = AUDIO_I2Sx_MCK_PIN;
		HAL_GPIO_Init(AUDIO_I2Sx_MCK_GPIO_PORT, &GPIO_InitStruct);

		/* Enable the DMA clock */
		AUDIO_I2Sx_DMAx_CLK_ENABLE();

	    /* Configure the hdma_i2sTx handle parameters */
	    hdma_i2sTx.Init.Channel             = AUDIO_I2Sx_DMAx_CHANNEL;
	    hdma_i2sTx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
	    hdma_i2sTx.Init.PeriphInc           = DMA_PINC_DISABLE;
	    hdma_i2sTx.Init.MemInc              = DMA_MINC_ENABLE;
	    hdma_i2sTx.Init.PeriphDataAlignment = AUDIO_I2Sx_DMAx_PERIPH_DATA_SIZE;
	    hdma_i2sTx.Init.MemDataAlignment    = AUDIO_I2Sx_DMAx_MEM_DATA_SIZE;
	    hdma_i2sTx.Init.Mode                = DMA_CIRCULAR;
	    hdma_i2sTx.Init.Priority            = DMA_PRIORITY_HIGH;
	    hdma_i2sTx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
	    hdma_i2sTx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
	    hdma_i2sTx.Init.MemBurst            = DMA_MBURST_SINGLE;
	    hdma_i2sTx.Init.PeriphBurst         = DMA_PBURST_SINGLE;

	    hdma_i2sTx.Instance = AUDIO_I2Sx_DMAx_STREAM;

	    /* Associate the DMA handle */
	    __HAL_LINKDMA(hi2s, hdmatx, hdma_i2sTx);

	    /* Deinitialize the Stream for new transfer */
	    HAL_DMA_DeInit(&hdma_i2sTx);

	    /* Configure the DMA Stream */
	    HAL_DMA_Init(&hdma_i2sTx);
	  }

	  /* I2S DMA IRQ Channel configuration */
	  HAL_NVIC_SetPriority(AUDIO_I2Sx_DMAx_IRQ, AUDIO_IRQ_PREPRIO, 0);
	  HAL_NVIC_DisableIRQ(AUDIO_I2Sx_DMAx_IRQ);
}


/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
