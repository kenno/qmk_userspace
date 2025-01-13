// Copyright 2024 Christopher Courtney, aka Drashna Jael're  (@drashna) <drashna@live.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include_next <mcuconf.h>

#if HAL_USE_WDG == TRUE
#    undef STM32_WDG_USE_IWDG
#    define STM32_WDG_USE_IWDG TRUE
#endif

#if HAL_USE_I2C == TRUE
#    undef STM32_I2C_BUSY_TIMEOUT
#    define STM32_I2C_BUSY_TIMEOUT 10
#endif

#ifdef USE_USB_OTG_HS_PORT
#    undef STM32_USB_USE_OTG1
#    define STM32_USB_USE_OTG1 FALSE
#    undef STM32_USB_USE_OTG2
#    define STM32_USB_USE_OTG2 TRUE
#endif
