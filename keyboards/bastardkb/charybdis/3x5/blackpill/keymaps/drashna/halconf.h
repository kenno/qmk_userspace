// Copyright 2025 Christopher Courtney, aka Drashna Jael're  (@drashna) <drashna@live.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#if defined(KEYBOARD_bastardkb_charybdis_3x5_blackpill)
#    define HAL_USE_PWM     TRUE
#    define HAL_USE_SERIAL  TRUE
#    define HAL_USE_I2C     TRUE
#    define HAL_USE_SPI     TRUE
#    define SPI_USE_WAIT    TRUE
#    define SPI_SELECT_MODE SPI_SELECT_MODE_PAD
#endif

#include_next <halconf.h>
