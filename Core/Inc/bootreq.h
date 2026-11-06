#ifndef __BOOTREQ_H__
#define __BOOTREQ_H__

#include "stm32f4xx_hal.h"

/* ------------------------------------------------------------------
 * “下次复位进 Bootloader”的请求标志
 *
 * 存放位置：配置扇区（Sector 3, 0x0800C000 ~ 0x0800FFFF）的最后一个字。
 *   APP  : 配置(20 字节)写在扇区开头，末尾这个字平时没人用
 *   BOOT : 上电/复位后先看这个字，等于 magic 就留在 Boot，并且立刻清零
 *
 * 为什么不用备份域寄存器：本板没接 VBAT，只有 flash 才百分百靠得住。
 * 为什么清标志是写 0 而不是擦除：flash 只能把 1 写成 0，写 0 不用擦除；
 * 下次 APP 再请求时 STMFLASH_Write 会把整个扇区擦一遍，末字自然回到 0xFFFFFFFF。
 * ------------------------------------------------------------------ */

#define BOOTREQ_MAGIC   0xB00710ADu
#define BOOTREQ_ADDR    0x0800FFFCu

/* APP 侧：写标志（会顺带把当前配置重新落盘一次） */
void BootReq_Request(void);

/* 只读检查：BOOT 上电时用 */
static inline uint8_t BootReq_IsSet(void)
{
  return (*(__IO uint32_t *)BOOTREQ_ADDR == BOOTREQ_MAGIC);
}

/* 清零：BOOT 读完立刻清，避免下次复位又回 Boot
 * （只把 1 写成 0，不需要擦除；已经是 0xFFFFFFFF 时不必再动） */
static inline void BootReq_Clear(void)
{
  if (*(__IO uint32_t *)BOOTREQ_ADDR == 0xFFFFFFFFu)
    return;

  HAL_FLASH_Unlock();
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, BOOTREQ_ADDR, 0x00000000u);
  HAL_FLASH_Lock();
}

#endif
