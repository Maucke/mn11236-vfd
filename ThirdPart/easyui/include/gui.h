/*!
 * Copyright (c) 2023, ErBW_s
 * All rights reserved.
 * 
 * @author  Baohan
 */

#ifndef GUI_h_
#define GUI_h_

#ifdef __cplusplus
extern "C"
{
#endif

#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include "spi.h"
#include "zf_common_font.h"
#include "my_math.h"
#include "zf_common_function.h"
#include "mn11236.h"

#define LCD_W VFD_WIDTH
#define LCD_H VFD_HEIGHT

typedef enum
{
	DMA_MEMINC_ENABLE = 0,
	DMA_MEMINC_DISABLE
} DMA_MEMINC_STATE;
     
#define GUI_DEFAULT_PENCOLOR         (MN11236_COLOR_BLUE + 1)            
#define GUI_DEFAULT_ACTCOLOR         (MN11236_COLOR_RED + 1)                           
#define GUI_DEFAULT_DISPLAY_FONT     (GUI_6X8_FONT)                      

#define CIRCLE_UPPER_RIGHT      0x01
#define CIRCLE_UPPER_LEFT       0x02
#define CIRCLE_LOWER_LEFT       0x04
#define CIRCLE_LOWER_RIGHT      0x08
#define CIRCLE_DRAW_ALL         (CIRCLE_UPPER_RIGHT | CIRCLE_UPPER_LEFT | CIRCLE_LOWER_LEFT | CIRCLE_LOWER_RIGHT)

extern uint16 GUI_penColor;
extern uint16 GUI_actColor;
extern uint16 GUI_backgroundColor;

typedef enum
{
    NORMAL = 1,
    XOR
} GUI_ColorMode_e;

typedef enum
{
    GUI_INVALID                      = -1,     
    GUI_PORTAIT                      = 0,                                    
    GUI_PORTAIT_180                  = 1,                                    
    GUI_CROSSWISE                    = 2,                                  
    GUI_CROSSWISE_180                = 3,                                
}GUI_dir_enum;

typedef enum
{
    GUI_6X8_FONT                     = 0,                                    // 6x8    
    GUI_8X16_FONT                    = 1,                                    // 8x16   
    GUI_8X16_OCRB,              
    GUI_10X16_OCR,    
    GUI_12X16_OCR,    
    GUI_12X16_OCRB,   
    GUI_12X24_AGENCY,    
    GUI_16X24_OCR,    
		GUI_16X24_OCRB,      //不好看       
    GUI_NUM_FONT          
}Font_Type_t;

void GUI_SetFont(Font_Type_t font);
void GUI_DrawBMP232(int16_t x, int16_t y, uint16_t width, uint16_t height, const uint8_t* pic);
void GUI_DrawBMP565(int16_t x, int16_t y, uint16_t width, uint16_t height, const uint8_t* pic);

void GUI_SendBuffer(void);
void GUI_ClearBuffer(void);

void GUI_SetDrawColor(GUI_ColorMode_e mode);
void GUI_DrawPoint (int16 x, int16 y, uint16 color);
void GUI_DrawLine (int16 x_start, int16 y_start, int16 x_end, int16 y_end, uint16 color);
void GUI_ShowChar(int16 x, int16 y, char dat);
void GUI_ShowStr (int16 x, int16 y, const char dat[]);
void GUI_ShowStrMutiRow(int16 x, int16 y, int16 width, const char dat[]);
void GUI_ShowInt(int16 x, int16 y, int32 dat, uint8 num);
void GUI_ShowUint(int16 x, int16 y, uint32 dat, uint8 num);
void GUI_ShowFloat(int16 x, int16 y, float dat, uint8 num, uint8 pointnum);

void GUI_ModifyColor(void);

void GUI_DrawCircle(int16_t x, int16_t y, uint16_t r, uint16_t color, uint8_t section);
void GUI_DrawDisc(int16_t x, int16_t y, uint16_t r, uint16_t color, uint8_t section);
void GUI_DrawFrame(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t color);
void GUI_DrawBox(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t color);
void GUI_DrawRFrame(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t color, uint8_t r);
void GUI_DrawRBox(int16_t x, int16_t y, uint16_t width, uint16_t height, uint16_t color, uint8_t r);
void GUI_DrawRBoxWithBlur(int16_t x, int16_t y, uint16_t width, uint16_t height);

void GUI_ShowBMP(int16_t x, int16_t y, uint16_t width, uint16_t height, const uint8_t *pic);
void GUI_ShowGrayImage(uint16_t x, uint16_t y, const uint8_t *image, uint16_t width, uint16_t height, uint16_t dis_width,
                          uint16_t dis_height, uint8_t threshold);

void GUI_Triangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
void GUI_FillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
void GUI_SetColor (const uint16 pen, const uint16 bgcolor);
void GUI_Init (void);
void GUI_FastHLine(int16_t x, int16_t y, int16_t length, uint16_t color);

#ifdef __cplusplus
}
#endif

#endif
