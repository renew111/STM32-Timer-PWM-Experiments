/**
 ******************************************************************************
 * @file    main.c
 * @brief   实验内容一：定时器计数方式控制 LED 以 2s 周期亮灭
 * ----------------------------------------------------------------------------
 * 芯片    : STM32F103C8T6（Blue Pill 最小系统板）
 * 开发环境: Keil MDK5 + STM32F10x 标准外设库 V3.5.0
 * 定时器  : TIM3（挂在 APB1 总线上，定时器时钟 72MHz）
 * 输出引脚: PA7 —— TIM3_CH2 的复用引脚（本实验先当普通 GPIO 用）
 *
 * 原理：
 *   TIM3 的时钟为 72MHz。设置预分频 PSC = 7199，则计数频率为
 *       72MHz / (7199 + 1) = 10kHz
 *   再设置自动重装值 ARR = 9999，则每计满 10000 个数产生一次更新事件：
 *       10kHz / 10000 = 1Hz  →  更新中断每 1s 触发一次
 *   在中断服务函数里翻转 PA7 的电平，LED 便以“亮 1s、灭 1s”的方式工作，
 *   完整亮灭周期 = 2s。
 *
 * 硬件连接（高电平点亮）：
 *   PA7 ——[220Ω]—— LED 阳极，LED 阴极 —— GND
 *   即 PA7 输出高电平时 LED 亮，输出低电平时 LED 灭。
 ******************************************************************************
 */

#include "stm32f10x.h"

/* ---------------- 硬件宏定义：换引脚时只改这里 ---------------- */
#define LED_PORT            GPIOA
#define LED_PIN             GPIO_Pin_7          /* PA7 = TIM3_CH2 */

/* 中断里翻转 LED 的次数，可在 Keil 的 Watch 窗口观察 */
volatile uint32_t g_toggle_cnt = 0;

/**
 * @brief  初始化 LED 引脚为推挽输出（高电平点亮）
 */
static void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* PA7 属于 GPIOA，GPIO 挂在 APB2 总线上 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin   = LED_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;    /* 通用推挽输出 */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LED_PORT, &GPIO_InitStructure);

    /* 上电默认熄灭（低电平） */
    GPIO_ResetBits(LED_PORT, LED_PIN);
}

/**
 * @brief  配置 TIM3 每 1s 产生一次更新中断
 * @note   TIM3 时钟 = 72MHz（APB1 分频系数为 2 时，定时器时钟为 PCLK1×2）
 */
static void TIM3_Init_1s(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef        NVIC_InitStructure;

    /* 1. 使能 TIM3 时钟（TIM3 挂在 APB1 总线上） */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    /* 2. 时基配置：72MHz / 7200 = 10kHz，10kHz / 10000 = 1Hz */
    TIM_TimeBaseStructure.TIM_Prescaler         = 7199;   /* PSC：7200 分频 */
    TIM_TimeBaseStructure.TIM_Period            = 9999;   /* ARR：计到 9999 溢出 */
    TIM_TimeBaseStructure.TIM_ClockDivision     = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode       = TIM_CounterMode_Up; /* 向上计数 */
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    /* 3. 清除可能存在的更新标志，再使能更新中断 */
    TIM_ClearFlag(TIM3, TIM_FLAG_Update);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

    /* 4. 配置 NVIC：TIM3 中断通道 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitStructure.NVIC_IRQChannel                   = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* 5. 启动定时器 */
    TIM_Cmd(TIM3, ENABLE);
}

/**
 * @brief  TIM3 中断服务函数：每 1s 进入一次，翻转 LED
 * @note   函数名必须与启动文件 startup_stm32f10x_md.s 中的向量表一致
 */
void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);   /* 先清标志，防止反复进入 */

        /* 读出当前输出电平并取反：0->1（亮），1->0（灭） */
        if (GPIO_ReadOutputDataBit(LED_PORT, LED_PIN))
            GPIO_ResetBits(LED_PORT, LED_PIN);
        else
            GPIO_SetBits(LED_PORT, LED_PIN);

        g_toggle_cnt++;
    }
}

int main(void)
{
    LED_Init();
    TIM3_Init_1s();

    while (1)
    {
        /* 主循环不做事，LED 完全由定时器中断控制；
         * WFI 让 CPU 在空闲时进入睡眠，中断到来后自动唤醒。 */
        __WFI();
    }
}
