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
#include "fatfs.h"
#include "usb_host.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "usb_update.h"
#include "uart_update.h"
#include "flash_engine.h"
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
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_usart2_rx;

/* USER CODE BEGIN PV */
extern ApplicationTypeDef Appli_state;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);
void MX_USB_HOST_Process(void);

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
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_USB_HOST_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */
  // 1. 하드웨어 초기화
  uartUpdateInit();

  printf("[BOOT] Bootloader Started. Checking for updates...\r\n");

  // ------------------------------------------------------------------
  // 0. 플래그 상태 점검 및 초기화
  //    - PASS/READY가 아니면 'NEW'로 초기화하여 반드시 업데이트 모드 진입
  //    - 필요시 'ING'로 바로 업데이트 모드 진입하도록 선택 구현 가능
  // ------------------------------------------------------------------
  uint32_t current_f = Get_Flag();
  if (current_f != FLAG_PASS && current_f !=  FLAG_READY  )
  {
    printf("[BOOT] Flash FLAG=0x%08lX (not PASS/READY). Set to NEW.\r\n", current_f);
    // Update_Flag(FLAG_ING); // 바로 업데이트 모드 진입 원할 때 사용
    Update_Flag(FLAG_NEW);    // 신규/이상치면 무조건 업데이트 유도
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    uint32_t start_tick = HAL_GetTick();
    bool usb_success = false;
    bool uart_success = false;

    /* ------------------------------------------------------------------ */
    /* STEP 1. USB 인식 대기 (최대 2초)               */
    /* ------------------------------------------------------------------ */
    printf("[BOOT] USB Waiting... (Max 2s)\r\n");
    while ((HAL_GetTick() - start_tick) < 2000)
    {
    /* USER CODE END WHILE */
    MX_USB_HOST_Process();

    /* USER CODE BEGIN 3 */
    if (Appli_state == APPLICATION_READY)
        {
            printf("[USB] File Found! Starting update...\r\n");
            Process_USB_Update(); // USB 업데이트 실행
            
            if (Get_Flag() == FLAG_PASS) {
                usb_success = true;
                break;
            }
            Appli_state = APPLICATION_IDLE;
        }
    }
    if (usb_success) break; // USB 성공 시 루프 탈출 -> 앱으로 점프

    /* ------------------------------------------------------------------ */
    /* STEP 2. UART 강제 업데이트 대기 (FLAG_PASS 일때 ) */
    /* ------------------------------------------------------------------ */

    // 2초 동안 강제 업데이트 트리거 대기
    uint32_t uart_trigger_start = HAL_GetTick();
    while ((HAL_GetTick() - uart_trigger_start) < 2000)
    {
      uartCheckForceUpdateTrigger(); // FLAG_PASS일 때 안드로이드의 0x73(Value 1) 수신 여부 확인 (강제 진입 트리거)
      HAL_Delay(1); // CPU 점유율 방지
    }

    /* ------------------------------------------------------------------ */
    /* STEP 3. UART 업데이트 대기 (NEW일 땐 무한, 아닐 땐 3초) */
    /* ------------------------------------------------------------------ */
    start_tick = HAL_GetTick();
    printf("[BOOT] UART Waiting... (FLAG=0x%08lX)\r\n", Get_Flag());

    while (1)
    {
      // 1. 상태를 먼저 확인 (가장 최신 플래그 값 읽기)
      uint32_t current_f = Get_Flag();

      // 2. 성공 시 즉시 탈출 (불필요한 노크 방지)
      if (current_f == FLAG_PASS) {
        uart_success = true;
        break;
      }

      // 3. PASS가 아닌 경우(NEW, READY, ING)에만 핸드쉐이크/업데이트 진행
      // 이 안에서 250ms 간격으로 노크를 보냅니다.
      uartUpdateProcess(); 

      // [참고] 만약 이 루프가 USB 체크 루프 내부에 있는 것이 아니라면 
      // CPU 점유율을 위해 아주 짧은 지연을 줄 수도 있습니다.
      HAL_Delay(1); 
    }

    if (uart_success) break; // UART 성공 시 루프 탈출 -> 앱으로 점프
  }

  /* ---------------------------------------------------------------------- */
  /* STEP 3. 최종 점프 (모든 업데이트 프로세스 종료 후)       */
  /* ---------------------------------------------------------------------- */
  printf("[BOOT] Update Done! Jumping to App...\r\n");

  // 인터럽트 정리 및 DeInit (점프 전 필수)
  __disable_irq();
  HAL_RCC_DeInit();
  HAL_DeInit();

  if (Is_App_Valid()) {
    Jump_To_Application(APP_START_ADDR);
  }

  while(1); // 점프 실패 시 대기
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_5);
  while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_5)
  {
  }
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
  LL_RCC_HSE_Enable();

   /* Wait till HSE is ready */
  while(LL_RCC_HSE_IsReady() != 1)
  {

  }
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_25, 336, LL_RCC_PLLP_DIV_2);
  LL_RCC_PLL_ConfigDomain_48M(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_25, 336, LL_RCC_PLLQ_DIV_7);
  LL_RCC_PLL_Enable();

   /* Wait till PLL is ready */
  while(LL_RCC_PLL_IsReady() != 1)
  {

  }
  while (LL_PWR_IsActiveFlag_VOS() == 0)
  {
  }
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_4);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_2);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {

  }
  LL_SetSystemCoreClock(168000000);

   /* Update the time base */
  if (HAL_InitTick (TICK_INT_PRIORITY) != HAL_OK)
  {
    Error_Handler();
  }
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
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOE);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOH);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOD);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);

  /**/
  LL_GPIO_ResetOutputPin(GPIOE, BUZZER_Pin|DBG_LED_Pin);

  /**/
  LL_GPIO_ResetOutputPin(USB_PENA_GPIO_Port, USB_PENA_Pin);

  /**/
  GPIO_InitStruct.Pin = BUZZER_Pin|DBG_LED_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = USB_PENA_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(USB_PENA_GPIO_Port, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = USB_PFLT_Pin;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(USB_PFLT_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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
