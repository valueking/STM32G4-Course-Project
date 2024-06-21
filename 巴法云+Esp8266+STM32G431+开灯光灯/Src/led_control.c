#include "led_control.h"
#include "main.h"
#include "tim.h"

TIM_HandleTypeDef htim1;

void Timer_Config(void)
{
    // 配置 TIM1 定时器，实现 1 毫秒定时器中断
    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 170 - 1; // 预分频器，使定时器时钟为 1MHz
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = 1000 - 1; // 计数周期为 1000，即 1 毫秒
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
    {
        Error_Handler();
    }
    HAL_TIM_Base_Start_IT(&htim1); // 启动定时器并启用定时器中断
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1) // 检查定时器实例是否正确
    {
        // 在这里执行延时操作，例如更新延时计数器或执行特定操作
    }
}

void Delay_ms(uint32_t ms)
{
    uint32_t start_time = HAL_GetTick(); // 获取起始时间

    while (HAL_GetTick() - start_time < ms)
    {
        // 等待时间到达，由定时器中断处理延时
    }
}

void LED_Control(uint8_t direction)
{
    static int currentLed = 0;
    uint16_t ledPins[] = {GPIO_PIN_10, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3, GPIO_PIN_4, GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7 };

    // 初始化 GPIO

    if (direction == 0) {
        // 正向跑马灯效果
        for (int i = 0; i < 8; i++) {
            HAL_GPIO_WritePin(GPIOA, ledPins[i], GPIO_PIN_RESET); // 点亮当前 LED
            Delay_ms(200); // 使用定时器实现精确延时
            HAL_GPIO_WritePin(GPIOA, ledPins[i], GPIO_PIN_SET); // 熄灭当前 LED
        }
    } else if (direction == 2) {
        // 逆向跑马灯效果
        for (int i = 7; i >= 0; i--) {
            HAL_GPIO_WritePin(GPIOA, ledPins[i], GPIO_PIN_RESET); // 点亮当前 LED
            Delay_ms(200); // 使用定时器实现精确延时
            HAL_GPIO_WritePin(GPIOA, ledPins[i], GPIO_PIN_SET); // 熄灭当前 LED
        }
    }
}



