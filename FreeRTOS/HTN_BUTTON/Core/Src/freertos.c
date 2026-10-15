/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os2.h"
#include "tim.h"
#include <stdint.h>

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
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for ButtonTask */
osThreadId_t ButtonTaskHandle;
const osThreadAttr_t ButtonTask_attributes = {
  .name = "ButtonTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for LED1Task */
osThreadId_t LED1TaskHandle;
const osThreadAttr_t LED1Task_attributes = {
  .name = "LED1Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for LED2Task */
osThreadId_t LED2TaskHandle;
const osThreadAttr_t LED2Task_attributes = {
  .name = "LED2Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for LED3Task */
osThreadId_t LED3TaskHandle;
const osThreadAttr_t LED3Task_attributes = {
  .name = "LED3Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for LED1Queue */
osMessageQueueId_t LED1QueueHandle;
const osMessageQueueAttr_t LED1Queue_attributes = {
  .name = "LED1Queue"
};
/* Definitions for LED2Queue */
osMessageQueueId_t LED2QueueHandle;
const osMessageQueueAttr_t LED2Queue_attributes = {
  .name = "LED2Queue"
};
/* Definitions for LED3Queue */
osMessageQueueId_t LED3QueueHandle;
const osMessageQueueAttr_t LED3Queue_attributes = {
  .name = "LED3Queue"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
static void SendBrightness(uint32_t led, uint32_t value);

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartTask02(void *argument);
void StartTask03(void *argument);
void StartTask04(void *argument);
void StartTask05(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of LED1Queue */
  LED1QueueHandle = osMessageQueueNew (1, sizeof(uint32_t), &LED1Queue_attributes);

  /* creation of LED2Queue */
  LED2QueueHandle = osMessageQueueNew (1, sizeof(uint32_t), &LED2Queue_attributes);

  /* creation of LED3Queue */
  LED3QueueHandle = osMessageQueueNew (1, sizeof(uint32_t), &LED3Queue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of ButtonTask */
  ButtonTaskHandle = osThreadNew(StartTask02, NULL, &ButtonTask_attributes);

  /* creation of LED1Task */
  LED1TaskHandle = osThreadNew(StartTask03, NULL, &LED1Task_attributes);

  /* creation of LED2Task */
  LED2TaskHandle = osThreadNew(StartTask04, NULL, &LED2Task_attributes);

  /* creation of LED3Task */
  LED3TaskHandle = osThreadNew(StartTask05, NULL, &LED3Task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTask02 */
/**
* @brief Function implementing the ButtonTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask02 */
void StartTask02(void *argument)
{
	
  /* USER CODE BEGIN StartTask02 */
  /* Infinite loop */
 uint32_t brightness[3] = {0U, 0U, 0U};
uint32_t selectedLed = 0U;

GPIO_PinState prevSelect = GPIO_PIN_SET;
GPIO_PinState prevIncrease = GPIO_PIN_SET;
GPIO_PinState prevDecrease = GPIO_PIN_SET;

for (;;)
{
    GPIO_PinState selectNow =
        HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12);

    GPIO_PinState increaseNow =
        HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13);

    GPIO_PinState decreaseNow =
        HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14);

    /* Nut 1: chuyen LED */
    if ((prevSelect == GPIO_PIN_SET) &&
        (selectNow == GPIO_PIN_RESET))
    {
        osDelay(20U);

        if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12)
            == GPIO_PIN_RESET)
        {
            selectedLed = (selectedLed + 1U) % 3U;
        }
    }

    /* Nut 2: tang do sang 10% */
    if ((prevIncrease == GPIO_PIN_SET) &&
        (increaseNow == GPIO_PIN_RESET))
    {
        osDelay(20U);

        if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13)
            == GPIO_PIN_RESET)
        {
            if (brightness[selectedLed] < 100U)
            {
                brightness[selectedLed] += 10U;

                if (brightness[selectedLed] > 100U)
                {
                    brightness[selectedLed] = 100U;
                }

                SendBrightness(selectedLed,
                               brightness[selectedLed]);
            }
        }
    }

    /* Nut 3: giam do sang 10% */
    if ((prevDecrease == GPIO_PIN_SET) &&
        (decreaseNow == GPIO_PIN_RESET))
    {
        osDelay(20U);

        if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14)
            == GPIO_PIN_RESET)
        {
            if (brightness[selectedLed] >= 10U)
            {
                brightness[selectedLed] -= 10U;
            }
            else
            {
                brightness[selectedLed] = 0U;
            }

            SendBrightness(selectedLed,
                           brightness[selectedLed]);
        }
    }

    prevSelect = selectNow;
    prevIncrease = increaseNow;
    prevDecrease = decreaseNow;

    osDelay(10U);
}
  /* USER CODE END StartTask02 */
}

/* USER CODE BEGIN Header_StartTask03 */
/**
* @brief Function implementing the LED1Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask03 */
void StartTask03(void *argument)
{
  /* USER CODE BEGIN StartTask03 */
  /* Infinite loop */
  uint32_t brightness = 0U;
uint32_t pulse;

if (LED1QueueHandle != NULL)
{
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0U);

    for (;;)
    {
        if (osMessageQueueGet(LED1QueueHandle,
                              &brightness,
                              NULL,
                              osWaitForever) == osOK)
        {
            if (brightness > 100U)
                brightness = 100U;

            pulse = (brightness * 999U) / 100U;

            __HAL_TIM_SET_COMPARE(&htim2,
                                  TIM_CHANNEL_1,
                                  pulse);
        }
    }
}
else
{
    for (;;)
    {
        osDelay(100U);
    }
}
  /* USER CODE END StartTask03 */
}

/* USER CODE BEGIN Header_StartTask04 */
/**
* @brief Function implementing the LED2Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask04 */
void StartTask04(void *argument)
{
  /* USER CODE BEGIN StartTask04 */
  /* Infinite loop */
  uint32_t brightness = 0U;
uint32_t pulse;

if (LED2QueueHandle != NULL)
{
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, 0U);

    for (;;)
    {
        if (osMessageQueueGet(LED2QueueHandle,
                              &brightness,
                              NULL,
                              osWaitForever) == osOK)
        {
            if (brightness > 100U)
                brightness = 100U;

            pulse = (brightness * 999U) / 100U;

            __HAL_TIM_SET_COMPARE(&htim2,
                                  TIM_CHANNEL_2,
                                  pulse);
        }
    }
}
else
{
    for (;;)
    {
        osDelay(100U);
    }
}
  /* USER CODE END StartTask04 */
}

/* USER CODE BEGIN Header_StartTask05 */
/**
* @brief Function implementing the LED3Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask05 */
void StartTask05(void *argument)
{
  /* USER CODE BEGIN StartTask05 */
  /* Infinite loop */
  uint32_t brightness = 0U;
uint32_t pulse;

if (LED3QueueHandle != NULL)
{
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 0U);

    for (;;)
    {
        if (osMessageQueueGet(LED3QueueHandle,
                              &brightness,
                              NULL,
                              osWaitForever) == osOK)
        {
            if (brightness > 100U)
                brightness = 100U;

            pulse = (brightness * 999U) / 100U;

            __HAL_TIM_SET_COMPARE(&htim2,
                                  TIM_CHANNEL_3,
                                  pulse);
        }
    }
}
else
{
    for (;;)
    {
        osDelay(100U);
    }
}

  /* USER CODE END StartTask05 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
static void SendBrightness(uint32_t led, uint32_t value)
{
    osMessageQueueId_t queue;
    uint32_t oldValue;

    switch (led)
    {
        case 0U:
            queue = LED1QueueHandle;
            break;

        case 1U:
            queue = LED2QueueHandle;
            break;

        case 2U:
            queue = LED3QueueHandle;
            break;

        default:
            return;
    }

    if (queue == NULL)
        return;

    /* Gui muc sang moi, khong cho task nut nhan bi block */
    if (osMessageQueuePut(queue, &value, 0U, 0U) != osOK)
    {
        /* Queue day: bo muc cu va gui muc moi nhat */
        (void)osMessageQueueGet(queue,
                                &oldValue,
                                NULL,
                                0U);

        (void)osMessageQueuePut(queue,
                                &value,
                                0U,
                                0U);
    }
}
/* USER CODE END Application */

