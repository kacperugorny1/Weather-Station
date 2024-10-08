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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "bmp280.h"
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define code "8Tar"
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

IWDG_HandleTypeDef hiwdg;

RTC_HandleTypeDef hrtc;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_RTC_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_IWDG_Init(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//SIM800l
uint8_t Rx_bit;
char Rx_placeholder[10];
char Rx_data[100] = "";
char Locs[50] = "";
char Num[50] = "";
int pos = 0;
int msg = 0;
uint32_t tm = 0;
bool completed = false;
bool read = false;

//BMP

BMP280_HandleTypedef bmp280;

float altitude ,pressure, p, temperature, humidity;
//AHT20
float temp;
float humi;
//commands
char Tx_data[15][200] = {"AT\r",
    "AT+SAPBR=3,1,Contype,\"GPRS\"\r",
    "AT+SAPBR=3,1,APN,\"internet\"\r",
    "AT+SAPBR=1,1\r",
    "AT+CLBS=1,1\r",
    "AT+CNUM\r",
    "AT+HTTPINIT\r",
    "AT+HTTPPARA=CID,\"1\"\r",
    "AT+HTTPSSL=0\r",
    "AT+HTTPPARA=URL,\"http://185.201.114.232:5000/NewData?num=123456789\"\r",
    "AT+HTTPACTION=1\r",
    "AT+HTTPTERM\r",
    "AT+SAPBR=0,1\r"
};
int __io_putchar(int ch)
{
  if (ch == '\n') {
    uint8_t ch2 = '\r';
    HAL_UART_Transmit(&huart2, &ch2, 1, HAL_MAX_DELAY);
  }
  HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
  return 1;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  //Sim800l repeats the command of the user so this line skips it
  if(read == false && Rx_bit == '\n') read = true;
  else if(read == true){
    Rx_data[pos++] = Rx_bit;
    if(Rx_bit == '\n'){
      HAL_UART_Transmit(&huart2, (uint8_t *)Rx_data, pos, 1000);
      //Read latitude and longitude
      if(msg == 4 && Rx_data[0] == '+'){
	HAL_UART_Receive(&huart1, (uint8_t*)Rx_placeholder, 6, 1000);
	printf("Komunikacja udana - LOCS: \n");
	strcpy(Locs,Rx_data);
	pos = 0;
      }
      //read number
      else if(msg == 5 && Rx_data[0] == '+'){
	HAL_UART_Receive(&huart1, (uint8_t*)Rx_placeholder, 6, 1000);
	printf("Komunikacja udana - tel_num\n");
	strcpy(Num,Rx_data);
	pos = 0;
      }
      //This line makes program wait for http response
      if(msg == 10 && ((Rx_data[0] == 'O' && Rx_data[1] == 'K') || Rx_data[0] == '\r')) {
	  pos = 0;
      }
      //Here it writes next functions, and sets the post url
      else{
	printf("Komunikacja udana %d: \n",msg);
	pos = 0; read = false; msg++;
	if(msg == 9){
	  float latitude = 0.0, longitude = 0.0;
	  char *numberStart = strstr(Num,"\"+");
	  char *numberEnd = strstr(numberStart + 2,"\"");
	  size_t len = numberEnd - numberStart;
	  char num[len + 1];
	  strncpy(num, numberStart, len);
	  num[len] = '\0';
	  strcpy(num, &num[4]);
	  sscanf(Locs, "+CLBS: 0,%f,%f,", &longitude, &latitude);
	  sprintf(Tx_data[msg], "AT+HTTPPARA=URL,\"http://185.201.114.232:5000/NewData?num=%s&latitude=%.6f&longitude=%.6f&temp=%.2f&humi=%.2f&press=%.2f&code=%s\"\r",num,latitude,longitude,temp-2,humi,pressure,code);
	}
	if(msg < 13)
	HAL_UART_Transmit_IT(&huart1, (uint8_t *)Tx_data[msg], strlen(Tx_data[msg]));
	else completed = true;

      }
    }
  }
  //keep receving data
  HAL_UART_Receive_IT(&huart1, &Rx_bit, 1);
  HAL_IWDG_Refresh(&hiwdg);

}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{

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
  MX_RTC_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  MX_IWDG_Init();
  /* USER CODE BEGIN 2 */

  const uint16_t addr = 0b0111000; //0x38
  const int addr_wr = addr<<1;
  const int addr_rc = addr_wr + 1;
  uint32_t temp_data;
  uint8_t init[3] = {0xbe, 0x08, 0x00};
  uint8_t measure[3] = {0xac, 0x33, 0x00};
  uint8_t data[6];
  uint32_t time_stamp = HAL_GetTick();
  HAL_IWDG_Refresh(&hiwdg);
  HAL_I2C_Master_Receive(&hi2c1, addr_rc, (uint8_t *)data, 6, 1000);
  if((data[0] >> 3 & 1) != 1){
    printf("Kalibracja\n");
    HAL_I2C_Master_Transmit(&hi2c1, addr_wr, (uint8_t *)init, 3, 1000);
  }
  time_stamp = HAL_GetTick();
  while(HAL_GetTick() - time_stamp < 10) ;
  HAL_I2C_Master_Transmit(&hi2c1, addr_wr, (uint8_t *)measure, 3, 1000);
  time_stamp = HAL_GetTick();
  while(HAL_GetTick() - time_stamp < 85) ;
  HAL_I2C_Master_Receive(&hi2c1, addr_rc, (uint8_t *)data, 6, 1000);
  //AHT20 COMM FINISHED
  HAL_IWDG_Refresh(&hiwdg);
  //check control byte and calculate values
  if(((data[0] >> 7) & 1) == 0) {
    printf("Komunikacja udana \n");
    temp_data = ((uint32_t)data[3] << 16) + ((uint32_t)data[4] << 8) + (uint32_t)data[5];
    temp_data = temp_data & (~(0xFFF00000));
    temp = ((float)temp_data/1048576) * 200 - 50;
    temp_data = ((uint32_t)data[1] << 16) + ((uint32_t)data[2] << 8) + (uint32_t)data[3];
    temp_data = temp_data >> 4;
    humi = ((float)temp_data/1048576) * 100;
  }
  else
  {
    printf("Komunikacja nie udana\n");
  }
  printf("Temperatura wynosi %f *C\n", temp);
  printf("Wilgotnosc  wynosi %f %%\n", humi);
  HAL_IWDG_Refresh(&hiwdg);



  bmp280_init_default_params(&bmp280.params);
  bmp280.addr = BMP280_I2C_ADDRESS_1;
  bmp280.i2c = &hi2c1;
  while (!(bmp280_init(&bmp280, &bmp280.params)))
  {
	time_stamp = HAL_GetTick();
	while(HAL_GetTick() - time_stamp < 2000) ;
  }

  HAL_IWDG_Refresh(&hiwdg);
  bool bme280p = bmp280.id == BMP280_CHIP_ID;
  time_stamp = HAL_GetTick();
  while(HAL_GetTick() - time_stamp < 100) ;
  while (!bmp280_read_float(&bmp280, &temperature, &pressure, &humidity)) {
	  time_stamp = HAL_GetTick();
	  while(HAL_GetTick() - time_stamp < 2000) ;
  }

  HAL_IWDG_Refresh(&hiwdg);
  p = pressure/100;
  altitude = 44330.0*(1-pow(p/1013.25, 1/5.255));

  time_stamp = HAL_GetTick();
  while(HAL_GetTick() - time_stamp < 500) ;

  HAL_IWDG_Refresh(&hiwdg);

  HAL_UART_Receive_IT(&huart1, &Rx_bit, 1);
  HAL_UART_Transmit_IT(&huart1, (uint8_t *)Tx_data[msg], strlen(Tx_data[msg]));
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
  while(!completed);

  printf("Going to sleep\n");
  HAL_PWR_EnterSTANDBYMode();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_LSE
                              |RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
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
  hi2c1.Init.Timing = 0x10909CEC;
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
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_32;
  hiwdg.Init.Window = 4000;
  hiwdg.Init.Reload = 4000;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

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

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */
  if (HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 30, RTC_WAKEUPCLOCK_CK_SPRE_16BITS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE END RTC_Init 2 */

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
  huart1.Init.BaudRate = 115200;
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
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

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
