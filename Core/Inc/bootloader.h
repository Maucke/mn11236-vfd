#ifndef __BOOT_LOADER_H
#define __BOOT_LOADER_H

#include "stm32f4xx_hal.h"
#include "usart.h"

void JumpToApplication(void);
void Flash_Erase(void);
void Flash_Write(uint32_t address, uint8_t* data, uint32_t len);

#endif
