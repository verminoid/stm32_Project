/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "RF24.h"
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
I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart1;

PCD_HandleTypeDef hpcd_USB_FS;

/* USER CODE BEGIN PV */
char str[64] = {0};
uint16_t buf = {0};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
static void MX_USB_PCD_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

int _write(int file, char *ptr, int len)
{
 HAL_UART_Transmit(&huart1, (uint8_t *) ptr, len, HAL_MAX_DELAY);
 return len;
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
//{
//	if(GPIO_Pin	== IRQ_Pin)
//	{
//		HAL_UART_Transmit(&huart1, (uint8_t*)"IRQ\n", strlen("IRQ\n"), 1000);
//	}
//}
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
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_USB_PCD_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  DWT_Init(); // счётчик для микросекундных пауз

  //const uint64_t pipe0 = 0x787878787878;
  const uint64_t pipe1 = 0xE8E8F0F0E2LL; // адрес первой трубы
  //const uint64_t pipe2 = 0xE8E8F0F0A2LL;
  //const uint64_t pipe3 = 0xE8E8F0F0D1LL;
  //const uint64_t pipe4 = 0xE8E8F0F0C3LL;
  //const uint64_t pipe5 = 0xE8E8F0F0E7LL;
  HAL_Delay(100);
  uint8_t res = isChipConnected(); // проверяет подключён ли модуль к SPI

  char str[64] = {0,};
  snprintf(str, 64, "Connected: %s\n", 1 ? "OK" : "NOT OK");
  HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 1000);

  res = NRF_Init(); // инициализация

  snprintf(str, 64, "Init: %s\n", res > 0 && res < 255 ? "OK" : "NOT OK");
  HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 1000);

  ////////////// SET ////////////////
  //enableAckPayload();
  //setAutoAck(false);
  //setPayloadSize(3);
  setChannel(19);
  openReadingPipe(1, pipe1);
  startListening();
  ///////////////////////////////////

  ////////////////////////// Вывод всяких статусов, для работы не нужно /////////////////////////////
  uint8_t status = get_status();
  snprintf(str, 64, "get_status: 0x%02x\n", status);
  HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 1000);

  status = getPALevel();
  snprintf(str, 64, "getPALevel: 0x%02x  ", status);
  HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 1000);

  if(status == 0x00)
  {
	  HAL_UART_Transmit(&huart1, (uint8_t*)"RF24_PA_MIN\n", strlen("RF24_PA_MIN\n"), 1000);
  }
  else if(status == 0x01)
  {
	  HAL_UART_Transmit(&huart1, (uint8_t*)"RF24_PA_LOW\n", strlen("RF24_PA_LOW\n"), 1000);
  }
  else if(status == 0x02)
  {
	  HAL_UART_Transmit(&huart1, (uint8_t*)"RF24_PA_HIGH\n", strlen("RF24_PA_HIGH\n"), 1000);
  }
  else if(status == 0x03)
  {
	  HAL_UART_Transmit(&huart1, (uint8_t*)"RF24_PA_MAX\n", strlen("RF24_PA_MAX\n"), 1000);
  }

  status = getChannel();
  snprintf(str, 64, "getChannel: 0x%02x № %d\n", status, status);
  HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 1000);

  status = getDataRate();
  snprintf(str, 64, "getDataRate: 0x%02x  ", status);
  HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 1000);

  if(status == 0x02)
  {
	  HAL_UART_Transmit(&huart1, (uint8_t*)"RF24_250KBPS\n", strlen("RF24_250KBPS\n"), 1000);
  }
  else if(status == 0x01)
  {
	  HAL_UART_Transmit(&huart1, (uint8_t*)"RF24_2MBPS\n", strlen("RF24_2MBPS\n"), 1000);
  }
  else
  {
	  HAL_UART_Transmit(&huart1, (uint8_t*)"RF24_1MBPS\n", strlen("RF24_1MBPS\n"), 1000);
  }

  status = getPayloadSize();
  snprintf(str, 64, "getPayloadSize: %d\n", status);
  HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 1000);

  status = getCRCLength();
  snprintf(str, 64, "getCRCLength: 0x%02x  ", status);
  HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 1000);

  if(status == 0x00)
  {
	  HAL_UART_Transmit(&huart1, (uint8_t*)"RF24_CRC_DISABLED\n", strlen("RF24_CRC_DISABLED\n"), 1000);
  }
  else if(status == 0x01)
  {
	  HAL_UART_Transmit(&huart1, (uint8_t*)"RF24_CRC_8\n", strlen("RF24_CRC_8\n"), 1000);
  }
  else if(status == 0x02)
  {
	  HAL_UART_Transmit(&huart1, (uint8_t*)"RF24_CRC_16\n", strlen("RF24_CRC_16\n"), 1000);
  }
  /////////////////////////////////////////////////////////////////////////////////////////////////////


  maskIRQ(true, true, true); // маскируем прерывания
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	///////////////////////////////////// ПР�?ЁМ /////////////////////////////////////////////
	uint8_t nrf_data[32] = {0,}; // буфер указываем максимального размера
	//static uint8_t remsg = 0;
	uint8_t pipe_num = 0;

	if(available(&pipe_num)) // проверяем пришло ли что-то
	{
		//remsg++;

		//writeAckPayload(pipe_num, &remsg, sizeof(remsg)); // отправляем полезную нагрузку вместе с подтверждением

		if(pipe_num == 0) // проверяем в какую трубу пришли данные
		{
			HAL_UART_Transmit(&huart1, (uint8_t*)"pipe 0\n", strlen("pipe 0\n"), 1000);
		}

		else if(pipe_num == 1)
		{
			HAL_UART_Transmit(&huart1, (uint8_t*)"pipe 1\n", strlen("pipe 1\n"), 1000);

			uint8_t count = getDynamicPayloadSize(); // смотрим сколько байт прилетело

			read(&nrf_data, count); // Читаем данные в массив nrf_data и указываем сколько байт читать

			for (uint8_t i = 0; i < count; i++)
			{
				snprintf(str, 64, "data[%d]=%d \n", i, nrf_data[i]);
				HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 1000);
			}
			/*if(nrf_data[0] == 77 && nrf_data[1] == 86 && nrf_data[2] == 97) // проверяем правильность данных
			{
			//	HAL_GPIO_TogglePin(ledpb11_GPIO_Port, ledpb11_Pin);
				snprintf(str, 64, "data[0]=%d data[1]=%d data[2]=%d\n", nrf_data[0], nrf_data[1], nrf_data[2]);
				HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), 1000);
			}*/
		}

		else if(pipe_num == 2)
		{
			HAL_UART_Transmit(&huart1, (uint8_t*)"pipe 2\n", strlen("pipe 2\n"), 1000);
		}

		else
		{
			while(availableMy()) // если данные придут от неуказанной трубы, то попадут сюда
			{
				read(&nrf_data, sizeof(nrf_data));
				HAL_UART_Transmit(&huart1, (uint8_t*)"Unknown pipe\n", strlen("Unknown pipe\n"), 1000);
			}
		}
	}
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB|RCC_PERIPHCLK_USART1
                              |RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  PeriphClkInit.USBClockSelection = RCC_USBCLKSOURCE_PLL;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x2000090E;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 38400;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USB Initialization Function
  * @param None
  * @retval None
  */
static void MX_USB_PCD_Init(void)
{

  /* USER CODE BEGIN USB_Init 0 */

  /* USER CODE END USB_Init 0 */

  /* USER CODE BEGIN USB_Init 1 */

  /* USER CODE END USB_Init 1 */
  hpcd_USB_FS.Instance = USB;
  hpcd_USB_FS.Init.dev_endpoints = 8;
  hpcd_USB_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_FS.Init.battery_charging_enable = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_FS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_Init 2 */

  /* USER CODE END USB_Init 2 */

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
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, CS_I2C_SPI_Pin|LD4_Pin|LD3_Pin|LD5_Pin
                          |LD7_Pin|LD9_Pin|LD10_Pin|LD8_Pin
                          |LD6_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, CE_Pin|CSN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : DRDY_Pin MEMS_INT3_Pin MEMS_INT4_Pin MEMS_INT1_Pin
                           MEMS_INT2_Pin */
  GPIO_InitStruct.Pin = DRDY_Pin|MEMS_INT3_Pin|MEMS_INT4_Pin|MEMS_INT1_Pin
                          |MEMS_INT2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : CS_I2C_SPI_Pin LD4_Pin LD3_Pin LD5_Pin
                           LD7_Pin LD9_Pin LD10_Pin LD8_Pin
                           LD6_Pin */
  GPIO_InitStruct.Pin = CS_I2C_SPI_Pin|LD4_Pin|LD3_Pin|LD5_Pin
                          |LD7_Pin|LD9_Pin|LD10_Pin|LD8_Pin
                          |LD6_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : CE_Pin CSN_Pin */
  GPIO_InitStruct.Pin = CE_Pin|CSN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
