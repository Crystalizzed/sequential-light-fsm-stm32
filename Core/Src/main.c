/* USER CODE BEGIN Header */
/***********************************************************************
 * @file			main.c
 * @brief	 		Embedded Non-Blocking Sequential Light Controller (FSM)
 * @date 			September 2026
 * @author			Crystalizzed
 *
 * @details			Event-driven Finite State Machine (FSM) driving a 3-light
 * 					sequential light loop and dynamic pitch-shifting passive buzzer.
 *
 * @note			Target Hardware: NUCLEO-F429ZI (ARM Cortex-M4 @ 180MHz)
 *
 * @note			Hardware Pinout Configuration (External Breadboard Prototype):
 * 					- PF13 (D7): pushbutton input (EXTI, hardware debounced, internal pull-up)
 * 					- PF15 (D2): red LED transistor (push-pull, active high)
 * 					- PE13 (D3): yellow LED transistor (push-pull, active high)
 * 					- PF14 (D4): green LED transistor (push-pull, active high)
 * 					- PE9  (D6): passive buzzer (TIM1_CH1 PWM alternate function)
 *
 ***********************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ***********************************************************************/
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

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
TIM_HandleTypeDef htim1;

/* USER CODE BEGIN PV */
// states of light cycle
typedef enum{
	STATE_IDLE,
	STATE_RED,
	STATE_YELLOW,
	STATE_GREEN,
} LightState;

volatile LightState currentState = STATE_IDLE; // tracker for state of the loop
volatile uint32_t stateStartTime=0, buzzerStartTime=0; // time keeping for state and buzzer timing
volatile uint8_t buzzerActive=0; // true/false for if buzzer is active
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
/* USER CODE BEGIN PFP */
void BuzzerActivation(LightState currentState);
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
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1){

	  // loop for changing light states
	if(currentState>STATE_IDLE){
		switch(currentState){
			case STATE_RED:
				// turn on red LED
				HAL_GPIO_WritePin(RED_TRANSISTOR_GPIO_Port, RED_TRANSISTOR_Pin, GPIO_PIN_SET);

				// check if 3 seconds have elapsed, if so update state and turn off red LED
				if(HAL_GetTick() - stateStartTime >= 3000){
					HAL_GPIO_WritePin(RED_TRANSISTOR_GPIO_Port, RED_TRANSISTOR_Pin, GPIO_PIN_RESET);
					currentState = STATE_YELLOW;
					stateStartTime = HAL_GetTick();
					BuzzerActivation(currentState); // call buzzer activation subroutine
				}
				break;

			case STATE_YELLOW:
				// turn on yellow LED
				HAL_GPIO_WritePin(YELLOW_TRANSISTOR_GPIO_Port, YELLOW_TRANSISTOR_Pin, GPIO_PIN_SET);

				// check if 3 seconds have elapsed, if so update state and turn off yellow LED
				if(HAL_GetTick() - stateStartTime >= 3000){
					HAL_GPIO_WritePin(YELLOW_TRANSISTOR_GPIO_Port, YELLOW_TRANSISTOR_Pin, GPIO_PIN_RESET);
					currentState = STATE_GREEN;
					stateStartTime = HAL_GetTick();
					BuzzerActivation(currentState); // call buzzer activation subroutine
				}
				break;

			case STATE_GREEN:
				// activate green LED and hold indefinitely, until pushbutton is ever released
				HAL_GPIO_WritePin(GREEN_TRANSISTOR_GPIO_Port, GREEN_TRANSISTOR_Pin, GPIO_PIN_SET);
				break;

			case STATE_IDLE: // unreachable at runtime
				break;

		} // switch(currentState)

		// restrict buzzer length to 1 second
		if(buzzerActive==1){
			// check if 1 second has elapsed, and if so turn off buzzer and update state
			if(HAL_GetTick() - buzzerStartTime >= 1000){
				HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
				buzzerActive = 0;
			}
		} // if(buzzerActive)
	} // if(currentState>STATE_IDLE)

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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
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

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
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
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, GREEN_TRANSISTOR_Pin|RED_TRANSISTOR_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(YELLOW_TRANSISTOR_GPIO_Port, YELLOW_TRANSISTOR_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : RMII_MDC_Pin RMII_RXD0_Pin RMII_RXD1_Pin */
  GPIO_InitStruct.Pin = RMII_MDC_Pin|RMII_RXD0_Pin|RMII_RXD1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : RMII_REF_CLK_Pin RMII_MDIO_Pin RMII_CRS_DV_Pin */
  GPIO_InitStruct.Pin = RMII_REF_CLK_Pin|RMII_MDIO_Pin|RMII_CRS_DV_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LD1_Pin LD3_Pin LD2_Pin */
  GPIO_InitStruct.Pin = LD1_Pin|LD3_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PUSHBUTTON_Pin */
  GPIO_InitStruct.Pin = PUSHBUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(PUSHBUTTON_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : GREEN_TRANSISTOR_Pin RED_TRANSISTOR_Pin */
  GPIO_InitStruct.Pin = GREEN_TRANSISTOR_Pin|RED_TRANSISTOR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pin : YELLOW_TRANSISTOR_Pin */
  GPIO_InitStruct.Pin = YELLOW_TRANSISTOR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(YELLOW_TRANSISTOR_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : RMII_TXD1_Pin */
  GPIO_InitStruct.Pin = RMII_TXD1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(RMII_TXD1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : STLK_RX_Pin STLK_TX_Pin */
  GPIO_InitStruct.Pin = STLK_RX_Pin|STLK_TX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USB_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_OverCurrent_Pin */
  GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : USB_SOF_Pin USB_ID_Pin USB_DM_Pin USB_DP_Pin */
  GPIO_InitStruct.Pin = USB_SOF_Pin|USB_ID_Pin|USB_DM_Pin|USB_DP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_VBUS_Pin */
  GPIO_InitStruct.Pin = USB_VBUS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_VBUS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : RMII_TX_EN_Pin RMII_TXD0_Pin */
  GPIO_InitStruct.Pin = RMII_TX_EN_Pin|RMII_TXD0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/***********************************************************************
 * @brief		Configures PWM frequency and starts piezobuzzer output.
 *
 * @details		Calculates timer autoreload (ARR) value based on current
 * 				FSM light state, updates TIM1 Channel 1 ARR and CCR registers for
 * 				a 50% duty cycle, and activates PWM generation.
 *
 * @param[in]	currentState light state used to compute target pitch frequency.
 *
 * @retval		None
 *
 * @note		- Called from both HAL_GPIO_EXTI_Callback (ISR) and the main loop (state
 * 				  transition logic). Must remain non-blocking and interrupt-safe - no
 * 				  HAL_Delay() or heavy computation. Relies on main loop execution to
 * 				  evaluate buzzerStartTime against target duration.
 * 				- Hardware touched: TIM1 CH1 (PWM output, MOE enabled)
 * 				- Globals modified: buzzerStartTime (timestamp), buzzerActive (set to 1)
 ***********************************************************************/
void BuzzerActivation(LightState currentState){

	// calculate frequency pitch for buzzer depending on currentState
	// base period of 250 ticks + 250*state
	uint32_t buzzerFreq = (16000000/(250+(250*(uint32_t)currentState)))-1;
	__HAL_TIM_SET_AUTORELOAD(&htim1, buzzerFreq);

	// duty cycle config
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, (buzzerFreq/2));

	// turn buzzer on
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	__HAL_TIM_MOE_ENABLE(&htim1);

	buzzerStartTime = HAL_GetTick();
	buzzerActive = 1;

}	// BuzzerActivation

/***********************************************************************
 * @brief		Interrupt based on state of external breadboard pushbutton state.
 *
 * @details		Evaluates pushbutton state transitions (press/release). If released,
 * 				resets all LED outputs, turns off the piezobuzzer, and forces the FSM
 * 				back to STATE_IDLE. If pressed while in STATE_IDLE, advances the FSM
 * 				to STATE_RED, records stateStartTime, and triggers the buzzer
 * 				pitch sequence.
 *
 * @param[in]	GPIO_Pin Pin mask identifying which EXTI line triggered the input.
 *
 * @retval		None
 *
 * @note		- Execution: ISR (Interrupt Service Routine) context via EXTI line.
 * 				- Keep Execution Short: Avoid adding blocking delays (e.g. HAL_Delay())
 * 				  or heavy computations within this handler.
 * 				- Hardware modified: PUSHBUTTON (PF13), RED_TRANSISTOR (PF15),
 * 				  YELLOW_TRANSISTOR (PE13), GREEN_TRANSISTOR (PF14), TIM1 CH1 PWM (PE9)
 * 				- Relies on main loop execution to evaluate buzzerStartTime against
 * 				  target duration.
 * 				- Globals modified: currentState, stateStartTime,
 * 				  buzzerStartTime (BuzzerActivation()), buzzerActive (BuzzerActivation())
 ***********************************************************************/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){

	// check if the pin matches the pushbutton pin
	if(GPIO_Pin == PUSHBUTTON_Pin){

		// if pushbutton is NOT pressed
		if(HAL_GPIO_ReadPin(PUSHBUTTON_GPIO_Port, PUSHBUTTON_Pin)==GPIO_PIN_SET){
			HAL_GPIO_WritePin(RED_TRANSISTOR_GPIO_Port, RED_TRANSISTOR_Pin, GPIO_PIN_RESET); 		// reset red LED
			HAL_GPIO_WritePin(YELLOW_TRANSISTOR_GPIO_Port, YELLOW_TRANSISTOR_Pin, GPIO_PIN_RESET);	// reset yellow LED
			HAL_GPIO_WritePin(GREEN_TRANSISTOR_GPIO_Port, GREEN_TRANSISTOR_Pin, GPIO_PIN_RESET);	// reset green LED
			HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1); 	// turn buzzer off
			currentState = STATE_IDLE;					// reset state
		}
		// otherwise pushbutton IS pressed, and advance to first state
		else{
			if(currentState == STATE_IDLE){
				currentState = STATE_RED;		// advance the state identifier to RED
				stateStartTime = HAL_GetTick(); // get current start time for main function while(1) loop
				BuzzerActivation(currentState); // call buzzer activation subroutine
			}
		}
	}
}	// HAL_GPIO_EXTI_Callback
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
