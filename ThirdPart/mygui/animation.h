#ifndef __ANIMATION_H
#define __ANIMATION_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

/* 动画编号：单选。菜单里是一个进度条，MOTION 界面长按上键循环切换。
 * 新增动画只要在 animation.c 的 anims[] 里加一行，并把这里 +1。
 * 0~7 是第一批（萤火虫/涟漪/数字雨/极光/天际线/星空穿越/气泡/烟花），
 * 8~15 是第二批（双螺旋/港口吊机/隧道/机场跑道灯/雷达/利萨茹/跨海大桥/星座）。 */
#define ANIM_COUNT 16

void Motion_Init(void);
void Motion_Run(uint8_t id); /* 每帧调用一次：更新 + 绘制（id 变了会自动重新初始化） */

#ifdef __cplusplus
}
#endif

#endif
