/*!
 * Copyright (c) 2023, ErBW_s
 * All rights reserved.
 *
 * @author  Baohan
 */

#ifndef _EASY_UI_H_
#define _EASY_UI_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "easy_key.h"
#include "gui.h"
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include "adc.h"
#include "cmsis_os.h"

// Operation response
extern uint8_t opnEnter, opnExit, opnUp, opnDown;

#define MPUCRL          1
#define KEY_NUM         3
#define ROTARY          0

#define CHECK_BOX_OFFSET        2
#define RADIO_BUTTON_OFFSET        3
#define SCROLL_BAR_WIDTH        4
//#define ITEM_LINES              ((uint8_t)(SCREEN_HEIGHT / ITEM_HEIGHT))
#define MAX_LAYER               10
#define ICON_SIZE               50

// Represent the time it takes to play the animation, smaller the quicker. Unit: ms
#define INDICATOR_MOVE_TIME     70
#define ITEM_MOVE_TIME          70
#define TRANSITION_TIME         60

#define OFFSET_X 0
#define OFFSET_Y 0
#define SCREEN_WIDTH            (LCD_W - OFFSET_X*2)
#define SCREEN_HEIGHT           (LCD_H - OFFSET_Y*2)
#define EasyUIScreenInit(gram)                                  (GUI_Init(gram))
#define EasyUISetFont(font)                                     (GUI_SetFont(font))
#define EasyUIDisplayStr(x, y, str)                             (GUI_ShowStr((x) + OFFSET_X, (y) + OFFSET_Y, str))
#define EasyUIDisplayStrMutiRow(x, y, w, str)                   (GUI_ShowStrMutiRow((x) + OFFSET_X, (y) + OFFSET_Y, w, str))
#define EasyUIDisplayFloat(x, y, dat, num, pointNum)            (GUI_ShowFloat((x) + OFFSET_X, (y) + OFFSET_Y, dat, num, pointNum))
#define EasyUIDrawDot(x, y, color)                              (GUI_DrawPoint((x) + OFFSET_X, (y) + OFFSET_Y, color))
#define EasyUIDrawBox(x, y, width, height, color)               (GUI_DrawBox((x) + OFFSET_X, (y) + OFFSET_Y, width, height, color))
#define EasyUIDrawFrame(x, y, width, height, color)             (GUI_DrawFrame((x) + OFFSET_X, (y) + OFFSET_Y, width, height, color))
#define EasyUIDrawRFrame(x, y, width, height, color, r)         (GUI_DrawRFrame((x) + OFFSET_X, (y) + OFFSET_Y, width, height, color, r))
#define EasyUIDrawRBox(x, y, width, height, color, r)           (GUI_DrawRBox((x) + OFFSET_X, (y) + OFFSET_Y, width, height, color, r))
#define EasyUIClearBuffer(void)                                 (GUI_ClearBuffer())
#define EasyUISendBuffer(void)                                  (GUI_SendBuffer())
#define EasyUISetDrawColor(mode)                                (GUI_SetDrawColor(mode))
#define EasyUIDisplayBMP(x, y, width, height, pic)              (GUI_ShowBMP((x) + OFFSET_X, (y) + OFFSET_Y, width, height, pic))
#define EasyUIModifyColor(void)                                 (GUI_ModifyColor())
#define EasyUIDrawCircle(x, y, r, color, section)               (GUI_DrawCircle((x) + OFFSET_X, (y) + OFFSET_Y, r, color, section))
#define EasyUIDrawDisc(x, y, r, color, section)                 (GUI_DrawDisc((x) + OFFSET_X, (y) + OFFSET_Y, r, color, section))

#define EasyUIDelay_ms(time)                                    (osDelay(time))

#define HOLDTIME 120 //0.5s
void ClearRemind(void);

typedef     uint8_t      paramType;

typedef struct {
    float current_value;  // ��ǰֵ
    float target_value;   // Ŀ��ֵ
} Filter;

void update_filter(Filter *filter);

typedef enum
{
    ITEM_PAGE_DESCRIPTION,
    ITEM_JUMP_PAGE,
    ITEM_SWITCH,
    ITEM_CHANGE_VALUE,
    ITEM_PROGRESS_BAR,
    ITEM_RADIO_BUTTON,
    ITEM_CHECKBOX,
    ITEM_MESSAGE,
    ITEM_DETAIL,
    ITEM_PROGRESS_LABEL,    /* 和 PROGRESS_BAR 一样，但把数字换成 labels[值] */
} EasyUIItem_e;

typedef enum
{
    PAGE_LIST,
    PAGE_ICON,
    PAGE_CUSTOM
} EasyUIPage_e;

typedef struct
{
		uint8_t width;
		uint8_t height;
		Font_Type_t type;
} Font_t;

typedef struct EasyUI_item
{
    struct EasyUI_item *next;

    EasyUIItem_e funcType;
    uint8_t id;
    int16_t lineId;
    float posForCal;
    float step;
    int16_t position;
    char *title;
    char *msg;                                  // ITEM_MESSAGE
    const char *const *labels;                  // ITEM_PROGRESS_LABEL：值 -> 显示名
    bool *flag;                                 // ITEM_CHECKBOX and ITEM_RADIO_BUTTON and ITEM_SWITCH
    bool flagDefault;                           // Factory default setting
    paramType *param;                           // ITEM_CHANGE_VALUE and ITEM_PROGRESS_BAR
    int paramMax;
    paramType paramDefault;                     // Factory default setting
    paramType paramBackup;                      // ITEM_CHANGE_VALUE and ITEM_PROGRESS_BAR
    uint8_t pageId;                             // ITEM_JUMP_PAGE
    void (*Event)(struct EasyUI_item *item);    // ITEM_CHANGE_VALUE and ITEM_PROGRESS_BAR
} EasyUIItem_t;

typedef struct EasyUI_page
{
    struct EasyUI_page *next;

		uint8_t rowheight;
		Font_t font;
    EasyUIPage_e funcType;
    EasyUIItem_t *itemHead, *itemTail;
    uint8_t id;

    void (*Event)(struct EasyUI_page *page);
} EasyUIPage_t;

extern char *EasyUIVersion;
extern bool functionIsRunning, listLoop, errorOccurred, batteryMonitor;
extern EasyUIPage_t *pageHead, *pageTail;

void EasyUIAddItem(EasyUIPage_t* page, EasyUIItem_t* item, char* _title, EasyUIItem_e func, ...);
void EasyUIAddPage(EasyUIPage_t* page, Font_Type_t fonttype, EasyUIPage_e func, ...);
void EasyUITransitionAnim(void);
void EasyUIBackgroundBlur(void);

void EasyUIDrawMsgBox(EasyUIPage_t* page, char *msg);
float EasyUIGetBatteryVoltage(void);

void EasyUIEventChangeUint(EasyUIPage_t* page, EasyUIItem_t *item);
void EasyUIEventChangeInt(EasyUIPage_t* page, EasyUIItem_t *item);
void EasyUIEventChangeFloat(EasyUIPage_t* page, EasyUIItem_t *item);
void EasyUIEventSaveSettings(EasyUIItem_t *item);
void EasyUIEventResetSettings(EasyUIItem_t *item);
void EasyUIEventChangeFloatForYaw(EasyUIItem_t *item);
void EasyUIInit(uint8_t mode);
void EasyUIEvent(uint8_t timer);
void EasyUIItemOperationResponse(EasyUIPage_t *page, EasyUIItem_t *item, uint8_t *index);

#ifdef __cplusplus
}
#endif

#endif
