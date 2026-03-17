#include "stm32f4xx_hal.h"
#include "stm32_mpu6050.h"
#include <stdbool.h>
#include "i2c.h"

#define MPU6050_ADDR (0x68 << 1)

// 初始化
bool MPU6050_Init(void)
{
    uint8_t data;

    // 唤醒（退出睡眠）
    data = 0x00;
    if (HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, MPU_PWR_MGMT1,
                          I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY) != HAL_OK)
        return false;

    // 采样率 = 1kHz / (1 + SMPLRT_DIV)
    data = 0x07; // 125Hz
    HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, MPU_SMPLRT_DIV,
                      I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);

    // DLPF
    data = 0x06;
    HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, MPU_CONFIG,
                      I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);

    // 陀螺仪 ±2000°/s
    data = 0x18;
    HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, MPU_GYRO_CONFIG,
                      I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);

    // 加速度 ±2g
    data = 0x00;
    HAL_I2C_Mem_Write(&hi2c1, MPU6050_ADDR, MPU_ACCEL_CONFIG,
                      I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);

    return true;
}

bool MPU6050_ReadRaw(int16_t *ax, int16_t *ay, int16_t *az,
                     int16_t *gx, int16_t *gy, int16_t *gz)
{
    uint8_t buf[14];

    if (HAL_I2C_Mem_Read(&hi2c1,
                         MPU6050_ADDR,
                         MPU_ACCEL_XOUT_H,
                         I2C_MEMADD_SIZE_8BIT,
                         buf,
                         14,
                         HAL_MAX_DELAY) != HAL_OK)
    {
        return false;
    }

    *ax = (int16_t)(buf[0] << 8 | buf[1]);
    *ay = (int16_t)(buf[2] << 8 | buf[3]);
    *az = (int16_t)(buf[4] << 8 | buf[5]);

    // buf[6][7] 是温度，可以不读

    *gx = (int16_t)(buf[8] << 8 | buf[9]);
    *gy = (int16_t)(buf[10] << 8 | buf[11]);
    *gz = (int16_t)(buf[12] << 8 | buf[13]);

    return true;
}

bool MPU6050_Read(float *ax, float *ay, float *az,
                  float *gx, float *gy, float *gz)
{
    int16_t raw_ax, raw_ay, raw_az;
    int16_t raw_gx, raw_gy, raw_gz;

    if (!MPU6050_ReadRaw(&raw_ax, &raw_ay, &raw_az,
                         &raw_gx, &raw_gy, &raw_gz))
        return false;

    // 加速度（±2g → 16384 LSB/g）
    *ax = raw_ax / 16384.0f;
    *ay = raw_ay / 16384.0f;
    *az = raw_az / 16384.0f;

    // 陀螺仪（±2000°/s → 16.4 LSB/(°/s)）
    *gx = raw_gx / 16.4f;
    *gy = raw_gy / 16.4f;
    *gz = raw_gz / 16.4f;

    return true;
}

bool MPU6050_ReadTemp(float *temp)
{
    uint8_t buf[2];
    int16_t raw;

    if (HAL_I2C_Mem_Read(&hi2c1,
                         MPU6050_ADDR,
                         0x41,
                         I2C_MEMADD_SIZE_8BIT,
                         buf,
                         2,
                         HAL_MAX_DELAY) != HAL_OK)
    {
        return false;
    }

    raw = (int16_t)(buf[0] << 8 | buf[1]);

    *temp = raw / 340.0f + 36.53f;

    return true;
}

#if false
float ax, ay, az;
float gx, gy, gz;

MPU6050_Init();

while (1)
{
    if (MPU6050_Read(&ax, &ay, &az, &gx, &gy, &gz))
    {
        // printf("A: %.2f %.2f %.2f\n", ax, ay, az);
        // printf("G: %.2f %.2f %.2f\n", gx, gy, gz);
    }

    HAL_Delay(10);
}
#endif
