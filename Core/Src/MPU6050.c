//
// Created by MuTouJianChi on 2026/11/1.
//

#include "../Inc/MPU6050.h"
#include "MPU6050_Reg.h"
#include "main.h"

uint8_t MPU6050_Init(I2C_HandleTypeDef *hi2c) {
    uint8_t flag,data;

    // 确认通信正常
    HAL_I2C_Mem_Read(hi2c, MPU6050_ADDR, MPU6050_WHO_AM_I, 1,&flag , 1, 100 );
    if (flag != 0x68) return 1;

    // 唤醒设备
    data = 0x00;
    HAL_I2C_Mem_Write(hi2c, MPU6050_ADDR, MPU6050_PWR_MGMT_1, 1, &data, 1, 100);

    // 设置加速度计量程 ±2g
    data = 0x00;
    HAL_I2C_Mem_Write(hi2c, MPU6050_ADDR, MPU6050_ACCEL_CONFIG, 1, &data, 1, 100);

    // 设置陀螺仪量程 ±2000°/s
    data = 0x18;
    HAL_I2C_Mem_Write(hi2c, MPU6050_ADDR, MPU6050_GYRO_CONFIG, 1, &data, 1, 100);

    // 设置滤波
    data = 0x03;
    HAL_I2C_Mem_Write(hi2c, MPU6050_ADDR, MPU6050_CONFIG, 1, &data, 1, 100);

    //设置采样率分频
    data = 0x09;
    HAL_I2C_Mem_Write(hi2c, MPU6050_ADDR, MPU6050_SMPLRT_DIV, 1, &data, 1, 100);

    return 0;

}

/**
 *
 * @param hi2c
 * @param accel 存储加速器数据的数组
 * @param gyro 存储陀螺仪数据的数组
 */
void MPU6050_GetDatas(I2C_HandleTypeDef *hi2c, int16_t *accel, int16_t *gyro) {
    // 缓存14个高低位数据
    uint8_t buff[14];

    // 一次性读完所有数据
    HAL_I2C_Mem_Read(hi2c, MPU6050_ADDR, MPU6050_ACCEL_XOUT_H, 1, buff, 14, 100);

    //高低位拼接并存入相应数组中
    accel[0] = (int16_t)((buff[0] << 8) | buff[1]);   // AccX
    accel[1] = (int16_t)((buff[2] << 8) | buff[3]);   // AccY
    accel[2] = (int16_t)((buff[4] << 8) | buff[5]);   // AccZ
    // buff[6], buff[7] 是温度，直接借过
    gyro[0] = (int16_t)((buff[8]  << 8) | buff[9]);   // GyroX
    gyro[1] = (int16_t)((buff[10] << 8) | buff[11]);  // GyroY
    gyro[2] = (int16_t)((buff[12] << 8) | buff[13]);  // GyroZ

}
