# STM32 定时器与 PWM 实验

本目录是「STM32 定时器计数 + PWM 呼吸灯」课程实验的完整交付物，包含两个可直接编译的 Keil 工程、实验报告（Markdown + PDF）、博客文章和烧录文件。

## 一、实验内容

| 内容 | 实现方式 | 现象 |
| --- | --- | --- |
| 内容一 | TIM3 更新中断（PSC=7199，ARR=9999） | LED 亮 1s、灭 1s，周期 2s |
| 内容二 | TIM3_CH2 PWM（PSC=71，ARR=999，1kHz）+ SysTick 5ms 调占空比 | LED 呼吸，渐亮 1s、渐灭 1s，周期约 2s |

- 芯片：STM32F103C8T6（Blue Pill，8MHz 晶振，72MHz 主频）
- 引脚：**PA7 = TIM3_CH2**
- LED 接法：PA7 → 220Ω → LED 阳极，LED 阴极 → GND（**高电平点亮**）

## 二、目录结构

```
STM32定时器PWM实验/
├── code/
│   ├── Task1_Timer_LED/          # 内容一：定时器计数，2s 周期亮灭
│   │   ├── Task1_Timer_LED.uvprojx
│   │   ├── main.c
│   │   ├── startup_stm32f10x_md.s
│   │   ├── system_stm32f10x.c/h
│   │   ├── stm32f10x.h / stm32f10x_conf.h
│   │   ├── CMSIS/                # core_cm3.h 等
│   │   └── stm32f10x_rcc/gpio/tim.c、misc.c
│   └── Task2_PWM_Breathing/      # 内容二：PWM 呼吸灯
│       └── ……（结构同上）
├── firmware/                     # 已编译的 hex，可直接烧录
│   ├── Task1_Timer_LED.hex
│   └── Task2_PWM_Breathing.hex
├── figures/                      # 实验报告插图
├── 实验报告.md                   # 报告源码
├── 实验报告.pdf                  # 报告 PDF
├── 博客.md                       # 博客文章源码
└── README.md                     # 本文件
```

## 三、如何编译与下载

1. 安装 **Keil MDK 5** 和 **Keil.STM32F1xx_DFP** 器件包（本工程基于 2.4.1）。
2. 打开对应工程：
   - 内容一：`code/Task1_Timer_LED/Task1_Timer_LED.uvprojx`
   - 内容二：`code/Task2_PWM_Breathing/Task2_PWM_Breathing.uvprojx`
3. 点击 **Build（F7）**，应显示 `0 Error(s), 0 Warning(s)`。
4. 下载：
   - 用 ST-Link 时，在 **Options for Target → Debug** 选择 **ST-Link Debugger**，勾选 **Reset and Run**，点击 **Download（F8）**；
   - 也可以直接用 STM32 ST-LINK Utility / STM32CubeProgrammer 烧录 `firmware/` 下的 hex。

> 两个工程都是自包含的：启动文件、CMSIS、标准外设库源码都在工程目录内，不依赖 CubeMX 或本机是否安装 SPL。

## 四、用 Keil 逻辑分析仪看波形

1. 打开工程并编译通过。
2. **Options for Target → Debug** → 选择 **Use Simulator**；
   - **Dialog DLL**：`DARMSTM.DLL`
   - **Parameter**：`-pSTM32F103C8`
3. **Ctrl+F5** 进入调试。
4. **View → Analysis Windows → Logic Analyzer**。
5. **Setup → New**：
   - Display Expression：`GPIOA->ODR`
   - And Mask：`0x0080`
   - Shift Right：`7`
   - （也可直接填 `PORTA.7`）
6. **F5** 运行，用 Zoom 和 Cursor 测量。

**预期：**

- 内容一：PA7 高低各 1.000s，周期 2.000s，0.5Hz；
- 内容二：PA7 周期 1.000ms（1kHz），高电平宽度随呼吸节奏在 0~1ms 变化。

## 五、报告与博客

- 报告：`实验报告.md` / `实验报告.pdf`
- 博客：`博客.md`。发布到 CSDN/博客园后，把地址填到学习通。
- 报告插图在 `figures/` 下，PDF 中已包含。

## 六、提交清单

- [x] 两个 Keil 工程源码（可直接编译）
- [x] 已编译 hex（`firmware/`）
- [x] 实验报告 Markdown
- [x] 实验报告 PDF
- [x] 博客文章源码
- [x] 项目完整 zip：`../STM32定时器PWM实验.zip`
- [x] GitHub 仓库：https://github.com/renew111/STM32-Timer-PWM-Experiments
- [ ] 学习通提交（zip + 报告 md/pdf + 博客地址 + GitHub 地址）


## 七、GitHub

仓库地址：**https://github.com/renew111/STM32-Timer-PWM-Experiments**

账号：renew111（HTTPS）
