#include "key_16.h"
#include "main.h"

//???????,?????8?GPIO
void KEY_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    __HAL_RCC_GPIOA_CLK_ENABLE();           //??GPIOG??
    
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

int KEY_Scan(void)
{
	int i;
	
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, GPIO_PIN_RESET);
	uint16_t GPIO_Pin[4] = {GPIO_PIN_4, GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7};
	for( i=0;i<=3;i++){
		HAL_GPIO_WritePin(GPIOA, GPIO_Pin[i], GPIO_PIN_SET);
		uint8_t l1 = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3);
		uint8_t l2 = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2);
		uint8_t l3 = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1);
		uint8_t l4 = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
		HAL_GPIO_WritePin(GPIOA, GPIO_Pin[i], GPIO_PIN_RESET);
		if(l1==1)return (1+i*4);
		else if(l2==1)return(2+i*4);
		else if(l3==1)return(3+i*4);
		else if(l4==1)return(4+i*4);
	}
	return 0;
}
