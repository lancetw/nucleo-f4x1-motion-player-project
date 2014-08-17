/*
 * Nucleo F4x1 Motion Player
 * by Tonsuke
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

#include "sd.h"
#include "lcd.h"
#include "usart.h"
#include "sound.h"
#include "pcf_font.h"
#include "wm8731.h"
#include "HMC5883L.h"
#include "settings.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

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
TIM_HandleTypeDef TimHandleDebounce;
TIM_HandleTypeDef TimHandleSwHold;
TIM_HandleTypeDef Tim1SecHandle;
TIM_HandleTypeDef TimProgressBar;
TIM_HandleTypeDef TimLcdPwm;

int time;

I2C_HandleTypeDef I2cHandle;

I2S_HandleTypeDef haudio_i2s;

#define I2S_STANDARD                  I2S_STANDARD_PHILLIPS

int8_t current_mode; // 0:Filer 1:Music


/* Private function prototypes -----------------------------------------------*/
static void Error_Handler(void);

/* Private functions ---------------------------------------------------------*/

void mem_clean(){
	void *p;
	p = malloc(28000);
	memset(p, 0, 28000);
	free(p);
}

void Delay_us(int us)
{
	volatile int delay;

	delay = (SystemCoreClock / 1000000) * us;

	while(delay-- > 0)
	{
		__NOP();
	}

}

char* ftostr(char* buffer, float value, int places)
{
    int whole;
    int fraction;
    char sign[2] = "";

    if(value < 0)
    {
        value = -value;
        sign[0] = '-';
        sign[1] = '\0';
    }

    whole = (int)value;
    fraction = (int)((value - whole) * powf(10.0f,places) + 0.5f);
    sprintf(buffer, "%s%d.%*.*d", sign, whole, places, places, fraction);

    return buffer;
}

/* These PLL parameters are valide when the f(VCO clock) = 1Mhz */
const uint32_t I2SFreq[] = {8000, 11025, 16000, 22050, 32000, 44100, 48000, 96000, 192000};
const uint32_t I2SPLLN[] = {256, 429, 213, 429, 426, 271, 258, 344, 344};
const uint32_t I2SPLLR[] = {5, 4, 4, 4, 4, 6, 3, 2, 2};


static void I2Sx_Init(uint8_t bitPerSample, uint32_t AudioFreq)
{
  HAL_StatusTypeDef errorStatus;

  /* Initialize the haudio_i2s Instance parameter */
  haudio_i2s.Instance = AUDIO_I2Sx;

 /* Disable I2S block */
  __HAL_I2S_DISABLE(&haudio_i2s);

  haudio_i2s.Init.Mode = I2S_MODE_MASTER_TX;
  haudio_i2s.Init.Standard = I2S_STANDARD_PHILLIPS;
  haudio_i2s.Init.DataFormat = bitPerSample > 16 ? I2S_DATAFORMAT_32B : I2S_DATAFORMAT_16B;
  haudio_i2s.Init.AudioFreq = AudioFreq;
  haudio_i2s.Init.CPOL = I2S_CPOL_LOW;
  haudio_i2s.Init.ClockSource = I2S_CLOCK_PLL;
  haudio_i2s.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
  haudio_i2s.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;

  /* Init the I2S */
  HAL_I2S_DeInit(&haudio_i2s);
  errorStatus = HAL_I2S_Init(&haudio_i2s);
  debug.printf("\r\nHAL_I2S_Init:%d", errorStatus);

}


uint8_t BSP_AUDIO_OUT_Init(uint16_t OutputDevice, uint8_t Volume, uint8_t bitPerSample, uint32_t AudioFreq)
{
	  uint8_t ret = AUDIO_ERROR, cnt;
	  RCC_PeriphCLKInitTypeDef RCC_ExCLKInitStruct;
	  uint8_t index = 0, freqindex = 0xFF;
	  uint16_t dummy[100];

	  memset(dummy, 0, sizeof(dummy));

	  for(cnt = 0;cnt < 2;cnt++){
		  HAL_I2S_Transmit(&haudio_i2s, dummy, sizeof(dummy) / sizeof(dummy[0]), 100);
		  HAL_Delay(10);

		  wm8731_set_active(0);
		  if(bitPerSample <= 16){
			  wm8731_set_digital_audio_data_bit_length(WM8731_REG_DIGITAL_AUDIO_INTERFACE_FORMAT_IWL_16_BIT);
		  } else {
			  wm8731_set_digital_audio_data_bit_length(WM8731_REG_DIGITAL_AUDIO_INTERFACE_FORMAT_IWL_32_BIT);
		  }
		  wm8731_set_active(1);
	  }

	  HAL_StatusTypeDef errorStatus;

	  for(index = 0; index < sizeof(I2SFreq) / sizeof(I2SFreq[0]); index++)
	  {
	    if(I2SFreq[index] == AudioFreq)
	    {
	      freqindex = index;
	    }
	  }

	  HAL_RCCEx_GetPeriphCLKConfig(&RCC_ExCLKInitStruct);

	  if(freqindex != 0xFF)
	  {
	    /* I2S clock config
	    PLLI2S_VCO = f(VCO clock) = f(PLLI2S clock input) ◊ (PLLI2SN/PLLM)
	    I2SCLK = f(PLLI2S clock output) = f(VCO clock) / PLLI2SR */
	    RCC_ExCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S;
	    RCC_ExCLKInitStruct.PLLI2S.PLLI2SN = I2SPLLN[freqindex];
	    RCC_ExCLKInitStruct.PLLI2S.PLLI2SR = I2SPLLR[freqindex];
	    RCC_ExCLKInitStruct.PLLI2S.PLLI2SM = 16;
	    errorStatus = HAL_RCCEx_PeriphCLKConfig(&RCC_ExCLKInitStruct);
	  }
	  else /* default PLL I2S configuration */
	  {
	    /* I2S clock config
	    PLLI2S_VCO = f(VCO clock) = f(PLLI2S clock input) ◊ (PLLI2SN/PLLM)
	    I2SCLK = f(PLLI2S clock output) = f(VCO clock) / PLLI2SR */
	    RCC_ExCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S;
	    RCC_ExCLKInitStruct.PLLI2S.PLLI2SN = 258;
	    RCC_ExCLKInitStruct.PLLI2S.PLLI2SR = 3;
	    RCC_ExCLKInitStruct.PLLI2S.PLLI2SM = 16;
	    errorStatus = HAL_RCCEx_PeriphCLKConfig(&RCC_ExCLKInitStruct);
	  }

	  ret = AUDIO_OK;
	  I2Sx_Init(bitPerSample, AudioFreq);

	  return ret;
}

void enterStopMode()
{
	  card_info_typedef cardInfo_cp;

	  memcpy((void*)&cardInfo_cp, (void*)&cardInfo, sizeof(cardInfo));

	  uint32_t GPIOA_BKBUFF[10];
	  uint32_t GPIOB_BKBUFF[10];
	  uint32_t GPIOC_BKBUFF[10];
	  uint32_t GPIOD_BKBUFF[10];
	  uint32_t GPIOH_BKBUFF[10];

	  HAL_NVIC_DisableIRQ(TIM_1SEC_IRQn);

	  LCDSetPWMValue(0);
	  LCD_DisplayOff();
	  LCD_SleepIn();

	  wm8731_set_power_off(1);

	  memcpy((void*)GPIOA_BKBUFF, (void*)GPIOA_BASE, sizeof(GPIOA_BKBUFF));
	  memcpy((void*)GPIOB_BKBUFF, (void*)GPIOB_BASE, sizeof(GPIOB_BKBUFF));
	  memcpy((void*)GPIOC_BKBUFF, (void*)GPIOC_BASE, sizeof(GPIOC_BKBUFF));
	  memcpy((void*)GPIOD_BKBUFF, (void*)GPIOD_BASE, sizeof(GPIOD_BKBUFF));
	  memcpy((void*)GPIOH_BKBUFF, (void*)GPIOH_BASE, sizeof(GPIOH_BKBUFF));

	  debug.printf("\r\nsleep in");

	  GPIO_InitTypeDef GPIO_InitStruct;

	  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Pin = GPIO_PIN_All;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

	  GPIO_InitStruct.Pin = ~SW_PUSH_LEFT_PIN & ~SDIO_CARD_DETECT_PIN;
	  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

//	  GPIO_InitStruct.Pin = ~WM8731_SUPPLY_PIN & ~SW_PUSH_ENTER_PIN & ~SW_PUSH_RIGHT_PIN & ~SW_PUSH_UP_PIN & ~SW_PUSH_DOWN_PIN;
	  GPIO_InitStruct.Pin = ~SW_PUSH_ENTER_PIN & ~SW_PUSH_RIGHT_PIN & ~SW_PUSH_UP_PIN & ~SW_PUSH_DOWN_PIN;

	  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	  HAL_PWREx_EnableFlashPowerDown();

	  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

	  sleep_time.flags.wakeup = 1;

	  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	  GPIO_InitStruct.Pull = GPIO_NOPULL;
	  GPIO_InitStruct.Pin = GPIO_PIN_All;
	  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

	  /* Configure the system clock */
	  SystemClock_Config(336, 16);

	  HAL_EnableCompensationCell();

	  HAL_Delay(200);

	  HAL_MspInit();

	  memcpy((void*)GPIOA_BASE, (void*)GPIOA_BKBUFF, sizeof(GPIOA_BKBUFF));
	  memcpy((void*)GPIOB_BASE, (void*)GPIOB_BKBUFF, sizeof(GPIOB_BKBUFF));
	  memcpy((void*)GPIOC_BASE, (void*)GPIOC_BKBUFF, sizeof(GPIOC_BKBUFF));
	  memcpy((void*)GPIOD_BASE, (void*)GPIOD_BKBUFF, sizeof(GPIOD_BKBUFF));
	  memcpy((void*)GPIOH_BASE, (void*)GPIOH_BKBUFF, sizeof(GPIOH_BKBUFF));

	  HAL_Delay(10);

	  debug.printf("\r\nwake up");

	  sleep_time.flags.dimLight = 0;
	  sleep_time.flags.stop_mode = 0;

	  sleep_time.curTime = 0, sleep_time.prevTime = 0;

	  init_dac();

	  int sdRes = SDInit();
	  if(sdRes == 0)
	  {
		  memcpy((void*)&cardInfo, (void*)&cardInfo_cp, sizeof(cardInfo));
	  } else if(sdRes == -1) {
		  const IWDG_HandleTypeDef iwdg = { \
		  	.Instance = IWDG
		  };
		  __HAL_IWDG_START(&iwdg);
		  while(1);
	  }

	  LCD_SleepOut();
	  HAL_Delay(200);
	  LCD_Config();
	  LCD_FRAME_BUFFER_Transmit(LCD_DMA_TRANSMIT_COMPBLOCKING);

	  LCDSetPWMValue(settings_group.disp_conf.brightness);

	  sleep_time.flags.wakeup = 0;

	  HAL_TIM_Base_Start_IT(&Tim1SecHandle);
	  HAL_NVIC_EnableIRQ(TIM_1SEC_IRQn);
}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  GPIO_InitTypeDef  GPIO_InitStruct;
  int16_t ret, prevRet, cnt;
 /* This sample code shows how to use STM32F4xx GPIO HAL API to toggle PA05 IOs 
    connected to LED2 on STM32F4xx-Nucleo board  
    in an infinite loop.
    To proceed, 3 steps are required: */

  /* STM32F4xx HAL library initialization:
       - Configure the Flash prefetch, instruction and Data caches
       - Configure the Systick to generate an interrupt each 1 msec
       - Set NVIC Group Priority to 4
       - Global MSP (MCU Support Package) initialization
     */
  HAL_Init();


  /* Configure the system clock */
  SystemClock_Config(336, 16);

//  HAL_EnableCompensationCell();

  //  /*##-1- Configure the I2C peripheral ######################################*/
  I2cHandle.Instance             = I2Cx;
  //
  I2cHandle.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
  I2cHandle.Init.ClockSpeed      = 100000;
  I2cHandle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLED;
  I2cHandle.Init.DutyCycle       = I2C_DUTYCYCLE_2;
  I2cHandle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLED;
  I2cHandle.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLED;
  I2cHandle.Init.OwnAddress1     = 0x33;
  I2cHandle.Init.OwnAddress2     = 0xFE;

  HAL_I2C_DeInit(&I2cHandle);

  if(HAL_I2C_Init(&I2cHandle) != HAL_OK)
  {
	  /* Initialization Error */
	  Error_Handler();
  }

  init_dac();

  mem_clean();

  current_mode = SW_MODE_FILER;

  USART_Init();
  SETTINGS_Init();

  LCD_Init();

  HAL_GPIO_WritePin(LCD_BKPWM_GPIO_PORT, LCD_BKPWM_PIN, GPIO_PIN_SET);
  LCDFadeIn(1, filer_bgimg_160x128);

  debug.printf("\r\n%s", __DATE__);
  debug.printf("\r\nSystemCoreClock:%d", SystemCoreClock);

  /* internal flash pcf font */
  C_PCFFontInit((uint32_t)internal_flash_pcf_font, (size_t)_sizeof_internal_flash_pcf_font);
  debug.printf("\r\ninternal flash font loaded.");
  LCD_FUNC.putChar = C_PCFPutChar;
  LCD_FUNC.putWideChar = C_PCFPutChar;
  LCD_FUNC.getCharLength = C_PCFGetCharPixelLength;
  pcf_font.ext_loaded = 0;

  extern uint16_t frame_buffer[160 * 128];
  char extensionName[4];

  HAL_SD_ErrorTypedef sdErrorState;

  sdErrorState = SDInit();
  debug.printf("\r\nSD_Init:%d", sdErrorState);

  ret = initFat();

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
  }


 LCDPrintFileList();

 extern LCDStatusStruct_typedef LCDStatusStruct;
 extern cursor_typedef cursor;
 LCDStatusStruct.waitExitKey = 0;

 TimLcdPwm.Instance = TIM_LCD_PWM;
 TimLcdPwm.Init.Period = LCD_PWM_PERIOD - 1;
 TimLcdPwm.Init.Prescaler = ((SystemCoreClock / 2) / 2100000) - 1;
 TimLcdPwm.Init.CounterMode = TIM_COUNTERMODE_UP;
 TimLcdPwm.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
 HAL_TIM_PWM_Init(&TimLcdPwm);

 /* Timer Output Compare Configuration Structure declaration */
 TIM_OC_InitTypeDef sConfig;

 sConfig.OCMode = TIM_OCMODE_PWM1;
 sConfig.OCPolarity = TIM_OCPOLARITY_HIGH;
 sConfig.OCFastMode = TIM_OCFAST_DISABLE;

 sConfig.Pulse = (float)settings_group.disp_conf.brightness / 100.0f * LCD_PWM_PERIOD - 1;
 HAL_TIM_PWM_ConfigChannel(&TimLcdPwm, &sConfig, TIM_CHANNEL_2);
 HAL_TIM_PWM_Start(&TimLcdPwm, TIM_CHANNEL_2);


 // switch input debounce timer setting HAL_TIM_Base_MspInit
 TimHandleDebounce.Instance = TIM_DEBOUNCE;
 HAL_TIM_Base_DeInit(&TimHandleDebounce);
 TimHandleDebounce.Init.Period = TIM_DEBOUNCE_MICROSECOND - 1;
 TimHandleDebounce.Init.Prescaler = (SystemCoreClock / 1000000UL) - 1; // 1us;
 TimHandleDebounce.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
 TimHandleDebounce.Init.CounterMode = TIM_COUNTERMODE_UP;
 HAL_TIM_Base_Init(&TimHandleDebounce);
 __HAL_TIM_CLEAR_IT(&TimHandleDebounce, TIM_IT_UPDATE);
 TIM_DEBOUNCE->CNT = 0;

 TimHandleSwHold.Instance = TIM_SW_HOLD;
 HAL_TIM_Base_DeInit(&TimHandleSwHold);
 TimHandleSwHold.Init.Period = TIM_SW_HOLD_INTERVAL - 1;
 TimHandleSwHold.Init.Prescaler = (SystemCoreClock / 1000UL) - 1; // 100us
 TimHandleSwHold.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
 TimHandleSwHold.Init.CounterMode = TIM_COUNTERMODE_UP;
 HAL_TIM_Base_Init(&TimHandleSwHold);
 __HAL_TIM_CLEAR_IT(&TimHandleSwHold, TIM_IT_UPDATE);
 TIM_SW_HOLD->CNT = 0;

 Tim1SecHandle.Instance = TIM_1SEC;
 HAL_TIM_Base_DeInit(&Tim1SecHandle);
 Tim1SecHandle.Init.Prescaler = 100 - 1;
 Tim1SecHandle.Init.Period = 10000 - 1;
 Tim1SecHandle.Init.RepetitionCounter = (SystemCoreClock / 1000000UL) - 1;
 Tim1SecHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
 Tim1SecHandle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
 HAL_TIM_Base_Init(&Tim1SecHandle);
 HAL_TIM_Base_Start_IT(&Tim1SecHandle);
 __HAL_TIM_CLEAR_IT(&Tim1SecHandle, TIM_IT_UPDATE);
 TIM_1SEC->CNT = 0;

 sleep_time.curTime = 0, sleep_time.prevTime = 0;
 sleep_time.flags.stop_mode = 0;
 sleep_time.flags.wakeup = 0;
 sleep_time.flags.enable = 1;

 music_control.b.navigation_loop_mode = NAV_PLAY_ENTIRE;
 music_control.b.mute = 0;

// HAL_NVIC_EnableIRQ(USARTx_IRQn);


  while (1)
  {
	  if(sleep_time.flags.stop_mode){
		  enterStopMode();
	  }

  	switch (LCDStatusStruct.waitExitKey) {
  		case FILE_TYPE_COMPASS:
			sleep_time.flags.enable = 0;

			current_mode = SW_MODE_PLAYER;
			HMC5883L();
			current_mode = SW_MODE_FILER;

			LCDPrintFileList();

			sleep_time.curTime = 0, sleep_time.prevTime = 0;
			sleep_time.flags.enable = 1;

  			break;
		case FILE_TYPE_WAV: // WAV
		case FILE_TYPE_MP3: // MP3
		case FILE_TYPE_AAC: // AAC
		case FILE_TYPE_MOV: // Motion Jpeg
			cnt = 0;
			shuffle_play.play_continuous = 0;
			shuffle_play.mode_changed = 0;
			shuffle_play.initial_mode = music_control.b.navigation_loop_mode;
			do{
				LCDFadeOut(2);
				current_mode = SW_MODE_PLAYER;
				sleep_time.flags.enable = 0;
				ret = PlayMusic(LCDStatusStruct.idEntry);
				sleep_time.curTime = 0, sleep_time.prevTime = 0;
				sleep_time.flags.enable = 1;
				debug.printf("\r\nret:%d LCDStatusStruct.idEntry:%d fat.fileCnt:%d navigation_loop.mode:%d", ret, LCDStatusStruct.idEntry, fat.fileCnt, music_control.b.navigation_loop_mode);
				switch(ret){
				case RET_PLAY_NORM:
					switch(music_control.b.navigation_loop_mode){
					case NAV_ONE_PLAY_EXIT:
						LCDStatusStruct.waitExitKey = 0;
						ret = RET_PLAY_STOP;
						break;
					case NAV_INFINITE_ONE_PLAY:
						debug.printf("\r\nNAV_INFINITE_ONE_PLAY:%d", LCDStatusStruct.idEntry);
						LCDStatusStruct.waitExitKey = 1;
						continue;
					case NAV_PLAY_ENTIRE:
					case NAV_INFINITE_PLAY_ENTIRE:
					case NAV_SHUFFLE_PLAY:
						goto PLAY_NEXT_ITEM;
					default:
						break;
					}
					break;
				case RET_PLAY_NEXT: // 次へ
			PLAY_NEXT_ITEM:
			if(shuffle_play.flag_make_rand && shuffle_play.mode_changed && (music_control.b.navigation_loop_mode != NAV_SHUFFLE_PLAY)){
				if(shuffle_play.initial_mode != NAV_SHUFFLE_PLAY){
					LCDStatusStruct.idEntry = shuffle_play.pRandIdEntry[LCDStatusStruct.idEntry];
				}
				shuffle_play.mode_changed = 0;
			}
			prevRet = LCDStatusStruct.idEntry;
					LCDStatusStruct.waitExitKey = 1;
					if(++LCDStatusStruct.idEntry >= fat.fileCnt){
						if(music_control.b.navigation_loop_mode == NAV_INFINITE_PLAY_ENTIRE || music_control.b.navigation_loop_mode == NAV_SHUFFLE_PLAY){
							LCDStatusStruct.idEntry = 1;
							LCDStatusStruct.waitExitKey = 1;
							continue;
						} else {
							LCDStatusStruct.idEntry--;
							LCDStatusStruct.waitExitKey = 0;
							ret = RET_PLAY_STOP;
						}
					}
					break;
				case RET_PLAY_PREV: // 前へ
					prevRet = RET_PLAY_PREV;
					LCDStatusStruct.waitExitKey = 1;
					if(--LCDStatusStruct.idEntry <= 0){
						LCDStatusStruct.idEntry = fat.fileCnt - 1;
					}
					continue;
				case RET_PLAY_INVALID: // invalid type
					ret = RET_PLAY_STOP;
					LCDStatusStruct.waitExitKey = 0;
					if(prevRet == RET_PLAY_PREV){
						LCDStatusStruct.idEntry++;
					} else {
						LCDStatusStruct.idEntry--;
					}
					break;
				default:
					break;
				}
				cnt++;
			}while((ret != RET_PLAY_STOP) || LCDStatusStruct.waitExitKey);
			if((cnt > 1) && shuffle_play.flag_make_rand && (music_control.b.navigation_loop_mode == NAV_SHUFFLE_PLAY)){
				LCDStatusStruct.idEntry = shuffle_play.pRandIdEntry[LCDStatusStruct.idEntry];
			}
			cursor.pos = LCDStatusStruct.idEntry % PAGE_NUM_ITEMS;
			cursor.pageIdx = LCDStatusStruct.idEntry / PAGE_NUM_ITEMS;
			current_mode = SW_MODE_FILER;
			LCDPrintFileList();
			break;
		case FILE_TYPE_JPG: // JPEG
			current_mode = SW_MODE_PLAYER;
			ret = 0, prevRet = 0, cnt = 0;

			sleep_time.flags.enable = 0;

			do{
				if(setExtensionName(extensionName, LCDStatusStruct.idEntry)){ // check if current entry is JPEG
					if(strncmp(extensionName, "JPG", 3) == 0 || strncmp(extensionName, "JPE", 3) == 0){
						ret = picojpeg(LCDStatusStruct.idEntry, ret);
					}
				}
				switch(ret)
				{
				case DJPEG_ARROW_LEFT:
					if(--LCDStatusStruct.idEntry < 0){
						LCDStatusStruct.idEntry = fat.fileCnt - 1;
					}
					break;
				case DJPEG_ARROW_RIGHT:
					if(++LCDStatusStruct.idEntry >= fat.fileCnt){
						LCDStatusStruct.idEntry = 0;
					}
					break;
				case DJPEG_PLAY:
					if(++LCDStatusStruct.idEntry >= fat.fileCnt){
						LCDStatusStruct.idEntry = 0;
					}
					break;
				case -1:
					if(prevRet == DJPEG_PLAY){
						if(++LCDStatusStruct.idEntry >= fat.fileCnt){
							LCDStatusStruct.idEntry = 0;
						}
						ret = DJPEG_PLAY;
						LCDStatusStruct.waitExitKey = 1;
					}
				default:
					break;
				}
				if(LCDStatusStruct.idEntry){
					cursor.pos = LCDStatusStruct.idEntry % PAGE_NUM_ITEMS;
					cursor.pageIdx = LCDStatusStruct.idEntry / PAGE_NUM_ITEMS;
				}
				prevRet = ret;
			}while(LCDStatusStruct.waitExitKey);

			current_mode = SW_MODE_FILER;
			LCDStatusStruct.waitExitKey = 0;

			LCDPrintFileList();

			sleep_time.curTime = 0, sleep_time.prevTime = 0;
			sleep_time.flags.enable = 1;

			break;
		case FILE_TYPE_PCF: // Display Font
			if(PCFFontInit(LCDStatusStruct.idEntry) != -1){
		    	debug.printf("\r\nexternal font loaded.");
		    	PCF_RENDER_FUNC_PCF();
			} else {
		    	debug.printf("\r\nexternal font load failed.");
			}
			LCDPrintFileList();
			LCDStatusStruct.waitExitKey = 0;
			break;
		default:
			break;
  	}

    /* Insert delay 10 ms */
    HAL_Delay(10);
  }
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 84000000
  *            HCLK(Hz)                       = 84000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 2
  *            APB2 Prescaler                 = 1
  *            HSI Frequency(Hz)              = 16000000
  *            PLL_M                          = 16
  *            PLL_N                          = 336
  *            PLL_P                          = 4
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale2 mode
  *            Flash Latency(WS)              = 2
  * @param  None
  * @retval None
  */
void SystemClock_Config(uint32_t pll_N, uint32_t pll_M)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  HAL_RCC_DeInit();
  /* Enable Power Control clock */
  __PWR_CLK_ENABLE();
  
  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  
  /* Enable HSE Oscillator and activate PLL with HSI as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = pll_M;
  RCC_OscInitStruct.PLL.PLLN = pll_N;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  while(1)
  {
  }
}

//void abort (void)
//{
//	USARTPutString("\r\nabort");
//}

void _init(void)
{
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	debug.printf("\r\n*assert_failed file:%s line:%d", file, line);
  /* Infinite loop */
  while (1)
  {
  }
}
#endif
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
