/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file         stm32f4xx_hal_msp.c
  * @brief        This file provides code for the MSP Initialization
  *               and de-Initialization codes.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN Define */

/* USER CODE END Define */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN Macro */

/* USER CODE END Macro */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
extern void BSP_MotorControl_StepClockHandler(uint8_t deviceId);
extern void BSP_MotorControl_FlagInterruptHandler(void);
/* USER CODE END PFP */

/* External functions --------------------------------------------------------*/
/* USER CODE BEGIN ExternalFunctions */

/* USER CODE END ExternalFunctions */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * Initializes the Global MSP.
  */
void HAL_MspInit(void)
{
  /* USER CODE BEGIN MspInit 0 */

  /* USER CODE END MspInit 0 */

  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();

  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_0);

  /* System interrupt init*/

  /* USER CODE BEGIN MspInit 1 */

  /* USER CODE END MspInit 1 */
}

/**
* @brief RTC MSP Initialization
* This function configures the hardware resources used in this example
* @param hrtc: RTC handle pointer
* @retval None
*/
void HAL_RTC_MspInit(RTC_HandleTypeDef* hrtc)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(hrtc->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */

  /* USER CODE END RTC_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* Peripheral clock enable */
    __HAL_RCC_RTC_ENABLE();
  /* USER CODE BEGIN RTC_MspInit 1 */

  /* USER CODE END RTC_MspInit 1 */
  }

}

/**
* @brief RTC MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hrtc: RTC handle pointer
* @retval None
*/
void HAL_RTC_MspDeInit(RTC_HandleTypeDef* hrtc)
{
  if(hrtc->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
  }

}

/**
* @brief UART MSP Initialization
* This function configures the hardware resources used in this example
* @param huart: UART handle pointer
* @retval None
*/
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(huart->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART2 interrupt Init */
    HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspInit 1 */

  /* USER CODE END USART2_MspInit 1 */
  }

}

/**
* @brief UART MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param huart: UART handle pointer
* @retval None
*/
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
  if(huart->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);

    /* USART2 interrupt DeInit */
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspDeInit 1 */

  /* USER CODE END USART2_MspDeInit 1 */
  }

}

/* USER CODE BEGIN 1 */
void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  if(hspi->Instance == SPIx)
  {
    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* Enable GPIO TX/RX clock */
    SPIx_SCK_GPIO_CLK_ENABLE();
    SPIx_MISO_GPIO_CLK_ENABLE();
    SPIx_MOSI_GPIO_CLK_ENABLE();
    /* Enable SPI clock */
    SPIx_CLK_ENABLE();

    /*##-2- Configure peripheral GPIO ##########################################*/
    /* SPI SCK GPIO pin configuration  */
    GPIO_InitStruct.Pin       = SPIx_SCK_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_MEDIUM;
    GPIO_InitStruct.Alternate = SPIx_SCK_AF;

    HAL_GPIO_Init(SPIx_SCK_GPIO_PORT, &GPIO_InitStruct);

    /* SPI MISO GPIO pin configuration  */
    GPIO_InitStruct.Pin = SPIx_MISO_PIN;
    GPIO_InitStruct.Alternate = SPIx_MISO_AF;

    HAL_GPIO_Init(SPIx_MISO_GPIO_PORT, &GPIO_InitStruct);

    /* SPI MOSI GPIO pin configuration  */
    GPIO_InitStruct.Pin = SPIx_MOSI_PIN;
    GPIO_InitStruct.Alternate = SPIx_MOSI_AF;

    HAL_GPIO_Init(SPIx_MOSI_GPIO_PORT, &GPIO_InitStruct);
  }
}

/**
  * @brief SPI MSP De-Initialization
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO configuration to its default state
  * @param[in] hspi SPI handle pointer
  * @retval None
  */
void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{
  if(hspi->Instance == SPIx)
  {
    /*##-1- Reset peripherals ##################################################*/
    SPIx_FORCE_RESET();
    SPIx_RELEASE_RESET();

    /*##-2- Disable peripherals and GPIO Clocks ################################*/
    /* Configure SPI SCK as alternate function  */
    HAL_GPIO_DeInit(SPIx_SCK_GPIO_PORT, SPIx_SCK_PIN);
    /* Configure SPI MISO as alternate function  */
    HAL_GPIO_DeInit(SPIx_MISO_GPIO_PORT, SPIx_MISO_PIN);
    /* Configure SPI MOSI as alternate function  */
    HAL_GPIO_DeInit(SPIx_MOSI_GPIO_PORT, SPIx_MOSI_PIN);
  }
}

/**
  * @brief PWM MSP Initialization
  * @param[in] htim_pwm PWM handle pointer
  * @retval None
  */
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* htim_pwm)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  if(htim_pwm->Instance == BSP_MOTOR_CONTROL_BOARD_TIMER_PWM1)
  {
    /* Peripheral clock enable */
    __BSP_MOTOR_CONTROL_BOARD_TIMER_PWM1_CLCK_ENABLE();

    /* GPIO configuration */
    GPIO_InitStruct.Pin = BSP_MOTOR_CONTROL_BOARD_PWM_1_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    GPIO_InitStruct.Alternate = BSP_MOTOR_CONTROL_BOARD_AFx_TIMx_PWM1;
    HAL_GPIO_Init(BSP_MOTOR_CONTROL_BOARD_PWM_1_PORT, &GPIO_InitStruct);

    /* Set Interrupt Group Priority of Timer Interrupt*/
    HAL_NVIC_SetPriority(BSP_MOTOR_CONTROL_BOARD_PWM1_IRQn, 4, 0);

    /* Enable the timer global Interrupt */
    HAL_NVIC_EnableIRQ(BSP_MOTOR_CONTROL_BOARD_PWM1_IRQn);
  }
  else if(htim_pwm->Instance == BSP_MOTOR_CONTROL_BOARD_TIMER_PWM2)
  {
    /* Peripheral clock enable */
    __BSP_MOTOR_CONTROL_BOARD_TIMER_PWM2_CLCK_ENABLE();

    /* GPIO configuration */
    GPIO_InitStruct.Pin = BSP_MOTOR_CONTROL_BOARD_PWM_2_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    GPIO_InitStruct.Alternate = BSP_MOTOR_CONTROL_BOARD_AFx_TIMx_PWM2;
    HAL_GPIO_Init(BSP_MOTOR_CONTROL_BOARD_PWM_2_PORT, &GPIO_InitStruct);

    /* Set Interrupt Group Priority of Timer Interrupt*/
    HAL_NVIC_SetPriority(BSP_MOTOR_CONTROL_BOARD_PWM2_IRQn, 4, 0);

    /* Enable the timer2 global Interrupt */
    HAL_NVIC_EnableIRQ(BSP_MOTOR_CONTROL_BOARD_PWM2_IRQn);

  }
  else if(htim_pwm->Instance == BSP_MOTOR_CONTROL_BOARD_TIMER_PWM3)
  {
    /* Peripheral clock enable */
    __BSP_MOTOR_CONTROL_BOARD_TIMER_PWM3_CLCK_ENABLE();

    /* GPIO configuration */
    GPIO_InitStruct.Pin = BSP_MOTOR_CONTROL_BOARD_PWM_3_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(BSP_MOTOR_CONTROL_BOARD_PWM_3_PORT, &GPIO_InitStruct);

    /* Set Interrupt Group Priority of Timer Interrupt*/
    HAL_NVIC_SetPriority(BSP_MOTOR_CONTROL_BOARD_PWM3_IRQn, 3, 0);

    /* Enable the timer global Interrupt */
    HAL_NVIC_EnableIRQ(BSP_MOTOR_CONTROL_BOARD_PWM3_IRQn);
  }
}

/**
  * @brief PWM MSP De-Initialization
  * @param[in] htim_pwm PWM handle pointer
  * @retval None
  */
void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef* htim_pwm)
{
  if(htim_pwm->Instance == BSP_MOTOR_CONTROL_BOARD_TIMER_PWM1)
  {
    /* Peripheral clock disable */
    __BSP_MOTOR_CONTROL_BOARD_TIMER_PWM1_CLCK_DISABLE();

    /* GPIO Deconfiguration */
    HAL_GPIO_DeInit(BSP_MOTOR_CONTROL_BOARD_PWM_1_PORT, BSP_MOTOR_CONTROL_BOARD_PWM_1_PIN);

  }
  else if(htim_pwm->Instance == BSP_MOTOR_CONTROL_BOARD_TIMER_PWM2)
  {
    /* Peripheral clock disable */
    __BSP_MOTOR_CONTROL_BOARD_TIMER_PWM2_CLCK_DISABLE();

    /* GPIO Deconfiguration */
    HAL_GPIO_DeInit(BSP_MOTOR_CONTROL_BOARD_PWM_2_PORT, BSP_MOTOR_CONTROL_BOARD_PWM_2_PIN);

  }
  else if(htim_pwm->Instance == BSP_MOTOR_CONTROL_BOARD_TIMER_PWM3)
  {
    /* Peripheral clock disable */
    __BSP_MOTOR_CONTROL_BOARD_TIMER_PWM3_CLCK_DISABLE();

    /* GPIO Deconfiguration */
    HAL_GPIO_DeInit(BSP_MOTOR_CONTROL_BOARD_PWM_3_PORT, BSP_MOTOR_CONTROL_BOARD_PWM_3_PIN);
  }
}

/**
  * @brief PWM Callback
  * @param[in] htim PWM handle pointer
  * @retval None
  */
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
  if ((htim->Instance == BSP_MOTOR_CONTROL_BOARD_TIMER_PWM1)&& (htim->Channel == BSP_MOTOR_CONTROL_BOARD_HAL_ACT_CHAN_TIMER_PWM1))
  {
    if (BSP_MotorControl_GetDeviceState(0) != INACTIVE)
    {
      BSP_MotorControl_StepClockHandler(0);
    }
  }
  if ((htim->Instance == BSP_MOTOR_CONTROL_BOARD_TIMER_PWM2)&& (htim->Channel == BSP_MOTOR_CONTROL_BOARD_HAL_ACT_CHAN_TIMER_PWM2))
  {
    if (BSP_MotorControl_GetDeviceState(1) != INACTIVE)
    {
      BSP_MotorControl_StepClockHandler(1);
    }
  }
  if ((htim->Instance == BSP_MOTOR_CONTROL_BOARD_TIMER_PWM3)&& (htim->Channel == BSP_MOTOR_CONTROL_BOARD_HAL_ACT_CHAN_TIMER_PWM3))
  {
    HAL_GPIO_TogglePin(BSP_MOTOR_CONTROL_BOARD_PWM_3_PORT, BSP_MOTOR_CONTROL_BOARD_PWM_3_PIN);
    if ((BSP_MotorControl_GetDeviceState(2) != INACTIVE)&&
        (HAL_GPIO_ReadPin(BSP_MOTOR_CONTROL_BOARD_PWM_3_PORT, BSP_MOTOR_CONTROL_BOARD_PWM_3_PIN) == GPIO_PIN_SET))
    {
      BSP_MotorControl_StepClockHandler(2);
    }
  }
}

/* USER CODE END 1 */
