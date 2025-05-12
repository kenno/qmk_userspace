// Copyright 2025 Christopher Courtney, aka Drashna Jael're  (@drashna) <drashna@live.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include_next <mcuconf.h>

#if defined(KEYBOARD_bastardkb_charybdis_3x5_blackpill)
#    undef STM32_I2C_USE_I2C1
#    define STM32_I2C_USE_I2C1 TRUE

// #undef STM32_I2C_I2C1_RX_DMA_STREAM
// #define STM32_I2C_I2C1_RX_DMA_STREAM STM32_DMA_STREAM_ID(1, 0)
#    undef STM32_I2C_I2C1_TX_DMA_STREAM
#    define STM32_I2C_I2C1_TX_DMA_STREAM STM32_DMA_STREAM_ID(1, 1)

#    undef STM32_PWM_USE_TIM2
#    define STM32_PWM_USE_TIM2 TRUE

#    undef STM32_PWM_USE_TIM3
#    define STM32_PWM_USE_TIM3 TRUE

#    undef STM32_SPI_USE_SPI1
#    define STM32_SPI_USE_SPI1 TRUE

#    undef STM32_SERIAL_USE_USART2
#    define STM32_SERIAL_USE_USART2 TRUE

#    undef STM32_ST_USE_TIMER
#    define STM32_ST_USE_TIMER 5

#    undef STM32_WDG_USE_IWDG
#    define STM32_WDG_USE_IWDG TRUE
#endif
