// Copyright 2025 Christopher Courtney, aka Drashna Jael're  (@drashna) <drashna@live.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include_next <mcuconf.h>

#if defined(KEYBOARD_crkbd) || defined(KEYBOARD_splitkb_aurora_corne)

#    undef STM32_PWM_USE_TIM15
#    define STM32_PWM_USE_TIM15 TRUE
#    undef STM32_PWM_USE_TIM3
#    define STM32_PWM_USE_TIM3 FALSE
#    undef STM32_GPT_USE_TIM15
#    define STM32_GPT_USE_TIM15 FALSE

#    undef STM32_SERIAL_USE_USART1
#    define STM32_SERIAL_USE_USART1 TRUE

#    undef STM32_WDG_USE_IWDG
#    define STM32_WDG_USE_IWDG TRUE
#endif
