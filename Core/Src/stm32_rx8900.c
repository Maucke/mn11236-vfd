#include "stm32f4xx_hal.h"
#include "stm32_rx8900.h"
#include "stdio.h"

#define RX8900_ADDR (0x32 << 1)

extern I2C_HandleTypeDef hi2c1;

static uint8_t B2D(uint8_t bcd);
static uint8_t D2B(uint8_t decimal);

void RX8900_Init(void)
{
    // 扩展寄存器
    uint8_t val = 8;
    HAL_I2C_Mem_Write(&hi2c1, RX8900_ADDR, RX8900_EXT_REG,
                      I2C_MEMADD_SIZE_8BIT, &val, 1, HAL_MAX_DELAY);

    // 状态寄存器清零
    val = 0;
    HAL_I2C_Mem_Write(&hi2c1, RX8900_ADDR, RX8900_REG_STATUS,
                      I2C_MEMADD_SIZE_8BIT, &val, 1, HAL_MAX_DELAY);

    // 控制寄存器
    val = 64;
    HAL_I2C_Mem_Write(&hi2c1, RX8900_ADDR, RX8900_REG_CONTROL,
                      I2C_MEMADD_SIZE_8BIT, &val, 1, HAL_MAX_DELAY);
}

bool RX8900_SetTime(struct tm *tm_local)
{
		uint8_t buffer[8] = {D2B(tm_local->tm_sec), D2B(tm_local->tm_min), D2B(tm_local->tm_hour), (tm_local->tm_wday), D2B(tm_local->tm_mday), D2B(tm_local->tm_mon + 1), D2B(tm_local->tm_year % 100)};

    if (HAL_I2C_Mem_Write(&hi2c1,
                          RX8900_ADDR,
                          RX8900_REG_TIME,
                          I2C_MEMADD_SIZE_8BIT,
                          buffer,
                          7,
                          HAL_MAX_DELAY) != HAL_OK)
    {
        return false;
    }

    return true;
}

bool RX8900_GetTime(struct tm *tm_local)
{
    uint8_t buffer[7] = {0};

    if (HAL_I2C_Mem_Read(&hi2c1,
                         RX8900_ADDR,
                         RX8900_REG_TIME,
                         I2C_MEMADD_SIZE_8BIT,
                         buffer,
                         7,
                         HAL_MAX_DELAY) != HAL_OK)
    {
        return false;
    }

    tm_local->tm_sec  = B2D(buffer[0] & 0x7F);
    tm_local->tm_min  = B2D(buffer[1] & 0x7F);
    tm_local->tm_hour = B2D(buffer[2] & 0x3F);
    tm_local->tm_wday = (buffer[3] & 0x7F);
    tm_local->tm_mday = B2D(buffer[4] & 0x3F);    
		tm_local->tm_mon = B2D(buffer[5] & 0x1F) - 1; 
    tm_local->tm_year = B2D(buffer[6]) % 100;

    return true;
}

void RX8900_Test(int interval)
{
    struct tm tm_local;
    static uint32_t last_update_time = 0;
    uint32_t now_tick = HAL_GetTick();

    if (now_tick - last_update_time > interval)
    {
        if (RX8900_GetTime(&tm_local))
        {
            // printf("Date: %04d-%02d-%02d\n", tm_local.tm_year + 1900, tm_local.tm_mon + 1, tm_local.tm_mday);
            // printf("Time: %02d:%02d:%02d\n", tm_local.tm_hour, tm_local.tm_min, tm_local.tm_sec);
        }
        last_update_time = now_tick;
    }
}

bool RX8900_ReadTemperature(float *rtctemp)
{
    uint8_t temp = 0;

    if (HAL_I2C_Mem_Read(&hi2c1,
                         RX8900_ADDR,
                         RX8900_REG_TEMP,
                         I2C_MEMADD_SIZE_8BIT,
                         &temp,
                         1,
                         HAL_MAX_DELAY) != HAL_OK)
    {
        return false;
    }

    // 官方公式
    *rtctemp = (temp * 2 - 187.19f) / 3.218f;

    return true;
}

bool RX8900_ClearAlarm1(void)
{
    uint8_t ctrlReg = 0;
    uint8_t statusReg = 0;

    // 读控制寄存器
    if (HAL_I2C_Mem_Read(&hi2c1,
                         RX8900_ADDR,
                         RX8900_REG_CONTROL,
                         I2C_MEMADD_SIZE_8BIT,
                         &ctrlReg,
                         1,
                         HAL_MAX_DELAY) != HAL_OK)
    {
        return false;
    }

    ctrlReg &= ~RX8900_CON_A1IE;

    HAL_I2C_Mem_Write(&hi2c1,
                      RX8900_ADDR,
                      RX8900_REG_CONTROL,
                      I2C_MEMADD_SIZE_8BIT,
                      &ctrlReg,
                      1,
                      HAL_MAX_DELAY);

    // 读状态寄存器
    if (HAL_I2C_Mem_Read(&hi2c1,
                         RX8900_ADDR,
                         RX8900_REG_STATUS,
                         I2C_MEMADD_SIZE_8BIT,
                         &statusReg,
                         1,
                         HAL_MAX_DELAY) != HAL_OK)
    {
        return false;
    }

    statusReg &= ~RX8900_STA_A1F;

    HAL_I2C_Mem_Write(&hi2c1,
                      RX8900_ADDR,
                      RX8900_REG_STATUS,
                      I2C_MEMADD_SIZE_8BIT,
                      &statusReg,
                      1,
                      HAL_MAX_DELAY);

    return true;
}

static uint8_t B2D(uint8_t bcd)
{
    return (bcd >> 4) * 10 + (bcd & 0x0F);
}

static uint8_t D2B(uint8_t decimal)
{
    return (((decimal / 10) << 4) | (decimal % 10));
}
