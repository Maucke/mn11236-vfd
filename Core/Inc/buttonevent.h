#ifndef __BUTTONEVENT_H
#define __BUTTONEVENT_H

#include <stdint.h>
#include <stdbool.h>

// 定义事件类型枚举
typedef enum {
    EVENT_ButtonNone,
    EVENT_ButtonPress,
    EVENT_ButtonLongPressed,
    EVENT_ButtonLongPressRepeat,
    EVENT_ButtonRelease,
    EVENT_ButtonChange,
    EVENT_ButtonClick,
    EVENT_ButtonDoubleClick,
} ButtonEvent_Event_Type;

// 定义按钮状态枚举
typedef enum {
    NoPress,
    Press,
    LongPress
} ButtonEvent_ButtonState;

// 定义回调函数类型
typedef void(*ButtonEvent_FuncCallBack_t)(void*, int);

// 定义ButtonEvent结构体
typedef struct {
    bool IsPressed;
    bool IsClicked;
    bool IsLongPressed;
    
    bool Button_NoPressState;
    uint8_t Button_NowState;
    uint16_t LongPressTimeMs;
    uint16_t LongPressTimeRepeatMs;
    uint32_t LastLongPress_TimePoint;
    uint32_t LastPress_TimePoint;
    uint32_t LastClick_TimePoint;
    uint8_t Click_Cnt;
    bool IS_Pressed;
    bool IS_LongPressed;

    ButtonEvent_FuncCallBack_t CallbackFunc;
} ButtonEvent;

// 初始化ButtonEvent结构体
void ButtonEvent_Init(ButtonEvent* handle, 
                      bool NoPressState, 
                      uint16_t LongPressTimeMs_Set, 
                      uint16_t LongPressTimeRepeatMs_Set);

// 绑定事件回调函数
void ButtonEvent_EventAttach(ButtonEvent* handle, ButtonEvent_FuncCallBack_t function);

// 监控按钮事件
void ButtonEvent_EventMonitor(ButtonEvent* handle, uint8_t NowState);

// 获取点击次数
uint8_t ButtonEvent_GetClickCnt(ButtonEvent* handle);

// 获取点击状态
bool ButtonEvent_GetClicked(ButtonEvent* handle);

// 获取按下状态
bool ButtonEvent_GetPressed(ButtonEvent* handle);

// 获取长按状态
bool ButtonEvent_GetLongPressed(ButtonEvent* handle);

// 获取当前按钮状态
uint8_t ButtonEvent_GetState(ButtonEvent* handle);

#endif

