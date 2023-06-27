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
#define SW_EXTENDED_PRESSED_TIME 7

#define LED_ON 0
#define LED_OFF htim1.Init.Period
#define WAIT_TIME   6

#define LED_MAX_PERIOD  htim1.Init.Period

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
int contOfLEDlighting = 0;
int inActCont = 0;
int increasement = 0;
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

void Set_AllLEDBreathing() {
    HAL_TIM_Base_Start_IT(&htim4);
    increasement = -20;
}

void Set_LEDPWMtimer(int LEDnum, int pwm) {
    if (LEDnum >= 0 && LEDnum <= 3)                                            //TIM1 PWM
        __HAL_TIM_SET_COMPARE(&htim1, LEDchannel[LEDnum], pwm);
    else if (LEDnum == 4)                                                     //TIM2 PWM
        __HAL_TIM_SET_COMPARE(&htim2, LEDchannel[LEDnum], pwm);
    else
        error("LEDnum is out range!");
}

void FSMunit_ChangeRandomEventTo_WaitForAct(int LED) {
    HAL_TIM_Base_Start_IT(&htim5);
    if (eventOfLED[LED] == 0) {                                         //select the inAct LED, change it to waitForAct
        eventOfLED[LED] = 1;
        info("eventOfLED[%d} is changed from 0 to 1!", LED);
        waitForAct_LEDnum = LED;                                        //save the LEDnum whose event is "waitForAct"
        HAL_TIM_Base_Start_IT(&htim5);                              //Start lightning!
    } else if (eventOfLED[LED] != 4) {
        info("the randomNum is not in [0, 4], let it ++ and retry!");
        FSMunit_ChangeRandomEventTo_WaitForAct((LED + 1) % 5);      //selected the acted LED, change the LED number
    }
    contOfLEDlighting = 0;
}

void FSM_ChangeRandomEventTo_WaitForAct() {
    FSMunit_ChangeRandomEventTo_WaitForAct(HAL_GetTick() % 5);
}

void FSM_ChangeAllEventTo_FailToAct() {
    for (int i = 0; i < 5; ++i) {
        eventOfLED[i] = 3;                                              //Turn all LEDs into "failToAct"
        Set_LEDPWMtimer(i, LED_OFF);                        //Turn off all LEDs
        info("%d  %d  %d  %d  %d, contOfLEDlighting == %d, inActCont == %d", eventOfLED[0], eventOfLED[1],
             eventOfLED[2], eventOfLED[3], eventOfLED[4], contOfLEDlighting, inActCont);
    }
    waitForAct_LEDnum = -1;                                             //delete the saved LEDnum whose event is "waitForAct"
    inActCont = 0;                                                      //count time(3s)
    increasement = 0;                                                   //Turn off the breathing LEDs
    contOfLEDlighting = 0;
}

void FSM_ChangeEventTo_Acted_AndChangeRandomLEDEventTo_WaitForAct(int actLEDnum) {
    if (eventOfLED[actLEDnum] == 1) {                                   //pressed the switch SWx correctly
        info("You pressed the switch SW%d correctly!", actLEDnum);
        eventOfLED[actLEDnum] = 2;                                      //Turn event of LEDx into "acted"
        Set_LEDPWMtimer(actLEDnum, LED_ON);                 //Turn on LEDx
        waitForAct_LEDnum = -1;                                         //delete the saved LEDnum whose event is "waitForAct"
        if (eventOfLED[0] == 2
            && eventOfLED[1] == 2
            && eventOfLED[2] == 2
            && eventOfLED[3] == 2
            && eventOfLED[4] == 2) {                                    //all LEDs are targetEvent
            for (int i = 0; i < 5; ++i) {
                eventOfLED[i] = 4;
            }
            Set_AllLEDBreathing();
            info("ALL LED ARE ACTED!!!");
        }
        int randomNum = HAL_GetTick() % 5;
        info("Change random LED event to WaitForAct, random num is %d", randomNum);
        FSM_ChangeRandomEventTo_WaitForAct(randomNum);
    } else if (eventOfLED[actLEDnum] != 1) {                            //Wrong SW is pressed,
        error("WRONG SW%d is pressed!", actLEDnum);                     //turn all event of LED into "failToAct"
        FSM_ChangeAllEventTo_FailToAct();
    }
}

void FSM_ChangeAllEventTo_InAct() {
    info("Change all event to \"inAct\"");
    for (int i = 0; i < 5; ++i) {
        eventOfLED[i] = 0;
        Set_LEDPWMtimer(i, LED_OFF);                                        //Turn off all LEDs
        error("%d  %d  %d  %d  %d, contOfLEDlighting == %d, inActCont == %d", eventOfLED[0], eventOfLED[1],
             eventOfLED[2], eventOfLED[3], eventOfLED[4], contOfLEDlighting, inActCont);
    }
    increasement = 0;                                                                    //Turn off the breathing LEDs
    waitForAct_LEDnum = -1;                                                              //Delete the "waitForAct" LEDnum
    HAL_TIM_Base_Start_IT(&htim5);                                                  //3s timer
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
        info("%d  %d  %d  %d  %d, contOfLEDlighting == %d, inActCont == %d", eventOfLED[0], eventOfLED[1],
             eventOfLED[2], eventOfLED[3], eventOfLED[4], contOfLEDlighting, inActCont);
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
        contOfLEDlighting = 0;
        info("contOfLEDlighting is set to 0!, pressedContOfSW[%d] == %d", LEDnum, pressedContOfSW[LEDnum]);
        if (eventOfLED[LEDnum] == 3 && pressedContOfSW[LEDnum] > SW_EXTENDED_PRESSED_TIME) {      //LEDx is "failToAct"
            pressedContOfSW[LEDnum] = 0;
            info("SW%d is pressed extendedly!");
            FSM_ChangeAllEventTo_InAct();
            return;
        }
        pressedContOfSW[LEDnum] = 0;
        if (eventOfLED[LEDnum] == 1)                                                             //LEDx is "waitToAct"
        {
            FSM_ChangeEventTo_Acted_AndChangeRandomLEDEventTo_WaitForAct(LEDnum);
        }
        else if (eventOfLED[LEDnum] == 4) {                                                      //LED is all acted
            return;
        } else {                                                                                   //Wrong SW is pressed
            FSM_ChangeAllEventTo_FailToAct();
            info("0");
        }
    }

}

void LEDlightingUnit(int LEDnum) {
    if (contOfLEDlighting >= WAIT_TIME) {                              //Time is out
        HAL_TIM_Base_Stop(&htim5);
        contOfLEDlighting = 0;
        FSM_ChangeAllEventTo_FailToAct();
        info("1");
    }
    if (contOfLEDlighting % 2 != LED_ON) {
        contOfLEDlighting++;
        Set_LEDPWMtimer(LEDnum, LED_ON);
        info("LED%d turn ON!, contOfLEDlighting == %d", LEDnum, contOfLEDlighting);
    } else {
        contOfLEDlighting++;
        Set_LEDPWMtimer(LEDnum, LED_OFF);
        info("LED%d turn OFF!, contOfLEDlighting == %d", LEDnum, contOfLEDlighting);
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == htim4.Instance) {
        static int periodOfBreathingLED;
        if (periodOfBreathingLED >= LED_MAX_PERIOD || periodOfBreathingLED <= 0) increasement *= -1;     //reverse the increasement's direction
        if (increasement != 0)                                              //if breathing LED should be on
            periodOfBreathingLED += increasement;                           //Refresh the breathing LED's period
            for (int i = 0; i < 5; ++i) {
                Set_LEDPWMtimer(i, periodOfBreathingLED);
            }
    }
    if (htim->Instance == htim3.Instance) {                             //40ms timer to smooth the SW
        TIM3unit(SW0_GPIO_Port, SW0_Pin, 0);
        TIM3unit(SW1_GPIO_Port, SW1_Pin, 1);
        TIM3unit(SW2_GPIO_Port, SW2_Pin, 2);
        TIM3unit(SW3_GPIO_Port, SW3_Pin, 3);
        TIM3unit(SW4_GPIO_Port, SW4_Pin, 4);

    }

    if (htim->Instance == htim5.Instance) {                             //0.5s timer
        if (eventOfLED[0] == 0 && eventOfLED[1] == 0 && eventOfLED[2] == 0 && eventOfLED[3] == 0 &&
            eventOfLED[4] == 0) {                                         //all LEDs are inactivated
            inActCont++;
            if (inActCont == 7) {                                       //3s
                FSM_ChangeRandomEventTo_WaitForAct();
                inActCont = 0;
                HAL_TIM_Base_Stop(&htim5);
            }
        }
        if (waitForAct_LEDnum != -1) {                //TIM1 PWM LEDs
            LEDlightingUnit(waitForAct_LEDnum);
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
