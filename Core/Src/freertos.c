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
#include "usb_device.h"
#include "gpio.h"
#include <stdarg.h>
#include <math.h>
#include <string.h>
// #include "gu20x8_301.h"
#include "mn11236.h"
#include "stmflash.h"
#include "stdbool.h"
#include "gui.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

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
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for myMutex01 */
osMutexId_t myMutex01Handle;
const osMutexAttr_t myMutex01_attributes = {
  .name = "myMutex01"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

osMessageQueueId_t usbQueueHandle;
const osMessageQueueAttr_t usbQueue_attributes = {
    .name = "usbQueue"};

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
	GUI_Init();
	GUI_ClearBuffer();
	GUI_ShowStr(5, 1, "Connect USB to PC");
	GUI_ShowStr(2, 1 + 8, "Drop file to Udisk");
	GUI_SendBuffer();
  /* Infinite loop */
  for (;;)
  {
    osDelay(30);
    extern uint8_t isUpgraded;
    if(isUpgraded)
    {
			GUI_ClearBuffer();
			GUI_SetColor(GUI_DEFAULT_ACTCOLOR, 0);
			GUI_ShowStr(10, 1, "Upgrade");
			GUI_ShowStr(10, 1 + 8, "Complete");
			GUI_SendBuffer();
      MX_USB_DEVICE_DeInit();
      for(int i = 0; i < 80; i++)
      {
        HAL_GPIO_TogglePin(SYS_LED_GPIO_Port, SYS_LED_Pin);
        osDelay(50);
      }
//      JumpToApplication();
			NVIC_SystemReset();
    }
    osDelay(500);
    HAL_GPIO_TogglePin(SYS_LED_GPIO_Port, SYS_LED_Pin);
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
//  const char *weekDays[7] = {
//      "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
//  static char tempstr[32] = {0};
#if true
	
  /* Infinite loop */
  for (;;)
  {
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
}

/* USER CODE END Application */

