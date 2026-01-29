#include "buttonevent.h"
#include "stm32f4xx_hal.h"

#define ButtonEvent_Millis HAL_GetTick

/**
  * @brief  初始化ButtonEvent结构体
  * @param  handle: ButtonEvent结构体指针
  * @param  NoPressState: 按键未按下时的状态
  * @param  LongPressTimeMs_Set: 按键长按触发超时设置
  * @param  LongPressTimeRepeatMs_Set: 长按重复触发时间
  * @retval 无
  */
void ButtonEvent_Init(ButtonEvent* handle, 
                      bool NoPressState, 
                      uint16_t LongPressTimeMs_Set, 
                      uint16_t LongPressTimeRepeatMs_Set)
{
    handle->Button_NoPressState = NoPressState;
    handle->LongPressTimeMs = LongPressTimeMs_Set;
    handle->LongPressTimeRepeatMs = LongPressTimeRepeatMs_Set;
    
    handle->LastLongPress_TimePoint = 0;
    handle->LastPress_TimePoint = 0;
    handle->LastClick_TimePoint = 0;
    handle->IS_LongPressed = false;
    handle->IS_Pressed = false;

    handle->CallbackFunc = 0;
    handle->Button_NowState = NoPress;
    handle->IsPressed = false;
    handle->IsClicked = false;
    handle->IsLongPressed = false;
    handle->Click_Cnt = 0;
}

/**
  * @brief  按键事件绑定
  * @param  handle: ButtonEvent结构体指针
  * @param  function: 回调函数指针
  * @retval 无
  */
void ButtonEvent_EventAttach(ButtonEvent* handle, ButtonEvent_FuncCallBack_t function)
{
    handle->CallbackFunc = function;
}

/**
  * @brief  监控事件，建议扫描周期10ms
  * @param  handle: ButtonEvent结构体指针
  * @param  NowState: 当前按键引脚状态
  * @retval 无
  */
void ButtonEvent_EventMonitor(ButtonEvent* handle, uint8_t NowState)
{
    if(!handle->CallbackFunc)
        return;
    
    if (!handle->IS_Pressed && (NowState != handle->Button_NoPressState))
    {
        handle->IsPressed = true;
        handle->IS_Pressed = true;
        handle->Button_NowState = Press;
        handle->LastPress_TimePoint = ButtonEvent_Millis();

        if(handle->CallbackFunc) {
            handle->CallbackFunc(handle, EVENT_ButtonPress);
            handle->CallbackFunc(handle, EVENT_ButtonChange);
        }
    }
    else if (handle->IS_Pressed && 
             (ButtonEvent_Millis() - handle->LastPress_TimePoint >= handle->LongPressTimeMs) && 
             (NowState != handle->Button_NoPressState))
    {
        handle->Button_NowState = LongPress;
        if(!handle->IS_LongPressed)
        {
            if(handle->CallbackFunc) {
                handle->CallbackFunc(handle, EVENT_ButtonLongPressed);
            }
            handle->LastLongPress_TimePoint = ButtonEvent_Millis();
            handle->IsLongPressed = true;
            handle->IS_LongPressed = true;
        }
        else if(ButtonEvent_Millis() - handle->LastLongPress_TimePoint >= handle->LongPressTimeRepeatMs)
        {
            handle->LastLongPress_TimePoint = ButtonEvent_Millis();
            if(handle->CallbackFunc) {
                handle->CallbackFunc(handle, EVENT_ButtonLongPressRepeat);
            }
        }
    }
    else if (handle->IS_Pressed && (NowState == handle->Button_NoPressState))
    {
        handle->IS_LongPressed = false;
        handle->IS_Pressed = false;
        handle->IsClicked = true;
        handle->Button_NowState = NoPress;
        handle->LastClick_TimePoint = ButtonEvent_Millis();

        if(ButtonEvent_Millis() - handle->LastPress_TimePoint < handle->LongPressTimeMs)
        {
            if(handle->CallbackFunc) {
                handle->CallbackFunc(handle, EVENT_ButtonClick);
            }
        }
        if(handle->CallbackFunc) {
            handle->CallbackFunc(handle, EVENT_ButtonRelease);
            handle->CallbackFunc(handle, EVENT_ButtonChange);
        }
    }

//    if(handle->IS_Pressed && ButtonEvent_Millis() - handle->LastClick_TimePoint < 400)
//    {
//        handle->Click_Cnt++;
//        if(handle->CallbackFunc) {
//            handle->CallbackFunc(handle, EVENT_ButtonDoubleClick);
//        }
//    }
}

/**
  * @brief  获取点击次数
  * @param  handle: ButtonEvent结构体指针
  * @retval 点击次数
  */
uint8_t ButtonEvent_GetClickCnt(ButtonEvent* handle)
{
    uint8_t cnt = handle->Click_Cnt + 1;
    handle->Click_Cnt = 0;
    return cnt;
}

/**
  * @brief  获取点击状态
  * @param  handle: ButtonEvent结构体指针
  * @retval 点击状态
  */
bool ButtonEvent_GetClicked(ButtonEvent* handle)
{
    bool n = handle->IsClicked;
    handle->IsClicked = false;
    return n;
}

/**
  * @brief  获取按下状态
  * @param  handle: ButtonEvent结构体指针
  * @retval 按下状态
  */
bool ButtonEvent_GetPressed(ButtonEvent* handle)
{
    bool n = handle->IsPressed;
    handle->IsPressed = false;
    return n;
}

/**
  * @brief  获取长按状态
  * @param  handle: ButtonEvent结构体指针
  * @retval 长按状态
  */
bool ButtonEvent_GetLongPressed(ButtonEvent* handle)
{
    bool n = handle->IsLongPressed;
    handle->IsLongPressed = false;
    return n;
}

/**
  * @brief  获取当前按钮状态
  * @param  handle: ButtonEvent结构体指针
  * @retval 当前按钮状态
  */
uint8_t ButtonEvent_GetState(ButtonEvent* handle)
{
    return handle->Button_NowState;
}
