#include "bootloader.h"

#define FLASH_USER_START_ADDR   FLASH_SECTOR_4   // Flash起始地址
#define FLASH_USER_END_ADDR     FLASH_SECTOR_5   // Flash结束地址

#define FLASH_ADDRESS 0x08010000 // Flash地址，这里是示例地址
// 擦除指定地址的Flash扇区
void Flash_Erase()
{
  // 初始化Flash擦除结构体
  FLASH_EraseInitTypeDef eraseInitStruct;
  uint32_t sectorError = 0;
  HAL_FLASH_Unlock();

  // 配置要擦除的扇区和电压范围
  eraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
  eraseInitStruct.Sector = FLASH_SECTOR_4;
  eraseInitStruct.NbSectors = 1; // 要擦除的扇区数量
  eraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3; // 电压范围

  // 执行擦除操作
  if(HAL_FLASHEx_Erase(&eraseInitStruct, &sectorError) != HAL_OK)
  {
  }

  eraseInitStruct.Sector = FLASH_SECTOR_5;
  // 执行擦除操作
  if(HAL_FLASHEx_Erase(&eraseInitStruct, &sectorError) != HAL_OK)
  {
  }

  HAL_FLASH_Lock();
}

// 在指定地址写入数据
void Flash_Write(uint32_t address, uint8_t* data, uint32_t len)
{
  HAL_FLASH_Unlock();

	for(int i=0;i<len;i++)
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, address + FLASH_ADDRESS + i, data[i]);

  HAL_FLASH_Lock();
}

// 从指定地址读取数据
uint32_t Flash_Read(uint32_t address)
{
  return (*(__IO uint32_t*)address);
}

typedef void (*pFunction)(void);
pFunction                     JumpAddress;

void JumpToApplication()
{
  JumpAddress = *(__IO pFunction*)(FLASH_ADDRESS + 4);
  __set_MSP(*(__IO uint32_t*) FLASH_ADDRESS);
  HAL_DeInit();
  JumpAddress();
}
