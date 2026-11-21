#include "stm32f10x.h"                  // Device header
#include <stdio.h>
#include <stdarg.h>
#include "Delay.h"
#include "Serial.h"

uint8_t Serial_RxData;		//定义串口接收的数据变量
uint8_t Serial_RxFlag;		//定义串口接收的标志位变量



/**
  * 函    数：串口初始化
  * 参    数：无
  * 返 回 值：无
  */
void Serial_Init(void) //江科大标准库串口1接受中断配置
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);	//开启USART1的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	//开启GPIOA的时钟
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//将PA9引脚初始化为复用推挽输出
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//将PA10引脚初始化为上拉输入
	
	/*USART初始化*/
	USART_InitTypeDef USART_InitStructure;					//定义结构体变量
	USART_InitStructure.USART_BaudRate = 9600;				//波特率
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//硬件流控制，不需要
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;	//模式，发送模式和接收模式均选择
	USART_InitStructure.USART_Parity = USART_Parity_No;		//奇偶校验，不需要
	USART_InitStructure.USART_StopBits = USART_StopBits_1;	//停止位，选择1位
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;		//字长，选择8位
	USART_Init(USART1, &USART_InitStructure);				//将结构体变量交给USART_Init，配置USART1
	
	/*中断输出配置*/
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);			//开启串口接收数据的中断
	
	/*NVIC中断分组*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);			//配置NVIC为分组2
	
	/*NVIC配置*/
	NVIC_InitTypeDef NVIC_InitStructure;					//定义结构体变量
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;		//选择配置NVIC的USART1线
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//指定NVIC线路使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;		//指定NVIC线路的抢占优先级为1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//指定NVIC线路的响应优先级为1
	NVIC_Init(&NVIC_InitStructure);							//将结构体变量交给NVIC_Init，配置NVIC外设
	
	/*USART使能*/
	USART_Cmd(USART1, ENABLE);								//使能USART1，串口开始运行
}

/**
  * 函    数：串口发送一个字节
  * 参    数：Byte 要发送的一个字节
  * 返 回 值：无
  */
void Serial_SendByte(uint8_t Byte)
{
	USART_SendData(USART1, Byte);		//将字节数据写入数据寄存器，写入后USART自动生成时序波形
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);	//等待发送完成
	/*下次写入数据寄存器会自动清除发送完成标志位，故此循环后，无需清除标志位*/
}

/**
  * 函    数：串口发送一个数组
  * 参    数：Array 要发送数组的首地址
  * 参    数：Length 要发送数组的长度
  * 返 回 值：无
  */
void Serial_SendArray(uint8_t *Array, uint16_t Length)
{
	uint16_t i;
	for (i = 0; i < Length; i ++)		//遍历数组
	{
		Serial_SendByte(Array[i]);		//依次调用Serial_SendByte发送每个字节数据
	}
}

/**
  * 函    数：串口发送一个字符串
  * 参    数：String 要发送字符串的首地址
  * 返 回 值：无
  */
void Serial_SendString(char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i ++)//遍历字符数组（字符串），遇到字符串结束标志位后停止
	{
		Serial_SendByte(String[i]);		//依次调用Serial_SendByte发送每个字节数据
	}
}

/**
  * 函    数：次方函数（内部使用）
  * 返 回 值：返回值等于X的Y次方
  */
uint32_t Serial_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;	//设置结果初值为1
	while (Y --)			//执行Y次
	{
		Result *= X;		//将X累乘到结果
	}
	return Result;
}

/**
  * 函    数：串口发送数字
  * 参    数：Number 要发送的数字，范围：0~4294967295
  * 参    数：Length 要发送数字的长度，范围：0~10
  * 返 回 值：无
  */
void Serial_SendNumber(uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i ++)		//根据数字长度遍历数字的每一位
	{
		Serial_SendByte(Number / Serial_Pow(10, Length - i - 1) % 10 + '0');	//依次调用Serial_SendByte发送每位数字
	}
}

/**
  * 函    数：使用printf需要重定向的底层函数
  * 参    数：保持原始格式即可，无需变动
  * 返 回 值：保持原始格式即可，无需变动
  */
//int fputc(int ch, FILE *f)
//{
//	Serial_SendByte(ch);			//将printf的底层重定向到自己的发送字节函数
//	return ch;
//}



int fputc(int ch, FILE *f)
{
    USART_SendData(USART1, (uint8_t)ch);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); 
    return ch;
}


/**
  * 函    数：自己封装的prinf函数
  * 参    数：format 格式化字符串
  * 参    数：... 可变的参数列表
  * 返 回 值：无
  */
void Serial_Printf(char *format, ...)
{
	char String[100];				//定义字符数组
	va_list arg;					//定义可变参数列表数据类型的变量arg
	va_start(arg, format);			//从format开始，接收参数列表到arg变量
	vsprintf(String, format, arg);	//使用vsprintf打印格式化字符串和参数列表到字符数组中
	va_end(arg);					//结束变量arg
	Serial_SendString(String);		//串口发送字符数组（字符串）
}

/**
  * 函    数：获取串口接收标志位
  * 参    数：无
  * 返 回 值：串口接收标志位，范围：0~1，接收到数据后，标志位置1，读取后标志位自动清零
  */
uint8_t Serial_GetRxFlag(void)
{
	if (Serial_RxFlag == 1)			//如果标志位为1
	{
		Serial_RxFlag = 0;
		return 1;					//则返回1，并自动清零标志位
	}
	return 0;						//如果标志位为0，则返回0
}

/**
  * 函    数：获取串口接收的数据
  * 参    数：无
  * 返 回 值：接收的数据，范围：0~255
  */
uint8_t Serial_GetRxData(void)
{
	return Serial_RxData;			//返回接收的数据变量
}

/**
  * 函    数：USART1中断函数
  * 参    数：无
  * 返 回 值：无
  * 注意事项：此函数为中断函数，无需调用，中断触发后自动执行
  *           函数名为预留的指定名称，可以从启动文件复制
  *           请确保函数名正确，不能有任何差异，否则中断函数将不能进入
  */
//void USART1_IRQHandler(void)
//{
//	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)		//判断是否是USART1的接收事件触发的中断
//	{
//		rx_cnt++;
//		Serial_RxData = USART_ReceiveData(USART1);				//读取数据寄存器，存放在接收的数据变量
//		Serial_RxFlag = 1;										//置接收标志位变量为1
//		USART_ClearITPendingBit(USART1, USART_IT_RXNE);			//清除USART1的RXNE标志位
//																//读取数据寄存器会自动清除此标志位
//																//如果已经读取了数据寄存器，也可以不执行此代码
//	}
//}


/**
* @brief  阻塞接收不定长数据，直到 IDLE 或收满 Size，或超时，不需要开启串口中断
 * @param  USARTx   串口实例，例如 USART1
 * @param  pData    接收缓冲区
 * @param  Size     最大接收长度
 * @param  RxLen    实际接收长度
 * @param  Timeout  超时时间，单位 ms；0 表示不等待
 * @return UART_OK / UART_TIMEOUT / UART_ERROR / UART_BUSY
 */


#include "stm32f10x.h"

void USART1_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    /* 1. 使能时钟：USART1 和 GPIOA 都在 APB2 上 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    /* 2. 配置 TX：PA9 复用推挽输出 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* 3. 配置 RX：PA10 上拉输入 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* 4. 配置 USART1 参数 */
    USART_InitStructure.USART_BaudRate            = 115200;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART1, &USART_InitStructure);

    /* 5. 使能串口 */
    USART_Cmd(USART1, ENABLE);
}


// DMA + 串口IDLE中断
uint8_t rx_buf[RX_BUF_SIZE]; // 接收缓冲区
uint16_t rx_len = 0;         // 实际收到的数据长度
//volatile uint8_t frame_ready = 0;  // 通知主循环

/**
 * @brief 串口与DMA初始化 (以USART1, PA9/PA10为例)
 */
void USART1_DMA_Init(uint32_t baudrate)
{
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    // 1. 开启外设时钟 (USART1, GPIOA, DMA1)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    // 2. 配置 GPIO (PA9 -> TX, PA10 -> RX)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // TX为复用推挽
    GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // RX改为上拉输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 3. 配置串口
    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    // 4. 开启串口IDLE空闲中断
    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);

    // 5. 配置 DMA1 Channel5 (USART1_RX 对应 DMA1 的通道5)
    DMA_DeInit(DMA1_Channel5);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&USART1->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)rx_buf;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC; // 外设是源 (数据流向内存)
    DMA_InitStructure.DMA_BufferSize = RX_BUF_SIZE;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal; // 普通模式，收满需重新配置
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel5, &DMA_InitStructure);

    // 6. 开启 DMA1_Channel5
    DMA_Cmd(DMA1_Channel5, ENABLE);

    // 7. 开启串口 DMA 接收请求
    USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);

    // 8. 配置NVIC中断 (注意：IDLE 标志位在串口中断中，需要开启串口全局中断)
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // 9. 启动串口
    USART_Cmd(USART1, ENABLE);
}



/**
 * @brief 串口1中断IDLE服务函数
 */
//void USART1_IRQHandler(void)
//{
//	if (USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
//	{
//		/* 1. 先关 DMA，锁住搬运进度 */
//		DMA_Cmd(DMA1_Channel5, DISABLE);

//		/* 2. 读 SR，准备清 IDLE */
//		volatile uint32_t tmp = USART1->SR;
//		(void)tmp;

//		/* 3. 计算 DMA 已经搬运了多少字节 */
//		rx_len = RX_BUF_SIZE - DMA_GetCurrDataCounter(DMA1_Channel5);

//		/* 4. 如果 RXNE 仍然为 1，说明最后一个字节还在 DR 里 */
//		if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET)
//		{
//			uint8_t last_byte = USART1->DR;   // 读 DR，既清 RXNE，也清 IDLE
//			rx_buf[rx_len] = last_byte;
//			rx_len++;
//		}
//		else
//		{
//			/* DR 已被 DMA 搬走，这里只读一次 DR 清 IDLE */
//			USART1->DR;
//		}
//		
//		//重启DMA
//		DMA_SetCurrDataCounter(DMA1_Channel5, RX_BUF_SIZE);
//        DMA_Cmd(DMA1_Channel5, ENABLE);

//        frame_ready = 1;  // 通知主循环

//		/* 5. rx_len 就是完整帧长度 */
//	}
//}



//此函数需要配合systick_ms 来联合使用



extern volatile uint32_t g_systick_ms;
UART_StatusTypeDef UART_ReceiveToIdle(USART_TypeDef *USARTx,
                                      uint8_t *pData,
                                      uint16_t Size,
                                      uint16_t *RxLen,
                                      uint32_t Timeout)
{
    uint32_t tickstart;
    uint16_t len = 0;

    if ((pData == NULL) || (Size == 0U) || (RxLen == NULL))
    {
        return UART_ERROR;
    }

    *RxLen = 0;

    /* 记录起始毫秒计数 */
    tickstart = g_systick_ms;

    while (len < Size)
    {
        /* 1. 优先检测 IDLE 标志位 (仿照 HAL 库，先判空闲) */
        if (USARTx->SR & USART_SR_IDLE)
        {
            /* 如果已经有收到数据，说明这是一个有效的帧尾 */
            if (len > 0U)
            {
                /* 标准库 F1：清除 IDLE 需要先读 SR，再读 DR */
                (void)USARTx->SR;
                (void)USARTx->DR;

                *RxLen = len;
                return UART_OK;
            }
            else
            {
                /* 没有收到任何数据就出现了 IDLE，清除标志位继续等待 */
                (void)USARTx->SR;
                (void)USARTx->DR;
            }
        }

        /* 2. 判断是否收到一个字节 */
        if (USART_GetFlagStatus(USARTx, USART_FLAG_RXNE) != RESET)
        {
            pData[len] = (uint8_t)(USART_ReceiveData(USARTx) & 0xFF);
            len++;
        }

        /* 3. 缓冲区溢出检查 */
        if (len >= Size)
        {
            *RxLen = len;
            return UART_BUSY; // 类似于 HAL 中的 BUSY，这里表示收满了
        }

        /* 4. 超时判断（单位：ms）*/
        if (Timeout != 0xFFFFFFFFU)
        {
            uint32_t elapsed = g_systick_ms - tickstart;
            if (elapsed >= Timeout)
            {
                *RxLen = len;
                return UART_TIMEOUT;
            }
        }
    }

    /* 5. 正常收满 Size 个字节，且没有触发 IDLE */
    *RxLen = len;
    return UART_OK;
}


////调用示例


//UART_StatusTypeDef status;

//		status = UART_ReceiveToIdle(USART1, rx_buf, 128, &rx_len, 1000);

//		if (status == UART_OK)
//		{
//			if (rx_len < sizeof(rx_buf))
//			{
//				rx_buf[rx_len] = '\0';
//			}

//			GPIO_ResetBits(GPIOC, GPIO_Pin_13);

//			printf("已收到数据，数据是：%s\r\n", (char *)rx_buf);

//			rx_len = 0;
//			memset(rx_buf, '\0', sizeof(rx_buf));
//		}

