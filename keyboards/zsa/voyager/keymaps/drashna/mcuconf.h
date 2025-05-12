// Copyright 2025 Christopher Courtney, aka Drashna Jael're  (@drashna) <drashna@live.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include_next <mcuconf.h>

#undef STM32_WDG_USE_IWDG
#define STM32_WDG_USE_IWDG TRUE
#undef STM32_IWDG_IS_WINDOWED
#define STM32_IWDG_IS_WINDOWED TRUE

#undef STM32_I2C_BUSY_TIMEOUT
#define STM32_I2C_BUSY_TIMEOUT 10
