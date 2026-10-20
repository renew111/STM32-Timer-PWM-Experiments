/**
 ******************************************************************************
 * @file    main.c
 * @brief   实验内容二：TIM3_CH2 PWM 呼吸灯（约 2s 一个完整呼吸周期）
 * ----------------------------------------------------------------------------
 * 芯片    : STM32F103C8T6（Blue Pill 最小系统板）
 * 开发环境: Keil MDK5 + STM32F10x 标准外设库 V3.5.0
 * 定时器  : TIM3_CH2，复用引脚 PA7
 *
 * 参数设计：
 *   1. PWM 频率：TIM3 时钟 72MHz，PSC = 71 → 计数频率 1MHz；
 *      ARR = 999 → PWM 频率 = 1MHz / 1000 = 1kHz（人眼看不到闪烁）。
 *   2. 占空比范围：CCR2 = 0 ~ 1000，对应占空比 0% ~ 100%。
 *   3. 呼吸速度：SysTick 每 5ms 中断一次，共 200 级亮度；
 *      渐亮 200×5ms = 1s，渐灭 1s，完整呼吸周期 2s。
 *   4. 亮度校正：人眼对亮度的感知近似 gamma=2.0，若占空比线性增加，
 *      低亮度段变化太快、高亮度段变化不明显。故用
 *          duty = step² × ARR / 200²
 *      做平方校正，使呼吸效果看起来均匀自然。
 *
 * 硬件连接（高电平点亮）：
 *   PA7 ——[220Ω]—— LED 阳极，LED 阴极 —— GND
 ******************************************************************************
 */

#include "stm32f10x.h"

/* ---------------- 参数宏定义 ---------------- */
#define PWM_ARR             999U     /* 自动重装值，PWM 频率 1kHz */
#define BREATH_HALF_STEPS   200U     /* 渐亮/渐灭各 200 级 */
#define SYSTICK_PERIOD_MS   5U       /* 每 5ms 更新一次占空比 */

/* SysTick 累加的毫秒节拍，在 SysTick 中断中自增 */
static volatile uint32_t s_tick = 0;

/**
 * @brief  初始化 PA7 为复用推挽输出（TIM3_CH2 的 PWM 输出脚）
 */
static void PWM_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_7;              /* PA7 = TIM3_CH2 */
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;         /* 复用推挽输出 */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/**
 * @brief  配置 TIM3_CH2 为 PWM1 模式
 * @note   PWM1 模式：CNT < CCR2 时输出高电平，CNT ≥ CCR2 时输出低电平，
 *         因此 CCR2 越大，高电平时间越长，LED 越亮（高电平点亮接法）。
 */
static void TIM3_PWM_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef       TIM_OCInitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    /* 时基：72MHz / (71+1) = 1MHz，1MHz / (999+1) = 1kHz */
    TIM_TimeBaseStructure.TIM_Prescaler         = 71;
    TIM_TimeBaseStructure.TIM_Period            = PWM_ARR;
    TIM_TimeBaseStructure.TIM_ClockDivision     = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode       = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    /* 通道 2 PWM 配置 */
    TIM_OCStructInit(&TIM_OCInitStructure);                  /* 先填默认值 */
    TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1;   /* PWM 模式 1 */
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse       = 0;                 /* 初始占空比 0%，LED 灭 */
    TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High; /* 高电平有效 */
    TIM_OC2Init(TIM3, &TIM_OCInitStructure);

    /* 使能 CCR2 预装载：写入的新占空比在下一个更新事件才生效，避免毛刺 */
    TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM3, ENABLE);

    TIM_Cmd(TIM3, ENABLE);
}

/**
 * @brief  配置 SysTick 每 5ms 中断一次，作为呼吸灯的时间基准
 */
static void SysTick_Init_5ms(void)
{
    /* SystemCoreClock = 72MHz，72000000 / 200 = 360000，即 5ms */
    SysTick_Config(SystemCoreClock / (1000U / SYSTICK_PERIOD_MS));
}

/**
 * @brief  SysTick 中断服务函数：只做计数，不执行耗时操作
 */
void SysTick_Handler(void)
{
    s_tick++;
}

/**
 * @brief  根据亮度等级计算并设置比较值
 * @param  step : 0 ~ BREATH_HALF_STEPS
 * @note   平方校正：duty = step² × ARR / 200²
 */
static void PWM_SetBrightness(uint32_t step)
{
    uint32_t duty;

    duty = (step * step * PWM_ARR) / (BREATH_HALF_STEPS * BREATH_HALF_STEPS);
    if (duty > PWM_ARR)
        duty = PWM_ARR;

    TIM_SetCompare2(TIM3, (uint16_t)duty);
}

int main(void)
{
    uint32_t last_tick = 0;
    uint32_t step      = 0;      /* 当前亮度等级 0~200 */
    uint8_t  rising    = 1;      /* 1 = 渐亮，0 = 渐灭 */

    PWM_GPIO_Init();
    TIM3_PWM_Init();
    SysTick_Init_5ms();

    while (1)
    {
        /* 每 5ms 更新一次占空比 */
        if ((s_tick - last_tick) >= 1)
        {
            last_tick = s_tick;

            PWM_SetBrightness(step);

            if (rising)
            {
                step++;
                if (step >= BREATH_HALF_STEPS)   /* 到达最亮，转为渐灭 */
                {
                    step    = BREATH_HALF_STEPS;
                    rising  = 0;
                }
            }
            else
            {
                if (step == 0)                   /* 到达最暗，转为渐亮 */
                    rising = 1;
                else
                    step--;
            }
        }
    }
}
