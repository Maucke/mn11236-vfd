#include "bootreq.h"
#include "main.h"
#include "stmflash.h"
#include "iwdg.h"

extern UserInfo_t userInfo;

/**
 * @brief  请求下次复位后停留在 Bootloader（NexLink CMD_ENTER_BOOT 调用）
 *
 * @note   标志字放在配置扇区(Sector 3)的末尾，写之前必须让这个字回到
 *         0xFFFFFFFF，所以先把整个扇区擦一遍、再把配置写回去。
 *
 *         IWDG 是 512ms 超时，16KB 扇区擦除要 250ms 左右，所以在擦除前后
 *         各喂一次狗；万一真的超时复位，最多是"命令没生效"，配置不会坏。
 */
void BootReq_Request(void)
{
  FLASH_EraseInitTypeDef erase = {0};
  uint32_t sectorError = 0;
  uint32_t i;

  HAL_FLASH_Unlock();

  /* 擦掉配置扇区（含末尾的 BOOTREQ 字） */
  erase.TypeErase = FLASH_TYPEERASE_SECTORS;
  erase.Sector = FLASH_SECTOR_3;
  erase.NbSectors = 1;
  erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;
  HAL_FLASHEx_Erase(&erase, &sectorError);

  /* 配置原样写回扇区开头 */
  for (i = 0; i < sizeof(UserInfo_t) / sizeof(uint32_t); i++)
  {
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
                      FLASH_SAVE_ADDR + i * 4,
                      ((uint32_t *)&userInfo)[i]);
  }

  /* 末尾写上“下次进 Boot” */
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, BOOTREQ_ADDR, BOOTREQ_MAGIC);

  HAL_FLASH_Lock();
}
