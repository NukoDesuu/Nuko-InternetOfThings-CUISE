/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "cmsis_os.h"
#include <string.h>
#include <stdio.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart2;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* USER CODE BEGIN PV */
volatile int status = 1;
volatile int lineTrackerLeft_Found = 0;
volatile int lineTrackerRight_Found = 0;

// For ULTRASONIC SENSOR measurements
#define TRIG_PIN GPIO_PIN_9
#define TRIG_PORT GPIOA
#define ECHO_PIN GPIO_PIN_8
#define ECHO_PORT GPIOA
uint32_t pMillis;
uint32_t Value1 = 0;
uint32_t Value2 = 0;
volatile uint16_t Distance  = 0;  // cm; //in a unit of cm
uint16_t currentDistance = 0;
uint16_t distanceLimit = 10; //in a unit of cm
// For LINE TRACKING sensor
volatile uint16_t lineL, lineR;
// For BUZZER melody
uint32_t OctaveFourCMajor[8] = {262, 294, 330, 349, 392, 440, 493, 523};
uint32_t OctaveFourCSharpMajor[8] = {277, 311, 349, 370, 415, 466, 523, 554};
uint32_t SingleMelody[1] = {415};
uint32_t WarningMelody[1] = {349};
uint32_t ExampleMelody[3] = {277, 349, 415};
uint8_t NoteIndex = 0;

// MULTITASKING handles
osThreadId_t measureDistanceHandle;
osThreadId_t detectLineHandle;
osThreadId_t buzzerMelodyHandle;
osThreadId_t blinkLEDHandle;

static void measureDistance() {
	// This function keeps running all the time
	while (1) {
		HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_SET);  // pull the TRIG pin HIGH
		__HAL_TIM_SET_COUNTER(&htim1, 0);
		while (__HAL_TIM_GET_COUNTER (&htim1) < 10);  // wait for 10 us
		HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);  // pull the TRIG pin low

		pMillis = HAL_GetTick(); // used this to avoid infinite while loop  (for timeout)
		// wait for the echo pin to go high
		while (!(HAL_GPIO_ReadPin (ECHO_PORT, ECHO_PIN)) && pMillis + 10 >  HAL_GetTick());
		Value1 = __HAL_TIM_GET_COUNTER (&htim1);

		pMillis = HAL_GetTick(); // used this to avoid infinite while loop (for timeout)
		// wait for the echo pin to go low
		while ((HAL_GPIO_ReadPin (ECHO_PORT, ECHO_PIN)) && pMillis + 50 > HAL_GetTick());
		Value2 = __HAL_TIM_GET_COUNTER (&htim1);

		Distance = (Value2-Value1)* 0.034/2;
		osDelay(10);
	}
}
static void detectLine() {
    uint32_t lineL, lineR;
    const uint32_t threshold = 300;  // Set threshold for black/white detection

    while (1) {

    	ADC_ChannelConfTypeDef sConfig = {0};
    		  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
    		  */
    		  sConfig.Channel = ADC_CHANNEL_14;
    		  sConfig.Rank = 1;
    		  //sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES;
    		  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    		  {
    		    Error_Handler();
    		  }
	    	HAL_ADC_Start(&hadc1);

	    	if (HAL_ADC_PollForConversion(&hadc1, 1000) == HAL_OK)
	    	{
	    		lineL = HAL_ADC_GetValue(&hadc1);
	    	}

	    	ADC_ChannelConfTypeDef sConfig2 = {0};
	    		  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	    		  */
	    		  sConfig2.Channel = ADC_CHANNEL_15;
	    		  sConfig2.Rank = 2;
	    		  //sConfig.SamplngTime = ADC_SAMPLETIME_28CYCLES;
	    		  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig2) != HAL_OK)
	    		  {
	    		    Error_Handler();
	    		  }
	    	HAL_ADC_Start(&hadc1);

	    	// Poll for conversion for the second channel
	    	if (HAL_ADC_PollForConversion(&hadc1, 1000) == HAL_OK)
	    	{
	    		lineR = HAL_ADC_GetValue(&hadc1);
	    	}


	    	lineTrackerLeft_Found = lineL;
	    	lineTrackerRight_Found = lineR;
//
//        if (lineL < threshold && lineR >= threshold) {
//        	lineTrackerLeft_Found = 0;
//        	lineTrackerRight_Found = 1;
//        }
//        else if (lineR < threshold && lineL >= threshold) {
//        	lineTrackerLeft_Found = 1;
//        	lineTrackerRight_Found = 0;
//        }
//        else if (lineL >= threshold && lineR >= threshold) {
//        	lineTrackerLeft_Found = 0;
//        	lineTrackerRight_Found = 0;
//        }
//        else if (lineL < threshold && lineR < threshold) {
//        	lineTrackerLeft_Found = 1;
//        	lineTrackerRight_Found = 1;
//        }

        osDelay(5);
    }
}

static void Tone(uint32_t Frequency, uint32_t Duration, int Volume) {
	TIM2->ARR = (1000000UL / Frequency) - 1; //Setting the PWM Frequency
	TIM2->CCR1 = (TIM2->ARR / 2 * Volume / 100); //Setting duty cycle to 50% (volume depending on frequency)
	HAL_Delay(Duration);
}

static void noTone() {
	TIM2->CCR1 = 0; //Mute by setting duty cycle 0%
}

static void buzzerMelody() {
	if (status == 1) {
		while (status == 1) {
			Tone(ExampleMelody[NoteIndex++], 250, 50);
			if (NoteIndex == 3) {
			  NoteIndex = 0;
			  noTone();
			  osDelay(2000);
			}
			osDelay(1000);
		}
	}
	else {
		noTone();
	}
}

static void blinkLED() {
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);
	if (status == 1) {
		while (status == 1) {
			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_13);
			osDelay(1000);
		}
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);
	}
}
// For SERIAL OUTPUT
char msg[256];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM2_Init(void);
static void MX_ADC1_Init(void);
void StartDefaultTask(void *argument);

/* USER CODE BEGIN PFP */
void driveCar(float powerL, float powerR, int direction);
static void Tone(uint32_t Frequency, uint32_t Duration, int Volume);
static void noTone();
static void detectLine();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
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
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_TIM2_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  // Initialization for BUZZER
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1); //For BUZZER
  // Initialization for WHEEL MOTORS
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1); //For LEFT PWM
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2); //For RIGHT PWM
  //driveCar(80, 80, 1);
  // Initialization for ULTRASONIC SENSOR
  HAL_TIM_Base_Start(&htim1);
  HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET); //pulls TRIG pin to LOW
  // Variables
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
//  const osThreadAttr_t measureDistance_attributes = {
//		  .name = "measureDistance",
//		  .priority = (osPriority_t) osPriorityNormal,
//		  .stack_size = 128
//  };

//  const osThreadAttr_t detectLine_attributes = {
//		  .name = "detectLine",
//		  .priority = (osPriority_t) osPriorityNormal,
//		  .stack_size = 128
//  };
//
//  const osThreadAttr_t buzzerMelody_attributes = {
//		  .name = "buzzerMelody",
//		  .priority = (osPriority_t) osPriorityNormal,
//		  .stack_size = 128
//  };
////
//  const osThreadAttr_t blinkLED_attributes = {
//		  .name = "blinkLED",
//		  .priority = (osPriority_t) osPriorityNormal,
//		  .stack_size = 128
//  };
//
    //measureDistanceHandle = osThreadNew(measureDistance, NULL, &measureDistance_attributes);
//    //detectLineHandle = osThreadNew(detectLine, NULL, &detectLine_attributes);
//	  buzzerMelodyHandle = osThreadNew(buzzerMelody, NULL, &buzzerMelody_attributes);
//	  blinkLEDHandle = osThreadNew(blinkLED, NULL, &blinkLED_attributes);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  //while (1)
  //{
//	  HAL_Delay(50);
//
//	  detectLine();
//	  HAL_Delay(50);
//	  sprintf(msg, "hello world\r\n");
//	  HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 1000);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  //}
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_14;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
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

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 71;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

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

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 71;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 65535;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

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
  htim3.Init.Prescaler = 71;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 4095;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
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
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

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
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LefttPair_DirectionR_Pin|LeftPair_DirectionF_Pin|RightPair_DirectionR_Pin|RightPair_DirectionF_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LD2_Pin|Ultrasonic_Trig_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_Module_GPIO_Port, LED_Module_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LefttPair_DirectionR_Pin LeftPair_DirectionF_Pin RightPair_DirectionR_Pin RightPair_DirectionF_Pin */
  GPIO_InitStruct.Pin = LefttPair_DirectionR_Pin|LeftPair_DirectionF_Pin|RightPair_DirectionR_Pin|RightPair_DirectionF_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LD2_Pin Ultrasonic_Trig_Pin */
  GPIO_InitStruct.Pin = LD2_Pin|Ultrasonic_Trig_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : LED_Module_Pin */
  GPIO_InitStruct.Pin = LED_Module_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_Module_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Green_Button_Pin */
  GPIO_InitStruct.Pin = Green_Button_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Green_Button_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Ultrasonic_Echo_Pin */
  GPIO_InitStruct.Pin = Ultrasonic_Echo_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Ultrasonic_Echo_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */


void ADC_Select_CH14 (void)
{
	ADC_ChannelConfTypeDef sConfig = {0};
	  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	  */
	  sConfig.Channel = ADC_CHANNEL_14;
	  sConfig.Rank = 1;
	  //sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES;
	  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	  {
	    Error_Handler();
	  }
}

void ADC_Select_CH15 (void)
{
	ADC_ChannelConfTypeDef sConfig = {0};
	  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	  */
	  sConfig.Channel = ADC_CHANNEL_15;
	  sConfig.Rank = 1;
	  //sConfig.SamplingTime = ADC_SAMPLETIME_84CYCLES;
	  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	  {
	    Error_Handler();
	  }
}

void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
	//driveCar(100, 100, 1);
  /* Infinite loop */
  for(;;)
  {
	  //sprintf(msg, "Distance: %d\r\n", Distance);
	  //HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 1000);
	  //sprintf(msg, "Left found: %d Right found: %d\r\n", lineTrackerLeft_Found, lineTrackerRight_Found);
	  //HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 1000);

	  // Line tracker

	  uint16_t lineL = 0;
	  uint16_t lineR = 0;
	  uint16_t distance = 0;
	  uint16_t threshold = 1000;

	  char buf[256];

	  ADC_Select_CH14();
	  HAL_ADC_Start(&hadc1);

	  // Poll for conversion for the first channel
	  if (HAL_ADC_PollForConversion(&hadc1, 1000) == HAL_OK)
	  {
		  lineL = HAL_ADC_GetValue(&hadc1);
	  }

	  ADC_Select_CH15();
	  HAL_ADC_Start(&hadc1);

	  // Poll for conversion for the second channel
	  if (HAL_ADC_PollForConversion(&hadc1, 1000) == HAL_OK)
	  {
		  lineR = HAL_ADC_GetValue(&hadc1);
	  }


	  // Read UltraSonic

	  HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_SET);  // pull the TRIG pin HIGH
	  __HAL_TIM_SET_COUNTER(&htim1, 0);
	  while (__HAL_TIM_GET_COUNTER (&htim1) < 10);  // wait for 10 us
	  HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);  // pull the TRIG pin low

	  pMillis = HAL_GetTick(); // used this to avoid infinite while loop  (for timeout)
	  		// wait for the echo pin to go high
	  while (!(HAL_GPIO_ReadPin (ECHO_PORT, ECHO_PIN)) && pMillis + 10 >  HAL_GetTick());
	  Value1 = __HAL_TIM_GET_COUNTER (&htim1);

	  pMillis = HAL_GetTick(); // used this to avoid infinite while loop (for timeout)
	  // wait for the echo pin to go low
	  while ((HAL_GPIO_ReadPin (ECHO_PORT, ECHO_PIN)) && pMillis + 50 > HAL_GetTick());
	  Value2 = __HAL_TIM_GET_COUNTER (&htim1);

	  distance = (Value2-Value1)* 0.034/2;
	  osDelay(10);


	  // Print

	  sprintf(msg, "Light: %d %d | Distance: %d \r\n", lineL, lineR, distance);
	  HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 1000);

	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);

	  driveCar(100, 100, 1);

	  // Logic
	  if(distance < 20) {
		  driveCar(0, 0, 1);
		  while(1) {
			  Tone(ExampleMelody[NoteIndex++], 250, 50);
			  if (NoteIndex == 3) {
			  	NoteIndex = 0;
			  	noTone();
			  	osDelay(1000);
			  	break;
			  }
			  noTone();
		  }

		  for (int i = 0; i < 4; i++) {
			  HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_13);
			  osDelay(500);
		  }
	  }
	  else {
		  driveCar(100, 100, 1);
	  }
//	  else {
//		  // Logic to control car based on line sensor readings
//		  if (lineL < threshold && lineR < threshold) {
//		      // Both sensors are off the line (white surface), stop or search for the line
//			  driveCar(0, 0, 1); // Stop
//		  }
//		  else if (lineL > threshold && lineR < threshold) {
//		      // Left sensor on line, right sensor off line, turn left
//		      driveCar(90, 50, 1); // Turn left with reduced speed on the left motor
//		  }
//		  else if (lineL < threshold && lineR > threshold) {
//		      // Right sensor on line, left sensor off line, turn right
//		      driveCar(50, 90, 1); // Turn right with reduced speed on the right motor
//		  }
//		  else {
//		     // Both sensors are on the line (black surface), move straight
//		      driveCar(90, 90, 1); // Move forward
//		  }
//	  }



	    //HAL_Delay(50);  // Add some delay between readings

//	  if(Distance < 20) {
//
//
//
//
//		  		    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_13);
//		  		    		      osDelay(1000);
//		  		    		    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_13);
//		  		    		    		      osDelay(1000);
//		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);
//	  }
//	  else {
//		  driveCar(100, 100, 1);
//	  }

//	  char msg[256];
//	  sprintf(msg, "Left found: %d Right found: %d\r\n", lineTrackerLeft_Found, lineTrackerRight_Found);
//	  HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 1000);
//


	  osDelay(50);

  }
  /* USER CODE END 5 */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM4 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM4) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
	sprintf(msg, "Error Error");
	HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 1000);

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
