/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <math.h>

#include "OLED.h"
#include "MPU6050.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
int16_t raw_accel[3], raw_gyro[3];
float accel[3], gyro[3];

volatile float pitch = 0.0f;
volatile float roll  = 0.0f;
volatile float yaw   = 0.0f;

float alpha = 0.98f;
uint32_t last_tick = 0;

float gyro_offset[3] = {0.0f, 0.0f, 0.0f};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C2_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

  OLED_Init();
  if (MPU6050_Init(&hi2c2) != 0) {
    OLED_ShowString(1,1,"MPU6050InitError");
    while(1);
  }

  // 计算零偏
  OLED_ShowString(1,1,"Waiting......");
  float sum[3] = {0.0f, 0.0f, 0.0f};
  const int N = 500;                  // 采样次数
  for (int i = 0; i < N; i++) {
    MPU6050_GetDatas(&hi2c2, raw_accel, raw_gyro);
    for (int j = 0; j < 3; j++) {
      sum[j] += raw_gyro[j] / 16.4f;   // 转成 °/s 再累加
    }
    // 由MPU6050更新频率决定
    HAL_Delay(10);
  }
  for (int j = 0; j < 3; j++) {
    gyro_offset[j] = sum[j] / N;    // 平均值就是零偏
  }

  OLED_ShowString(1, 1, "Pitch:");
  OLED_ShowString(2, 1, "Roll :");
  OLED_ShowString(3, 1, "Yaw  :");

  last_tick = HAL_GetTick();
  HAL_TIM_Base_Start_IT(&htim2);   // 启动 TIM2 中断

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    OLED_ShowFloat(1, 7, pitch, 3, 2);
    OLED_ShowFloat(2, 7, roll,  3, 2);
    OLED_ShowFloat(3, 7, yaw,   3, 2);

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM2)
  {
    // 计算真实 dt 防止各种原因干扰
    uint32_t now = HAL_GetTick();
    // 将时间差转换成秒
    float dt = (now - last_tick) * 0.001f;
    // 更新last_tick时间值
    last_tick = now;

    // 读取原始数据
    MPU6050_GetDatas(&hi2c2, raw_accel, raw_gyro);

    // 处理原始数据，将其转换成实际物理量
    for (int i = 0; i < 3; i++) {
      // 16384.0 是因为加速度计的测量范围是±2g，而int16_t范围是-32768~32767，除2得到
      accel[i]  = raw_accel[i] / 16384.0f;
      // 同理，当前配置量程是±2000°/s,每 1°/s 对应的刻度数 = 65536 / 4000 = 16.384 ≈ 16.4 LSB/(°/s)
      gyro[i] = raw_gyro[i] / 16.4f;
    }

    // 减去零偏
    gyro[0] -= gyro_offset[0];
    gyro[1] -= gyro_offset[1];
    gyro[2] -= gyro_offset[2];

    //  加速度计算角度,其中180° = π 弧度 ≈ 3.14159 弧度 ==》1 弧度 = 180 / π ≈ 57.29578°
    float acc_pitch = atan2f(accel[1] , sqrtf(accel[1]*accel[1] + accel[2]*accel[2])) * 57.29578f;
    float acc_roll  = atan2f(accel[0] , sqrtf(accel[0]*accel[0] + accel[2]*accel[2])) * 57.29578f;

    // 一阶互补滤波
    pitch = alpha * (pitch + gyro[0] * dt) + (1.0f - alpha) * acc_pitch;
    roll  = alpha * (roll  + gyro[1] * dt) + (1.0f - alpha) * acc_roll;

    yaw += gyro[2] * dt;
  }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
