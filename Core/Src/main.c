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
const uint32_t LEDchannel[5] = {TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3, TIM_CHANNEL_4, TIM_CHANNEL_1};

#define SW_ON  SET
#define SW_OFF RESET
#define SW_EXTENDED_PRESSED_TIME 30

#define LED_ON 0
#define LED_OFF htim1.Init.Period

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
int eventOfLED[5] = {0, 0, 0, 0, 0};      //0:inAct   1:waitForAct    2.acted 3.failToAct 4.allActed
uint16_t pressedContOfSW[5] = {0};
int waitForAct_LEDnum = -1;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint32_t Get_RandomNum() {
//    HAL_FLASH_Unlock();
//    uint32_t randomData = *(__IO uint32_t*)0x08030000;
//    info("%lX", randomData);
//    HAL_FLASH_Lock();
//    HAL_FLASH_Lock();
    return HAL_GetTick();
}

void Renew_RandomNum() {
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
        waitForAct_LEDnum = LED;                                //save the LEDnum whose event is "waitForAct"
    } else if (eventOfLED[LED] != 4) {
        info("the randomNum is not in [0, 4], let it ++ and retry!");
        FSMunit_ChangeEventTo_WaitForAct((LED + 1) % 5);    //selected the acted LED, change the LED number
    }
}

void FSM_ChangeEventTo_WaitForAct() {
    FSMunit_ChangeEventTo_WaitForAct(HAL_GetTick() % 5);
}

void FSM_ChangeEventTo_Acted_AndChangeRandomLEDEventTo_WaitForAct(int actLEDnum) {
    if (eventOfLED[actLEDnum] == 1) {
        info("You pressed the switch SW%d correctly!", actLEDnum);
        eventOfLED[actLEDnum] = 2;
        waitForAct_LEDnum = -1;                                     //delete the saved LEDnum whose event is "waitForAct"
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
        int randomNum = HAL_GetTick() % 5;
        info("Change random LED event to WaitForAct, random num is %d", randomNum);
        FSM_ChangeEventTo_WaitForAct(randomNum);
    } else if (eventOfLED[actLEDnum] != 1) {                           //Wrong SW is pressed,
        error("WRONG SW%d is pressed!", actLEDnum);                 //turn all event of LED into "failToAct"
        for (int i = 0; i < 5; ++i) {
            eventOfLED[i] = 3;
        }
        waitForAct_LEDnum = -1;                                     //delete the saved LEDnum whose event is "waitForAct"
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void) {
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
        HAL_Delay(500);
        info("%d  %d  %d  %d  %d", eventOfLED[0], eventOfLED[1], eventOfLED[2], eventOfLED[3], eventOfLED[4]);
        for (int i = 0; i < 4; ++i) {
            switch (eventOfLED[i]) {
                case 0:
                    __HAL_TIM_SET_COMPARE(&htim1, LEDchannel[i], 1000);
                    break;
                case 1:
                    HAL_TIM_Base_Start_IT(&htim5);
                    break;
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
void SystemClock_Config(void) {
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
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
}

/* USER CODE BEGIN 4 */
void TIM3unit(GPIO_TypeDef *SW_GPIO_Port, uint16_t SW_Pin, int LEDnum) {
    if (HAL_GPIO_ReadPin(SW_GPIO_Port, SW_Pin) == SW_ON) {                  //SWx is under pressed
        pressedContOfSW[LEDnum]++;
        info("SW%d Pressed! pressedContOfSW[%d] == %d", LEDnum, LEDnum, pressedContOfSW[LEDnum]);
    } else if (HAL_GPIO_ReadPin(SW_GPIO_Port, SW_Pin) == SW_OFF && pressedContOfSW[LEDnum] != 0) {  //SWx is releasing
        if (eventOfLED[LEDnum] == 1)                                                             //LEDx is "waitToAct"
            FSM_ChangeEventTo_Acted_AndChangeRandomLEDEventTo_WaitForAct(LEDnum);
        else{                                                                                   //Wrong SW is pressed
            waitForAct_LEDnum = -1;                                     //delete the saved LEDnum whose event is "waitForAct"
            for (int i = 0; i < 5; ++i){
                eventOfLED[i] = 3;                                          //Change all events into "failToAct"
            }
        }
        if (eventOfLED[LEDnum] == 3 && pressedContOfSW[LEDnum] > SW_EXTENDED_PRESSED_TIME) {      //LEDx is "failToAct"
            info("SW%d is pressed extendedly!");
            for (int i = 0; i < 5; ++i) {
                eventOfLED[i] = 0;
            }
            HAL_TIM_Base_Start_IT(&htim5);                                                  //3s timer
        }
        pressedContOfSW[LEDnum] = 0;
    }

}

void LEDlightingUnit(int LEDnum) {
    static int statusOfLED;
    if (statusOfLED == LED_OFF) {
        statusOfLED = LED_ON;
        __HAL_TIM_SET_COMPARE(&htim1, LEDchannel[LEDnum], LED_ON);
        info("LED%d turn ON!", LEDnum);
    } else {
        statusOfLED = LED_OFF;
        __HAL_TIM_SET_COMPARE(&htim1, LEDchannel[LEDnum], LED_OFF);
        info("LED%d turn OFF!", LEDnum);
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == htim3.Instance) {                             //40ms timer to smooth the SW
        TIM3unit(SW0_GPIO_Port, SW0_Pin, 0);
        TIM3unit(SW1_GPIO_Port, SW1_Pin, 1);
        TIM3unit(SW2_GPIO_Port, SW2_Pin, 2);
        TIM3unit(SW3_GPIO_Port, SW3_Pin, 3);
        TIM3unit(SW4_GPIO_Port, SW4_Pin, 4);
    }
//        if (HAL_GPIO_ReadPin(SW0_GPIO_Port, SW0_Pin) == SW_ON) {
//            pressedContOfSW[0]++;
//            info("SW0 Pressed! cont == %d", pressedContOfSW[0]);
////            FSM_ChangeEventTo_Acted_AndChangeRandomLEDEventTo_WaitForAct(0);
//        } else if (HAL_GPIO_ReadPin(SW0_GPIO_Port, SW0_Pin) == SW_OFF && pressedContOfSW[0] != 0) {
//            if (eventOfLED[0] ==
//                1) {                                                                 //LED0 is "waitToAct"
//                FSM_ChangeEventTo_Acted_AndChangeRandomLEDEventTo_WaitForAct(0);
//            }
//            if (eventOfLED[0] == 3 &&
//                pressedContOfSW[0] > SW_EXTENDED_PRESSED_TIME) {                           //LED0 is "failToAct"
//                info("SW0 is pressed extendedly!");
//                for (int i = 0; i < 5; ++i) {
//                    eventOfLED[i] = 0;
//                }
//                HAL_TIM_Base_Start_IT(&htim5);
//            }
//        }
//        if (HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin) == SW_ON) {
//            pressedContOfSW[1]++;
//            info("SW1 Pressed! cont == %d", pressedContOfSW[1]);
////            FSM_ChangeEventTo_Acted_AndChangeRandomLEDEventTo_WaitForAct(1);
//        } else if (HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin) == SW_OFF && pressedContOfSW[1] != 0) {
//            if (eventOfLED[1] ==
//                1) {                                                                 //LED1 is "waitToAct"
//                FSM_ChangeEventTo_Acted_AndChangeRandomLEDEventTo_WaitForAct(1);
//            }
//            if (eventOfLED[1] == 3 &&
//                pressedContOfSW[1] > SW_EXTENDED_PRESSED_TIME) {                           //LED1 is "failToAct"
//                info("SW1 is pressed extendedly!");
//                for (int i = 0; i < 5; ++i) {
//                    eventOfLED[i] = 0;
//                }
//                HAL_TIM_Base_Start_IT(&htim5);
//            }
//        } if (HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin) == SW_ON) {
//            pressedContOfSW[2]++;
//            info("SW2 Pressed! cont == %d", pressedContOfSW[2]);
////            FSM_ChangeEventTo_Acted_AndChangeRandomLEDEventTo_WaitForAct(2);
//        } else if (HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin) == SW_OFF && pressedContOfSW[2] != 0) {
//            if (eventOfLED[2] ==
//                1) {                                                                 //LED2 is "waitToAct"
//                FSM_ChangeEventTo_Acted_AndChangeRandomLEDEventTo_WaitForAct(2);
//            }
//            if (eventOfLED[2] == 3 &&
//                pressedContOfSW[2] > SW_EXTENDED_PRESSED_TIME) {                           //LED2 is "failToAct"
//                info("SW2 is pressed extendedly!");
//                for (int i = 0; i < 5; ++i) {
//                    eventOfLED[i] = 0;
//                }
//                HAL_TIM_Base_Start_IT(&htim5);
//            }
//        } if (HAL_GPIO_ReadPin(SW3_GPIO_Port, SW3_Pin) == SW_ON) {
//            pressedContOfSW[3]++;
//            info("SW3 Pressed! cont == %d", pressedContOfSW[3]);
////            FSM_ChangeEventTo_Acted_AndChangeRandomLEDEventTo_WaitForAct(3);
//        } else if (HAL_GPIO_ReadPin(SW3_GPIO_Port, SW3_Pin) == SW_OFF && pressedContOfSW[3] != 0) {
//            if (eventOfLED[3] == 1) {                                             //LED3 is "waitToAct"
//                FSM_ChangeEventTo_Acted_AndChangeRandomLEDEventTo_WaitForAct(3);
//            }
//            if (eventOfLED[3] == 3 &&
//                pressedContOfSW[3] > SW_EXTENDED_PRESSED_TIME) {                           //LED3 is "failToAct"
//                info("SW3 is pressed extendedly!");
//                for (int i = 0; i < 5; ++i) {
//                    eventOfLED[i] = 0;
//                }
//                HAL_TIM_Base_Start_IT(&htim5);
//            }
//        } if (HAL_GPIO_ReadPin(SW4_GPIO_Port, SW4_Pin) == SW_ON) {
//            pressedContOfSW[4]++;
//            info("SW4 Pressed! cont == %d", pressedContOfSW[4]);
////            FSM_ChangeEventTo_Acted_AndChangeRandomLEDEventTo_WaitForAct(4);
//        } else if (HAL_GPIO_ReadPin(SW4_GPIO_Port, SW4_Pin) == SW_OFF && pressedContOfSW[4] != 0) {
//            if (eventOfLED[4] == 1) {                                                      //LED4 is "waitToAct"
//                FSM_ChangeEventTo_Acted_AndChangeRandomLEDEventTo_WaitForAct(4);
//            }
//            if (eventOfLED[4] == 3 &&
//                pressedContOfSW[4] > SW_EXTENDED_PRESSED_TIME) {                           //LED4 is "failToAct"
//                info("SW4 is pressed extendedly!");
//                for (int i = 0; i < 5; ++i) {
//                    eventOfLED[i] = 0;
//                }
//                HAL_TIM_Base_Start_IT(&htim5);
//            }
//            Renew_RandomNum();
//        }

    if (htim->Instance == htim5.Instance) {                             //0.5s timer
        if (eventOfLED[0] == 0 && eventOfLED[1] == 0 && eventOfLED[2] == 0 && eventOfLED[3] == 0 &&
            eventOfLED[4] == 0) {                                         //all LEDs are inactivated
            static int inActCont;
            inActCont++;
            if (inActCont == 7) {                                       //3s
                info("random number is %lX", Get_RandomNum());
                FSM_ChangeEventTo_WaitForAct();
                inActCont = 0;
                HAL_TIM_Base_Stop(&htim5);
            }
        }
        if (waitForAct_LEDnum != -1 && waitForAct_LEDnum != 4) {          //TIM1 PWM LEDs
            LEDlightingUnit(waitForAct_LEDnum);
        } else if (waitForAct_LEDnum == 4) {
            static int statusOfLED;
            if (statusOfLED == LED_OFF) {
                statusOfLED = LED_ON;
                __HAL_TIM_SET_COMPARE(&htim1, LEDchannel[4], LED_ON);
                info("LED4 turn ON!");
            } else {
                statusOfLED = LED_OFF;
                __HAL_TIM_SET_COMPARE(&htim1, LEDchannel[4], LED_OFF);
                info("LED4 turn OFF!");
            }
        }
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == SW0_Pin) {                   //SW0
        HAL_TIM_Base_Start_IT(&htim3);
        info("SW0");
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
void Error_Handler(void) {
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
