#ifndef __ANIMATION_H
#define __ANIMATION_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

/* 动画编号：单选。菜单里是一个进度条，MOTION 界面长按上键循环切换。
 * 新增动画只要在 animation.c 的 anims[] 里加一行，并把这里 +1。 */
#define ANIM_COUNT 8

void Motion_Init(void);
void Motion_Run(uint8_t id); /* 每帧调用一次：更新 + 绘制（id 变了会自动重新初始化） */

#ifdef __cplusplus
}
#endif

#endif
