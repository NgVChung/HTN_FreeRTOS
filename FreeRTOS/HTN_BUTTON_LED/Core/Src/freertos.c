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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "queue.h"
#include "tim.h"
#include <stdint.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct {
    uint8_t brightness;          /* do sang 0..100 (%) */
} LedMsg_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define NUM_LED          3
#define BRIGHT_STEP      10      /* moi lan nhan +/- 10% */
#define BRIGHT_INIT      0       /* do sang ban dau (%): 3 LED tat, bam UP de tang */
#define PWM_PERIOD       1000    /* ARR + 1 -> 1kHz */
#define BTN_POLL_MS      20      /* chu ky quet nut */
#define DEBOUNCE_COUNT   3       /* 3 x 20ms = 60ms on dinh */

/* ===== CAU HINH CHAN NUT NHAN - SUA CHO DUNG VOI MACH CUA BAN ===== */
#define BTN_SEL_PORT     GPIOB
#define BTN_SEL_PIN      GPIO_PIN_12   /* nut chuyen LED */
#define BTN_UP_PORT      GPIOB
#define BTN_UP_PIN       GPIO_PIN_13   /* nut tang do sang */
#define BTN_DN_PORT      GPIOB
#define BTN_DN_PIN       GPIO_PIN_14   /* nut giam do sang */

/* Muc logic khi nhan nut:
 *   GPIO_PIN_RESET : nut noi chan -> GND (dung pull-up)   <- mac dinh
 *   GPIO_PIN_SET   : nut noi chan -> 3.3V (dung pull-down) */
#define BTN_PRESSED_STATE  GPIO_PIN_RESET
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
static QueueHandle_t ledQueue[NUM_LED];
static const uint32_t pwmChannel[NUM_LED] = { TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3 };

/* Bien theo doi - them vao cua so Watch cua Keil de kiem tra */
volatile uint8_t g_btnRaw   = 0;   /* bit0=SELECT, bit1=UP, bit2=DOWN (1 = dang nhan, chua loc) */
volatile uint8_t g_btnStable = 0;  /* nhu tren nhung da loc nhieu */
volatile uint8_t g_selected = 0;   /* LED dang duoc chon (0,1,2) */
volatile uint8_t g_level[NUM_LED]; /* do sang 3 LED (%) */
volatile uint32_t g_btnPressCount = 0; /* so lan phat hien nhan nut */
/* USER CODE END Variables */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void App_Init(void);
static void ButtonTask(void *argument);
static void LedTask(void *argument);
/* USER CODE END FunctionPrototypes */

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* Dat do sang LED: percent 0..100 -> CCR 0..1000 */
static void Led_SetBrightness(uint8_t led, uint8_t percent)
{
    if (percent > 100) percent = 100;
    /* Hieu chinh gamma (binh phuong): mat nguoi cam nhan do sang phi tuyen,
       neu dung duty tuyen tinh thi tu 50% len 100% gan nhu khong thay doi */
    uint32_t ccr = ((uint32_t)percent * percent * PWM_PERIOD) / 10000U;
    __HAL_TIM_SET_COMPARE(&htim3, pwmChannel[led], ccr);
}

/* Cau hinh 3 chan nut la Input + Pull phu hop (khong phu thuoc CubeMX) */
static void Button_GpioInit(void)
{
    GPIO_InitTypeDef gi = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    gi.Mode = GPIO_MODE_INPUT;
    gi.Pull = (BTN_PRESSED_STATE == GPIO_PIN_RESET) ? GPIO_PULLUP : GPIO_PULLDOWN;

    gi.Pin = BTN_SEL_PIN; HAL_GPIO_Init(BTN_SEL_PORT, &gi);
    gi.Pin = BTN_UP_PIN;  HAL_GPIO_Init(BTN_UP_PORT,  &gi);
    gi.Pin = BTN_DN_PIN;  HAL_GPIO_Init(BTN_DN_PORT,  &gi);
}

/* Doc 1 nut: tra ve 1 neu dang nhan */
static inline uint8_t Btn_IsPressed(GPIO_TypeDef *port, uint16_t pin)
{
    return (HAL_GPIO_ReadPin(port, pin) == BTN_PRESSED_STATE) ? 1U : 0U;
}

/* Loc nhieu: chi doi trang thai khi doc giong nhau DEBOUNCE_COUNT lan lien tiep */
static uint8_t Debounce(uint8_t raw, uint8_t *cnt, uint8_t *stable)
{
    if (raw == *stable) {
        *cnt = 0;
    } else if (++(*cnt) >= DEBOUNCE_COUNT) {
        *stable = raw;
        *cnt = 0;
    }
    return *stable;
}

/* Task doc 3 nut nhan, gui du lieu qua Queue sang cac task LED */
static void ButtonTask(void *argument)
{
    (void)argument;

    uint8_t selected = 0;                         /* LED dang chon */
    uint8_t level[NUM_LED] = { BRIGHT_INIT, BRIGHT_INIT, BRIGHT_INIT };
    uint8_t cnt[3] = {0, 0, 0};
    uint8_t stb[3];
    uint8_t prev[3];
    LedMsg_t msg;
    uint8_t i;

    /* Khoi tao trang thai on dinh = trang thai thuc te luc bat dau,
       tranh bi "nhan gia" ngay luc khoi dong */
    stb[0] = Btn_IsPressed(BTN_SEL_PORT, BTN_SEL_PIN);
    stb[1] = Btn_IsPressed(BTN_UP_PORT,  BTN_UP_PIN);
    stb[2] = Btn_IsPressed(BTN_DN_PORT,  BTN_DN_PIN);
    prev[0] = stb[0]; prev[1] = stb[1]; prev[2] = stb[2];

    /* Gui do sang ban dau cho ca 3 LED */
    for (i = 0; i < NUM_LED; i++) {
        g_level[i] = level[i];
        msg.brightness = level[i];
        xQueueSend(ledQueue[i], &msg, 0);
    }

    for (;;) {
        uint8_t rawSel = Btn_IsPressed(BTN_SEL_PORT, BTN_SEL_PIN);
        uint8_t rawUp  = Btn_IsPressed(BTN_UP_PORT,  BTN_UP_PIN);
        uint8_t rawDn  = Btn_IsPressed(BTN_DN_PORT,  BTN_DN_PIN);

        uint8_t curSel = Debounce(rawSel, &cnt[0], &stb[0]);
        uint8_t curUp  = Debounce(rawUp,  &cnt[1], &stb[1]);
        uint8_t curDn  = Debounce(rawDn,  &cnt[2], &stb[2]);

        g_btnRaw    = (uint8_t)(rawSel | (rawUp << 1) | (rawDn << 2));
        g_btnStable = (uint8_t)(curSel | (curUp << 1) | (curDn << 2));

        /* Nut chuyen LED: LED1 -> LED2 -> LED3 -> LED1 */
        if (curSel && !prev[0]) {
            selected = (uint8_t)((selected + 1U) % NUM_LED);
            g_selected = selected;
            g_btnPressCount++;
        }

        /* Nut tang do sang */
        if (curUp && !prev[1]) {
            level[selected] = (level[selected] + BRIGHT_STEP > 100) ? 100 : (uint8_t)(level[selected] + BRIGHT_STEP);
            g_level[selected] = level[selected];
            msg.brightness = level[selected];
            xQueueSend(ledQueue[selected], &msg, 0);
            g_btnPressCount++;
        }

        /* Nut giam do sang */
        if (curDn && !prev[2]) {
            level[selected] = (level[selected] < BRIGHT_STEP) ? 0 : (uint8_t)(level[selected] - BRIGHT_STEP);
            g_level[selected] = level[selected];
            msg.brightness = level[selected];
            xQueueSend(ledQueue[selected], &msg, 0);
            g_btnPressCount++;
        }

        prev[0] = curSel;
        prev[1] = curUp;
        prev[2] = curDn;

        vTaskDelay(pdMS_TO_TICKS(BTN_POLL_MS));
    }
}

/* Task LED: 3 task dung chung ham nay, tham so = chi so LED (0,1,2) */
static void LedTask(void *argument)
{
    uint8_t idx = (uint8_t)(uintptr_t)argument;
    LedMsg_t msg;

    for (;;) {
        /* Cho cho den khi Task nut nhan gui du lieu moi */
        if (xQueueReceive(ledQueue[idx], &msg, portMAX_DELAY) == pdPASS) {
            Led_SetBrightness(idx, msg.brightness);
        }
    }
}

/* Tao queue + task, goi tu MX_FREERTOS_Init() */
void App_Init(void)
{
    uint8_t i;

    Button_GpioInit();

    for (i = 0; i < NUM_LED; i++) {
        ledQueue[i] = xQueueCreate(4, sizeof(LedMsg_t));
        if (ledQueue[i] == NULL) { Error_Handler(); }   /* thieu heap */
    }

    if (xTaskCreate(LedTask,    "Led1Task",   128, (void *)0, 1, NULL) != pdPASS) { Error_Handler(); }
    if (xTaskCreate(LedTask,    "Led2Task",   128, (void *)1, 1, NULL) != pdPASS) { Error_Handler(); }
    if (xTaskCreate(LedTask,    "Led3Task",   128, (void *)2, 1, NULL) != pdPASS) { Error_Handler(); }
    if (xTaskCreate(ButtonTask, "ButtonTask", 256, NULL,      1, NULL) != pdPASS) { Error_Handler(); }
}

/* Ham nay duoc main.c goi (main.c da khai bao san MX_FREERTOS_Init) */
void MX_FREERTOS_Init(void)
{
    App_Init();
}

/* USER CODE END Application */