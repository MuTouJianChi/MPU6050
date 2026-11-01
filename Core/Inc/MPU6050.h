//
// Created by MuTouJianChi on 2026/11/1.
//

#include "main.h"

#ifndef HARDWAREI2C_MPU6050_MPU6050_H
#define HARDWAREI2C_MPU6050_MPU6050_H

uint8_t MPU6050_Init(I2C_HandleTypeDef *hi2c) ;

void MPU6050_GetDatas(I2C_HandleTypeDef *hi2c, int16_t *accel, int16_t *gyro) ;

#endif //HARDWAREI2C_MPU6050_MPU6050_H
