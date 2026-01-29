#include "button.h"
#include "main.h"
#include "stmflash.h"
#include <stdio.h>
#include <time.h>
#include "stm32_rx8900.h"
#include "nexlink_app.h"

/*定义按键对象*/
ButtonEvent btKey1;
ButtonEvent btKey2;

char *Event_Type[] = {
    "EVENT_ButtonNone",
    "EVENT_ButtonPress",
    "EVENT_ButtonLongPressed",
    "EVENT_ButtonLongPressRepeat",
    "EVENT_ButtonRelease",
    "EVENT_ButtonChange",
    "EVENT_ButtonClick",
    "EVENT_ButtonDoubleClick"};

extern UserInfo_t userInfo;
extern uint32_t backToDefaultTimestamp;
extern struct tm tm_set;
extern struct tm tm_local;

/**
 * @brief  按键事件回调处理
 * @param  btn:按键对象地址
 * @param  event:事件类型
 * @retval 无
 */
static void Button_EventHandler(void *btn, int event)
{
    /*传递至页面事件*/
		nexlink_log("size: %d, %d, %d, %d\r\n", sizeof(UserInfo_t), sizeof(userInfo), sizeof(UserInfo_t)/ sizeof(uint32_t),sizeof(UserInfo_t)/ sizeof(uint32_t));
    nexlink_log("event:%s\r\n", Event_Type[event]);
    if (btn == &btKey1)
    {
        if (event == EVENT_ButtonLongPressed)
        {
            if (userInfo.sysmode == SetMin) // lastmode
            {
                RX8900_SetTime(&tm_set);
                RX8900_GetTime(&tm_local);
            }
            userInfo.sysmode++;
            userInfo.sysmode %= Mode_Max;
            if (userInfo.sysmode > Snake)
            {
                backToDefaultTimestamp = HAL_GetTick() + 30 * 1000;
                if (userInfo.sysmode == SetDimm)
                {
                }
                else if (userInfo.sysmode == SetYear)
                {
                    tm_set.tm_year = 25;
                    tm_set.tm_mon = 0;
                    tm_set.tm_mday = 1;
                    tm_set.tm_wday = 0;
                    tm_set.tm_hour = 0;
                    tm_set.tm_min = 0;
                    tm_set.tm_sec = 0;
                }
            }
            else
                STMFLASH_Write(FLASH_SAVE_ADDR, (uint32_t *)&userInfo, sizeof(UserInfo_t)/ sizeof(uint32_t));
        }
        else if (event == EVENT_ButtonClick)
        {
            if (userInfo.sysmode > Snake)
                backToDefaultTimestamp = HAL_GetTick() + 30 * 1000;
            if (userInfo.sysmode == Time)
            {
                userInfo.clocktheme++;
                userInfo.clocktheme %= 4;
                STMFLASH_Write(FLASH_SAVE_ADDR, (uint32_t *)&userInfo, sizeof(UserInfo_t)/ sizeof(uint32_t));
            }
            else if (userInfo.sysmode == Snake)
            {
                userInfo.sysmode = Time;
                STMFLASH_Write(FLASH_SAVE_ADDR, (uint32_t *)&userInfo, sizeof(UserInfo_t)/ sizeof(uint32_t));
            }
            else if (userInfo.sysmode == Music)
            {
                userInfo.musictheme++;
                userInfo.musictheme %= 8;
                STMFLASH_Write(FLASH_SAVE_ADDR, (uint32_t *)&userInfo, sizeof(UserInfo_t)/ sizeof(uint32_t));
            }
            else if (userInfo.sysmode == SetDimm)
            {
                userInfo.dimming++;
                userInfo.dimming %= 4;
                STMFLASH_Write(FLASH_SAVE_ADDR, (uint32_t *)&userInfo, sizeof(UserInfo_t)/ sizeof(uint32_t));
            }
            else if (userInfo.sysmode == SetYear)
            {
                tm_set.tm_year++;
                tm_set.tm_year %= 100;
            }
            else if (userInfo.sysmode == SetMonth)
            {
                tm_set.tm_mon++;
                if (tm_set.tm_mon > 11)
                    tm_set.tm_mon = 0;
            }
            else if (userInfo.sysmode == SetDay)
            {
                tm_set.tm_mday++;
                if (tm_set.tm_mday > 31)
                    tm_set.tm_mday = 1;
            }
            else if (userInfo.sysmode == SetWeek)
            {
                tm_set.tm_wday++;
                tm_set.tm_wday %= 7;
            }
            else if (userInfo.sysmode == SetHour)
            {
                tm_set.tm_hour++;
                tm_set.tm_hour %= 24;
            }
            else if (userInfo.sysmode == SetMin)
            {
                tm_set.tm_min++;
                tm_set.tm_min %= 60;
            }
        }
        else if (event == EVENT_ButtonDoubleClick)
        {
        }
    }
}

/**
 * @brief  按键初始化
 * @param  无
 * @retval 无
 */
void Button_Init()
{
    /*初始化按键对象*/
    ButtonEvent_Init(&btKey1, 0, 500, 200);
    ButtonEvent_Init(&btKey2, 0, 500, 200);

    /*关联事件*/
    ButtonEvent_EventAttach(&btKey1, Button_EventHandler);
    ButtonEvent_EventAttach(&btKey2, Button_EventHandler);
}

/**
 * @brief  按键监控更新
 * @param  无
 * @retval 无
 */
void Button_Update()
{
    ButtonEvent_EventMonitor(&btKey1, !HAL_GPIO_ReadPin(KEY_SW1_GPIO_Port, KEY_SW1_Pin));
    ButtonEvent_EventMonitor(&btKey2, !HAL_GPIO_ReadPin(KEY_SW2_GPIO_Port, KEY_SW2_Pin));
}
