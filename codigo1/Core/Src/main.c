/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include <stdlib.h>
#include <string.h>
#include "angulo.h"
#include "RTC.h"
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
 RTC_HandleTypeDef hrtc;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

//Variables comunicación UART
#define TAM_BUF 200
uint8_t tx_buff[TAM_BUF]; //Buffer de transmisión de datos
uint8_t rx_buff[TAM_BUF]; //Buffer de recibir datos
char *eptr; //Puntero para separación cadena

//Variables para guardado en memoria RAM
#define BKPSRAM_BASE 0x20017000 //Dirección del primer byte donde se va a almacenar la información
#define OFFSET 8                //Tamaño de un double (8 bytes)

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_RTC_Init(void);
/* USER CODE BEGIN PFP */
volatile int first_movement = 0;
volatile int pos0 = 0;
volatile int pos1 = 0;
volatile int pos2 = 0;

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//Variables motor
static volatile uint16_t gLastError;

//Variables efemérides
struct constantes c;                         //constantes utilizadas en los cálculos de las efemérides
struct tiempo t = {20,07,2022,20,31,50};      //estructura que contiene la fecha y hora
struct localizacion local = {40.45,-3.73,0}; //valores de la ubicación del seguidor
struct ephemeris efemerides;                 //ángulos más importantes del cálculo de las efemérides
struct posSol pos;                           //coordenadas cartesianas de la posición del Sol
struct posSol pos_final;                     //coordenadas cartesianas de la posición del Sol teniendo en cuenta seguidor desviado
float wid;                                   //�?ngulo óptimo de inclinación para módulos monofaciales
float prev_wid;                              //�?ngulo wid cálculo anterior
volatile float widc = 0;                     //�?ngulo óptimo de inclinación para módulos bifaciales


//Variables RTC
volatile RTC_TimeTypeDef sTime; //Inicialización hora
volatile RTC_DateTypeDef sDate; //Inicialización fecha

//Valores inicialización motores
L6474_Init_t gL6474InitParams =
{
    160,                               /// Acceleration rate in step/s2. Range: (0..+inf).
    160,                               /// Deceleration rate in step/s2. Range: (0..+inf).
    600,                              /// Maximum speed in step/s. Range: (30..10000].
    600,                               ///Minimum speed in step/s. Range: [30..10000).
    500,                               ///Torque regulation current in mA. (TVAL register) Range: 31.25mA to 4000mA.
    750,                               ///Overcurrent threshold (OCD_TH register). Range: 375mA to 6000mA.
    L6474_CONFIG_OC_SD_ENABLE,         ///Overcurrent shutwdown (OC_SD field of CONFIG register).
    L6474_CONFIG_EN_TQREG_TVAL_USED,   /// Torque regulation method (EN_TQREG field of CONFIG register).
	L6474_STEP_SEL_1_16,//L6474_STEP_SEL_1_16,               /// Step selection (STEP_SEL field of STEP_MODE register).
    L6474_SYNC_SEL_1_2,                /// Sync selection (SYNC_SEL field of STEP_MODE register).
    L6474_FAST_STEP_12us,              /// Fall time value (T_FAST field of T_FAST register). Range: 2us to 32us.
    L6474_TOFF_FAST_8us,               /// Maximum fast decay time (T_OFF field of T_FAST register). Range: 2us to 32us.
    3,                                 /// Minimum ON time in us (TON_MIN register). Range: 0.5us to 64us.
    21,                                /// Minimum OFF time in us (TOFF_MIN register). Range: 0.5us to 64us.
    L6474_CONFIG_TOFF_044us,           /// Target Swicthing Period (field TOFF of CONFIG register).
    L6474_CONFIG_SR_320V_us,           /// Slew rate (POW_SR field of CONFIG register).
    L6474_CONFIG_INT_16MHZ,            /// Clock setting (OSC_CLK_SEL field of CONFIG register).
    (L6474_ALARM_EN_OVERCURRENT      |
     L6474_ALARM_EN_THERMAL_SHUTDOWN |
     L6474_ALARM_EN_THERMAL_WARNING  |
     L6474_ALARM_EN_UNDERVOLTAGE     |
     L6474_ALARM_EN_SW_TURN_ON       |
     L6474_ALARM_EN_WRONG_NPERF_CMD)    /// Alarm (ALARM_EN register).
};

//Variables estados funcionamiento
volatile uint8_t inicio = 0;    //Variable usada al arrancar el programa para calibrar el motor
volatile uint8_t modo = 0;      //modo = 0: llevar el seguidor a la posición horizontal, modo = 1: seguidor moviéndose según efemérides
volatile uint8_t izquierda = 0; //final de carrera izquierdo pulsado
volatile uint8_t derecha = 0;   //final de carrera derecho pulsado

volatile uint8_t usart_tx = 0;  //Botón de transmisión de datos pulsado
volatile uint8_t usart_rx = 0;  //Botón de recibir datos pulsado

volatile float posicion;        //variable con la posición del motor
volatile float ang_eq;          //Conversión del ángulo de grados a pasos

volatile int amanecer = 0;      //Variable para calibrado en el amanecer
volatile int hardstop = 0;      //Variable para parada seguidor si se pulsan dos veces el FC de la izquierda

//Funciones
//Funciones control Shield
void ErrorHandler_Shield(uint16_t error);
void MyFlagInterruptHandler(void)
{
  /* Get the value of the status register via the L6474 command GET_STATUS */
  uint16_t statusRegister = BSP_MotorControl_CmdGetStatus(0);

  /* Check HIZ flag: if set, power brigdes are disabled */
  if ((statusRegister & L6474_STATUS_HIZ) == L6474_STATUS_HIZ)
  {
    // HIZ state
    // Action to be customized
  }

  /* Check direction bit */
  if ((statusRegister & L6474_STATUS_DIR) == L6474_STATUS_DIR)
  {
    // Forward direction is set
    // Action to be customized
  }
  else
  {
    // Backward direction is set
    // Action to be customized
  }

  /* Check NOTPERF_CMD flag: if set, the command received by SPI can't be performed */
  /* This often occures when a command is sent to the L6474 */
  /* while it is in HIZ state */
  if ((statusRegister & L6474_STATUS_NOTPERF_CMD) == L6474_STATUS_NOTPERF_CMD)
  {
      // Command received by SPI can't be performed
     // Action to be customized
  }

  /* Check WRONG_CMD flag: if set, the command does not exist */
  if ((statusRegister & L6474_STATUS_WRONG_CMD) == L6474_STATUS_WRONG_CMD)
  {
     //command received by SPI does not exist
     // Action to be customized
  }

  /* Check UVLO flag: if not set, there is an undervoltage lock-out */
  if ((statusRegister & L6474_STATUS_UVLO) == 0)
  {
     //undervoltage lock-out
     // Action to be customized
  }

  /* Check TH_WRN flag: if not set, the thermal warning threshold is reached */
  if ((statusRegister & L6474_STATUS_TH_WRN) == 0)
  {
    //thermal warning threshold is reached
    // Action to be customized
  }

  /* Check TH_SHD flag: if not set, the thermal shut down threshold is reached */
  if ((statusRegister & L6474_STATUS_TH_SD) == 0)
  {
    //thermal shut down threshold is reached
    // Action to be customized
  }

  /* Check OCD  flag: if not set, there is an overcurrent detection */
  if ((statusRegister & L6474_STATUS_OCD) == 0)
  {
    //overcurrent detection
    // Action to be customized
  }

}

//Callback de las interrupciones
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){

	if(GPIO_Pin == GPIO_PIN_2){ //Final de carrera de la izquierda
		//BSP_MotorControl_HardStop(0); //Parada brusca motor
		modo = 0;                     //Indicación modo de ir a posición seguridad
		izquierda = 1;                //Final de carrera pulsado izquierdo
	}
	else if(GPIO_Pin == GPIO_PIN_1){ //Final de carrera derecho
		inicio = 1;                   //Calibración motor inicial finalizada
		//BSP_MotorControl_HardStop(0); //Parada brusca motor
		modo = 0;                     //Indicación modo de ir a posición de seguridad
		derecha = 1;                  //Final de carrera pulsado derecho
	}
	else if (GPIO_Pin == BSP_MOTOR_CONTROL_BOARD_FLAG_PIN) //Interrupción si la Shield detecta algo mal
	{
		BSP_MotorControl_FlagInterruptHandler();
	}
}

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
  MX_USART2_UART_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */

  //Inicialización driver
  BSP_MotorControl_SetNbDevices(BSP_MOTOR_CONTROL_BOARD_ID_L6474, 3);         //Se establece número de motores a controlar
  BSP_MotorControl_Init(BSP_MOTOR_CONTROL_BOARD_ID_L6474, &gL6474InitParams);//Parámetros iniciales del motor
  BSP_MotorControl_Init(BSP_MOTOR_CONTROL_BOARD_ID_L6474, &gL6474InitParams);
  BSP_MotorControl_Init(BSP_MOTOR_CONTROL_BOARD_ID_L6474, &gL6474InitParams);
  BSP_MotorControl_AttachFlagInterrupt(MyFlagInterruptHandler);               //Asociar controlador de interrupciones
  BSP_MotorControl_AttachErrorHandler(ErrorHandler_Shield);                   //Asociar función control errores

  //Inicialización programa la primera vez que se carga el programa

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  if (first_movement == 0){//
		  	  pos0 = BSP_MotorControl_GetPosition(0);
		  	  pos1 = BSP_MotorControl_GetPosition(1);
		  	  pos2 = BSP_MotorControl_GetPosition(2);
	 		  BSP_MotorControl_SetHome(0,pos0);//Set the step 0 in this position
	 		  BSP_MotorControl_SetHome(1,pos1);
	 		  BSP_MotorControl_SetHome(2,pos2);
	 		  BSP_MotorControl_SetMaxSpeed(0,600);//Set the speed limits
	 		  BSP_MotorControl_SetMaxSpeed(1,600);
	 		  BSP_MotorControl_SetMaxSpeed(2,600);
	 		  BSP_MotorControl_SetMinSpeed(0,600);
	 		  BSP_MotorControl_SetMinSpeed(1,600);
	 		  BSP_MotorControl_SetMinSpeed(2,600);
	 		  BSP_MotorControl_Move(0, FORWARD, 7500);//Va hacia 45º aprox
	 		  BSP_MotorControl_Move(1, FORWARD, 7500);
	 		  BSP_MotorControl_Move(2, FORWARD, 7500);
	 		  BSP_MotorControl_WaitWhileActive(0);//Wait to the previous movement to finish
	 		  BSP_MotorControl_WaitWhileActive(1);
	 		  BSP_MotorControl_WaitWhileActive(2);
	 		  BSP_MotorControl_Move(0, BACKWARD, 15000);//Va hacia -45º aprox
	 		  BSP_MotorControl_Move(1, BACKWARD, 15000);
	 		  BSP_MotorControl_Move(2, BACKWARD, 15000);
	 		  BSP_MotorControl_WaitWhileActive(0);//Wait to the previous movement to finish
	 		  BSP_MotorControl_WaitWhileActive(1);
	 		  BSP_MotorControl_WaitWhileActive(2);
	 		  first_movement = 1;
	 	  }
	 	  BSP_MotorControl_Move(0, FORWARD, 15000);//Va hacia 45º aprox
	 	  BSP_MotorControl_Move(1, FORWARD, 15000);
	 	  BSP_MotorControl_Move(2, FORWARD, 15000);
	 	  BSP_MotorControl_WaitWhileActive(0);//Wait to the previous movement to finish
	 	  BSP_MotorControl_WaitWhileActive(1);
	 	  BSP_MotorControl_WaitWhileActive(2);
	 	  BSP_MotorControl_Move(0, BACKWARD, 15000);//Va hacia -45º aprox
	 	  BSP_MotorControl_Move(1, BACKWARD, 15000);
	 	  BSP_MotorControl_Move(2, BACKWARD, 15000);
	 	  BSP_MotorControl_WaitWhileActive(0);//Wait to the previous movement to finish
	 	  BSP_MotorControl_WaitWhileActive(1);
	 	  BSP_MotorControl_WaitWhileActive(2);


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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
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

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11|GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC4 PC5 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PB1 PB2 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PA11 PA12 */
  GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

}

/* USER CODE BEGIN 4 */
int _write(int file, char *ptr, int len){
	int DataIdx;

	for(DataIdx = 0; DataIdx < len; DataIdx++){
		ITM_SendChar(*ptr++);
	}
	return len;
}

void ErrorHandler_Shield(uint16_t error)
{
  /* Backup error number */
  gLastError = error;

  /* Infinite loop */
  while(1)
  {
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

#ifdef  USE_FULL_ASSERT
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
