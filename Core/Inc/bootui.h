#ifndef __BOOTUI_H__
#define __BOOTUI_H__

#include <stdint.h>

/* Bootloader 界面状态。IDLE/READY 由 USB 状态自动判断，
 * WRITING/DONE/ERROR 由 usbd_storage_if.c 里的升级流程回报 */
typedef enum
{
    BOOTUI_IDLE = 0, /* 没连 USB，等着插线 */
    BOOTUI_READY,    /* USB 已枚举，等着拖 bin 进 U 盘 */
    BOOTUI_WRITING,  /* 正在收固件并写 flash */
    BOOTUI_DONE,     /* 写完，准备重启 */
    BOOTUI_ERROR     /* 文件头不对，没写 */
} bootui_state_t;

void bootui_init(void);

/* UI 任务里循环调用，内部按状态变化/动画节拍重绘，非阻塞 */
void bootui_poll(void);

bootui_state_t bootui_get_state(void);

/* 升级完成，任务据此决定什么时候重启 */
uint8_t bootui_is_finished(void);

/* 下面几个在 USB MSC 回调里调用，只更新状态，不碰屏幕 */
void bootui_msc_begin(void);
void bootui_msc_progress(uint32_t bytes);
void bootui_msc_end(void);
void bootui_msc_error(void);

#endif
