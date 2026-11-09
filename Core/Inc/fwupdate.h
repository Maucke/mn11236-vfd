#ifndef __FWUPDATE_H__
#define __FWUPDATE_H__

#include <stdint.h>

/* ------------------------------------------------------------------
 * NexLink 固件升级（目前只开放 Bootloader 区）
 *
 * 上位机把 .bin 原样发过来（可以带 appheader 头），这里负责：
 *   1. 认头：第一个数据块如果以 "appheader" 开头，就跳到这一行末尾（现在是 32 字节）
 *   2. 校验：payload 前 8 字节必须是合法向量表（SP 在 RAM、复位向量在 0x08000000~0x0800BFFF），
 *            选错文件（比如把 APP 的 bin 发给 BOOT 区）在这一步就被拒掉，此时还没擦 flash
 *   3. 擦除：只擦扇区 0/1/2（48 KB）。扇区 3 是配置区（含“下次进 Boot”标志），绝对不碰
 *   4. 写入：512 字节缓冲，凑满 4 字节就按字编程
 *   5. 校验：收完按 payload 算 CRC32，再回读 flash 算一遍对比
 *
 * 只在 NexLink 的 USB 接收任务里调用（单实例，不需要额外加锁）。
 * 返回值为 nl_err_t 的数值。
 * ------------------------------------------------------------------ */

#define FW_TARGET_BOOT   1u   /* 0 预留给 APP，暂不支持：擦 64K/128K 扇区会超过 IWDG 时间 */

uint8_t FwUpdate_Begin(uint8_t target, uint32_t total_len);
uint8_t FwUpdate_Data(uint32_t offset, const uint8_t *data, uint16_t len);
uint8_t FwUpdate_End(uint32_t crc32);

#endif
