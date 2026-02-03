#ifndef __MN11236_H__
#define __MN11236_H__

#include "main.h"

/* MN11236 Display Parameters */
#define VFD_WIDTH           112
#define VFD_HEIGHT          18
#define VFD_GRIDS           38              // 总栅极数     
#define VFD_FRAMES          (VFD_GRIDS-1)   // 每帧扫描行数
#define VFD_COLOR_OFFSET    6               //红色颜色位下标偏移量
#define VFD_LINE_OFFSET     12              //每行数据位下标偏移量

#define GRIDS_OFFSET 253 //栅极控制位下标起点G0

/* Memory Buffers */
#define DMA_BUFFER_SIZE 32  // 256 bits / 8 bits per byte


typedef enum {
    MN11236_COLOR_RED = 0,
    MN11236_COLOR_BLUE = 1,
} mn11236_color_t;

void mn11236_init(void);


/**
 * 清空全部缓冲区
 */
void mn11236_clear_buffers(void);

/**
 * 发送缓冲区数据到屏幕
 */
void mn11236_send_buffer(void);

/**
 * 画一个点,起点从0,0开始
 */
void mn11236_draw_pixel(uint8_t x, uint8_t y, mn11236_color_t color);

/**
 * 清一个点,起点从0,0开始
 */
void mn11236_clear_pixel(uint8_t x, uint8_t y, mn11236_color_t color);
	
/**
 * 亮度调整0~255级
 */
void mn11236_set_brightness(uint8_t level);

/* Development test function */
void mn11236_development_test(void);



#endif