#ifndef __MPU6050_H
#define __MPU6050_H

#include "stm32f4xx_hal.h"
#include <stdbool.h>

// 寄存器地址
#define MPU_PWR_MGMT1     0x6B
#define MPU_SMPLRT_DIV    0x19
#define MPU_CONFIG        0x1A
#define MPU_GYRO_CONFIG   0x1B
#define MPU_ACCEL_CONFIG  0x1C
#define MPU_ACCEL_XOUT_H  0x3B

// ==================== 函数声明 ====================

// 初始化
bool MPU6050_Init(void);

// 读取原始数据
bool MPU6050_ReadRaw(int16_t *ax, int16_t *ay, int16_t *az,
                     int16_t *gx, int16_t *gy, int16_t *gz);

// 读取转换后的物理值
bool MPU6050_Read(float *ax, float *ay, float *az,
                  float *gx, float *gy, float *gz);

// 读取温度
bool MPU6050_ReadTemp(float *temp);

#endif /* __MPU6050_H */
