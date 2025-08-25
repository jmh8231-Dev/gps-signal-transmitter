/* USER CODE BEGIN Header */
/**
 *
 * GPS데이터 송신기
 * 21.12.12 업데이트 내용: 주변 밝기에 따라 LED 제어 구현
 * 24.05.05 업데이트 내용: 함수 최적화 및 USB 연결 버그 수정
 * 24.06.06 업데이트 내용: GPS 통신 프로토콜 변경(UBX) 및 함수 최적화
 * 					    디스플레이 정보중 연결된 위성 수 삭제 -> 연결된 RF Node 수로 변경(기능구현 X)
 * 24.06.07 업데이트 내용: 시간정보 표시 알고리즘 수정 + LCD제어 함수 최적화
 * 						윤년계산 및 날짜 계산 알고리즘 수정 + 보정시간 변경(매 시간마다)
 * 						ATmega328p 리셋 알고리즘 수정(일정 횟수 이상 리셋을 진행한 경우 더이상 리셋하지 않고 오류메세지 출력)
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
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "iwdg.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "CLCD.h"
#include "GPS.h"
#include "DS3231.h"
#include "GPS_Data_Repeater.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
int _write(int file, char *ptr, int len){
	HAL_GPIO_WritePin(USB_EN_GPIO_Port, USB_EN_Pin, GPIO_PIN_SET);
    CDC_Transmit_FS(ptr, len);
    return (len);
}

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

uint8_t rx_uart1_data;
extern uint8_t m6n_rx_buf[TIMEUTC_LEN];
extern uint8_t m6n_rx_cplt_flag;

extern uint8_t rx_uart3_Data[20] = {0, };

bool TIM2_flag = false;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_NVIC_Init(void);
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
  MX_I2C1_Init();
  MX_ADC1_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  MX_USB_DEVICE_Init();
  MX_TIM2_Init();
  MX_TIM1_Init();
  MX_IWDG_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */

  //LCD 초기화
  HAL_TIM_Base_Start(&htim1);
  lcd_Init(20, 4);

  //GPS Module 초기화
  M6N_Init();
  HAL_UART_Receive_IT(&huart1, &rx_uart1_data, 1);

  //RTC 초기화
  DS3231_Init(&hi2c1);

  //ATmega328p 초기화
  ATmega328p_Reset();
  HAL_UART_Receive_DMA(&huart3, rx_uart3_Data, 12);

  //500ms TIM 인터럽트 시작
  HAL_TIM_Base_Start_IT(&htim2);

  //ADC변환 시작
  uint16_t ADCVAL[2] = {0, };
  HAL_ADC_Start_DMA(&hadc1, &ADCVAL[0], 2);

  //모든 장치 초기화를 마치고 WDT가동
  HAL_IWDG_Init(&hiwdg);

  bool led_flag;

  _RTC rtc;
  TIME time;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  HAL_IWDG_Refresh(&hiwdg);
	  Time_Calibration(&time, m6n_rx_buf, &rtc);
	  if(TIM2_flag)
	  {
		  send_Data(&time);
		  TIM2_flag = false;
		  Display(&time, &ADCVAL);
		  led_flag = SubMCU_Check();
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC|RCC_PERIPHCLK_USB;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* USART1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);
  /* TIM2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(TIM2_IRQn);
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	static unsigned char cnt = false;
	if(huart->Instance == USART1)
	{
		switch(cnt)
		{
		case 0:
			if(rx_uart1_data == 0xb5)
			{
				m6n_rx_buf[cnt] = rx_uart1_data;
				cnt++;
			}
			break;
		case 1:
			if(rx_uart1_data == 0x62)
			{
				m6n_rx_buf[cnt] = rx_uart1_data;
				cnt++;
			}
			else
				cnt = 0;
			break;
		case 27:
			m6n_rx_buf[cnt] = rx_uart1_data;
			cnt = 0;
			m6n_rx_cplt_flag = true;
			break;
		default:
			m6n_rx_buf[cnt] = rx_uart1_data;
			cnt++;
			break;
		}
		HAL_UART_Receive_IT(&huart1, &rx_uart1_data, 1);
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM2)
	{
		TIM2_flag = true;
		HAL_GPIO_TogglePin(LD_Debug_GPIO_Port, LD_Debug_Pin);
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
