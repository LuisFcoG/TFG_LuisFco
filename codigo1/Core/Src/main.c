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
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

RTC_HandleTypeDef hrtc;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_RTC_Init(void);
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */
volatile int first_movement = 0;
volatile int pos0 = 0;
volatile int pos1 = 0;
volatile int pos2 = 0;
volatile int state = 0;
volatile int counter = 0;
volatile int FCS = 0;
volatile int FCC = 0;
volatile int ang = 1;
volatile int aux = 1;
volatile float A0 = 0;
volatile float A1 = 0;
volatile float A2 = 0;
volatile int step = 169;
volatile int sweep = 99;//100 grados es barrido nominal
volatile int half_sweep = 50;
volatile int init_pos = 10;
uint32_t minute = 60000;
uint32_t sweep_frec = 0;

uint32_t Vb1;
uint32_t Vc0;
uint32_t VpromS;
uint32_t VpromC;
volatile int Vcount = 0;
uint32_t ADC_VAL[2], ADC_BUFF[2];
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//Variables motor
static volatile uint16_t gLastError;

//Variables RTC
volatile RTC_TimeTypeDef sTime; //Inicialización hora
volatile RTC_DateTypeDef sDate; //Inicialización fecha

//Valores inicialización motores
L6474_Init_t gL6474InitParams =
{
    160,                               /// Acceleration rate in step/s2. Range: (0..+inf).
    160,                               /// Deceleration rate in step/s2. Range: (0..+inf).
    600,                              /// Maximum speed in step/s. Range: (30..10000].
    300,                               ///Minimum speed in step/s. Range: [30..10000).
    1300,                               ///Torque regulation current in mA. (TVAL register) Range: 31.25mA to 4000mA.
    2000,                               ///Overcurrent threshold (OCD_TH register). Range: 375mA to 6000mA.
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

	if (GPIO_Pin == BSP_MOTOR_CONTROL_BOARD_FLAG_PIN) //Shield interrupt handler
	{
		BSP_MotorControl_FlagInterruptHandler();
	}
}

void HAL_ADC_ConvCpltCallback (ADC_HandleTypeDef * hadc){

	if (hadc->Instance == ADC1){
		ADC_VAL[0]=ADC_BUFF[0];
		ADC_VAL[1]=ADC_BUFF[1];
  	  	Vb1 = ADC_VAL[0];
  	  	Vc0 = ADC_VAL[1];
	}
	if (Vcount < 50){
			 VpromS = (VpromS*Vcount + Vb1)/(Vcount + 1);
			 VpromC = (VpromC*Vcount + Vc0)/(Vcount + 1);
			 Vcount++;
		 }
		 if (Vcount == 50){
			 if (VpromS > 2500){
				 FCS = 1;
			 }
			 if (VpromS < 1000){
			 	 FCS = 0;
			 	 if(state == 2 && first_movement == 1){
			 		 state = 3;
	  	  	  		 BSP_MotorControl_HardStop(0);
	  	  	  		 BSP_MotorControl_HardStop(1);
	  	  	  		 BSP_MotorControl_HardStop(2);
	  	  	  	  	 pos0 = BSP_MotorControl_GetPosition(0);
	  	  	  	  	 pos1 = BSP_MotorControl_GetPosition(1);
	  	  	  	  	 pos2 = BSP_MotorControl_GetPosition(2);
	  	  	  	  	 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, SET);
	  	  	  	  	 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, SET);
	  	  	  	  	 while(1){

	  	  	  	  	 }
			 	 }
			 }
			 if (VpromC > 2500){
				 FCC = 1;
			 }
			 if (VpromC < 1000){
			 	 FCC = 0;
			 	 if (state == 1){
				 	 state = 2; //If calibrating precision during sweep use state 4
			 	 }
			 }
			 Vcount = 0;
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
  MX_DMA_Init();
  MX_RTC_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */

  //Start timer
  //HAL_TIM_Base_Start_IT(&htim5);

  //Start DMA
  HAL_ADC_Start_DMA(&hadc1, ADC_BUFF, 2);

  //Inicialización driver
  BSP_MotorControl_SetNbDevices(BSP_MOTOR_CONTROL_BOARD_ID_L6474, 3);         //Se establece número de motores a controlar
  BSP_MotorControl_Init(BSP_MOTOR_CONTROL_BOARD_ID_L6474, &gL6474InitParams); //Parámetros iniciales del motor
  BSP_MotorControl_Init(BSP_MOTOR_CONTROL_BOARD_ID_L6474, &gL6474InitParams);
  BSP_MotorControl_Init(BSP_MOTOR_CONTROL_BOARD_ID_L6474, &gL6474InitParams);
  BSP_MotorControl_AttachFlagInterrupt(MyFlagInterruptHandler);               //Asociar controlador de interrupciones
  BSP_MotorControl_AttachErrorHandler(ErrorHandler_Shield);                   //Asociar función control errores

  //Inicialización programa la primera vez que se carga el programa
  state = 1; //Initial state, 1->Normal, 4->Accuracy calibration
  sweep_frec = 5; //Delay between sweeps in minutes

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  /* USER CODE END WHILE */
	  if (state == 1){
		  first_movement = 0;
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, RESET);
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, RESET);
		  pos0 = BSP_MotorControl_GetPosition(0);
		  pos1 = BSP_MotorControl_GetPosition(1);
		  pos2 = BSP_MotorControl_GetPosition(2);
		  BSP_MotorControl_SetHome(0,pos0);//Set the step 0 in this position
		  BSP_MotorControl_SetHome(1,pos1);
		  BSP_MotorControl_SetHome(2,pos2);
		  HAL_Delay(1000);
		  BSP_MotorControl_SetMaxSpeed(0,566);//Set the speed limits
		  BSP_MotorControl_SetMaxSpeed(1,566);
		  BSP_MotorControl_SetMaxSpeed(2,566);
		  BSP_MotorControl_SetMinSpeed(0,566);
		  BSP_MotorControl_SetMinSpeed(1,566);
		  BSP_MotorControl_SetMinSpeed(2,566);
		  BSP_MotorControl_Run(0, BACKWARD);//Va hacia FCC
		  BSP_MotorControl_Run(1, BACKWARD);
		  BSP_MotorControl_Run(2, BACKWARD);
	  }
	  else if (state == 2){
		  if(first_movement == 0){
			  BSP_MotorControl_Move(0, FORWARD, step*init_pos);//Va al punto inicial de barrido, -50º
			  BSP_MotorControl_Move(1, FORWARD, step*init_pos);
			  BSP_MotorControl_Move(2, FORWARD, step*init_pos);
			  BSP_MotorControl_WaitWhileActive(0);//Wait to the previous movement to finish
			  BSP_MotorControl_WaitWhileActive(1);
			  BSP_MotorControl_WaitWhileActive(2);
			  first_movement = 1;
		  }
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, SET);
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, RESET);
		  BSP_MotorControl_Move(0, FORWARD, step*sweep);//Va hacia -50º aprox
		  BSP_MotorControl_Move(1, FORWARD, step*sweep);
		  BSP_MotorControl_Move(2, FORWARD, step*sweep);
		  BSP_MotorControl_WaitWhileActive(0);//Wait to the previous movement to finish
		  BSP_MotorControl_WaitWhileActive(1);
		  BSP_MotorControl_WaitWhileActive(2);

		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, RESET);
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, SET);
		  BSP_MotorControl_Move(0, BACKWARD, step*sweep);//Va hacia +50º aprox
		  BSP_MotorControl_Move(1, BACKWARD, step*sweep);
		  BSP_MotorControl_Move(2, BACKWARD, step*sweep);
		  BSP_MotorControl_WaitWhileActive(0);//Wait to the previous movement to finish
		  BSP_MotorControl_WaitWhileActive(1);
		  BSP_MotorControl_WaitWhileActive(2);
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, RESET);
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, RESET);
		  HAL_Delay(minute*sweep_frec);
		  counter++;
		  if (counter >= 12){
			  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, RESET);
			  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, RESET);
			  BSP_MotorControl_Move(0, FORWARD, step*half_sweep);//Va hacia 0º aprox
			  BSP_MotorControl_Move(1, FORWARD, step*half_sweep);
			  BSP_MotorControl_Move(2, FORWARD, step*half_sweep);
			  BSP_MotorControl_WaitWhileActive(0);//Wait to the previous movement to finish
			  BSP_MotorControl_WaitWhileActive(1);
			  BSP_MotorControl_WaitWhileActive(2);
			  BSP_MotorControl_HardStop(0);//Decelerate until stopping
			  BSP_MotorControl_HardStop(1);
			  BSP_MotorControl_HardStop(2);
			  counter = 0;
			  HAL_Delay(minute/2);
			  state = 1;
		  }
	  }
	  else if (state == 3){ //Security
		  BSP_MotorControl_HardStop(0);//Decelerate until stopping
		  BSP_MotorControl_HardStop(1);
		  BSP_MotorControl_HardStop(2);
		  //Wait to manual check the model, mandatory reset
		  //Place all the trackers horizontally
	  }
	  else if (state == 4){ //Prueba
		  if(first_movement == 0){
			  BSP_MotorControl_HardStop(0);
			  BSP_MotorControl_HardStop(1);
			  BSP_MotorControl_HardStop(2);
			  pos0 = BSP_MotorControl_GetPosition(0);
			  pos1 = BSP_MotorControl_GetPosition(1);
			  pos2 = BSP_MotorControl_GetPosition(2);
			  BSP_MotorControl_SetHome(0,pos0);//Set the step 0 in this position
			  BSP_MotorControl_SetHome(1,pos1);
			  BSP_MotorControl_SetHome(2,pos2);
			  A0 = pos0 / 169;
			  A1 = pos1 / 169;
			  A2 = pos2 / 169;
			  first_movement = 1;
			  HAL_Delay(minute/2);
		  }
		  if(ang < 25){
			  BSP_MotorControl_GoTo(0, 169*5*ang);//Avanza 5 grados
			  BSP_MotorControl_GoTo(1, 169*5*ang);
			  BSP_MotorControl_GoTo(2, 169*5*ang);
			  BSP_MotorControl_WaitWhileActive(0);//Wait to the previous movement to finish
			  BSP_MotorControl_WaitWhileActive(1);
			  BSP_MotorControl_WaitWhileActive(2);
			  pos0 = BSP_MotorControl_GetPosition(0);
			  pos1 = BSP_MotorControl_GetPosition(1);
			  pos2 = BSP_MotorControl_GetPosition(2);
			  A0 = pos0 / 169;
			  A1 = pos1 / 169;
			  A2 = pos2 / 169;
			  ang++;
			  HAL_Delay(minute/2);
		  }
		  if(ang > 24){
			  aux++;
			  BSP_MotorControl_GoTo(0, 169*5*(ang-aux));//Va hacia 50º
			  BSP_MotorControl_GoTo(1, 169*5*(ang-aux));
			  BSP_MotorControl_GoTo(2, 169*5*(ang-aux));
			  BSP_MotorControl_WaitWhileActive(0);//Wait to the previous movement to finish
			  BSP_MotorControl_WaitWhileActive(1);
			  BSP_MotorControl_WaitWhileActive(2);
			  pos0 = BSP_MotorControl_GetPosition(0);
			  pos1 = BSP_MotorControl_GetPosition(1);
			  pos2 = BSP_MotorControl_GetPosition(2);
			  A0 = pos0 / 169;
			  A1 = pos1 / 169;
			  A2 = pos2 / 169;
			  aux++;
			  ang++;
			  HAL_Delay(minute/2);
		  }
		  if(ang > 48){
			  ang = 1;
			  aux = 1;
			  HAL_Delay(minute/2);
		  }
	  }
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
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 2;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_10;
  sConfig.Rank = 2;
  sConfig.SamplingTime = ADC_SAMPLETIME_144CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

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
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11|GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA11 PA12 */
  GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
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
