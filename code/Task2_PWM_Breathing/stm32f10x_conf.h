/**
 ******************************************************************************
 * @file    stm32f10x_conf.h
 * @brief   标准外设库配置文件：按需包含用到的外设驱动头文件
 ******************************************************************************
 */

#ifndef __STM32F10x_CONF_H
#define __STM32F10x_CONF_H

/* 本工程用到的外设驱动 */
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "misc.h"

#ifdef  USE_FULL_ASSERT
  #define assert_param(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))
  void assert_failed(uint8_t* file, uint32_t line);
#else
  #define assert_param(expr) ((void)0)
#endif

#endif /* __STM32F10x_CONF_H */
