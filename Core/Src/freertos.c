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

#include <stdarg.h>
#include <math.h>
#include <string.h>
// #include "gu20x8_301.h"
#include "mn11236.h"
#include "stm32_dsp.h"
#include "table_fft.h"
#include "stm32_rx8900.h"
#include "button.h"
#include "stmflash.h"
#include "stdbool.h"
#include "usbd_nex_link.h"
#include "nexlink_tasks.h"
#include "nexlink_app.h"
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
    .priority = (osPriority_t)osPriorityNormal,
};
/* Definitions for displayTask */
osThreadId_t displayTaskHandle;
const osThreadAttr_t displayTask_attributes = {
    .name = "displayTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityLow,
};
/* Definitions for flushTask */
osThreadId_t flushTaskHandle;
const osThreadAttr_t flushTask_attributes = {
    .name = "flushTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityRealtime,
};
/* Definitions for myMutex01 */
osMutexId_t myMutex01Handle;
const osMutexAttr_t myMutex01_attributes = {
    .name = "myMutex01"};

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

// const ClockTheme_t clockTheme[4] = {
//     {Red, WhiteWithCyan, 0},
//     {Cyan, WhiteWithRed, 0},
//     {Red, WhiteWithCyan, 1},
//     {Cyan, WhiteWithRed, 1}};

// const MusicTheme_t musicTheme[8] = {
//     {Cyan, Red},
//     {Cyan, White},
//     {Red, White},
//     {White, Red},
//     {CyanWithCyan, RedWithRed},
//     {CyanWithCyan, WhiteWithWhite},
//     {RedWithRed, WhiteWithWhite},
//     {WhiteWithWhite, RedWithRed},
// };

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartdisplayTask(void *argument);
void StartflushTask(void *argument);

extern void MX_USB_DEVICE_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void)
{
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

  /* creation of flushTask */
  flushTaskHandle = osThreadNew(StartflushTask, NULL, &flushTask_attributes);

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
  Button_Init();
  FFT_Start();
  for (;;)
  {
    FFT_Pro();
    Button_Update();
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
#if false
  vfd_init();
  beginSnake();
  //  for (size_t i = 0; i < MAX_X; i++)
  //    for (size_t j = 0; j < MAX_Y; j++)
  //    {
  //      if ((j + i) % 3 == 0)
  //        draw_point(i, j, Red);
  //      else if ((j + i) % 3 == 1)
  //        draw_point(i, j, Cyan);
  //      else if ((j + i) % 3 == 2)
  //        draw_point(i, j, White);
  //    }
  //  refrash_gram();
  //  LongStr_t *s = draw_long_str("hello world, this is for long str present ", WhiteWithCyan);
  /* Infinite loop */
  for (;;)
  {
    set_dimming(userInfo.dimming);
    clear_gram(White);

    if (userInfo.sysmode == Music)
    {
      clear_all_long_str();
      draw_fft(lBufMagArray, NPT / 2, 20, musicTheme[userInfo.musictheme].bar, musicTheme[userInfo.musictheme].dot);
    }
    else if (userInfo.sysmode == Time)
    {
      if (clockTheme[userInfo.clocktheme].type == 0)
      {
        for (int i = 0 + time_xDelta; i < MAX_X + time_xDelta; i++)
        {
          draw_point(i, 0, clockTheme[userInfo.clocktheme].border);
          draw_point(i, MAX_Y - 1, clockTheme[userInfo.clocktheme].border);
        }
        for (int j = 0; j < MAX_Y; j++)
        {
          draw_point(0 + time_xDelta, j, clockTheme[userInfo.clocktheme].border);
          draw_point(MAX_X - 1 + time_xDelta, j, clockTheme[userInfo.clocktheme].border);
        }
        draw_str_3x5(1 + time_xDelta, 1, timeStr, clockTheme[userInfo.clocktheme].font);
      }
      else
      {
        draw_num_str(time_xDelta, 0, hourMinute, 8, clockTheme[userInfo.clocktheme].font);
      }

      roll_all_long_str();
      {
        if (time_xDelta < 0 && time_xDelta >= -MAX_X)
          time_xDelta--;

        if (time_xDelta > 0)
          time_xDelta--;

        if (get_long_str_count() == 0 && time_xDelta < 0)
          time_xDelta = MAX_X;
      }
    }
    else if (userInfo.sysmode == Snake)
    {
      //      draw_num_str(0, 0, "12", 3, WhiteWithRed);
      //      draw_num_str(0, 4, "34", 4, WhiteWithRed);
      refrashSnake();
      osDelay(30);
    }
    else if (userInfo.sysmode == SetDimm)
    {
      for (int i = 0; i < MAX_X; i++)
      {
        for (int j = 0; j < 2; j++)
          if (i / 5 > (userInfo.dimming))
            draw_point(i, j, Red);
          else if (i / 5 < (userInfo.dimming))
            draw_point(i, j, Cyan);
          else
            draw_point(i, j, White);
      }

      draw_str_3x5(0, 2, "brigh", WhiteWithRed);
      if (backToDefaultTimestamp < HAL_GetTick())
      {
        userInfo.sysmode = Time;
        STMFLASH_Write(FLASH_SAVE_ADDR, (uint32_t *)&userInfo, sizeof(UserInfo_t) / sizeof(uint32_t));
      }
    }
    else if (userInfo.sysmode == SetYear)
    {
      for (int i = 0; i < MAX_X; i++)
      {
        if (i / 5 >= 1)
          draw_point(i, 7, Red);
        else
          draw_point(i, 7, Cyan);
      }
      if (flash)
        snprintf(tempstr, sizeof tempstr, "%04d", tm_set.tm_year + 2000);
      else
        snprintf(tempstr, sizeof tempstr, "    ");

      draw_str_3x5(2, 1, tempstr, WhiteWithRed);
      if (backToDefaultTimestamp < HAL_GetTick())
      {
        userInfo.sysmode = Time;
        STMFLASH_Write(FLASH_SAVE_ADDR, (uint32_t *)&userInfo, sizeof(UserInfo_t) / sizeof(uint32_t));
      }
    }
    else if (userInfo.sysmode == SetMonth || userInfo.sysmode == SetDay)
    {
      for (int i = 0; i < MAX_X; i++)
      {
        if (i / 5 >= 2)
          draw_point(i, 7, Red);
        else
          draw_point(i, 7, Cyan);
      }
      if (flash)
        snprintf(tempstr, sizeof tempstr, "%02d-%02d", tm_set.tm_mon + 1, tm_set.tm_mday);
      else if (userInfo.sysmode == SetMonth)
        snprintf(tempstr, sizeof tempstr, "    -%02d", tm_set.tm_mday);
      else if (userInfo.sysmode == SetDay)
        snprintf(tempstr, sizeof tempstr, "%02d-  ", tm_set.tm_mon + 1);

      draw_str_3x5(1, 1, tempstr, WhiteWithRed);
      if (backToDefaultTimestamp < HAL_GetTick())
      {
        userInfo.sysmode = Time;
        STMFLASH_Write(FLASH_SAVE_ADDR, (uint32_t *)&userInfo, sizeof(UserInfo_t) / sizeof(uint32_t));
      }
    }
    else if (userInfo.sysmode == SetWeek)
    {
      for (int i = 0; i < MAX_X; i++)
      {
        if (i / 5 >= 3)
          draw_point(i, 7, Red);
        else
          draw_point(i, 7, Cyan);
      }
      if (flash)
        snprintf(tempstr, sizeof tempstr, "%s", weekDays[tm_set.tm_wday]);
      else
        snprintf(tempstr, sizeof tempstr, "    ");

      draw_str_3x5(4, 1, tempstr, WhiteWithRed);
      if (backToDefaultTimestamp < HAL_GetTick())
      {
        userInfo.sysmode = Time;
        STMFLASH_Write(FLASH_SAVE_ADDR, (uint32_t *)&userInfo, sizeof(UserInfo_t) / sizeof(uint32_t));
      }
    }
    else if (userInfo.sysmode == SetHour || userInfo.sysmode == SetMin)
    {
      for (int i = 0; i < MAX_X; i++)
      {
        if (i / 5 >= 4)
          draw_point(i, 7, Red);
        else
          draw_point(i, 7, Cyan);
      }
      if (flash)
        snprintf(tempstr, sizeof tempstr, "%02d:%02d", tm_set.tm_hour, tm_set.tm_min);
      else if (userInfo.sysmode == SetHour)
        snprintf(tempstr, sizeof tempstr, "    :%02d", tm_set.tm_min);
      else if (userInfo.sysmode == SetMin)
        snprintf(tempstr, sizeof tempstr, "%02d:  ", tm_set.tm_hour);

      draw_str_3x5(1, 1, tempstr, WhiteWithRed);
      if (backToDefaultTimestamp < HAL_GetTick())
      {
        userInfo.sysmode = Time;
        STMFLASH_Write(FLASH_SAVE_ADDR, (uint32_t *)&userInfo, sizeof(UserInfo_t) / sizeof(uint32_t));
      }
    }

    refrash_gram();

    osDelay(30);
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

/* USER CODE BEGIN StartClockTask */
__weak void StartClockTask(void *argument)
{
  static char tempstr[32] = {0};
  static int count = 50;
  const char *weekDays[7] = {
      "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
  RX8900_Init();
  // RX8900_SetTime(&tm_local);
  //  if (RX8900_GetTime(&tm_local))
  //  {
  osDelay(500);
  //    if (RX8900_GetTime(&tm_local))
  //		{
  //			if (tm_local.tm_hour > 23 || tm_local.tm_wday > 6)
  //			{
  //				tm_set.tm_year = 25;
  //				tm_set.tm_mon = 1;
  //				tm_set.tm_mday = 1;
  //				tm_set.tm_wday = 1;
  //				tm_set.tm_hour = 0;
  //				tm_set.tm_min = 0;
  //				tm_set.tm_sec = 0;
  //				RX8900_SetTime(&tm_set);
  //			}
  //		}
  //  }
  for (;;)
  {
    if (RX8900_GetTime(&tm_local))
    {
      nexlink_log("%02d:%02d:%02d\r\n", tm_local.tm_hour, tm_local.tm_min, tm_local.tm_sec);
      nexlink_log("20%02d-%02d-%02d %d\r\n", tm_local.tm_year, tm_local.tm_mon + 1, tm_local.tm_mday, tm_local.tm_wday);
      flash = !flash;
      if (flash)
        snprintf(timeStr, sizeof timeStr, "%02d:%02d", tm_local.tm_hour, tm_local.tm_min);
      else
        snprintf(timeStr, sizeof timeStr, "%02d %02d", tm_local.tm_hour, tm_local.tm_min);
      snprintf(hourMinute, sizeof hourMinute, "%02d%02d", tm_local.tm_hour, tm_local.tm_min);
//      if (userInfo.sysmode == Time && count++ > 60)
//      {
//        count = 0;
//        time_xDelta--;
//        snprintf(tempstr, sizeof tempstr, "20%02d-%02d-%02d %s", tm_local.tm_year, tm_local.tm_mon + 1, tm_local.tm_mday, weekDays[tm_local.tm_wday % 7]);
//        draw_long_str(tempstr, clockTheme[userInfo.clocktheme].font, 0);
//      }
    }
    osDelay(500);
  }
}
/**
 * @brief Function implementing the usbTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END StartClockTask */
void StartflushTask(void *argument)
{
  /* USER CODE BEGIN StartflushTask */
  /* Infinite loop */
  for (;;)
  {
    osDelay(2);
  }
  /* USER CODE END StartflushTask */
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
