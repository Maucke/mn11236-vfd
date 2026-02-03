/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "adc.h"
#include "dma.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"
#include "clockui.h"
#include <stdarg.h>
#include <math.h>
#include <string.h>
// #include "gu20x8_301.h"
#include "mn11236.h"
#include "stm32_dsp.h"
#include "table_fft.h"
#include "stm32_rx8900.h"
#include "stmflash.h"
#include "stdbool.h"
#include "usbd_nex_link.h"
#include "nexlink_tasks.h"
#include "nexlink_app.h"
#include "easy_ui.h"
#include "easy_ui_user_app.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define NPT 256
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

uint32_t adc_buf[NPT] = {0};
static long lBufInArray[NPT];
static long lBufOutArray[NPT];
long lBufMagArray[NPT];
extern struct tm tm_local;
char timeStr[10] = {0};
char hourMinute[10] = {0};
uint32_t backToDefaultTimestamp = 0;
struct tm tm_set;
static int flash = 0;

int time_xDelta = 0;

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for displayTask */
osThreadId_t displayTaskHandle;
const osThreadAttr_t displayTask_attributes = {
  .name = "displayTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for myMutex01 */
osMutexId_t myMutex01Handle;
const osMutexAttr_t myMutex01_attributes = {
  .name = "myMutex01"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
osThreadId_t clockTaskHandle;
const osThreadAttr_t clockTask_attributes = {
    .name = "clockTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal,
};

osMessageQueueId_t usbQueueHandle;
const osMessageQueueAttr_t usbQueue_attributes = {
    .name = "usbQueue"};

void FFT_Start(void)
{
  HAL_ADC_Start_DMA(&hadc1, adc_buf, NPT);
  HAL_TIM_Base_Start(&htim3);
}
void FFT_Stop(void)
{
  HAL_ADC_Stop_DMA(&hadc1);
  HAL_TIM_Base_Stop(&htim3);
}
void GetPowerMag(void)
{
  signed short lX, lY;
  float X, Y, Mag;
  unsigned short i;

  for (i = 0; i < NPT; i++)
  {
    lX = (lBufOutArray[i] << 16) >> 16;
    lY = (lBufOutArray[i] >> 16);

    X = NPT * ((float)lX) / 32768;
    Y = NPT * ((float)lY) / 32768;
    Mag = sqrt(X * X + Y * Y) * 1.0f / NPT;
    if (i == 0)
      lBufMagArray[i] = (unsigned long)(Mag * 32768);
    else
      lBufMagArray[i] = (unsigned long)(Mag * 65536);
  }
}
void FFT_Pro(void)
{
  uint16_t i = 0;
  for (i = 0; i < NPT; i++)
  {
    lBufInArray[i] = ((signed short)(adc_buf[i]) - 1551) << 18;
  }
  cr4_fft_256_stm32(lBufOutArray, lBufInArray, NPT);
  GetPowerMag();
}

UserInfo_t userInfo;

 const ClockTheme_t clockTheme[4] = {
     {Red, WhiteWithCyan, 0},
     {Cyan, WhiteWithRed, 0},
     {Red, WhiteWithCyan, 1},
     {Cyan, WhiteWithRed, 1}};

 const MusicTheme_t musicTheme[8] = {
     {Cyan, Red},
     {Cyan, White},
     {Red, White},
     {White, Red},
     {CyanWithCyan, RedWithRed},
     {CyanWithCyan, WhiteWithWhite},
     {RedWithRed, WhiteWithWhite},
     {WhiteWithWhite, RedWithRed},
 };

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartdisplayTask(void *argument);

extern void MX_USB_DEVICE_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of myMutex01 */
  myMutex01Handle = osMutexNew(&myMutex01_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  //  usbQueueHandle = osMessageQueueNew(16, sizeof(uint8_t) * DEBUG_BUF_SIZE, &usbQueue_attributes);
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of displayTask */
  displayTaskHandle = osThreadNew(StartdisplayTask, NULL, &displayTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  //  clockTaskHandle = osThreadNew(StartClockTask, NULL, &clockTask_attributes);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
__weak void StartDefaultTask(void *argument)
{
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN StartDefaultTask */
  osDelay(200);
  NexLinkInit();
  /* Infinite loop */
  FFT_Start();
  for (;;)
  {
    FFT_Pro();
    osDelay(30);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartdisplayTask */
/**
 * @brief Function implementing the displayTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartdisplayTask */
__weak void StartdisplayTask(void *argument)
{
  /* USER CODE BEGIN StartdisplayTask */
  const char *weekDays[7] = {
      "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
  static char tempstr[32] = {0};
	MenuInit();
	EasyUIInit(1);
#if true
//  clock_init();
//  beginSnake();
  /* Infinite loop */
  for (;;)
  {
//    clear_gram(White);
//      clear_all_long_str();
//      draw_fft(lBufMagArray, NPT / 2, 20, musicTheme[userInfo.musictheme].bar, musicTheme[userInfo.musictheme].dot);

//		refrash_gram();
		EasyUIEvent(5);
    osDelay(5);
  }
#else
	mn11236_init();
  for (;;)
  {
		mn11236_development_test();
    osDelay(1000);
  }
#endif
  /* USER CODE END StartdisplayTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  FFT_Stop();
  FFT_Pro();
  FFT_Start();
}

/* USER CODE END Application */

