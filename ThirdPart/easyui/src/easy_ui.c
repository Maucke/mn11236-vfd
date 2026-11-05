/*!
 * Copyright (c) 2023, ErBW_s
 * All rights reserved.
 *
 * @author  Baohan
 */

#include "easy_ui.h"
#include "easy_ui_user_app.h"
#include "stm32_rx8900.h"

EasyUIPage_t *pageHead = NULL, *pageTail = NULL;

uint8_t pageIndex[MAX_LAYER] = {0};
uint8_t itemIndex[MAX_LAYER] = {0};
uint8_t layer = 0;

uint8_t opnEnter, opnExit, opnUp, opnDown;

bool functionIsRunning = false, listLoop = true, errorOccurred = false, batteryMonitor = true;

void EasyUIAddItem(EasyUIPage_t *page, EasyUIItem_t *item, char *_title, EasyUIItem_e func, ...)
{
  *item->flag = false;
  item->flagDefault = false;
  *item->param = 0;
  item->paramDefault = 0;
  item->paramBackup = 0;
  item->pageId = 0;
  item->Event = NULL;

  va_list variableArg;
  va_start(variableArg, func);
  item->title = _title;
  item->funcType = func;
  switch (item->funcType)
  {
  case ITEM_JUMP_PAGE:
    item->pageId = va_arg(variableArg, int);
    break;
  case ITEM_CHECKBOX:
  case ITEM_RADIO_BUTTON:
  case ITEM_SWITCH:
    item->flag = va_arg(variableArg, bool *);
    item->flagDefault = *item->flag;
    break;
  case ITEM_PROGRESS_BAR:
  case ITEM_CHANGE_VALUE:
    item->param = va_arg(variableArg, paramType *);
    item->paramBackup = *item->param;
    item->paramDefault = *item->param;
    item->paramMax = va_arg(variableArg, int);
    item->Event = va_arg(variableArg, void (*)(EasyUIItem_t *));
    break;
  case ITEM_MESSAGE:
    item->msg = va_arg(variableArg, char *);
    item->Event = va_arg(variableArg, void (*)(EasyUIItem_t *));
    break;
  default:
    break;
  }

  va_end(variableArg);

  item->next = NULL;

  if (page->itemHead == NULL)
  {
    item->id = 0;
    page->itemHead = item;
    page->itemTail = item;
  }
  else
  {
    item->id = page->itemTail->id + 1;
    page->itemTail->next = item;
    page->itemTail = page->itemTail->next;
  }

  item->lineId = item->id;
  item->posForCal = 0;
  item->step = 0;
  item->position = item->posForCal;
}

/*!
 * @brief   Add page to UI
 *
 * @param   page    EasyUI page struct
 * @param   func    See EasyUIPage_e
 * @param   ...     PAGE_LIST: ignore this
 *                  PAGE_CUSTOM: fill with certain function
 * @return  void
 *
 * @note    Do not modify, the first page should always be the fist one to be added.
 */
void EasyUIAddPage(EasyUIPage_t *page, Font_Type_t fonttype, EasyUIPage_e func, ...)
{
  page->Event = NULL;

  va_list variableArg;
  va_start(variableArg, func);
  page->itemHead = NULL;
  page->itemTail = NULL;
  page->next = NULL;

  page->funcType = func;
  if (page->funcType == PAGE_CUSTOM)
    page->Event = va_arg(variableArg, void (*)(EasyUIPage_t *));
  va_end(variableArg);

  page->font.type = fonttype;
  switch (page->font.type)
  {
  case GUI_6X8_FONT:
    page->font.width = 6;
    page->font.height = 8;
    page->rowheight = 10;
    break;
  case GUI_8X16_FONT:
  case GUI_8X16_OCRB:
    page->font.width = 8;
    page->font.height = 16;
    page->rowheight = 20;
    break;
  case GUI_10X16_OCR:
    page->font.width = 10;
    page->font.height = 16;
    page->rowheight = 20;
    break;
  case GUI_12X16_OCR:
  case GUI_12X16_OCRB:
    page->font.width = 12;
    page->font.height = 16;
    page->rowheight = 20;
    break;
  case GUI_12X24_AGENCY:
    page->font.width = 12;
    page->font.height = 24;
    page->rowheight = 30;
    break;
  case GUI_16X24_OCR:
  case GUI_16X24_OCRB:
    page->font.width = 16;
    page->font.height = 24;
    page->rowheight = 30;
    break;
  default:
    page->font.width = 8;
    page->font.height = 16;
    page->rowheight = 20;
    break;
  }

  if (pageHead == NULL)
  {
    page->id = 0;
    pageHead = page;
    pageTail = page;
  }
  else
  {
    page->id = pageTail->id + 1;
    pageTail->next = page;
    pageTail = pageTail->next;
  }
}

/*!
 * @brief   Blur transition animation
 *
 * @param   void
 * @return  void
 *
 * @note    Use before clearing the buffer
 *          Also use after all the initialization is done for better experience
 */
void EasyUITransitionAnim()
{
  for (int j = 1; j < LCD_H + 1; j += 2)
  {
    for (int i = 0; i < LCD_W + 1; i += 2)
    {
      GUI_DrawPoint(i, j, GUI_backgroundColor);
    }
  }
  EasyUIDelay_ms(TRANSITION_TIME / 4);
  EasyUISendBuffer();
  for (int j = 1; j < LCD_H + 1; j += 2)
  {
    for (int i = 1; i < LCD_W + 1; i += 2)
    {
      GUI_DrawPoint(i, j, GUI_backgroundColor);
    }
  }
  EasyUIDelay_ms(TRANSITION_TIME / 4);
  EasyUISendBuffer();
  for (int j = 0; j < LCD_H + 1; j += 2)
  {
    for (int i = 1; i < LCD_W + 1; i += 2)
    {
      GUI_DrawPoint(i, j, GUI_backgroundColor);
    }
  }
  EasyUIDelay_ms(TRANSITION_TIME / 4);
  EasyUISendBuffer();
  for (int j = 1; j < LCD_H + 1; j += 2)
  {
    for (int i = 1; i < LCD_W + 1; i += 2)
    {
      GUI_DrawPoint(i - 1, j - 1, GUI_backgroundColor);
    }
  }
  EasyUIDelay_ms(TRANSITION_TIME / 4);
  EasyUISendBuffer();
}

/*!
 * @brief   Blur the background for other use
 *
 * @param   void
 * @return  void
 */
void EasyUIBackgroundBlur()
{
  for (int j = 1; j < LCD_H + 1; j += 2)
  {
    for (int i = 0; i < LCD_W + 1; i += 2)
    {
      GUI_DrawPoint(i, j, GUI_backgroundColor);
    }
  }
  EasyUIDelay_ms(TRANSITION_TIME / 3);
  EasyUISendBuffer();
  for (int j = 1; j < LCD_H + 1; j += 2)
  {
    for (int i = 1; i < LCD_W + 1; i += 2)
    {
      GUI_DrawPoint(i, j, GUI_backgroundColor);
    }
  }
  EasyUIDelay_ms(TRANSITION_TIME / 3);
  EasyUISendBuffer();
  for (int j = 0; j < LCD_H + 1; j += 2)
  {
    for (int i = 1; i < LCD_W + 1; i += 2)
    {
      GUI_DrawPoint(i, j, GUI_backgroundColor);
    }
  }
  EasyUIDelay_ms(TRANSITION_TIME / 3);
  EasyUISendBuffer();
}

/*!
 * @brief   Draw message box
 *
 * @param   msg     The message need to be displayed
 * @return  void
 */
void EasyUIDrawMsgBox(EasyUIPage_t *page, char *msg)
{
  uint16_t width = strlen(msg) * page->font.width + 5;
  uint8_t height = ((width / SCREEN_WIDTH) + 1) * page->rowheight;
  if (width > SCREEN_WIDTH)
    width = SCREEN_WIDTH;
  uint16_t x, y;
  uint8_t offset = 2;
  x = (SCREEN_WIDTH - width) / 2;
  y = (SCREEN_HEIGHT - height) / 2;
  EasyUIBackgroundBlur();
  EasyUIDrawRFrame(x + offset, y - offset, width + 2, height + 2, GUI_penColor, 1);
  EasyUIDrawRBox(x - offset, y + offset, width + 2, height + 2, GUI_penColor, 1);
  EasyUISetDrawColor(XOR);
  EasyUISetFont(page->font.type);
  EasyUIDisplayStrMutiRow(x - offset + 2 + 1, y + offset + 2 + 1, width, msg);
  EasyUISetDrawColor(NORMAL);
  EasyUISendBuffer();
}

/*!
 * @brief   Draw progress bar
 *
 * @param   item    EasyUI item struct
 * @return  void
 *
 * @note    Internal call
 */
void EasyUIDrawProgressBar(EasyUIPage_t *page, EasyUIItem_t *item)
{
	char tempstr[20];
  static int16_t x, y;
  static uint16_t width, height;
  static uint8_t itemHeightOffset = 0;
  static uint16_t barWidth;
  itemHeightOffset = (page->rowheight - page->font.height) / 2 - 1;
  EasyUISetFont(page->font.type);
  EasyUISetDrawColor(NORMAL);

  // Display information and draw box
  height = page->rowheight * 2 - 2;
  if (strlen(item->title) + 1 > 12)
    width = (strlen(item->title) + 1) * page->font.width + 7 * 4;
  else
    width = 12 * page->font.width + 7 * 4;
  if (width < 2 * SCREEN_WIDTH / 3)
    width = 4 * SCREEN_WIDTH / 5;
  x = (SCREEN_WIDTH - width) / 2;
  y = (SCREEN_HEIGHT - height) / 2;

  barWidth = width - 2 * page->font.width - 0 - 8;

  EasyUIDrawFrame(x - 1, y - 1, width + 2, height + 2, GUI_penColor);
  EasyUIDrawBox(x, y, width, height, GUI_backgroundColor);
  EasyUIDisplayStr(x + 3, y + itemHeightOffset, item->title);
  EasyUIDisplayStr(x + 3 + strlen(item->title) * page->font.width, y + itemHeightOffset, ":");
  EasyUIDrawFrame(x + 3, y + page->rowheight + itemHeightOffset, barWidth, page->font.height, GUI_penColor);
  EasyUIDrawBox(x + 5, y + page->rowheight + itemHeightOffset + 2, (float)*item->param / item->paramMax * barWidth - 4,
                page->font.height - 4, GUI_penColor);
#if 0
  EasyUIDisplayFloat(x + width - 5 * page->font.width - 4, y + page->rowheight + itemHeightOffset, *item->param, 3, 2);
#else
	snprintf(tempstr, sizeof tempstr, "%d", *item->param);
	EasyUIDisplayStr(x + width - 2 * page->font.width - 2, y + page->rowheight + itemHeightOffset,
										 tempstr); 
#endif
  EasyUISendBuffer();
}

/*!
 * @brief   Draw check box
 *
 * @param   x           Check box position x
 * @param   y           Check box position y
 * @param   size        Size of check box
 * @param   offset      Offset of selected rounded box
 * @param   boolValue   True of false
 * @return  void
 *
 * @note    Internal call
 */
void EasyUIDrawCheckbox(int16_t x, int16_t y, uint16_t size, uint8_t offset, bool boolValue, uint8_t r)
{
  EasyUIDrawRFrame(x, y, size, size, GUI_penColor, r);
  if (boolValue)
    EasyUIDrawRBox(x + offset, y + offset, size - 2 * offset, size - 2 * offset, GUI_penColor, r);
}

/*!
 * @brief   Draw radio button
 *
 * @param   x           Radio button position x
 * @param   y           Radio button position y
 * @param   size        Size of radio button
 * @param   offset      Offset of selected rounded box
 * @param   boolValue   True of false
 * @return  void
 *
 * @note    Internal call
 */
void EasyUIDrawRadio(int16_t x, int16_t y, uint16_t size, uint8_t offset, bool boolValue, uint8_t r)
{
  EasyUIDrawRFrame(x, y, size, size, GUI_penColor, r - 1);
  if (boolValue)
    EasyUIDrawRBox(x + offset, y + offset, size - 2 * offset, size - 2 * offset, GUI_penColor, r - 2 * offset);
}

/*!
 * @brief   Get position of item with linear animation
 *
 * @param   page    Struct of page
 * @param   item    Struct of item
 * @param   index   Current index
 * @param   timer   Fill this with interrupt trigger time
 * @return  void
 *
 * @note    Internal call
 */
void EasyUIGetItemPos(EasyUIPage_t *page, EasyUIItem_t *item, uint8_t index, uint8_t timer)
{
  uint8_t itemHeightOffset = 0;
  static uint16_t time = 0;
  static int16_t move = 0, target = 0;
  static uint8_t lastIndex = 0, moveFlag = 0;
  uint8_t speed = ITEM_MOVE_TIME / timer;
  itemHeightOffset = (page->rowheight - page->font.height) / 2;
  // Item need to move or not
  if (moveFlag == 0)
  {
    for (EasyUIItem_t *itemTmp = page->itemHead; itemTmp != NULL; itemTmp = itemTmp->next)
    {
      if (index == itemTmp->id && itemTmp->lineId < 0)
      {
        move = itemTmp->lineId;
        moveFlag = 1;
        break;
      }
      else if (index == itemTmp->id && itemTmp->lineId > ((uint8_t)(SCREEN_HEIGHT / page->rowheight)) - 1)
      {
        move = itemTmp->lineId - ((uint8_t)(SCREEN_HEIGHT / page->rowheight)) + 1;
        moveFlag = 1;
        break;
      }
    }
  }

  page->itemHead->lineId -= move;
  // Change the item lineId and get target position
  for (EasyUIItem_t *itemTmp = page->itemHead; itemTmp != NULL; itemTmp = itemTmp->next)
  {
    if (itemTmp->next != NULL)
      itemTmp->next->lineId = itemTmp->lineId + 1;
  }
  move = 0;
  moveFlag = 0;
  target = itemHeightOffset + item->lineId * page->rowheight;

  /*
   * 首次显示该页面：条目位置还没初始化（都是 0），
   * 若走下面的动画就会从屏幕顶部“掉下来”。这里直接落到目标位置。
   */
  if (time == 0 && item->posForCal == 0.0f && item->position == 0 && target != 0)
  {
    item->posForCal = (float)target;
    item->position = (int16_t)target;
    item->step = 0;
    lastIndex = index;
    if (item->next == NULL)
      time = 0;
    return;
  }

  // Calculate current position
  if (time == 0 || index != lastIndex)
  {
    item->step = ((float)target - (float)item->position) / (float)speed;
  }
  if (time >= ITEM_MOVE_TIME)
  {
    item->posForCal = target;
  }
  else
    item->posForCal += item->step;

  item->position = (int16_t)item->posForCal;
  lastIndex = index;

  // Time counter
  if (item->next == NULL)
  {
    if (target == item->position)
      time = 0;
    else
      time += timer;
  }
}

/*!
 * @brief   Display item according to its funcType
 * @param   item    Struct of item
 * @return  void
 *
 * @note    Internal call
 */
void EasyUIDisplayItem(EasyUIPage_t *page, EasyUIItem_t *item)
{
	char tempstr[20];
  EasyUISetFont(page->font.type);
  switch (item->funcType)
  {
  case ITEM_JUMP_PAGE:
    EasyUIDisplayStr(2, item->position, "+");
    EasyUIDisplayStr(5 + page->font.width, item->position, item->title);
    break;
  case ITEM_PAGE_DESCRIPTION:
    EasyUIDisplayStr(2, item->position, item->title);
    break;
  case ITEM_RADIO_BUTTON:
    EasyUIDisplayStr(2, item->position, "-");
    EasyUIDisplayStr(5 + page->font.width, item->position, item->title);
    EasyUIDrawRadio(SCREEN_WIDTH - 7 - SCROLL_BAR_WIDTH - page->rowheight + 2,
                    item->position - (page->rowheight - page->font.height) / 2 + 1, page->rowheight - 2, RADIO_BUTTON_OFFSET,
                    *item->flag, (page->rowheight - 2) / 2);
    break;
  case ITEM_CHECKBOX:
    EasyUIDisplayStr(2, item->position, "-");
    EasyUIDisplayStr(5 + page->font.width, item->position, item->title);
    EasyUIDrawCheckbox(SCREEN_WIDTH - 7 - SCROLL_BAR_WIDTH - page->rowheight + 2,
                       item->position - (page->rowheight - page->font.height) / 2 + 1, page->rowheight - 2, CHECK_BOX_OFFSET,
                       *item->flag, 1);
    break;
  case ITEM_SWITCH:
    EasyUIDisplayStr(2, item->position, "-");
    EasyUIDisplayStr(5 + page->font.width, item->position, item->title);
    if (*item->flag)
      EasyUIDisplayStr(SCREEN_WIDTH - 7 - 2 * page->font.width - SCROLL_BAR_WIDTH, item->position, "ON");
    else
      EasyUIDisplayStr(SCREEN_WIDTH - 7 - 3 * page->font.width - SCROLL_BAR_WIDTH, item->position, "OFF");
    break;
  case ITEM_PROGRESS_BAR:
  case ITEM_CHANGE_VALUE:
    EasyUIDisplayStr(2, item->position, "-");
    EasyUIDisplayStr(5 + page->font.width, item->position, item->title);
    if (*item->param < 10)
		{
			snprintf(tempstr, sizeof tempstr, "%d", *item->param);
      EasyUIDisplayStr(SCREEN_WIDTH - 0 - 2 * page->font.width - SCROLL_BAR_WIDTH, item->position,
                         tempstr); 
		}
//    else if (*item->param < 100 && *item->param > -10)
//      EasyUIDisplayFloat(SCREEN_WIDTH - 0 - 5 * page->font.width - SCROLL_BAR_WIDTH, item->position,
//                         *item->param, 3, 0);
//    else if (*item->param < 1000 && *item->param > -100)
//      EasyUIDisplayFloat(SCREEN_WIDTH - 0 - 6 * page->font.width - SCROLL_BAR_WIDTH, item->position,
//                         *item->param, 4, 0);
//    else if (*item->param < 10000 && *item->param > -1000)
//      EasyUIDisplayFloat(SCREEN_WIDTH - 0 - 7 * page->font.width - SCROLL_BAR_WIDTH, item->position,
//                         *item->param, 4, 0);
    else if (*item->param < 100)
		{
			snprintf(tempstr, sizeof tempstr, "%2d", *item->param);
      EasyUIDisplayStr(SCREEN_WIDTH - 0 - 3 * page->font.width - SCROLL_BAR_WIDTH, item->position,
                         tempstr); 
		}
    break;
  case ITEM_DETAIL:
    EasyUIDisplayStr(5 + page->font.width, item->position, item->title);
    break;
  default:
    EasyUIDisplayStr(2, item->position, "-");
    EasyUIDisplayStr(5 + page->font.width, item->position, item->title);
    break;
  }
}

/*!
 * @brief   Get position of indicator and scroll bar with linear animation
 *
 * @param   page    Struct of page
 * @param   index   Current index
 * @param   timer   Fill this with interrupt trigger time
 * @param   status  Fill this with 1 to reset height
 * @return  void
 *
 * @note    Internal call
 */
void EasyUIDrawIndicator(EasyUIPage_t *page, uint8_t index, uint8_t timer, uint8_t status)
{
  static float stepLength = 0, stepY = 0, length = 0, y = 0;
  static uint16_t time = 0;
  static uint8_t lastIndex = 0;
  static uint16_t lengthTarget = 0, yTarget = 0;
  static float stepbarPos = 0, barPos = 0, barPosTarget = 0;
  uint8_t speed = INDICATOR_MOVE_TIME / timer;
  if (status)
    y = 0;

  if (page->funcType != PAGE_LIST)
    return;

  // Get Initial length
  if ((int)length == 0)
  {
    length = (float)(strlen(page->itemHead->title) + 1) * page->font.width + 8;
  }

  // Get target length and y
  for (EasyUIItem_t *itemTmp = page->itemHead; itemTmp != NULL; itemTmp = itemTmp->next)
  {
    if (index == itemTmp->id)
    {
      if (page->itemTail->id * page->rowheight > SCREEN_HEIGHT)
        barPosTarget = index * (SCREEN_HEIGHT - page->rowheight) / page->itemTail->id;
      else
        barPosTarget = index * page->itemTail->id * page->rowheight / page->itemTail->id;

      if (itemTmp->funcType == ITEM_PAGE_DESCRIPTION || itemTmp->funcType == ITEM_DETAIL)
        lengthTarget = (strlen(itemTmp->title)) * page->font.width + 8;
      else
        lengthTarget = (strlen(itemTmp->title) + 1) * page->font.width + 8;
      yTarget = itemTmp->lineId * page->rowheight;
      if (index != lastIndex && abs(index - lastIndex) < page->itemTail->id)
      {
        if (itemTmp->position < 0)
          y = (float)3 * page->rowheight / 4;
        else if (itemTmp->position >= (((uint8_t)(SCREEN_HEIGHT / page->rowheight))) * page->rowheight)
          y = (((uint8_t)(SCREEN_HEIGHT / page->rowheight)) - 2) * page->rowheight + (float)page->rowheight / 4;
      }
      break;
    }
  }

  // Calculate current position
  if (time == 0 || index != lastIndex)
  {
    stepLength = ((float)lengthTarget - (float)length) / (float)speed;
    stepY = ((float)yTarget - (float)y) / (float)speed;
    stepbarPos = ((float)barPosTarget - (float)barPos) / (float)speed;
  }
  if (time >= ITEM_MOVE_TIME)
  {
    length = lengthTarget;
    y = yTarget;
    barPos = barPosTarget;
  }
  else
  {
    length += stepLength;
    y += stepY;
    barPos += stepbarPos;
  }

  // Draw rounded box and scroll bar
  EasyUISetDrawColor(XOR);
  EasyUIDrawRBox(0, (int16_t)y, (int16_t)length, page->rowheight, GUI_actColor, 1);
  EasyUISetDrawColor(NORMAL);
  EasyUIDrawRBox(SCREEN_WIDTH - SCROLL_BAR_WIDTH, barPos, SCROLL_BAR_WIDTH, page->rowheight, GUI_penColor, 1);
  lastIndex = index;

  // Time counter
  if ((int)length == lengthTarget && (int)y == yTarget)
    time = 0;
  else
    time += timer;
}

/*!
 * @brief   Different response to operation according to funcType
 *
 * @param   page    Struct of page
 * @param   item    Struct of item
 * @param   index   Current index
 * @return  void
 *
 * @note    Internal call
 */
void EasyUIItemOperationResponse(EasyUIPage_t *page, EasyUIItem_t *item, uint8_t *index)
{
  switch (item->funcType)
  {
  case ITEM_JUMP_PAGE:
    if (layer == MAX_LAYER - 1)
      break;
    if (pageIndex[layer] == item->pageId)
      break;
    itemIndex[layer++] = *index;
    pageIndex[layer] = item->pageId;
    *index = 0;
    for (EasyUIItem_t *itemTmp = page->itemHead; itemTmp != NULL; itemTmp = itemTmp->next)
    {
      if (itemTmp->lineId < 0)
        continue;

      itemTmp->position = 0;
      itemTmp->posForCal = 0;
    }
    EasyUITransitionAnim();
    break;
  case ITEM_CHECKBOX:
  case ITEM_SWITCH:
    *item->flag = !*item->flag;
    break;
  case ITEM_RADIO_BUTTON:
    for (EasyUIItem_t *itemTmp = page->itemHead; itemTmp != NULL; itemTmp = itemTmp->next)
    {
      if (itemTmp->funcType == ITEM_RADIO_BUTTON && itemTmp->id != item->id)
        *itemTmp->flag = false;
    }
    *item->flag = !*item->flag;
    break;
  case ITEM_PROGRESS_BAR:
  case ITEM_CHANGE_VALUE:
    functionIsRunning = true;
    EasyUIBackgroundBlur();
    break;
  case ITEM_MESSAGE:
    functionIsRunning = true;
    EasyUIDrawMsgBox(page, item->msg);
    break;
  case ITEM_DETAIL:
    functionIsRunning = true;
    EasyUIDrawMsgBox(page, item->msg);
    break;
  default:
    break;
  }
}

/*!
 * @brief   Event: change variable's value
 *
 * @param   item    EasyUI item struct
 * @return  void
 */
void EasyUIEventChangeUint(EasyUIPage_t *page, EasyUIItem_t *item)
{
  static int16_t x, y;
  static uint16_t width, height;
  static uint8_t index = 1, step = 1;
  static uint8_t itemHeightOffset = 0;
  static bool changeVal = false;
  itemHeightOffset = (page->rowheight - page->font.height) / 2 + 1;
  EasyUISetDrawColor(NORMAL);
  EasyUISetFont(page->font.type);

  // Display information and draw box
  height = page->rowheight * 4 + 2;
  if (strlen(item->title) + 1 > 12)
    width = (strlen(item->title) + 1) * page->font.width + 7;
  else
    width = 12 * page->font.width + 7;
  if (width < 2 * SCREEN_WIDTH / 3)
    width = 2 * SCREEN_WIDTH / 3;
  x = (SCREEN_WIDTH - width) / 2;
  y = (SCREEN_HEIGHT - height) / 2;
  EasyUIDrawFrame(x - 1, y - 1, width + 2, height + 2, GUI_penColor);
  EasyUIDrawBox(x, y, width, height, GUI_backgroundColor);
  EasyUIDisplayStr(x + 3, y + itemHeightOffset, item->title);
  EasyUIDisplayStr(x + 3 + strlen(item->title) * page->font.width, y + itemHeightOffset, ":");
  EasyUIDisplayStr(x + 3, y + 2 * page->rowheight + itemHeightOffset, "Step:");
  EasyUIDisplayStr(x + 3, y + 3 * page->rowheight + itemHeightOffset, "Save");
  EasyUIDisplayStr(x + width - 6 * page->font.width - 4, y + 3 * page->rowheight + itemHeightOffset, "Return");

  // Change value of param or step
  if (changeVal)
  {
    EasyUISetDrawColor(XOR);
    EasyUIDrawBox(x + 2, y + 2, (strlen(item->title) + 1) * page->font.width + 3, page->rowheight - 2, GUI_penColor);
    EasyUISetDrawColor(NORMAL);
    if (opnDown)
      *item->param += step;
    if (opnUp)
    {
      if (*item->param - step >= 0)
        *item->param -= step;
      else
        *item->param = 0;
    }
  }
  else
  {
    if (opnUp)
    {
      if (index < 4)
        index++;
      else
        index = 1;
    }
    if (opnDown)
    {
      if (index > 1)
        index--;
      else
        index = 4;
    }
  }

  // Display step
  EasyUIDisplayFloat(x + 3, y + page->rowheight + itemHeightOffset, *item->param, 8, 2);
  if (step == 1)
    EasyUIDisplayStr(x + 3 + 6 * page->font.width, y + 2 * page->rowheight + itemHeightOffset, "+1");
  else if (step == 10)
    EasyUIDisplayStr(x + 3 + 6 * page->font.width, y + 2 * page->rowheight + itemHeightOffset, "+10");
  else
    EasyUIDisplayStr(x + 3 + 6 * page->font.width, y + 2 * page->rowheight + itemHeightOffset, "+100");

  // Draw indicator
  if (index == 1)
    EasyUIDrawRFrame(x + 1, y + 1, (strlen(item->title) + 1) * page->font.width + 5, page->rowheight, GUI_penColor, 1);
  else if (index == 2)
    EasyUIDrawRFrame(x + 1, y + 1 + 2 * page->rowheight, 5 * page->font.width + 5, page->rowheight, GUI_penColor, 1);
  else if (index == 3)
    EasyUIDrawRFrame(x + 1, y + 1 + 3 * page->rowheight, 4 * page->font.width + 5, page->rowheight, GUI_penColor, 1);
  else
    EasyUIDrawRFrame(x + width - 6 * page->font.width - 6, y + 1 + 3 * page->rowheight, 6 * page->font.width + 5, page->rowheight,
                     GUI_penColor, 1);

  // Operation move reaction
  if (opnEnter)
  {
    if (index == 1)
      changeVal = true;
    else if (index == 2)
    {
      item->paramBackup = *item->param;
      functionIsRunning = false;
      EasyUIBackgroundBlur();
      index = 1;
      step = 1;
    }
    else
    {
      *item->param = item->paramBackup;
      functionIsRunning = false;
      EasyUIBackgroundBlur();
      index = 1;
      step = 1;
    }
  }
  if (opnExit)
  {
    if (index == 1)
      changeVal = false;
  }

  // Clear the states of key to monitor next key action
  opnEnter = opnExit = opnUp = opnDown = false;

  GUI_SendBuffer();
}

void EasyUIEventChangeInt(EasyUIPage_t *page, EasyUIItem_t *item)
{
  static int16_t x, y;
  static uint16_t width, height;
  static uint8_t index = 1, step = 1;
  static uint8_t itemHeightOffset = 0;
  static bool changeVal = false;
  itemHeightOffset = (page->rowheight - page->font.height) / 2 + 1;
  EasyUISetFont(page->font.type);
  EasyUISetDrawColor(NORMAL);

  // Display information and draw box
  height = page->rowheight * 4 + 2;
  if (strlen(item->title) + 1 > 12)
    width = (strlen(item->title) + 1) * page->font.width + 7;
  else
    width = 12 * page->font.width + 7;
  if (width < 2 * SCREEN_WIDTH / 3)
    width = 2 * SCREEN_WIDTH / 3;
  x = (SCREEN_WIDTH - width) / 2;
  y = (SCREEN_HEIGHT - height) / 2;
  EasyUIDrawFrame(x - 1, y - 1, width + 2, height + 2, GUI_penColor);
  EasyUIDrawBox(x, y, width, height, GUI_backgroundColor);
  EasyUIDisplayStr(x + 3, y + itemHeightOffset, item->title);
  EasyUIDisplayStr(x + 3 + strlen(item->title) * page->font.width, y + itemHeightOffset, ":");
  EasyUIDisplayStr(x + 3, y + 2 * page->rowheight + itemHeightOffset, "Step:");
  EasyUIDisplayStr(x + 3, y + 3 * page->rowheight + itemHeightOffset, "Save");
  EasyUIDisplayStr(x + width - 6 * page->font.width - 4, y + 3 * page->rowheight + itemHeightOffset, "Return");

  // Change value of param or step
  if (changeVal)
  {
    EasyUISetDrawColor(XOR);
    EasyUIDrawBox(x + 2, y + 2, (strlen(item->title) + 1) * page->font.width + 3, page->rowheight - 2, GUI_penColor);
    EasyUISetDrawColor(NORMAL);
    if (opnDown)
      *item->param += step;
    if (opnUp)
      *item->param -= step;
  }
  else
  {
    if (opnUp)
    {
      if (index < 4)
        index++;
      else
        index = 1;
    }
    if (opnDown)
    {
      if (index > 1)
        index--;
      else
        index = 4;
    }
  }

  // Display step
  EasyUIDisplayFloat(x + 3, y + page->rowheight + itemHeightOffset, *item->param, 8, 2);
  if (step == 1)
    EasyUIDisplayStr(x + 3 + 6 * page->font.width, y + 2 * page->rowheight + itemHeightOffset, "+1");
  else if (step == 10)
    EasyUIDisplayStr(x + 3 + 6 * page->font.width, y + 2 * page->rowheight + itemHeightOffset, "+10");
  else
    EasyUIDisplayStr(x + 3 + 6 * page->font.width, y + 2 * page->rowheight + itemHeightOffset, "+100");

  // Draw indicator
  if (index == 1)
    EasyUIDrawRFrame(x + 1, y + 1, (strlen(item->title) + 1) * page->font.width + 5, page->rowheight, GUI_penColor, 1);
  else if (index == 2)
    EasyUIDrawRFrame(x + 1, y + 1 + 2 * page->rowheight, 5 * page->font.width + 5, page->rowheight, GUI_penColor, 1);
  else if (index == 3)
    EasyUIDrawRFrame(x + 1, y + 1 + 3 * page->rowheight, 4 * page->font.width + 5, page->rowheight, GUI_penColor, 1);
  else
    EasyUIDrawRFrame(x + width - 6 * page->font.width - 6, y + 1 + 3 * page->rowheight, 6 * page->font.width + 5, page->rowheight,
                     GUI_penColor, 1);

  // Operation move reaction
  if (opnEnter)
  {
    if (index == 1)
      changeVal = true;
    else if (index == 2)
    {
      item->paramBackup = *item->param;
      functionIsRunning = false;
      EasyUIBackgroundBlur();
      index = 1;
      step = 1;
    }
    else
    {
      *item->param = item->paramBackup;
      functionIsRunning = false;
      EasyUIBackgroundBlur();
      index = 1;
      step = 1;
    }
  }
  if (opnExit)
  {
    if (index == 1)
      changeVal = false;
  }

  // Clear the states of key to monitor next key action
  opnEnter = opnExit = opnUp = opnDown = false;

  GUI_SendBuffer();
}

void EasyUIEventChangeFloat(EasyUIPage_t *page, EasyUIItem_t *item)
{
  static int16_t x, y;
  static uint16_t width, height;
  static uint8_t index = 1;
  static double step = 0.01;
  static uint8_t itemHeightOffset = 0;
  static bool changeVal = false;
  itemHeightOffset = (page->rowheight - page->font.height) / 2 + 1;
  EasyUISetFont(page->font.type);
  EasyUISetDrawColor(NORMAL);

  // Display information and draw box
  height = page->rowheight * 4 + 2;
  if (strlen(item->title) + 1 > 12)
    width = (strlen(item->title) + 1) * page->font.width + 7;
  else
    width = 12 * page->font.width + 7;
  if (width < 2 * SCREEN_WIDTH / 3)
    width = 2 * SCREEN_WIDTH / 3;
  x = (SCREEN_WIDTH - width) / 2;
  y = (SCREEN_HEIGHT - height) / 2;
  EasyUIDrawFrame(x - 1, y - 1, width + 2, height + 2, GUI_penColor);
  EasyUIDrawBox(x, y, width, height, GUI_backgroundColor);
  EasyUIDisplayStr(x + 3, y + itemHeightOffset, item->title);
  EasyUIDisplayStr(x + 3 + strlen(item->title) * page->font.width, y + itemHeightOffset, ":");
  EasyUIDisplayStr(x + 3, y + 2 * page->rowheight + itemHeightOffset, "Step:");
  EasyUIDisplayStr(x + 3, y + 3 * page->rowheight + itemHeightOffset, "Save");
  EasyUIDisplayStr(x + width - 6 * page->font.width - 4, y + 3 * page->rowheight + itemHeightOffset, "Return");

  // Change value of param or step
  if (changeVal)
  {
    EasyUISetDrawColor(XOR);
    EasyUIDrawBox(x + 2, y + 2, (strlen(item->title) + 1) * page->font.width + 3, page->rowheight - 2, GUI_penColor);
    EasyUISetDrawColor(NORMAL);
    if (opnDown)
      *item->param += step;
    if (opnUp)
      *item->param -= step;
  }
  // else if (changeStep)
  // {
  //   EasyUISetDrawColor(XOR);
  //   EasyUIDrawBox(x + 2, y + 2 + 2 * page->rowheight, 5 * page->font.width + 3, page->rowheight - 2, GUI_penColor);
  //   EasyUISetDrawColor(NORMAL);
  //   if (opnUp)
  //   {
  //     if (step == 0.01)
  //       step = 0.1;
  //     else if (step == 0.1)
  //       step = 1;
  //     else
  //       step = 0.01;
  //   }
  //   if (opnDown)
  //   {
  //     if (step == 0.01)
  //       step = 1;
  //     else if (step == 1)
  //       step = 0.1;
  //     else
  //       step = 0.01;
  //   }
  // }
  else
  {
    if (opnUp)
    {
      if (index < 4)
        index++;
      else
        index = 1;
    }
    if (opnDown)
    {
      if (index > 1)
        index--;
      else
        index = 4;
    }
  }

  // Display step
  EasyUIDisplayFloat(x + 3, y + page->rowheight + itemHeightOffset, *item->param, 8, 2);
  if (step == 0.01)
    EasyUIDisplayStr(x + 3 + 6 * page->font.width, y + 2 * page->rowheight + itemHeightOffset, "+0.01");
  else if (step == 0.1)
    EasyUIDisplayStr(x + 3 + 6 * page->font.width, y + 2 * page->rowheight + itemHeightOffset, "+0.1");
  else
    EasyUIDisplayStr(x + 3 + 6 * page->font.width, y + 2 * page->rowheight + itemHeightOffset, "+1");

  // Draw indicator
  if (index == 1)
    EasyUIDrawRFrame(x + 1, y + 1, (strlen(item->title) + 1) * page->font.width + 5, page->rowheight, GUI_penColor, 1);
  else if (index == 2)
    EasyUIDrawRFrame(x + 1, y + 1 + 2 * page->rowheight, 5 * page->font.width + 5, page->rowheight, GUI_penColor, 1);
  else if (index == 3)
    EasyUIDrawRFrame(x + 1, y + 1 + 3 * page->rowheight, 4 * page->font.width + 5, page->rowheight, GUI_penColor, 1);
  else
    EasyUIDrawRFrame(x + width - 6 * page->font.width - 6, y + 1 + 3 * page->rowheight, 6 * page->font.width + 5, page->rowheight,
                     GUI_penColor, 1);

  // Operation move reaction
  if (opnEnter)
  {
    if (index == 1)
      changeVal = true;
    else if (index == 2)
    {
      item->paramBackup = *item->param;
      functionIsRunning = false;
      EasyUIBackgroundBlur();
      index = 1;
      step = 0.01;
    }
    else
    {
      *item->param = item->paramBackup;
      functionIsRunning = false;
      EasyUIBackgroundBlur();
      index = 1;
      step = 0.01;
    }
  }
  if (opnExit)
  {
    if (index == 1)
      changeVal = false;
  }

  // Clear the states of key to monitor next key action
  opnEnter = opnExit = opnUp = opnDown = false;

  GUI_SendBuffer();
}

/*!
 * @brief   Event: Save and reset settings in flash
 *
 * @param   item    Useless param, just be there to meet the function requirement;
 */
void EasyUIEventSaveSettings(EasyUIItem_t *item)
{
  //    interrupt_global_disable();
  //    for (EasyUIPage_t *page = pageHead; page != NULL; page = page->next)
  //    {
  //        for (EasyUIItem_t *itemTmp = page->itemHead; itemTmp != NULL; itemTmp = itemTmp->next)
  //        {
  //            switch (itemTmp->funcType)
  //            {
  //            case ITEM_CHECKBOX:
  //            case ITEM_RADIO_BUTTON:
  //            case ITEM_SWITCH:
  //                SaveToFlash((int32_t *) itemTmp->flag);
  //                break;
  //            case ITEM_PROGRESS_BAR:
  //            case ITEM_CHANGE_VALUE:
  //                SaveToFlashWithConversion((double *) itemTmp->param);
  //                break;
  //            default:
  //                break;
  //            }
  //        }
  //    }
  //    FlashOperationEnd();
  //    interrupt_global_enable(1);
  functionIsRunning = false;
  EasyUIBackgroundBlur();
}

void EasyUIEventResetSettings(EasyUIItem_t *item)
{
  for (EasyUIPage_t *page = pageHead; page != NULL; page = page->next)
  {
    for (EasyUIItem_t *itemTmp = page->itemHead; itemTmp != NULL; itemTmp = itemTmp->next)
    {
      switch (itemTmp->funcType)
      {
      case ITEM_CHECKBOX:
      case ITEM_RADIO_BUTTON:
      case ITEM_SWITCH:
        *itemTmp->flag = itemTmp->flagDefault;
        break;
      case ITEM_PROGRESS_BAR:
      case ITEM_CHANGE_VALUE:
        *itemTmp->param = itemTmp->paramDefault;
      default:
        break;
      }
    }
  }
  functionIsRunning = false;
  EasyUIBackgroundBlur();
}

/*!
 * @brief   Welcome Page with two size of photo, and read params from flash if not empty
 *
 * @param   mode    choose the size of photo (0 for smaller one and 1 for bigger one)
 * @return  void
 */
void EasyUIInit(uint8_t mode)
{
  EasyUIScreenInit();

  // Power-off storage
  //    if (flash_check(flashSecIndex, flashPageIndex))
  //    {
  //        interrupt_global_disable();
  //        for (EasyUIPage_t *page = pageHead; page != NULL; page = page->next)
  //        {
  //            for (EasyUIItem_t *itemTmp = page->itemHead; itemTmp != NULL; itemTmp = itemTmp->next)
  //            {
  //                switch (itemTmp->funcType)
  //                {
  //                case ITEM_CHECKBOX:
  //                case ITEM_RADIO_BUTTON:
  //                case ITEM_SWITCH:
  //                    ReadFlash((int32_t *) itemTmp->flag);
  //                    break;
  //                case ITEM_PROGRESS_BAR:
  //                case ITEM_CHANGE_VALUE:
  //                    ReadFlashWithConversion((double *) itemTmp->param);
  //                    break;
  //                default:
  //                    break;
  //                }
  //            }
  //        }
  //        FlashOperationEnd();
  //        interrupt_global_enable(1);
  //    }
}

void update_filter(Filter *filter)
{
  float diff = fabs(filter->target_value - filter->current_value);

  if (diff > 20.0f)
  {
    filter->current_value = filter->target_value;
  }
  else if (diff > 10.0f)
  {
    if (filter->target_value > filter->current_value)
      filter->current_value += 2.0f;
    else
      filter->current_value -= 2.0f;
  }
  else if (diff > 5.0f)
  {
    if (filter->target_value > filter->current_value)
      filter->current_value += 1.0f;
    else
      filter->current_value -= 1.0f;
  }
  else if (diff > 2.0f)
  {
    if (filter->target_value > filter->current_value)
      filter->current_value += 0.5f;
    else
      filter->current_value -= 0.5f;
  }
  else if (diff > 0.5f)
  {
    if (filter->target_value > filter->current_value)
      filter->current_value += 0.1f;
    else
      filter->current_value -= 0.1f;
  }
  else
  {
    filter->current_value = filter->target_value;
  }
}

/*!
 * @brief   Main function of EasyUI
 *
 * @param   timer   Fill this with interrupt trigger time
 * @return  void
 */
void EasyUIEvent(uint8_t timer)
{
  static uint8_t index = 0, itemSum = 0;

  EasyUIModifyColor();
  EasyUISetDrawColor(NORMAL);

  // Get current page by id
  EasyUIPage_t *page = pageHead;
  while (page->id != pageIndex[layer])
  {
    page = page->next;
  }

  // Quit UI to run function
  // If running function and hold the confirm button, quit the function
  if (functionIsRunning)
  {
    dbmsg("functionIsRunning");
    for (EasyUIItem_t *item = page->itemHead; item != NULL; item = item->next)
    {
      if (item->id != index)
      {
        continue;
      }

      switch (item->funcType)
      {
      case ITEM_PROGRESS_BAR:
        EasyUIDrawProgressBar(page, item);
        item->Event(item);
        break;
      case ITEM_DETAIL:
        break;
      default:
        item->Event(item);
        break;
      }
      break;
    }
    if (opnEnter || opnExit || opnUp || opnDown)
    {
      functionIsRunning = false;
      opnEnter = opnExit = opnUp = opnDown = false;
    }
    return;
  }

  EasyUIClearBuffer();

  switch (page->funcType)
  {
  case PAGE_CUSTOM:
  {
    page->Event(page);

    // Clear the states of key to monitor next key action
    opnEnter = opnUp = opnDown = false;

    if (layer == 0)
    {
      opnExit = false;
      break;
    }

    if (opnExit)
    {
      opnExit = false;
      pageIndex[layer] = 0;
      itemIndex[layer--] = 0;
      index = itemIndex[layer];
      EasyUITransitionAnim();
      EasyUIDrawIndicator(page, index, timer, 1);
    }
    break;
  }

  case PAGE_ICON:
  {

    // Clear the states of key to monitor next key action
    opnEnter = opnUp = opnDown = false;

    if (layer == 0)
    {
      opnExit = false;
      break;
    }

    if (opnExit)
    {
      opnExit = false;
      pageIndex[layer] = 0;
      itemIndex[layer--] = 0;
      index = itemIndex[layer];
      EasyUITransitionAnim();
      EasyUIDrawIndicator(page, index, timer, 1);
    }

    break;
  }
	
  case PAGE_LIST:
    for (EasyUIItem_t *item = page->itemHead; item != NULL; item = item->next)
    {
      EasyUIGetItemPos(page, item, index, timer);
      EasyUIDisplayItem(page, item);
    }
    // Draw indicator and scroll bar
    EasyUIDrawIndicator(page, index, timer, 0);

    if (itemSum == index && index != 0) // Indicator aways on last item when the indicator on last item and new item added
    {
      if (page->itemTail->id > itemSum)
        index = page->itemTail->id;
    }
    // Operation move reaction
    itemSum = page->itemTail->id;
    if (itemSum < index) // Indicator return to first item when the page items cleared
      index = 0;

    if (opnDown)
    {
      if (index < itemSum)
        index++;
      else if (listLoop)
        index = 0;
    }
    if (opnUp)
    {
      if (index > 0)
        index--;
      else if (listLoop)
        index = itemSum;
    }
    if (opnEnter)
    {
      for (EasyUIItem_t *item = page->itemHead; item != NULL; item = item->next)
      {
        if (item->id != index)
        {
          continue;
        }

        EasyUIItemOperationResponse(page, item, &index);
        break;
      }
    }

    // Clear the states of key to monitor next key action
    opnEnter = opnUp = opnDown = false;

    if (layer == 0)
    {
      // opnExit = false;
      break;
    }
    if (opnExit)
    {
      opnExit = false;
      pageIndex[layer] = 0;
      itemIndex[layer--] = 0;
      index = itemIndex[layer];
      for (EasyUIItem_t *itemTmp = page->itemHead; itemTmp != NULL; itemTmp = itemTmp->next)
      {
        itemTmp->position = 0;
        itemTmp->posForCal = 0;
      }
      EasyUITransitionAnim();
    }
		
    break;
  default:
    dbmsg("funcType: %d", page->funcType);
    break;
  }

  EasyUISendBuffer();
}
