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
#include "math.h"
#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define sensorCount 5
#define maxValue 1000
/*Configure GPIO pins : Line_2_Pin Line_3_Pin Line_4_Pin Line_5_Pin
                           M2_B_Pin */

  /*Configure GPIO pin : M2_A_Pin */
  //M2_A_Pin/M2_A_GPIO_Port

  /*Configure GPIO pin : Echo_Pin */
  //Echo_Pin/Echo_GPIO_Port

  /*Configure GPIO pins : Trig_Pin BIN_2_Pin */
  //Trig_Pin|BIN_2_Pin;

  /*Configure GPIO pins : AIN_2_Pin AIN_1_Pin */
  //AIN_2_Pin|AIN_1_Pin;

  /*Configure GPIO pin : BIN_1_Pin */
  //BIN_1_Pin/ BIN_1_GPIO_Port

  /*Configure GPIO pin : Line_1_Pin */
  //Line_1_Pin/ Line_1_GPIO_Port
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
 ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

/* USER CODE BEGIN PV */
uint16_t val1 = 0;	// Counter value when ultrasonic generates wave
uint16_t val2 = 0;	// Counter value when sensor receive echo wave
uint16_t dis = 0;		// Raw distance
int is_dis_unread = 1;
uint16_t dis_final; // Distance after filter
int count_filter = 0;
uint32_t sum, mean, std;
int k, num, size = 5; // constances for filter sensitive
uint16_t dis_arr[10];
int32_t temp;
/* ----------------------------------------------*/

// Robot parameter

float b=168;    //khoang cach giua hai banh xe
float r=32.5;  //Ban kinh banh xe
float d=185.0;    //Khoang cach giua tam hai banh xe va
float bl=26;   //Do dai duong line
float vr = 850; //Toc do du kien

int id;

	// Motor 1
int32_t time_1, now_1;
int32_t delta_1;
int16_t count_1;
int32_t pre_count_1;
float w_1;					// RPM 1
int pwm_1;

	// Motor 2
uint32_t time_2, count_2, now_2; int16_t count_pre = 0; int16_t count_now = 0;
int32_t delta_2;
float w_2;					// RPM 2
int dir_2, pwm_2;


// PID he thong
//float kp = 0.12,ki=0.0, kd = 0.05;  //PID v = 0.2m/s t= 0.025 ****
float kp = 0.03,ki=0.004, kd = 5
	; //P 0.06,D 4.2 is temporary good
  //PID v = 0.2m/s t= 0.025
//float kp = 0.25,ki=0.06, kd = 0.04;  //PID v = 0.8m/s t= 0.025
float e2 =0,sum_e2=0,last_e2=0;
float w1_ref, w2_ref;
float pi=3.14;
//PID motor
float kp1 = 0.042, ki1 = 1.678 , kd1 = 0;
float kp2 = 0.042, ki2 =1.678, kd2 = 0;
float er_1, er_2;
float sum_er_1=0, sum_er_2=0;
int id;
int pwm_max = 100;
float samp_t = 0.02;
int turn_left = 0;
	// Servo
int pos, pwm;
// Lyapunov

float k2 = 1.2, k3 = 1;
float e3 = 0;
float v = 200;
/* ----------------------------------------------*/


// IR Sensor variables


/* ----------------------------------------------*/
uint16_t sensorValues[sensorCount];
float y[sensorCount];
/* --------------------CALIB LAN 1---------------*/
float sensorMaxCalib[sensorCount]={3488.07,3442.84,3613.55,3482.51,3563.02};
float sensorMinCalib[sensorCount]={1598.63,1538.70,2119.90,1423.57,1729.37};
float sensorMinValue = 1682.03;
float gain[sensorCount]={0.972,0.964,1.229,0.892,1.001};
/* --------------------CALIB LAN 2---------------*/
//float sensorMaxCalib[sensorCount]={3467.708,3523.66,3637.598,3546.236,3465.299};
//float sensorMinCalib[sensorCount]={1309.464,1375.44,1971.779,1260.745,1488.556};
//float sensorMinValue = 1480.027;
//float gain[sensorCount]={0.947,0.953,1.229,0.895,1.038};
uint16_t sensorPins[sensorCount] = {GPIO_PIN_2,GPIO_PIN_3,GPIO_PIN_4,GPIO_PIN_5,GPIO_PIN_0};
float position;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM4_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */
// Ultrasonic sensor function
void delay(uint32_t time);
void delay_us(uint32_t us);
void trig_hc04(void);
void read_hc04(void);
void filter(void);
void turn_sg90(int pos);
void read_speed_1(void);
void read_speed_2(void);
/* ----------------------------------------------*/
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void delay_us (uint32_t us)
{
	__HAL_TIM_SET_COUNTER(&htim1,0);  // set the counter value a 0
	while (__HAL_TIM_GET_COUNTER(&htim1) < us);  // wait for the counter to reach the us input in the parameter
}
// Servo function
void turn_sg90(int pos){
		pwm = (int)((1+pos/180.0)*10);
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2, pwm);
}
/* ----------------------------------------------*/

void filter(){
	sum = 0; std = 0;
	// Calculate sum, mean
	for(int i = 0; i < size; i++){
		sum += dis_arr[i];
	}
	mean = sum /size;

	// Calculate standard deviation
	for(int i = 0; i < size; i++){
		std = (dis_arr[i] - mean)*(dis_arr[i] - mean) + std;
	}
	std = sqrt(std/size);

	// Filter Outlier
	sum = 0; num = 0;
	for(int i = 0; i < size; i++){
		if ((dis_arr[i] - mean)*(dis_arr[i] - mean) > pow(std,2)){
			sum += dis_arr[i];
			num ++;
		}
	}
	if (num == 0){
		num = 1;
	}
	else{
		dis_final = (sum / num);
	}
}
/* ----------------------------------------------*/

// Motor function
void read_speed_1(){
	//now_1 = HAL_GetTick();
	delta_1 = HAL_GetTick() - time_1;
	count_1 = (int16_t)__HAL_TIM_GET_COUNTER(&htim2);

	w_1 = -count_1*1000.0*60/(4*374*delta_1);
	__HAL_TIM_SET_COUNTER(&htim2, 0);
	//pre_count_1 =__HAL_TIM_GET_COUNTER(&htim2) ;
	time_1 = HAL_GetTick();
}

void read_speed_2(){
	delta_2 = HAL_GetTick() - time_2;
	count_now = count_2 - count_pre;
	w_2 = (float)count_now*1000*60/(delta_2*374);
	count_pre = count_2;
	time_2 = HAL_GetTick();
}

void pid_motor_1(float v1, float v1_ref){
	er_1 = v1_ref - v1;
	sum_er_1 += er_1*samp_t;
	//d_er = (er - pre_er_1)/samp_t;
	pwm_1 =(int)(kp1 * er_1 + ki1 * sum_er_1);
	//pre_er_1 = er_1;
	if (pwm_1 > pwm_max)
		pwm_1= pwm_max;
	else if (pwm_1 < 0)
		pwm_1= 0;
}

void pid_motor_2(float v2, float v2_ref){
	er_2 = v2_ref - v2;
	sum_er_2 += er_2*samp_t;
//	d_er = (er - pre_er_2)/samp_t;
	pwm_2 = (uint32_t)kp2 * er_2 + ki2 * sum_er_2;
	//pre_er_2 = er_2;
	if (pwm_2 > pwm_max)
		pwm_2= pwm_max;
	else if (pwm_2 < 0)
		pwm_2= 0;
}

/* ----------------------------------------------*/


/* ----------------------------------------------*/



int read_linesensor(){
	HAL_ADC_Start_DMA(&hadc1,(uint32_t *)sensorValues,sensorCount);
	int i;
	for (i=0;i<sensorCount;i++)
	{
		if(sensorValues[i]>sensorMaxCalib[i])sensorValues[i]=sensorMaxCalib[i];
		if(sensorValues[i]<sensorMinCalib[i])sensorValues[i]=sensorMinCalib[i];
		y[i]=sensorMinValue+gain[i]*(sensorValues[i]-sensorMinCalib[i]);
	}
	position = (200*y[0]+100*y[1]+0*y[2]-100*y[3]-200*y[4])/(y[0]+y[1]+y[2]+y[3]+y[4]);
	return position;
}

int PID(float position){
	e2= 0.0-position;
	sum_e2 = sum_e2 + e2;

	float P = kp*e2;
	float I = ki*sum_e2;
	float D = kd*(e2 - last_e2);
	float w = P+I+D;
	w2_ref = (1.0/r)*(vr+b/2.0*w)*(60.0/(2*pi));
	w1_ref = (1.0/r)*(vr-b/2.0*w)*(60.0/(2*pi));
	if (w1_ref>250)w1_ref = 250;
	if (w2_ref>250)w2_ref = 250;
	if (w1_ref<0)w1_ref = 0;
	if (w2_ref<0)w2_ref = 0;
	last_e2=e2;
}

int Tracking_control(float position){
	e2= 0.0-position;
	e3 = atan((e2-last_e2)/(v*samp_t));
	v = vr*cos(e3);
	float w = k2*vr*e2+k3*sin(e3);
	w2_ref = (1.0/r)*(v+b/2.0*w)*(60.0/(2*pi));
	w1_ref = (1.0/r)*(v-b/2.0*w)*(60.0/(2*pi));
	if (w1_ref>250)w1_ref = 250;
	if (w2_ref>250)w2_ref = 250;
	if (w1_ref<0)w1_ref = 0;
	if (w2_ref<0)w2_ref = 0;
	last_e2=e2;

}
/* USER CODE END 0 */

void driveCar(float powerL, float powerR, int direction) {
	//Change the DIRECTION of the car
	switch (direction) {
	case 0: //Reverse direction
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET); //LEFT pair REVERSE off
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET); //LEFT pair FORWARD on
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET); //RIGHT pair reverse off
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_RESET); //RIGHT pair forward on
	case 1: //Forward direction
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET); //LEFT pair REVERSE off
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET); //LEFT pair FORWARD on
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET); //RIGHT pair reverse off
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET); //RIGHT pair forward on
	}
	//Change the POWER OUTPUT of the car
	TIM3->CCR1 = 4095 * (powerL/100); //Duty time output per 100% power for LEFT pair
	TIM3->CCR2 = 4095 * (powerR/100); //Duty time output per 100% power for RIGHT pair
}


/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration------------------------------------------
																																--------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  __HAL_RCC_AFIO_CLK_ENABLE();
  __HAL_AFIO_REMAP_SWJ_NONJTRST();
  AFIO_MAPR_SWJ_CFG_JTAGDISABLE_Pos;
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_DMA_Init();
  MX_TIM4_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
	HAL_TIM_Base_Start(&htim1);
  HAL_TIM_Base_Start(&htim2);
  HAL_TIM_Base_Start(&htim3);
  HAL_TIM_Base_Start(&htim4);

  HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_1);
	HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
//  HAL_ADC_Start_IT(&hadc1);
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_8, 1);
  	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_9, 0);
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6, 0);
  	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7, 1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  driveCar(100, 100, 1);
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
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

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 5;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = ADC_REGULAR_RANK_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 71;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 99;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_Encoder_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 65535;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 0;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 0;
  if (HAL_TIM_Encoder_Init(&htim2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 7199;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 199;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 71;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 65535;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, Trig_Pin|BIN_1_Pin|BIN_2_Pin|GPIO_PIN_6
                          |GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10|GPIO_PIN_11|AIN_2_Pin|AIN_1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC13 PC15 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : M2_A_Pin */
  GPIO_InitStruct.Pin = M2_A_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(M2_A_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : M2_B_Pin */
  GPIO_InitStruct.Pin = M2_B_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(M2_B_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Echo_Pin */
  GPIO_InitStruct.Pin = Echo_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(Echo_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Trig_Pin BIN_1_Pin BIN_2_Pin PB6
                           PB7 PB8 PB9 */
  GPIO_InitStruct.Pin = Trig_Pin|BIN_1_Pin|BIN_2_Pin|GPIO_PIN_6
                          |GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PA10 PA11 AIN_2_Pin AIN_1_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11|AIN_2_Pin|AIN_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if (GPIO_Pin == Echo_Pin){
		if (HAL_GPIO_ReadPin(Echo_GPIO_Port, Echo_Pin) == 1){
			val1 = __HAL_TIM_GET_COUNTER(&htim4);
		}
		else if (HAL_GPIO_ReadPin(Echo_GPIO_Port, Echo_Pin) == 0){
			val2 = __HAL_TIM_GET_COUNTER(&htim4);
		}
	}
	if (GPIO_Pin == M2_A_Pin){
		count_2 = count_2 + 1;
		if (HAL_GPIO_ReadPin(M2_B_GPIO_Port, M2_B_Pin) == 1){
			dir_2 = 1;
		}
		else{
			dir_2 = -1;
		}
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
