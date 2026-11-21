#include "stm32f10x.h"




// Delayus里的systick单独拎出来，然后基本定时器7作为delayus使用

// 初始化delay前记得初始化一下TIM7_Delay_Init,放在开头调用
void TIM3_Delay_Init(void) 
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;   /* 开 TIM3 时钟 */
    TIM3->PSC = 72 - 1;                   /* 1MHz 计数 */
    TIM3->ARR = 0xFFFF;                   /* 默认最大 */
    TIM3->CNT = 0;
    TIM3->CR1 |= TIM_CR1_CEN;             /* 一直开着 */
}

void Delay_us(uint32_t xus) //HAL库里也没有delay_us函数，这里标准库和HAL库同一，如果设置us，就用个基本的定时器即可。
{
    if (xus == 0) return;
    if (xus > 65535) xus = 65535;

    TIM3->ARR = xus;
    TIM3->CNT = 0;
    TIM3->SR &= ~TIM_SR_UIF;

    while (!((volatile uint32_t)TIM3->SR & TIM_SR_UIF));
}
/**
  * @brief  微秒级延时
  * @param  xus 延时时长，范围：0~233015
  * @retval 无
  */
//void Delay_us(uint32_t xus)
//{
//	SysTick->LOAD = 72 * xus;				//设置定时器重装值
//	SysTick->VAL = 0x00;					//清空当前计数值
//	SysTick->CTRL = 0x00000005;				//设置时钟源为HCLK，启动定时器
//	while(!(SysTick->CTRL & 0x00010000));	//等待计数到0
//	SysTick->CTRL = 0x00000004;				//关闭定时器
//}

/**
  * @brief  毫秒级延时
  * @param  xms 延时时长，范围：0~4294967295
  * @retval 无
  */
void Delay_ms(uint32_t xms)
{
	while(xms--)
	{
		Delay_us(1000);
	}
}
 
/**
  * @brief  秒级延时
  * @param  xs 延时时长，范围：0~4294967295
  * @retval 无
  */
void Delay_s(uint32_t xs)
{
	while(xs--)
	{
		Delay_ms(1000);
	}
} 



//systick
volatile uint32_t g_systick_ms = 0;   /* 毫秒计数 */

/**
  * @brief  SysTick 初始化，1ms 中断一次
  * @note   必须在 main() 开头尽早调用
  */
void SysTick_Init(void) //  
{
    /* SysTick 时钟源 = HCLK = 72MHz */
    SysTick->CTRL = 0;
    SysTick->VAL  = 0;
    SysTick->LOAD = 72000 - 1;              /* 72MHz / 1000 = 72000 */

    /* 使能 SysTick 中断 + 使用 HCLK + 启动定时器 */
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |
                    SysTick_CTRL_TICKINT_Msk |
                    SysTick_CTRL_ENABLE_Msk;
}

/**
  * @brief  获取系统运行毫秒数
  */
uint32_t SysTick_GetTick(void)
{
    return g_systick_ms;
}

/**
  * @brief  毫秒级阻塞延时
  */
//void Delay_ms(uint32_t ms)
//{
//    uint32_t start = g_systick_ms;
//    while ((g_systick_ms - start) < ms);
//}

/**
  * @brief  SysTick 中断服务函数
  * @note   必须放在启动文件对应的中断向量表里，
  *         标准库工程通常写在 stm32f10x_it.c
  */
void SysTick_Handler(void)
{
    g_systick_ms++;
}
