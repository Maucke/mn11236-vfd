#include "mn11236.h"
#include <string.h>
#include <stdint.h>
#include "cmsis_os.h"
#include "spi.h"
#include "tim.h"

// 用户绘制的像素缓冲区
static uint8_t display_buffer[VFD_WIDTH][VFD_HEIGHT]; // [x][y][color]

/* 过渡动画期间暂停送屏（避免新画面先闪一帧完整的图） */
static uint8_t send_hold = 0;

/* 扫描开关：0 = DMA 不再接力，屏停在最后一段，完全不刷新 */
static volatile uint8_t scan_run = 1;

// DMA传输缓冲区（当前正在传输的行）
#define GRAY_BITS 3
#define GRAY_PLANES ((1 << GRAY_BITS) - 1)   // =7

static uint8_t dma_tx_buffer[VFD_FRAMES][GRAY_BITS][DMA_BUFFER_SIZE];

// 当前扫描行号
static volatile uint8_t current_grid = 0;
static volatile uint8_t current_plane = 0;
static volatile uint8_t plane_repeat = 0;

/**
 * 设置Bit位
 */
static void set_bit(uint8_t *buf, uint16_t bit_pos)
{
    buf[(bit_pos / 8)] |= (1 << (7 - (bit_pos % 8)));
}

/**
 * 清除Bit位
 */
static void reset_bit(uint8_t *buf, uint16_t bit_pos)
{
    buf[(bit_pos / 8)] &= ~(1 << (7 - (bit_pos % 8)));
}

///**
// * 获取Bit位
// */
//static uint8_t get_bit(const uint8_t *buf, uint16_t bit_pos)
//{
//    return buf[(bit_pos / 8)] & (1 << (7 - (bit_pos % 8)));
//}

/**
 * 初始化扫描Grid的激活栅极模板
 */
void init_grids_template(void)
{
    memset(dma_tx_buffer, 0x00, sizeof(dma_tx_buffer));
    // T0 = G1+G2、T1 = G2+G3 .....
    for (uint8_t t = 0; t < VFD_FRAMES; t++)
    {
			for (uint8_t g = 0; g < GRAY_BITS; g++)
			{
					uint8_t *buf = dma_tx_buffer[t][g];
					set_bit(buf, GRIDS_OFFSET - t);
					set_bit(buf, GRIDS_OFFSET - (t + 1));
			}
    }
}

/**
 * 下标转换
 */
static uint8_t abcdef_conversion(uint8_t idx)
{
    uint8_t x_abcdef;
    switch (idx)
    {
    case 0:
        x_abcdef = 0;
        break; /* a -> a */
    case 1:
        x_abcdef = 2;
        break; /* b -> c */
    case 2:
        x_abcdef = 4;
        break; /* c -> e */
    case 3:
        x_abcdef = 5;
        break; /* d -> f */
    case 4:
        x_abcdef = 3;
        break; /* e -> d */
    case 5:
        x_abcdef = 1;
        break; /* f -> b */
    }
    return x_abcdef;
}

void mn11236_setbrightness(uint8_t bright)
{
	if(bright)
		HAL_GPIO_WritePin(VFD_BK_GPIO_Port, VFD_BK_Pin, GPIO_PIN_RESET);
	else
		HAL_GPIO_WritePin(VFD_BK_GPIO_Port, VFD_BK_Pin, GPIO_PIN_SET);
}

void mn11236_init(void)
{

    // 初始化LAT和BK引脚状态
    mn11236_setbrightness(0);
//    // 高电平关闭显示
//    HAL_GPIO_WritePin(VFD_BK_GPIO_Port, VFD_BK_Pin, GPIO_PIN_SET);

    mn11236_setbrightness(50);
	
    // 初始化栅极模板
    init_grids_template();

    // 清空所有缓冲区
    mn11236_clear_buffers();

    current_grid = 0;
		current_plane = 0;
		plane_repeat = 0;
    HAL_SPI_Transmit_DMA(
        &hspi2,
        (uint8_t *)dma_tx_buffer[current_grid][current_plane],
        DMA_BUFFER_SIZE);
}

int fps = 0;

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi != &hspi2)
        return;

    HAL_GPIO_WritePin(VFD_LAT_GPIO_Port, VFD_LAT_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(VFD_LAT_GPIO_Port, VFD_LAT_Pin, GPIO_PIN_RESET);

    /* 停扫：不再接力。注意这里不能做"送全 0 遮挡"——写 Flash 时这个
     * 中断本身就会被 Flash 总线停住推迟到擦除结束，等于没遮。 */
    if (!scan_run)
        return;
		   
		plane_repeat++;

		if (plane_repeat >= (1 << current_plane))
		{
				plane_repeat = 0;
				current_plane++;

				if (current_plane >= GRAY_BITS)
				{
						current_plane = 0;
						current_grid = (current_grid + 1) % VFD_FRAMES;
						if(current_grid == 0)
							fps++;
				}
		}
		
    HAL_SPI_Transmit_DMA(
        &hspi2,
        (uint8_t *)dma_tx_buffer[current_grid][current_plane],
        DMA_BUFFER_SIZE);
}

/**
 * 清空全部缓冲区
 */
void mn11236_clear_buffers(void)
{
    // 清空像素缓冲区
    memset(display_buffer, 0, sizeof(display_buffer));
}

/* 把 DMA 缓冲的像素位清 0，保留每帧的栅极激活位
 * （栅极位一起清掉的话，恢复后 send_buffer 只写像素位，栅极就再也没人设置了） */
static void blank_dma_pixels(void)
{
    for (uint8_t f = 0; f < VFD_FRAMES; f++)
    {
        for (uint8_t g = 0; g < GRAY_BITS; g++)
        {
            memset(dma_tx_buffer[f][g], 0, DMA_BUFFER_SIZE);
            set_bit(dma_tx_buffer[f][g], GRIDS_OFFSET - f);
            set_bit(dma_tx_buffer[f][g], GRIDS_OFFSET - (f + 1));
        }
    }
}

/**
 * 停止整屏扫描。
 *
 * VFD 是动态扫描，停在哪一格哪一格就一直亮（占空比从 1/37 变成 100%，
 * 看起来是一条特别亮的线），所以停之前必须让屏上锁存的最后一段是黑的。
 *
 * 做法：把整块 DMA 缓冲换成黑画面，靠正在跑的扫描把它扫上去并锁存，
 * 再停链条。不能在 SPI 完成中断里送全 0 —— 写 Flash 时那个中断本身
 * 会被 Flash 总线停住，推迟到擦除结束才执行，等于没遮。
 */
void mn11236_scan_stop(void)
{
    if (!scan_run)
        return;

    /* 1. 整块 DMA 缓冲换成黑画面（保留栅极位），之后每一段都是暗的 */
    blank_dma_pixels();

    /* 2. 等黑画面真正扫上去并锁存。
     *    这里必须给足 1ms 以上：osDelay(1) 是"等到下一个 tick"，可能
     *    几乎立刻返回，那时正在传的还是旧数据，一停就是一条亮线。 */
    osDelay(2);

    /* 3. 停链条：此刻最后锁存的一定是黑的那一段 */
    scan_run = 0;
}

/**
 * 恢复扫描。停扫时链条是自然结束的（SPI 已回到 READY），直接从上次的
 * 格/子场接着扫；万一调用时刚好还有一段在传，它传完的回调会看到
 * scan_run=1 自己接上。
 */
void mn11236_scan_start(void)
{
    if (scan_run)
        return;

    scan_run = 1;

    if (hspi2.State == HAL_SPI_STATE_READY)
        HAL_SPI_Transmit_DMA(
            &hspi2,
            (uint8_t *)dma_tx_buffer[current_grid][current_plane],
            DMA_BUFFER_SIZE);
}

/**
 * 暂停/恢复送屏
 * hold = 1：mn11236_send_buffer() 直接被忽略，屏幕保持上一帧内容
 * 过渡动画里用来避免“新界面先闪一帧完整的图”
 */
void mn11236_hold_send(uint8_t hold)
{
    send_hold = hold;
}

/**
 * 每送完一帧回调一次（默认空实现，应用层可覆写）
 * 用来把刚送上屏的画面同步给上位机：碎裂/拼合、菜单模糊等
 * 过渡动画的中间帧也会跟着上传，不再只有主循环的最后一张。
 */
__weak void mn11236_on_frame_sent(void)
{
}

/**
 * 发送缓冲区数据到屏幕
 */
void mn11236_send_buffer(void)
{
	uint8_t blue = 0, red = 0;

	if (send_hold)
		return;
    /**
     * 一帧点3个像素段： T0 = abc, T1 = def, T2 = ghi ...
     * 每个像素段包含红蓝两种颜色数据
     *
     * 帧与像素对应关系：
     * - 奇数帧(0,2,4...): 处理 abc 段 (offset=0)
     * - 偶数帧(1,3,5...): 处理 def 段 (offset=3)
     * - 每帧处理3个水平像素
     */

    // 遍历每一行
    for (uint8_t y = 0; y < VFD_HEIGHT; y++)
    {
        uint8_t line_offset = VFD_LINE_OFFSET * y; // 当前行的位偏移

        // 遍历每一帧
        for (uint8_t frame = 0; frame < VFD_FRAMES; frame++)
        {
            uint8_t even = (frame + 1) % 2 == 0; // 判断是否是偶数帧
            uint8_t seg_offset = even * 3;       // abc=0, def=3

            // 计算当前帧对应的起始X坐标
            // 帧0: x=0,1,2  帧1: x=3,4,5  帧2: x=6,7,8 ...
            uint8_t base_x = frame * 3;

            // 处理3个像素（或最后一帧的特殊情况）
            uint8_t pixel_count = 3;
            if (frame == VFD_FRAMES - 1)
            {
                // 最后一帧需要额外处理一个像素
                pixel_count = 4;
            }

            for (uint8_t p = 0; p < pixel_count; p++)
            {
                uint8_t x = base_x + p;
                if (x >= VFD_WIDTH)
                    break;

                uint8_t bit_idx;
                if (p < 3)
                {
                    bit_idx = abcdef_conversion(seg_offset + p);
                }
                else
                {
                    // 最后一帧的第4个像素使用 def 段的第一个位置
                    bit_idx = abcdef_conversion(3);
                }

                // 蓝色像素：根据display_buffer状态设置或清除bit
								blue = display_buffer[x][y]&0xF;
								if(blue&1)
                    set_bit(dma_tx_buffer[frame][0], line_offset + bit_idx);
                else
                    reset_bit(dma_tx_buffer[frame][0], line_offset + bit_idx);
								
								if(blue&2)
                    set_bit(dma_tx_buffer[frame][1], line_offset + bit_idx);
                else
                    reset_bit(dma_tx_buffer[frame][1], line_offset + bit_idx);
								
								if(blue&4)
                    set_bit(dma_tx_buffer[frame][2], line_offset + bit_idx);
                else
                    reset_bit(dma_tx_buffer[frame][2], line_offset + bit_idx);

                // 红色像素：根据display_buffer状态设置或清除bit
								red = display_buffer[x][y]>>4;
								if(red&1)
                    set_bit(dma_tx_buffer[frame][0], line_offset + VFD_COLOR_OFFSET + bit_idx);
                else
                    reset_bit(dma_tx_buffer[frame][0], line_offset + VFD_COLOR_OFFSET + bit_idx);
								
								if(red&2)
                    set_bit(dma_tx_buffer[frame][1], line_offset + VFD_COLOR_OFFSET + bit_idx);
                else
                    reset_bit(dma_tx_buffer[frame][1], line_offset + VFD_COLOR_OFFSET + bit_idx);
								
								if(red&4)
                    set_bit(dma_tx_buffer[frame][2], line_offset + VFD_COLOR_OFFSET + bit_idx);
                else
                    reset_bit(dma_tx_buffer[frame][2], line_offset + VFD_COLOR_OFFSET + bit_idx);
            }
        }
    }

    /* 整帧数据已经写好，通知应用层同步（默认空实现） */
    mn11236_on_frame_sent();
}

/**
 * 画一个点,起点从0,0开始
 */
void mn11236_draw_pixel(uint8_t x, uint8_t y, mn11236_color_t color)
{
    if (x >= VFD_WIDTH || y >= VFD_HEIGHT)
    {
        return; // 超出范围
    }
		if(color == MN11236_COLOR_RED)
		{
			display_buffer[x][y] |= 7<<4;
		}
		else
		{
			display_buffer[x][y] |= 7;
		}
}

void mn11236_draw_pixel_gray(uint8_t x,
                             uint8_t y,
                             mn11236_color_t color,
                             uint8_t gray)
{
    if (x >= VFD_WIDTH || y >= VFD_HEIGHT)
        return;

    if (gray > 7)
        gray = 7;

		if(color == MN11236_COLOR_RED)
		{
			display_buffer[x][y] &= 0xF;
			display_buffer[x][y] |= gray<<4;
		}
		else
		{
			display_buffer[x][y] &= 0xF0;
			display_buffer[x][y] |= gray;
		}
}

/**
 * 清一个点,起点从0,0开始
 */
void mn11236_clear_pixel(uint8_t x, uint8_t y, mn11236_color_t color)
{
    if (x >= VFD_WIDTH || y >= VFD_HEIGHT)
    {
        return; // 超出范围
    }
		if(color == MN11236_COLOR_RED)
			display_buffer[x][y] &= 0xF;
		else
			display_buffer[x][y] &= 0xF0;
}

/**
 * 读取一个点的颜色值,起点从0,0开始
 */
uint8_t mn11236_read_pixel(uint8_t x, uint8_t y, mn11236_color_t color)
{
    if (x >= VFD_WIDTH || y >= VFD_HEIGHT)
    {
        return 0; // 超出范围
    }
		if(color == MN11236_COLOR_RED)
			return display_buffer[x][y] >> 4;
		else
			return display_buffer[x][y] & 0xF;
}

/**
 * 获取显示缓冲区首地址
 * 布局 [x][y]，索引 x * VFD_HEIGHT + y
 * 每字节低 4 位为蓝色灰度，高 4 位为红色灰度
 */
uint8_t *mn11236_get_buffer(void)
{
    return &display_buffer[0][0];
}

void mn11236_gray_test(void)
{
    uint8_t x, y;

    mn11236_clear_buffers();

    uint8_t block_width = VFD_WIDTH / 16;  // 16个灰度块

    for (x = 0; x < VFD_WIDTH; x++)
    {
        uint8_t block = x / block_width;

        for (y = 0; y < VFD_HEIGHT; y++)
        {
            if (block < 8)
            {
                /* 蓝色 0~7 */
                mn11236_draw_pixel_gray(
                    x,
                    y,
                    MN11236_COLOR_BLUE,
                    block   // 灰度 0~7
                );
            }
            else if (block < 16)
            {
                /* 红色 0~7 */
                mn11236_draw_pixel_gray(
                    x,
                    y,
                    MN11236_COLOR_RED,
                    block - 8
                );
            }
        }
    }

    mn11236_send_buffer();
}
/**
 * 开发测试函数 - 动画效果测试
 */
void mn11236_development_test(void)
{
    uint8_t x, y;
    // 动画1: 垂直扫描线从左到右（蓝色）
    for (x = 0; x < VFD_WIDTH; x++)
    {
        mn11236_clear_buffers();
        for (y = 0; y < VFD_HEIGHT; y++)
        {
            mn11236_draw_pixel(x, y, MN11236_COLOR_BLUE);
        }
        mn11236_send_buffer();
        osDelay(20);
    }

    // 动画2: 水平扫描线从上到下（红色）
    for (y = 0; y < VFD_HEIGHT; y++)
    {
        mn11236_clear_buffers();
        for (x = 0; x < VFD_WIDTH; x++)
        {
            mn11236_draw_pixel(x, y, MN11236_COLOR_RED);
        }
        mn11236_send_buffer();
        osDelay(50);
    }

    // 动画3: 逐行填充（蓝色从上往下填满）
    mn11236_clear_buffers();
    for (y = 0; y < VFD_HEIGHT; y++)
    {
        for (x = 0; x < VFD_WIDTH; x++)
        {
            mn11236_draw_pixel(x, y, MN11236_COLOR_BLUE);
        }
        mn11236_send_buffer();
        osDelay(80);
    }

    osDelay(500);
		
    // 动画5: 扩展矩形（从中心向外扩展）
    mn11236_clear_buffers();
    uint8_t cx = VFD_WIDTH / 2;
    uint8_t cy = VFD_HEIGHT / 2;
    uint8_t max_r = (VFD_WIDTH > VFD_HEIGHT) ? VFD_WIDTH / 2 : VFD_HEIGHT / 2;

    for (uint8_t r = 0; r <= max_r; r++)
    {
        // 画矩形边框
        for (x = 0; x < VFD_WIDTH; x++)
        {
            for (y = 0; y < VFD_HEIGHT; y++)
            {
                int16_t dx = (x > cx) ? (x - cx) : (cx - x);
                int16_t dy = (y > cy) ? (y - cy) : (cy - y);
                if (dx == r || dy == r)
                {
                    if (dx <= r && dy <= r)
                    {
                        mn11236_draw_pixel(x, y, (r % 2) ? MN11236_COLOR_RED : MN11236_COLOR_BLUE);
                    }
                }
            }
        }
        mn11236_send_buffer();
        osDelay(50);
    }

    osDelay(1000);

    // 动画6: 蛇形填充
    mn11236_clear_buffers();
    for (y = 0; y < VFD_HEIGHT; y++)
    {
        if (y % 2 == 0)
        {
            // 从左到右
            for (x = 0; x < VFD_WIDTH; x++)
            {
                mn11236_draw_pixel(x, y, MN11236_COLOR_BLUE);
                if (x % 4 == 0)
                {
                    mn11236_send_buffer();
                    osDelay(5);
                }
            }
        }
        else
        {
            // 从右到左
            for (int16_t rx = VFD_WIDTH - 1; rx >= 0; rx--)
            {
                mn11236_draw_pixel((uint8_t)rx, y, MN11236_COLOR_RED);
                if (rx % 4 == 0)
                {
                    mn11236_send_buffer();
                    osDelay(5);
                }
            }
        }
    }
    mn11236_send_buffer();

    osDelay(2000);

    // 清屏结束
    mn11236_clear_buffers();
    mn11236_send_buffer();
}
