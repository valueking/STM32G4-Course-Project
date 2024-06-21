#include "led_control.h"
#include "main.h"
#include "tim.h"

extern TIM_HandleTypeDef htim1;
void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    __HAL_RCC_GPIOA_CLK_ENABLE(); // Enable GPIOA clock

    GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_10| GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL; // Change GPIO_PULLDOWN to GPIO_NOPULL
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_10, GPIO_PIN_SET); // Set GPIO pins
	HAL_GPIO_WritePin(GPIOA,  GPIO_PIN_0, GPIO_PIN_RESET);
}


void LED_Control(uint8_t direction)
{
    static int currentLed = 0;
    uint16_t ledPins[] = {GPIO_PIN_10, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3, GPIO_PIN_4, GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7 };

    // Turn off all LEDs
    for (int i = 0; i < 8; i++)
    {
        HAL_GPIO_WritePin(GPIOA, ledPins[i], GPIO_PIN_RESET);
    }

    // Turn on the current LED
    HAL_GPIO_WritePin(GPIOA, ledPins[currentLed], GPIO_PIN_SET);

    // Update current LED index for next iteration
    currentLed++;

    if (currentLed >= 8)
    {
        currentLed = 0; // Reset to the first LED
    }
}
    



