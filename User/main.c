#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Serial.h"
#include <string.h> //memset



extern uint8_t rx_buf[RX_BUF_SIZE]; // 接收缓冲区
extern uint16_t rx_len;         // 实际收到的数据长度

UART_StatusTypeDef status;


int main(void)
{
        SysTick_Init();
	    TIM3_Delay_Init();
        USART1_Config();
		/* 开启时钟 */
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);  // 开启 GPIOC 的时钟

		/* GPIO 初始化 */
		GPIO_InitTypeDef GPIO_InitStructure;
		GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_13;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;         // PC13 建议不超过 2MHz
		GPIO_Init(GPIOC, &GPIO_InitStructure);                   // 将 PC13 初始化为推挽输出

		/* 设置 GPIO 初始化后的默认电平 */
		GPIO_SetBits(GPIOC, GPIO_Pin_13);                        // 设置 PC13 为高电平	
	    printf("初始化完毕\r\n");

	while (1)
	{

		status = UART_ReceiveToIdle(USART1, rx_buf, RX_BUF_SIZE, &rx_len, 100);

		if (status == UART_OK)
		{
			if (rx_len < sizeof(rx_buf))
			{
				rx_buf[rx_len] = '\0';
			}
            printf("收到数据（长度: %d）：%s\r\n", rx_len, (char*)rx_buf);
 
			rx_len = 0;
			memset(rx_buf, '\0', sizeof(rx_buf));
		}


	}
		
}
