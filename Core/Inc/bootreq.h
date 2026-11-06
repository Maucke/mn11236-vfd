#ifndef __BOOTREQ_H__
#define __BOOTREQ_H__

#include "stm32f4xx_hal.h"

/* ------------------------------------------------------------------
 * “下次复位进 Bootloader”的请求标志
 *
 * 存放位置：配置扇区（Sector 3, 0x0800C000 ~ 0x0800FFFF）的最后一个字。
 *   APP  : 收到 NexLink CMD_ENTER_BOOT 时写 magic，然后复位
 *   BOOT : 上电/复位后先看这个字，等于 magic 就留在 Boot，并且立刻清零
 *
 * 注意：常量必须和 APP 分支 Core/Inc/bootreq.h 保持一致。
 * ------------------------------------------------------------------ */

#define BOOTREQ_MAGIC   0xB00710ADu
#define BOOTREQ_ADDR    0x0800FFFCu

/* 只读检查 */
static inline uint8_t BootReq_IsSet(void)
{
  return (*(__IO uint32_t *)BOOTREQ_ADDR == BOOTREQ_MAGIC);
}

/* 清零：只把 1 写成 0，不需要擦除；已经是 0xFFFFFFFF 时不必再动 */
static inline void BootReq_Clear(void)
{
  if (*(__IO uint32_t *)BOOTREQ_ADDR == 0xFFFFFFFFu)
    return;

  HAL_FLASH_Unlock();
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, BOOTREQ_ADDR, 0x00000000u);
  HAL_FLASH_Lock();
}

#endif
