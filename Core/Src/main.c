/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LED0  &htim1, TIM_CHANNEL_1
#define LED1  &htim1, TIM_CHANNEL_2
#define LED2  &htim1, TIM_CHANNEL_3
#define LED3  &htim1, TIM_CHANNEL_4
#define LED4  &htim2, TIM_CHANNEL_1
#define DEBUG
#define TAG "main"

#ifdef DEBUG
#define printf_t(format, ...) printf( format "\r\n", ##__VA_ARGS__)
#define info(format, ...) printf("[\t"TAG"]info:" format "\r\n", ##__VA_ARGS__)
#define debug(format, ...) printf("[\t"TAG"]debug:" format "\r\n", ##__VA_ARGS__)
#define error(format, ...) printf("[\t"TAG"]error:" format "\r\n",##__VA_ARGS__)
#else
#define printf(format, ...)
#define info(format, ...)
#define debug(format, ...)
#define error(format, ...)
#endif
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
int eventOfLED[5]={0, 0, 0, 0, 0};      //0:inAct   1:waitForAct    2.acted 3.failToAct 4.allActed
uint16_t SW0cont = 0;
uint16_t SW1cont = 0;
uint16_t SW2cont = 0;
uint16_t SW3cont = 0;
uint16_t SW4cont = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint32_t Get_RandomNum(){
//    HAL_FLASH_Unlock();
//    uint32_t randomData = *(__IO uint32_t*)0x08030000;
//    info("%lX", randomData);
//    HAL_FLASH_Lock();
//    HAL_FLASH_Lock();
    return HAL_GetTick();
}

void Renew_RandomNum(){
//    HAL_FLASH_Unlock();
//    uint32_t randomData = *(__IO uint32_t*)0x08030000;
//    uint32_t newRandomData = randomData;
//    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, 0x08030000, newRandomData ^ (HAL_GetTick() & 0x12344321));
//    uint32_t NewRandomData = *(__IO uint32_t*)0x08030000;
//    info("Renew! Data == %lX", NewRandomData);
//    info("getData == %lX", *(__IO uint32_t*)0x08030000);
//    HAL_FLASH_Lock();
}


void FSMunit_ChangeEventTo_WaitForAct(int LED) {
    if (eventOfLED[LED] == 0) {               //select the inAct LED, change it to waitForAct
        eventOfLED[LED] = 1;
        info("eventOfLED[%d} is changed from 0 to 1!", LED);
        if (eventOfLED[0] == 2
            && eventOfLED[1] == 2
            && eventOfLED[2] == 2
            && eventOfLED[3] == 2
            && eventOfLED[4] == 2) {                                //all LEDs are targetEvent
            for (int i = 0; i < 5; ++i) {
                eventOfLED[i] = 4;
            }
            info("ALL LED ARE ACTED!!!");
        }
    } else if (eventOfLED[LED] != 4) {
        info("the randomNum is not in [0, 4], let it ++ and retry!");
        FSMunit_ChangeEventTo_WaitForAct((LED + 1) % 5);    //selected the acted LED, change the LED number
    }
}

void FSM_ChangeEventTo_WaitForAct() {
    FSMunit_ChangeEventTo_WaitForAct(HAL_GetTick() % 5);
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
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_TIM5_Init();
  /* USER CODE BEGIN 2 */
    info("STM32, Start!");
//    FLASH_EraseInitTypeDef EraseInitStruct;
//    EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
//    EraseInitStruct.Sector = 5;
//    EraseInitStruct.NbSectors = 1;
//    HAL_FLASHEx_Erase(&EraseInitStruct, 0x0);
    HAL_TIM_PWM_Start(LED0);           //LED PWM Start! !!!you should shut off after test.
    HAL_TIM_PWM_Start(LED1);
    HAL_TIM_PWM_Start(LED2);
    HAL_TIM_PWM_Start(LED3);
    HAL_TIM_PWM_Start(LED4);

    HAL_TIM_Base_Start_IT(&htim5);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1) {
    /* USER CODE END WHILE */
//        info("random number is %lX", Get_RandomNum());
        HAL_Delay(500);
//Renew_RandomNum();
//        Get_RandomNum();
//        Get_RandomNum();
        FSM_ChangeEventTo_WaitForAct();
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 84;
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

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == htim3.Instance) {                             //5ms timer to smooth the SW
        if (HAL_GPIO_ReadPin(SW0_GPIO_Port, SW0_Pin) == SET) {
            SW0cont++;
            info("SW0 Pressed! cont == %d", SW0cont);
        } else if (HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin) == SET) {
            SW1cont++;
            info("SW1 Pressed! cont == %d", SW1cont);
        } else if (HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin) == SET) {
            SW2cont++;
            info("SW2 Pressed! cont == %d", SW2cont);
        } else if (HAL_GPIO_ReadPin(SW3_GPIO_Port, SW3_Pin) == SET) {
            SW3cont++;
            info("SW3 Pressed! cont == %d", SW3cont);
        } else if (HAL_GPIO_ReadPin(SW4_GPIO_Port, SW4_Pin) == SET) {
            SW4cont++;
            info("SW4 Pressed! cont == %d", SW4cont);
        }
        Renew_RandomNum();
        HAL_TIM_Base_Stop(&htim3);
    }

    if (htim->Instance == htim5.Instance) {                             //0.5s timer
        if (eventOfLED[0] == 0 && eventOfLED[1] == 0 && eventOfLED[2] == 0 && eventOfLED[3] == 0 && eventOfLED[4] == 0) {                                         //all LEDs are inactivated
            static int inActCont;
            inActCont++;
            if (inActCont == 7) {                                  //3s
                info("random number is %lX", Get_RandomNum());
                if (Get_RandomNum() == 0) error("random number is 0!!!");
                FSM_ChangeEventTo_WaitForAct();
                inActCont = 0;
            }
        }
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == SW0_Pin) {                   //SW0
        HAL_TIM_Base_Start_IT(&htim3);
        __HAL_GPIO_EXTI_CLEAR_FLAG(GPIO_PIN_0);
    }
    if (GPIO_Pin == SW1_Pin) {                   //SW1
        HAL_TIM_Base_Start_IT(&htim3);
    }
    if (GPIO_Pin == SW2_Pin) {                   //SW2
        HAL_TIM_Base_Start_IT(&htim3);
    }
    if (GPIO_Pin == SW3_Pin) {                   //SW3
        HAL_TIM_Base_Start_IT(&htim3);
    }
    if (GPIO_Pin == SW4_Pin) {                   //SW4
        HAL_TIM_Base_Start_IT(&htim3);
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
    while (1) {
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
