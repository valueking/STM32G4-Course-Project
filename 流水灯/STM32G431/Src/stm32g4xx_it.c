/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32g4xx_hal.h"

uint32_t delayCount = 1;
extern TIM_HandleTypeDef htim1;
 int direction=0  ; // 0表示正向，1表示反向

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


void LED_PaoMaDeng(uint8_t direction);
void TIM1_UP_TIM16_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim1);
static int count=0;
	count++;
	
	if(count>500){
	count=0;
		LED_PaoMaDeng( direction);
		
	}
           
}

void LED_PaoMaDeng(uint8_t direction)
{
    static int currentLed = 0;
    uint16_t ledPins[] = {GPIO_PIN_10, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3, GPIO_PIN_4, GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7 };

    // Turn off all LEDs
    for (int i = 0; i < 8; i++)
    {
        HAL_GPIO_WritePin(GPIOA, ledPins[i], GPIO_PIN_SET);
    }

    // Turn on the current LED
    HAL_GPIO_WritePin(GPIOA, ledPins[currentLed], GPIO_PIN_RESET);

    if (direction == 0) {
        currentLed++;
        if (currentLed >= 8)
        {
            currentLed = 0; // Reset to the first LED
        }
    } else {
        currentLed--;
        if (currentLed < 0)
        {
            currentLed = 7; // Reset to the last LED
        }
    }
}
  
  
void NMI_Handler(void) { }
void HardFault_Handler(void) { while (1) { } }
void MemManage_Handler(void) { while (1) { } }
void BusFault_Handler(void) { while (1) { } }
void UsageFault_Handler(void) { while (1) { } }
void SVC_Handler(void) { }
void DebugMon_Handler(void) { }
void PendSV_Handler(void) { }
void SysTick_Handler(void) { HAL_IncTick(); }

