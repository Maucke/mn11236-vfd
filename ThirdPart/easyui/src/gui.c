/*!
 * Copyright (c) 2023, ErBW_s
 * All rights reserved.
 *
 * @author  Baohan
 */

#include "gui.h"

uint16 GUI_penColor = GUI_DEFAULT_PENCOLOR;
uint16 GUI_actColor = GUI_DEFAULT_ACTCOLOR;
uint16 GUI_backgroundColor = 0;

static Font_Type_t GUI_display_font = GUI_DEFAULT_DISPLAY_FONT;
static uint16 GUI_x_max = LCD_W;
static uint16 GUI_y_max = LCD_H;
static uint8_t GUI_colorMode = NORMAL;

void GUI_SetFont(Font_Type_t font)
{
  GUI_display_font = font;
}

void GUI_SendBuffer(void)
{
  mn11236_send_buffer();
}

/*!
 * @brief   Clear buffer array
 *
 * @param   void
 * @return  void
 *
 * @note    Use black background color can easily clear the buffer by memset,
 *          but other color cannot and on board flash is not enough for another buffer
 *          to use memcpy, so using color other than black will slower this function.
 */
void GUI_ClearBuffer()
{
  mn11236_clear_buffers();
}

void GUI_SetDrawColor(GUI_ColorMode_e mode)
{
  GUI_colorMode = mode;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     GUI 画点
// 参数说明     x               坐标x方向的起点 参数范围 [0, GUI_x_max-1]
// 参数说明     y               坐标y方向的起点 参数范围 [0, GUI_y_max-1]
// 参数说明     color           颜色格式 RGB565 或者可以使用 zf_common_font.h 内 rgb565_color_enum 枚举值或者自行写入
// 返回参数     void
// 使用示例     GUI_draw_point(0, 0, RGB565_RED);            // 坐标 0,0 画一个红色的点
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
// 0 无颜色，1 红色，2 蓝色
__inline void GUI_DrawPoint(int16 x, int16 y, const uint16 color)
{
	mn11236_color_t color_ = (mn11236_color_t)((color - 1)&1);
  if (x < GUI_x_max && y < GUI_y_max && x >= 0 && y >= 0)
  {
		
    if (GUI_colorMode == XOR)
    {
      if (mn11236_read_pixel(x, y, MN11236_COLOR_RED))
        mn11236_clear_pixel(x, y, MN11236_COLOR_RED);
      else if (mn11236_read_pixel(x, y, MN11236_COLOR_BLUE))
        mn11236_clear_pixel(x, y, MN11236_COLOR_BLUE);
      else
        mn11236_draw_pixel(x, y, color_);
    }
    else
		{
			if(color == 0)
			{
        mn11236_clear_pixel(x, y, MN11236_COLOR_BLUE);
        mn11236_clear_pixel(x, y, MN11236_COLOR_RED);
			}
			else if(color < 3)
			{
				mn11236_draw_pixel(x, y, color_);
			}
			else
			{
        mn11236_draw_pixel(x, y, MN11236_COLOR_BLUE);
        mn11236_draw_pixel(x, y, MN11236_COLOR_RED);
			}
		}
  }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     GUI 画线
// 参数说明     x_start         坐标x方向的起点
// 参数说明     y_start         坐标y方向的起点
// 参数说明     x_end           坐标x方向的终点
// 参数说明     y_end           坐标y方向的终点
// 参数说明     color           颜色格式 RGB565 或者可以使用 zf_common_font.h 内 rgb565_color_enum 枚举值或者自行写入
// 返回参数     void
// 使用示例     GUI_draw_line(0, 0, 10, 10, RGB565_RED);     // 坐标 0,0 到 10,10 画一条红色的线
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void GUI_DrawLine(int16 x_start, int16 y_start, int16 x_end, int16 y_end, const uint16 color)
{
  int16 x_dir = (x_start < x_end ? 1 : -1);
  int16 y_dir = (y_start < y_end ? 1 : -1);
  float temp_rate = 0;
  float temp_b = 0;

  do
  {
    if (x_start != x_end)
    {
      temp_rate = (float)(y_start - y_end) / (float)(x_start - x_end);
      temp_b = (float)y_start - (float)x_start * temp_rate;
    }
    else
    {
      while (y_start != y_end)
      {
        GUI_DrawPoint(x_start, y_start, color);
        y_start += y_dir;
      }
      break;
    }
    if (func_abs(y_start - y_end) > func_abs(x_start - x_end))
    {
      while (y_start != y_end)
      {
        GUI_DrawPoint(x_start, y_start, color);
        y_start += y_dir;
        x_start = (int16)(((float)y_start - temp_b) / temp_rate);
      }
    }
    else
    {
      while (x_start != x_end)
      {
        GUI_DrawPoint(x_start, y_start, color);
        x_start += x_dir;
        y_start = (int16)((float)x_start * temp_rate + temp_b);
      }
    }
  } while (0);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     GUI 显示字符
// 参数说明     x               坐标x方向的起点 参数范围 [0, GUI_x_max-1]
// 参数说明     y               坐标y方向的起点 参数范围 [0, GUI_y_max-1]
// 参数说明     dat             需要显示的字符
// 返回参数     void
// 使用示例     GUI_show_char(0, 0, 'x');                    // 坐标 0,0 写一个字符 x
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void GUI_ShowChar(int16 x, int16 y, const char dat)
{
  uint8 i, j;
  switch (GUI_display_font)
  {
  case GUI_6X8_FONT:
    for (i = 0; i < 6; i++)
    {
      // 减 32 因为是取模是从空格开始取得 空格在 ascii 中序号是 32
      uint8 temp_top = ascii_default_6x8[dat - 32][i];
      for (j = 0; j < 8; j++)
      {
        if (temp_top & 0x01)
        {
          GUI_DrawPoint(x + i, y + j, GUI_penColor);
        }
        temp_top >>= 1;
      }
    }
    break;
  case GUI_8X16_FONT:
    for (i = 0; i < 8; i++)
    {
      // 减 32 因为是取模是从空格开始取得 空格在 ascii 中序号是 32
      uint8 temp_top = ascii_default_8x16[dat - 32][i];
      uint8 temp_bottom = ascii_default_8x16[dat - 32][i + 8];
      for (j = 0; j < 8; j++)
      {
        if (temp_top & 0x01)
        {
          GUI_DrawPoint(x + i, y + j, GUI_penColor);
        }
        temp_top >>= 1;
      }
      for (j = 0; j < 8; j++)
      {
        if (temp_bottom & 0x01)
        {
          GUI_DrawPoint(x + i, y + j + 8, GUI_penColor);
        }
        temp_bottom >>= 1;
      }
    }
    break;

  case GUI_8X16_OCRB:
    for (i = 0; i < 8; i++)
    {
      // 减 32 因为是取模是从空格开始取得 空格在 ascii 中序号是 32
      uint8 temp_top = ascii_OCRB_8x16[dat - 32][i];
      uint8 temp_bottom = ascii_OCRB_8x16[dat - 32][i + 8];
      for (j = 0; j < 8; j++)
      {
        if (temp_top & 0x01)
        {
          GUI_DrawPoint(x + i, y + j, GUI_penColor);
        }
        temp_top >>= 1;
      }
      for (j = 0; j < 8; j++)
      {
        if (temp_bottom & 0x01)
        {
          GUI_DrawPoint(x + i, y + j + 8, GUI_penColor);
        }
        temp_bottom >>= 1;
      }
    }
    break;
  case GUI_10X16_OCR:
    for (i = 0; i < 10; i++)
    {
      // 减 32 因为是取模是从空格开始取得 空格在 ascii 中序号是 32
      uint8 temp_top = ascii_OCR_10x16[dat - 32][i];
      uint8 temp_bottom = ascii_OCR_10x16[dat - 32][i + 10];
      for (j = 0; j < 8; j++)
      {
        if (temp_top & 0x01)
        {
          GUI_DrawPoint(x + i, y + j, GUI_penColor);
        }
        temp_top >>= 1;
      }
      for (j = 0; j < 8; j++)
      {
        if (temp_bottom & 0x01)
        {
          GUI_DrawPoint(x + i, y + j + 8, GUI_penColor);
        }
        temp_bottom >>= 1;
      }
    }
    break;
  case GUI_12X16_OCR:
    for (i = 0; i < 12; i++)
    {
      // 减 32 因为是取模是从空格开始取得 空格在 ascii 中序号是 32
      uint8 temp_top = ascii_OCR_12x16[dat - 32][i];
      uint8 temp_bottom = ascii_OCR_12x16[dat - 32][i + 12];
      for (j = 0; j < 8; j++)
      {
        if (temp_top & 0x01)
        {
          GUI_DrawPoint(x + i, y + j, GUI_penColor);
        }
        temp_top >>= 1;
      }
      for (j = 0; j < 8; j++)
      {
        if (temp_bottom & 0x01)
        {
          GUI_DrawPoint(x + i, y + j + 8, GUI_penColor);
        }
        temp_bottom >>= 1;
      }
    }
    break;
  case GUI_12X16_OCRB:
    for (i = 0; i < 12; i++)
    {
      // 减 32 因为是取模是从空格开始取得 空格在 ascii 中序号是 32
      uint8 temp_top = ascii_OCRB_12x16[dat - 32][i];
      uint8 temp_bottom = ascii_OCRB_12x16[dat - 32][i + 12];
      for (j = 0; j < 8; j++)
      {
        if (temp_top & 0x01)
        {
          GUI_DrawPoint(x + i, y + j, GUI_penColor);
        }
        temp_top >>= 1;
      }
      for (j = 0; j < 8; j++)
      {
        if (temp_bottom & 0x01)
        {
          GUI_DrawPoint(x + i, y + j + 8, GUI_penColor);
        }
        temp_bottom >>= 1;
      }
    }
    break;
  case GUI_12X24_AGENCY:
    for (i = 0; i < 12; i++)
    {
      // 减 32 因为是取模是从空格开始取得 空格在 ascii 中序号是 32
      uint8 temp_top = ascii_agency_12x24[dat - 32][i];
      uint8 temp_mid = ascii_agency_12x24[dat - 32][i + 12];
      uint8 temp_bottom = ascii_agency_12x24[dat - 32][i + 24];
      for (j = 0; j < 8; j++)
      {
        if (temp_top & 0x01)
        {
          GUI_DrawPoint(x + i, y + j, GUI_penColor);
        }
        temp_top >>= 1;
      }
      for (j = 0; j < 8; j++)
      {
        if (temp_mid & 0x01)
        {
          GUI_DrawPoint(x + i, y + j + 8, GUI_penColor);
        }
        temp_mid >>= 1;
      }
      for (j = 0; j < 8; j++)
      {
        if (temp_bottom & 0x01)
        {
          GUI_DrawPoint(x + i, y + j + 16, GUI_penColor);
        }
        temp_bottom >>= 1;
      }
    }
    break;
  case GUI_16X24_OCR:
    for (i = 0; i < 16; i++)
    {
      // 减 32 因为是取模是从空格开始取得 空格在 ascii 中序号是 32
      uint8 temp_top = ascii_OCR_16x24[dat - 32][i];
      uint8 temp_mid = ascii_OCR_16x24[dat - 32][i + 16];
      uint8 temp_bottom = ascii_OCR_16x24[dat - 32][i + 32];
      for (j = 0; j < 8; j++)
      {
        if (temp_top & 0x01)
        {
          GUI_DrawPoint(x + i, y + j, GUI_penColor);
        }
        temp_top >>= 1;
      }
      for (j = 0; j < 8; j++)
      {
        if (temp_mid & 0x01)
        {
          GUI_DrawPoint(x + i, y + j + 8, GUI_penColor);
        }
        temp_mid >>= 1;
      }
      for (j = 0; j < 8; j++)
      {
        if (temp_bottom & 0x01)
        {
          GUI_DrawPoint(x + i, y + j + 16, GUI_penColor);
        }
        temp_bottom >>= 1;
      }
    }
    break;
  case GUI_16X24_OCRB:
    for (i = 0; i < 16; i++)
    {
      // 减 32 因为是取模是从空格开始取得 空格在 ascii 中序号是 32
      uint8 temp_top = ascii_OCRB_16x24[dat - 32][i];
      uint8 temp_mid = ascii_OCRB_16x24[dat - 32][i + 16];
      uint8 temp_bottom = ascii_OCRB_16x24[dat - 32][i + 32];
      for (j = 0; j < 8; j++)
      {
        if (temp_top & 0x01)
        {
          GUI_DrawPoint(x + i, y + j, GUI_penColor);
        }
        temp_top >>= 1;
      }
      for (j = 0; j < 8; j++)
      {
        if (temp_mid & 0x01)
        {
          GUI_DrawPoint(x + i, y + j + 8, GUI_penColor);
        }
        temp_mid >>= 1;
      }
      for (j = 0; j < 8; j++)
      {
        if (temp_bottom & 0x01)
        {
          GUI_DrawPoint(x + i, y + j + 16, GUI_penColor);
        }
        temp_bottom >>= 1;
      }
    }
    break;

  default:
    break;
  }
}

void GUI_ShowStrMutiRow(int16 x, int16 y, int16 width, const char dat[])
{
  uint16 j = 0;
  int16 x_start = x;
  //  int16 y_start = y;
  while (dat[j] != '\0')
  {
    switch (GUI_display_font)
    {
    case GUI_6X8_FONT:
      if (dat[j] == '\n')
      {
        x = x_start;
        y += 8 + 2;
        while (dat[j] == ' ')
        {
          j++;
        }
      }
      else
      {
        if (x >= x_start + width - 6)
        {
          x = x_start;
          y += 8 + 2;
          while (dat[j] == ' ')
          {
            j++;
          }
        }
        GUI_ShowChar(x, y, dat[j]);
        x += 6;
      }
      break;
    default:
      break;
    }
    if (y >= LCD_H)
      return;
    j++;
  }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     GUI 显示字符串
// 参数说明     x               坐标x方向的起点 参数范围 [0, GUI_x_max-1]
// 参数说明     y               坐标y方向的起点 参数范围 [0, GUI_y_max-1]
// 参数说明     dat             需要显示的字符串
// 返回参数     void
// 使用示例     GUI_show_string(0, 0, "seekfree");
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void GUI_ShowStr(int16 x, int16 y, const char dat[])
{
  uint16 j = 0;
  int16 x_start = x;
  //  int16 y_start = y;
  while (dat[j] != '\0')
  {
    switch (GUI_display_font)
    {
    case GUI_6X8_FONT:
      if (dat[j] == '\n')
      {
        x = x_start;
        y += 8 + 2;
      }
      else
      {
//        if (x > LCD_W - 6 * 8)
//        {
//          if (dat[j] != '\0' && dat[j + 1] != '\0' && dat[j + 2] != '\0')
//          {
//            for (int i = 0; i < 3; i++)
//            {
//              GUI_ShowChar(x, y, '.');
//              x += 6;
//            }
//            return;
//          }
//        }
        GUI_ShowChar(x, y, dat[j]);
        x += 6;
      }
      break;
    case GUI_8X16_FONT:
    case GUI_8X16_OCRB:
      GUI_ShowChar(x, y, dat[j]);
      x += 8;
      if (x >= LCD_W - 8)
      {
        x = x_start;
        y += 16 + 4;
      }
      break;
    case GUI_10X16_OCR:
      GUI_ShowChar(x, y, dat[j]);
      x += 10;
      if (x >= LCD_W - 10)
      {
        x = x_start;
        y += 16 + 4;
      }
      break;
    case GUI_12X16_OCR:
    case GUI_12X16_OCRB:
      GUI_ShowChar(x, y, dat[j]);
      x += 12;
      if (x >= LCD_W - 12)
      {
        x = x_start;
        y += 16 + 24;
      }
      break;
    case GUI_12X24_AGENCY:
      GUI_ShowChar(x, y, dat[j]);
      x += 12;
      if (x >= LCD_W - 12)
      {
        x = x_start;
        y += 24 + 6;
      }
      break;
    case GUI_16X24_OCR:
    case GUI_16X24_OCRB:
      GUI_ShowChar(x, y, dat[j]);
      x += 16;
      if (x >= LCD_W - 16)
      {
        x = x_start;
        y += 24 + 6;
      }
      break;
    default:
      break;
    }
    if (y >= LCD_H)
      return;
    j++;
  }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     GUI 显示32位有符号 (去除整数部分无效的0)
// 参数说明     x               坐标x方向的起点 参数范围 [0, GUI_x_max-1]
// 参数说明     y               坐标y方向的起点 参数范围 [0, GUI_y_max-1]
// 参数说明     dat             需要显示的变量 数据类型 int32
// 参数说明     num             需要显示的位数 最高10位  不包含正负号
// 返回参数     void
// 使用示例     GUI_show_int(0, 0, x, 3);                    // x 可以为 int32 int16 int8 类型
// 备注信息     负数会显示一个 ‘-’号   正数显示一个空格
//-------------------------------------------------------------------------------------------------------------------
void GUI_ShowInt(int16 x, int16 y, const int32 dat, uint8 num)
{
  assert_param(num > 0);
  assert_param(num <= 10);

  int32 dat_temp = dat;
  int32 offset = 1;
  char data_buffer[12];

  memset(data_buffer, 0, 12);
  memset(data_buffer, ' ', num + 1);

  if (num < 10)
  {
    for (; num > 0; num--)
    {
      offset *= 10;
    }
    dat_temp %= offset;
  }
  func_int_to_str(data_buffer, dat_temp);
  GUI_ShowStr(x, y, (const char *)&data_buffer);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     GUI 显示32位无符号 (去除整数部分无效的0)
// 参数说明     x               坐标x方向的起点 参数范围 [0, GUI_x_max-1]
// 参数说明     y               坐标y方向的起点 参数范围 [0, GUI_y_max-1]
// 参数说明     dat             需要显示的变量 数据类型 uint32
// 参数说明     num             需要显示的位数 最高10位  不包含正负号
// 返回参数     void
// 使用示例     GUI_show_uint(0, 0, x, 3);                   // x 可以为 uint32 uint16 uint8 类型
// 备注信息     负数会显示一个 ‘-’号   正数显示一个空格
//-------------------------------------------------------------------------------------------------------------------
void GUI_ShowUint(int16 x, int16 y, const uint32 dat, uint8 num)
{
  assert_param(num > 0);
  assert_param(num <= 10);

  uint32 dat_temp = dat;
  int32 offset = 1;
  char data_buffer[12];
  memset(data_buffer, 0, 12);
  memset(data_buffer, ' ', num);

  if (num < 10)
  {
    for (; num > 0; num--)
    {
      offset *= 10;
    }
    dat_temp %= offset;
  }
  func_uint_to_str(data_buffer, dat_temp);
  GUI_ShowStr(x, y, (const char *)&data_buffer);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     GUI 显示浮点数 (去除整数部分无效的0)
// 参数说明     x               坐标x方向的起点 参数范围 [0, GUI_x_max-1]
// 参数说明     y               坐标y方向的起点 参数范围 [0, GUI_y_max-1]
// 参数说明     dat             需要显示的变量 数据类型 float 或 double
// 参数说明     num             整数位显示长度   最高8位
// 参数说明     pointnum        小数位显示长度   最高6位
// 返回参数     void
// 使用示例     GUI_show_float(0, 0, x, 2, 3);               // 显示浮点数 整数显示 2 位 小数显示 3 位
// 备注信息     特别注意当发现小数部分显示的值与你写入的值不一样的时候，
//              可能是由于浮点数精度丢失问题导致的，这并不是显示函数的问题，
//              有关问题的详情，请自行百度学习   浮点数精度丢失问题。
//              负数会显示一个 ‘-’号   正数显示一个空格
//-------------------------------------------------------------------------------------------------------------------
void GUI_ShowFloat(int16 x, int16 y, const float dat, uint8 num, uint8 pointnum)
{
  assert_param(num > 0);
  assert_param(num <= 8);
  assert_param(pointnum > 0);
  assert_param(pointnum <= 6);

  float dat_temp = dat;
  float offset = 1.0;
  char data_buffer[17];
  memset(data_buffer, 0, 17);
  memset(data_buffer, ' ', num + pointnum + 2);

  if (num < 10)
  {
    for (; num > 0; num--)
    {
      offset *= 10;
    }
    dat_temp = dat_temp - ((int)dat_temp / (int)offset) * offset;
  }
  func_float_to_str(data_buffer, dat_temp, pointnum);
  GUI_ShowStr(x, y, data_buffer);
}

void GUI_ModifyColor(void)
{
}

/*!
 * @brief   Draw circle / disc with 5 choices of section
 *
 * @param   x           Center x of the circle
 * @param   y           Center y of the circle
 * @param   r           Radius
 * @param   color       Color
 * @param   section     See definition in user_GUI.h
 * @return  void
 */
void GUI_DrawCircle(int16_t x, int16_t y, uint16_t r, const uint16_t color, uint8_t section)
{
  // y = kx, k = 1
  uint16_t x0 = (uint16_t)(r * cos(0.01745 * 45));
  uint16_t fx;

  // x^2 + y^2 = r^2, y->x / x->y
  for (int i = -x0 + 1; i < 0; ++i)
  {
    fx = (uint16_t)sqrt(pow(r, 2) - pow(i, 2));
    if (section & CIRCLE_UPPER_RIGHT)
    {
      GUI_DrawPoint(x - i, y - fx, color);
      GUI_DrawPoint(x + fx, y + i, color);
    }
    if (section & CIRCLE_UPPER_LEFT)
    {
      GUI_DrawPoint(x + i, y - fx, color);
      GUI_DrawPoint(x - fx, y + i, color);
    }
    if (section & CIRCLE_LOWER_LEFT)
    {
      GUI_DrawPoint(x + i, y + fx, color);
      GUI_DrawPoint(x - fx, y - i, color);
    }
    if (section & CIRCLE_LOWER_RIGHT)
    {
      GUI_DrawPoint(x - i, y + fx, color);
      GUI_DrawPoint(x + fx, y - i, color);
    }
  }

  // Add support for XOR color mode
  fx = (uint16_t)sqrt(pow(r, 2) - pow(x0, 2));
  if (section & CIRCLE_UPPER_RIGHT)
  {
    if (r > 1)
    {
      GUI_DrawPoint(x + r, y, color);
      GUI_DrawPoint(x, y - r, color);
    }
    if (x0 == fx)
      GUI_DrawPoint(x + x0, y - x0, color);
    else
    {
      GUI_DrawPoint(x + x0, y - fx, color);
      GUI_DrawPoint(x + fx, y - x0, color);
    }
  }
  if (section & CIRCLE_UPPER_LEFT)
  {
    if (r > 1)
    {
      GUI_DrawPoint(x - r, y, color);
      GUI_DrawPoint(x, y - r, color);
    }
    if (x0 == fx)
      GUI_DrawPoint(x - x0, y - x0, color);
    else
    {
      GUI_DrawPoint(x - x0, y - fx, color);
      GUI_DrawPoint(x - fx, y - x0, color);
    }
  }
  if (section & CIRCLE_LOWER_LEFT)
  {
    if (r > 1)
    {
      GUI_DrawPoint(x - r, y, color);
      GUI_DrawPoint(x, y + r, color);
    }
    if (x0 == fx)
      GUI_DrawPoint(x - x0, y + x0, color);
    else
    {
      GUI_DrawPoint(x - x0, y + fx, color);
      GUI_DrawPoint(x - fx, y + x0, color);
    }
  }
  if (section & CIRCLE_LOWER_RIGHT)
  {
    if (r > 1)
    {
      GUI_DrawPoint(x + r, y, color);
      GUI_DrawPoint(x, y + r, color);
    }
    if (x0 == fx)
      GUI_DrawPoint(x + x0, y + x0, color);
    else
    {
      GUI_DrawPoint(x + x0, y + fx, color);
      GUI_DrawPoint(x + fx, y + x0, color);
    }
  }
  if (section == CIRCLE_DRAW_ALL)
  {
    GUI_DrawPoint(x + r, y, color);
    GUI_DrawPoint(x - r, y, color);
    GUI_DrawPoint(x, y - r, color);
    GUI_DrawPoint(x, y + r, color);
  }
}
void GUI_DrawDisc(int16_t x, int16_t y, uint16_t r, const uint16_t color, uint8_t section)
{
  // y = kx, k = 1
  uint16_t x0 = (uint16_t)(r * cos(0.01745 * 45));
  uint16_t fx;

  // x^2 + y^2 = r^2, y->x / x->y
  for (int i = -x0 + 1; i < 0; ++i)
  {
    fx = (uint16_t)sqrt(pow(r, 2) - pow(i, 2));
    if (section & CIRCLE_UPPER_RIGHT)
    {
      GUI_DrawLine(x - i, y - fx, x - i, y + i, color);
      GUI_DrawLine(x + fx, y + i, x - i, y + i, color);
    }
    if (section & CIRCLE_UPPER_LEFT)
    {
      GUI_DrawLine(x + i, y - fx, x + i, y + i, color);
      GUI_DrawLine(x - fx, y + i, x + i, y + i, color);
    }
    if (section & CIRCLE_LOWER_LEFT)
    {
      GUI_DrawLine(x + i, y + fx, x + i, y - i, color);
      GUI_DrawLine(x - fx, y - i, x + i, y - i, color);
    }
    if (section & CIRCLE_LOWER_RIGHT)
    {
      GUI_DrawLine(x - i, y + fx, x - i, y - i, color);
      GUI_DrawLine(x + fx, y - i, x - i, y - i, color);
    }
  }

  // Add support for XOR color mode
  GUI_DrawPoint(x, y, color);
  if (r != 2)
    GUI_DrawPoint(x, y, color);

  fx = (uint16_t)sqrt(pow(r, 2) - pow(x0, 2));
  if (section & CIRCLE_UPPER_RIGHT)
  {
    if (r > 1)
    {
      GUI_DrawLine(x + r, y, x, y, color);
      GUI_DrawLine(x, y - r, x, y, color);
    }
    if (r > 2)
      GUI_DrawLine(x, y, x + x0, y - x0, color);
    if (x0 == fx)
      GUI_DrawPoint(x + x0, y - x0, color);
    else
    {
      GUI_DrawPoint(x + x0, y - fx, color);
      GUI_DrawPoint(x + fx, y - x0, color);
      GUI_DrawPoint(x + x0, y - x0, color);
    }
  }
  if (section & CIRCLE_UPPER_LEFT)
  {
    if (r > 1)
    {
      GUI_DrawLine(x - r, y, x, y, color);
      GUI_DrawLine(x, y - r, x, y, color);
    }
    if (r > 2)
      GUI_DrawLine(x, y, x - x0, y - x0, color);
    if (x0 == fx)
      GUI_DrawPoint(x - x0, y - x0, color);
    else
    {
      GUI_DrawPoint(x - x0, y - fx, color);
      GUI_DrawPoint(x - fx, y - x0, color);
      GUI_DrawPoint(x - x0, y - x0, color);
    }
  }
  if (section & CIRCLE_LOWER_LEFT)
  {
    if (r > 1)
    {
      GUI_DrawLine(x - r, y, x, y, color);
      GUI_DrawLine(x, y + r, x, y, color);
    }
    if (r > 2)
      GUI_DrawLine(x, y, x - x0, y + x0, color);
    if (x0 == fx)
      GUI_DrawPoint(x - x0, y + x0, color);
    else
    {
      GUI_DrawPoint(x - x0, y + fx, color);
      GUI_DrawPoint(x - fx, y + x0, color);
      GUI_DrawPoint(x - x0, y + x0, color);
    }
  }
  if (section & CIRCLE_LOWER_RIGHT)
  {
    if (r > 1)
    {
      GUI_DrawLine(x + r, y, x, y, color);
      GUI_DrawLine(x, y + r, x, y, color);
    }
    if (r > 2)
      GUI_DrawLine(x, y, x + x0, y + x0, color);
    if (x0 == fx)
      GUI_DrawPoint(x + x0, y + x0, color);
    else
    {
      GUI_DrawPoint(x + x0, y + fx, color);
      GUI_DrawPoint(x + fx, y + x0, color);
      GUI_DrawPoint(x + x0, y + x0, color);
    }
  }
  if (section == CIRCLE_DRAW_ALL)
  {
    GUI_DrawLine(x + r, y, x, y, color);
    GUI_DrawLine(x - r, y, x, y, color);
    GUI_DrawLine(x, y - r, x, y, color);
    GUI_DrawLine(x, y + r, x, y, color);
    GUI_DrawPoint(x, y, color);
  }
}

/*!
 * @brief   Draw box / filled box
 *
 * @param   x       Starting x
 * @param   y       Starting y
 * @param   width   Box width
 * @param   height  Box height
 * @param   color   Color
 * @return  void
 */
void GUI_DrawFrame(int16_t x, int16_t y, uint16_t width, uint16_t height, const uint16_t color)
{
  for (int i = x; i < x + width; i++)
  {
    GUI_DrawPoint(i, y, color);
    GUI_DrawPoint(i, y + height - 1, color);
  }
  for (int j = y; j < y + height; j++)
  {
    GUI_DrawPoint(x, j, color);
    GUI_DrawPoint(x + width - 1, j, color);
  }
}
void GUI_DrawBox(int16_t x, int16_t y, uint16_t width, uint16_t height, const uint16_t color)
{
  for (int j = y; j < y + height; ++j)
  {
    for (int i = x; i < x + width; ++i)
    {
      //					dbmsg("X:%d,Y:%d",i, j);
      GUI_DrawPoint(i, j, color);
    }
  }
}

/*!
 * @brief   Draw rounded box / filled box
 *
 * @param   x       Starting x
 * @param   y       Starting y
 * @param   width   Box width
 * @param   height  Box height
 * @param   color   Color
 * @param   r       Radius
 * @return  void
 */
void GUI_DrawRFrame(int16_t x, int16_t y, uint16_t width, uint16_t height, const uint16_t color, uint8_t r)
{
  for (int i = x + r + 1; i < x + width - r - 1; i++)
  {
    GUI_DrawPoint(i, y, color);
    GUI_DrawPoint(i, y + height - 1, color);
  }
  for (int j = y + r + 1; j < y + height - r - 1; j++)
  {
    GUI_DrawPoint(x, j, color);
    GUI_DrawPoint(x + width - 1, j, color);
  }

  GUI_DrawCircle(x + r, y + r, r, color, CIRCLE_UPPER_LEFT);
  GUI_DrawCircle(x + width - 1 - r, y + r, r, color, CIRCLE_UPPER_RIGHT);
  GUI_DrawCircle(x + r, y + height - 1 - r, r, color, CIRCLE_LOWER_LEFT);
  GUI_DrawCircle(x + width - 1 - r, y + height - 1 - r, r, color, CIRCLE_LOWER_RIGHT);
}
void GUI_DrawRBox(int16_t x, int16_t y, uint16_t width, uint16_t height, const uint16_t color, uint8_t r)
{
  GUI_DrawDisc(x + r, y + r, r, color, CIRCLE_UPPER_LEFT);
  GUI_DrawDisc(x + width - 1 - r, y + r, r, color, CIRCLE_UPPER_RIGHT);
  GUI_DrawDisc(x + r, y + height - 1 - r, r, color, CIRCLE_LOWER_LEFT);
  GUI_DrawDisc(x + width - 1 - r, y + height - 1 - r, r, color, CIRCLE_LOWER_RIGHT);

  GUI_DrawBox(x + r + 1, y, width - 2 - 2 * r, r + 1, color);
  GUI_DrawBox(x, y + r + 1, width, height - 2 * r - 2, color);
  GUI_DrawBox(x + r + 1, y + height - 1 - r, width - 2 - 2 * r, r + 1, color);
}

/*!
 * @brief   Show Binary BMP photo on screen
 *
 * @param   x       Starting x
 * @param   y       Starting y
 * @param   width   Pic width
 * @param   height  Pic height
 * @param   pic     The array of picture(阴码 逐行式 逆向)
 */
void GUI_ShowBMP(int16_t x, int16_t y, uint16_t width, uint16_t height, const uint8_t *pic)
{
  uint8_t temp, j;
  uint8_t x0 = x;
  uint8_t *tmp = (uint8_t *)pic;
  uint16_t i, picSize = 0;

  picSize = (width / 8 + ((width % 8) ? 1 : 0)) * height;

  for (i = 0; i < picSize; i++)
  {
    temp = tmp[i];
    for (j = 0; j < 8; j++)
    {
      if (temp & 0x01)
      {
        GUI_DrawPoint(x, y, GUI_penColor);
      }
      temp >>= 1;
      x++;

      if ((x - x0) == width)
      {
        x = x0;
        y++;
        break;
      }
    }
  }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     GUI 显示 8bit 灰度图像 带二值化阈值
// 参数说明     x               坐标x方向的起点 参数范围 [0, GUI_x_max-1]
// 参数说明     y               坐标y方向的起点 参数范围 [0, GUI_y_max-1]
// 参数说明     *image          图像数组指针
// 参数说明     width           图像实际宽度
// 参数说明     height          图像实际高度
// 参数说明     dis_width       图像显示宽度 参数范围 [0, GUI_x_max]
// 参数说明     dis_height      图像显示高度 参数范围 [0, GUI_y_max]
// 参数说明     threshold       二值化显示阈值 0-不开启二值化
// 返回参数     void
// 使用示例     GUI_show_gray_image(0, 0, mt9v03x_image[0], MT9V03X_W, MT9V03X_H, MT9V03X_W, MT9V03X_H, 0);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void GUI_ShowGrayImage(uint16_t x, uint16_t y, const uint8_t *image, uint16_t width, uint16_t height, uint16_t dis_width,
                       uint16_t dis_height, uint8_t threshold)
{
  uint16_t i, j;
  uint16_t color, temp;
  uint32_t width_index, height_index;

  for (j = 0; j < dis_height; j++)
  {
    height_index = j * height / dis_height;
    for (i = 0; i < dis_width; i++)
    {
      width_index = i * width / dis_width;
      temp = *(image + height_index * width + width_index);
      if (threshold == 0)
      {
        color = (0x001f & ((temp) >> 3)) << 11;
        color = color | (((0x003f) & ((temp) >> 2)) << 5);
        color = color | (0x001f & ((temp) >> 3));
        GUI_DrawPoint(x + i, y + j, color);
      }
      else if (temp < threshold)
        GUI_DrawPoint(x + i, y + j, RGB565_BLACK);
      else
        GUI_DrawPoint(x + i, y + j, RGB565_WHITE);
    }
  }
}
void GUI_Triangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
  GUI_DrawLine(x0, y0, x1, y1, color);
  GUI_DrawLine(x1, y1, x2, y2, color);
  GUI_DrawLine(x2, y2, x0, y0, color);
}

void GUI_FastHLine(int16_t x, int16_t y, int16_t length, uint16_t color)
{
  // Bounds check
  int16_t x0 = x;
  do
  {
    GUI_DrawPoint(x, y, color); // 逐点显示，描出垂直线
    x++;
  } while (x0 + length >= x);
}
#define swap(a, b)  \
  {                 \
    uint16_t t = a; \
    a = b;          \
    b = t;          \
  }
void GUI_FillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{

  int16_t a, b, y, last;
  // Sort coordinates by Y order (y2 >= y1 >= y0)
  if (y0 > y1)
  {
    swap(y0, y1);
    swap(x0, x1);
  }
  if (y1 > y2)
  {
    swap(y2, y1);
    swap(x2, x1);
  }
  if (y0 > y1)
  {
    swap(y0, y1);
    swap(x0, x1);
  }

  if (y0 == y2)
  {
    // Handle awkward all-on-same-line case as its own thing
    a = b = x0;
    if (x1 < a)
    {
      a = x1;
    }
    else if (x1 > b)
    {
      b = x1;
    }
    if (x2 < a)
    {
      a = x2;
    }
    else if (x2 > b)
    {
      b = x2;
    }
    GUI_FastHLine(a, y0, b - a, color);
    return;
  }

  int16_t dx01 = x1 - x0,
          dy01 = y1 - y0,
          dx02 = x2 - x0,
          dy02 = y2 - y0,
          dx12 = x2 - x1,
          dy12 = y2 - y1,
          sa = 0,
          sb = 0;

  // For upper part of triangle, find scanline crossings for segments
  // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
  // is included here (and second loop will be skipped, avoiding a /0
  // error there), otherwise scanline y1 is skipped here and handled
  // in the second loop...which also avoids a /0 error here if y0=y1
  // (flat-topped triangle).
  if (y1 == y2)
  {
    last = y1; // Include y1 scanline
  }
  else
  {
    last = y1 - 1; // Skip it
  }

  for (y = y0; y <= last; y++)
  {
    a = x0 + sa / dy01;
    b = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;

    if (a > b)
    {
      swap(a, b);
    }

    GUI_FastHLine(a, y, b - a, color);
  }

  // For lower part of triangle, find scanline crossings for segments
  // 0-2 and 1-2.  This loop is skipped if y1=y2.
  sa = dx12 * (y - y1);
  sb = dx02 * (y - y0);

  for (; y <= y2; y++)
  {
    a = x1 + sa / dy12;
    b = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;

    if (a > b)
    {
      swap(a, b);
    }

    GUI_FastHLine(a, y, b - a, color);
  }
}

void GUI_SetDirection(GUI_dir_enum dir)
{
}

void GUI_DrawBMP565(int16_t x, int16_t y, uint16_t width, uint16_t height, const uint8_t *pic)
{
  for (int j = y; j < y + height; ++j)
  {
    for (int i = x; i < x + width; ++i)
    {
      //					dbmsg("X:%d,Y:%d",i, j);
      GUI_DrawPoint(i, j, (pic[(j - y) * width * 2 + 2 * (i - x) + 1]) | (pic[(j - y) * width * 2 + 2 * (i - x)] << 8));
    }
  }
}

void GUI_SetColor(const uint16 pen, const uint16 bgcolor)
{
  GUI_penColor = pen;
}

void GUI_Init()
{
  mn11236_init();
}
