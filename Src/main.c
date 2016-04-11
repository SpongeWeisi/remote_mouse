/**
  ******************************************************************************
  * @file    USB_Device/HID_Standalone/Src/main.c
  * @author  MCD Application Team
  * @version V1.4.3
  * @date    29-January-2016 
  * @brief   USB device HID demo main file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"  

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
USBD_HandleTypeDef USBD_Device;
extern PCD_HandleTypeDef hpcd;

/* UART handler declaration */
UART_HandleTypeDef UartHandle;
uint8_t UartData = 0;
uint8_t UartBuffer[50] = {0};
uint8_t UartDataUnpackState = 0;
uint8_t CheckSum = 0;
uint8_t MouseInfoFlag = 0;

extern uint8_t HID_Buffer[4];

/* Private function prototypes -----------------------------------------------*/
static void Error_Handler(void);
void SystemClock_Config(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  /* STM32F4xx HAL library initialization:
       - Configure the Flash prefetch, instruction and Data caches
       - Configure the Systick to generate an interrupt each 1 msec
       - Set NVIC Group Priority to 4
       - Global MSP (MCU Support Package) initialization
     */
  HAL_Init();
 
  /* Configure the system clock to 168 MHz */
  SystemClock_Config();
  
  /* Configure LED1, LED2, LED3 and LED4 */
  BSP_LED_Init(LED1);
  BSP_LED_Init(LED2);
  BSP_LED_Init(LED3);
  BSP_LED_Init(LED4);
	
	/*##-1- Configure the UART peripheral ######################################*/
  /* Put the USART peripheral in the Asynchronous mode (UART Mode) */
  /* UART1 configured as follow:
      - Word Length = 8 Bits
      - Stop Bit = One Stop bit
      - Parity = None
      - BaudRate = 9600 baud
      - Hardware flow control disabled (RTS and CTS signals) */
  UartHandle.Instance          = USARTx;
  
  UartHandle.Init.BaudRate     = 115200;
  UartHandle.Init.WordLength   = UART_WORDLENGTH_8B;
  UartHandle.Init.StopBits     = UART_STOPBITS_1;
  UartHandle.Init.Parity       = UART_PARITY_NONE;
  UartHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  UartHandle.Init.Mode         = UART_MODE_TX_RX;
  UartHandle.Init.OverSampling = UART_OVERSAMPLING_16;
    
  if(HAL_UART_Init(&UartHandle) != HAL_OK)
  {
    Error_Handler();
  }
  
  /* Initialize Joystick */
  //BSP_JOY_Init(JOY_MODE_GPIO);  
  
  /* Configure Key Button, used for remote wakeup */
  //BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);
  
  /* Init Device Library */
  USBD_Init(&USBD_Device, &HID_Desc, 0);
  
  /* Add Supported Class */
  USBD_RegisterClass(&USBD_Device, USBD_HID_CLASS);
  
  /* Start Device Process */
  USBD_Start(&USBD_Device);

  /* Run Application (Interrupt mode) */
  while (1)
  {
		if(HAL_UART_Receive(&UartHandle, &UartData, 1, 0xffffffff)== HAL_OK)
		{
			static uint32_t counter = 0;
			UartBuffer[counter] = UartData;
			if(counter >= 50)
				counter = 0;
			counter++;
			
			switch(UartDataUnpackState)
			{
				case 0:
					if(UartData == 'm')	
						UartDataUnpackState = 1;
					else 
						UartDataUnpackState = 0;
					break;
				case 1:
					if(UartData == 'o')	UartDataUnpackState = 2;
					else UartDataUnpackState = 0;
					break;
				case 2:
					if(UartData == 'u')	UartDataUnpackState = 3;
					else UartDataUnpackState = 0;
					break;
				case 3:
					if(UartData == 's')	UartDataUnpackState = 4;
					else UartDataUnpackState = 0;
					break;
				case 4:
					if(UartData == 'e')	UartDataUnpackState = 5;
					else UartDataUnpackState = 0;
					break;
				case 5:
					CheckSum = 0;
					CheckSum += UartData;
					HID_Buffer[0] = UartData;
					UartDataUnpackState = 6;
					break;
				case 6:
					CheckSum += UartData;
					HID_Buffer[1] = UartData;
					UartDataUnpackState = 7;
					break;
				case 7:
					CheckSum += UartData;
					HID_Buffer[2] = UartData;
					UartDataUnpackState = 8;
					break;
				case 8:
					CheckSum += UartData;
					HID_Buffer[3] = UartData;
					UartDataUnpackState = 9;
					break;
				case 9:
					if(CheckSum == UartData)
					{
						MouseInfoFlag = 1;
						Toggle_Leds();
					}
					UartDataUnpackState = 0;
					break;
				default:
					break;
			}//switch
		}//if(uart...)
  }//while(1)
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 168000000
  *            HCLK(Hz)                       = 168000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 8
  *            PLL_N                          = 336
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();
  
  /* The voltage scaling allows optimizing the power consumption when the device is 
     clocked below the maximum system frequency, to update the voltage scaling value 
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }

  /* STM32F405x/407x/415x/417x Revision Z devices: prefetch is supported  */
  if (HAL_GetREVID() == 0x1001)
  {
    /* Enable the Flash prefetch */
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
  }
}
/**
  * @brief  Toggles LEDs to show user input state.
  * @param  None
  * @retval None
  */
void Toggle_Leds(void)
{
    BSP_LED_Toggle(LED1);
    BSP_LED_Toggle(LED2);
    BSP_LED_Toggle(LED3);
    BSP_LED_Toggle(LED4); 
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  /* Turn LED5 on */
  BSP_LED_On(LED2);
  while(1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
