/* USER CODE BEGIN Header */
/***************************************************************************//**
 yangcc dx1211    

*******************************************************************************/
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "gpio.h"
#include "usart.h"
#include "string.h"
#include "oled.h"
#include "key_16.h"
volatile uint8_t UartRxData;	
uint8_t UartTxbuf[1000]={1,2,3,4,5,6,7,8,9,10};
uint8_t UartRxbuf[1024],UartIntRxbuf[1024];
uint16_t UartRxIndex=0,UartRxFlag,UartRxLen=0,UartRxTimer,UartRxOKFlag,UartIntRxLen;
uint8_t zd_value=0;
uint8_t UartTxbuf_sendstr[1024];
uint16_t UartTxLen_sendstr=0;
// 全局变量，用于存储接收到的数据
volatile uint8_t receivedChar;
volatile uint8_t receivedFlag = 0;
extern char receivedMessage[];
/* Private includes ----------------------------------------------------------*/
/////////////////////////////////////////////////////////////////
//???????
void EXTI_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    __HAL_RCC_GPIOB_CLK_ENABLE();               //??GPIOB??
    
    GPIO_Initure.Pin=GPIO_PIN_0;       
    GPIO_Initure.Mode=GPIO_MODE_IT_RISING;      //?????
    GPIO_Initure.Pull=GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOA,&GPIO_Initure);  
	//???4-PB4
    HAL_NVIC_SetPriority(EXTI0_IRQn,2,1);       //??????2,?????1
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);             //?????0   
}



///////////////////////////////////////////////////
 /*用于一键联网的操作
 */
uint8_t sendstep=0;
uint8_t sending = 0; // 新增变量，标记是否正在发送指令
#define MAX_RESPONSE_SIZE 256
#define TIMEOUT_MS 5000
void SendCommandsWithDelay(void);
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (!sending) // 如果没有正在发送指令，则开始发送
    {
        sending = 1; // 标记为正在发送指令
        sendstep = 0; // 重置发送步骤
        // 调用发送指令的函数
        SendCommandsWithDelay();
    }

}	

 int ReadLineUntilResponse(char *buffer, size_t buffer_size, const char *expected_response, uint32_t timeout_ms);

void SendCommandsWithDelay(void)
{
    char *commands[] = {
        "AT+CWMODE=3\r\n",
        "AT+CWJAP=\"MIX 3\",\"12345678\"\r\n",
        "AT+MQTTUSERCFG=0,1,\"aee3a1ecc6754bc39f486a0459b3ecd0\",\"\",\"\",0,0,\"\"\r\n",
        "AT+MQTTCONN=0,\"bemfa.com\",9501,0\r\n",
        "AT+MQTTSUB=0,\"Y\",1\r\n",
        "AT+MQTTPUB=0,\"Y\",\"succeed to connect\",0,0\r\n"
    };
    char response[MAX_RESPONSE_SIZE];
    uint8_t command_count = sizeof(commands) / sizeof(char *);

    for (int i = 0; i < command_count; ++i)
    {
        HAL_UART_Transmit(&huart1, (uint8_t *)commands[i], strlen(commands[i]), HAL_MAX_DELAY);
        // 等待响应"OK"
        if (ReadLineUntilResponse(response, MAX_RESPONSE_SIZE, "OK", TIMEOUT_MS) != 0)
        {
            // 处理接收到的响应
            // 如果需要，可以添加错误处理代码
            break; // 如果没有接收到OK，跳出循环
        }
    }
    
    // 发送完成后，标记为不在发送指令（如果sending是全局变量）
    sending = 0;
}

// 实现串口读取函数，等待特定的响应或超时
int ReadLineUntilResponse(char *buffer, size_t buffer_size, const char *expected_response, uint32_t timeout_ms) {
    uint32_t start_time = HAL_GetTick(); // 获取当前时间
    char *ptr = buffer;
    
    while (HAL_GetTick() - start_time < timeout_ms) {
        if (receivedFlag) {
            receivedFlag = 0; // 清除接收标志
            *ptr = receivedChar; // 将接收到的数据存入缓冲区
            ptr++;
            if (receivedChar == '\n' || (ptr - buffer) >= buffer_size - 1) {
                *ptr = '\0'; // 确保字符串以null结尾
                // 检查是否接收到预期的响应
                if (strstr(buffer, expected_response) != NULL) {
                    return 1; // 接收到预期响应
                } else {
                    return 0; // 未接收到预期响应
                }
            }
        }
    }
    
    return 0; // 超时未接收到预期响应
}

///////////////////////////////////////////////////////////////
 /**
[2024/06/03 01:50:48.282] ← AT+MQTTPUB=0,"Y","succeed to connect",0,0

[2024/06/03 01:50:48.392] → AT+MQTTPUB=0,"Y","succeed to connect",0,0

OK
[2024/06/03 01:50:48.556] → +MQTTSUBRECV:0,"Y",18,succeed to connect

[2024/06/03 01:57:06.928] → +MQTTSUBRECV:0,"Y",2,on

[2024/06/03 01:57:17.904] → +MQTTSUBRECV:0,"Y",2,on

[2024/06/03 01:57:30.169] → +MQTTSUBRECV:0,"Y",9,123456789

*/

// 定义全局变量
char receivedMessage[1024] = {0};
char lastReceivedCommand = '\0';

// 解析订阅消息的函数
void ParseMqttSubRecv(char *message) {
    // 定义命令数组和对应的指令
    char *commands[] = {
        "+MQTTSUBRECV:0,\"Y\",2,on",
        "+MQTTSUBRECV:0,\"Y\",3,off"
    };
    char commandLetters[] = {'o', 'f'};  
    
    // 遍历命令数组，匹配特定的命令格式
    for (int i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
        if (strstr(message, commands[i])) {
            lastReceivedCommand = commandLetters[i];
            return;
        }
    }

    // 如果没有匹配的命令，设定为无效命令
    lastReceivedCommand = '\0';
}


// 控制LED并在OLED上显示信息的函数
void ControlLedAndShowOnOled(void) {
    const char *ledStatusOn = "on"  ;
    const char *ledStatusOff = "off" ;
    const char *ledStatusNoCmd = "No command";
    
    OLED_Clear_Line(6); // 清屏


    switch (lastReceivedCommand) {
        case 'o': // 打开LED
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
            OLED_ShowString(0, 6, (uint8_t *)ledStatusOn);
            break;
        case 'f': // 关闭LED
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
            OLED_ShowString(0, 6, (uint8_t *)ledStatusOff);
            break;
        default:
            // 无效命令
            OLED_ShowString(0, 6, (uint8_t *)ledStatusNoCmd);
            break;
    }
}

 // 主循环中处理接收消息并执行相应操作
void ProcessReceivedMessage(void) {
    if (receivedMessage[0] != '\0') { // 检查是否有接收到消息
        ParseMqttSubRecv(receivedMessage); // 解析接收到的消息
        ControlLedAndShowOnOled(); // 控制LED并在OLED上显示信息
        receivedMessage[0] = '\0'; // 清空接收到的消息
    }
}



  // 显示接收到的消息的函数
void DisplayReceivedMessage(void) {
    if (receivedMessage[0] != '\0') { // 检查消息是否为空
       // OLED_Clear(); // 清屏
        OLED_ShowString(0, 6, (uint8_t *)receivedMessage); // 显示消息
//        memset(receivedMessage, 0, sizeof(receivedMessage)); // 清空消息缓冲区
    }
}



void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart == &huart1) {
        UartIntRxbuf[UartRxIndex] = UartRxData;
        UartRxIndex++;

        if (UartRxData == '\n') {
            UartIntRxbuf[UartRxIndex] = '\0'; // 确保接收到完整的一行数据
            UartRxFlag = 0; // 标记接收完成

            // 解析接收到的消息
            ParseMqttSubRecv((char *)UartIntRxbuf);

            // 控制 LED 并在 OLED 上显示信息
            ControlLedAndShowOnOled();

            // 清空接收缓冲区，准备接收新数据
            UartRxIndex = 0;
        }

        // 准备接收下一个字符
        HAL_UART_Receive_IT(&huart1, (uint8_t *)&UartRxData, 1);
    }

}


/////////////////////////////////////////////////////////////////
/* USER CODE BEGIN Includes */
uint8_t UartRecv_Clear(void)
{
	UartRxOKFlag=0;
	UartRxLen=0;
	UartIntRxLen=0;
	UartRxIndex=0;
	return 1;
}


uint8_t Uart_RecvFlag(void)
{
		if(UartRxOKFlag==0x55)
		{
			UartRxOKFlag=0;
			UartRxLen=UartIntRxLen;

			memcpy(UartRxbuf,UartIntRxbuf,UartIntRxLen);
			UartRxbuf[UartIntRxLen]='\0';
			UartIntRxLen=0;
			return 1;
		}
		return 0;
}



//1ms????UART_RecvDealwith
void UART_RecvDealwith(void)
{
	if(UartRxFlag==0x55)
	{
		if(UartIntRxLen<UartRxIndex)
		{
		UartIntRxLen=UartRxIndex;
		}else
		{
			UartRxTimer++;
			if(UartRxTimer>=50)
			{
				UartRxTimer=0;
				UartRxFlag=0;
				UartRxOKFlag=0x55;
				UartRxIndex=0;
			}
		}
	}
}
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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
    char t;
  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();//16MĚ²¿¾§ձ£¬16MַƵ

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */
	MX_USART1_UART_Init();
	HAL_UART_Receive_IT(&huart1, (uint8_t *)&UartRxData, 1);
  
EXTI_Init();
  /* Infinite loop */
	OLED_Init();	 //OLED³õʼ»¯  
	OLED_Clear();//ǥƁ
	t=' ';
		OLED_ShowCHinese(8,0,0); //电
		OLED_ShowCHinese(24,0,1);//信
	
		OLED_ShowString(40,0,"1211");
		OLED_ShowCHinese(72,0,2); //某
		OLED_ShowCHinese(88,0,3); //某
		OLED_ShowCHinese(104,0,4);//某
		OLED_ShowString(19,3,":");
		OLED_ShowString(20,3,"10214421000");
	
/* USER CODE END 2 */
//通电自动发送
  SendCommandsWithDelay();
/* USER CODE BEGIN 3 */
	while(1) 
	{	
	
		
		 // 等待接收消息
        memset(receivedMessage, 0, sizeof(receivedMessage)); // 清空消息缓冲区
        if (ReadLineUntilResponse(receivedMessage, sizeof(receivedMessage), "OK", TIMEOUT_MS)) {
            // 假设我们收到了一个响应并存储在 receivedMessage 中
            // 现在我们解析接收到的消息
            ParseMqttSubRecv(receivedMessage);
            

			  // 主循环中处理接收到的消息
				ProcessReceivedMessage();
			    DisplayReceivedMessage();

        }
	
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV2;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;	//?PLL?????170M
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

//  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */


void USART1_IRQHandler(void)
{

  HAL_UART_IRQHandler(&huart1); //???????????:HAL_UART_RxCpltCallback

}
/* USER CODE END 4 */

//??????
void EXTI0_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);//??????????
}


/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/


