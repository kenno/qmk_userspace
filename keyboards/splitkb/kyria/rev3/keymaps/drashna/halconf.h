// Copyright 2025 Christopher Courtney, aka Drashna Jael're  (@drashna) <drashna@live.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#if defined(KEYBOARD_splitkb_kyria_rev3)
#    define HAL_USE_I2C    TRUE
#    define HAL_USE_PWM    TRUE
#    define HAL_USE_SERIAL TRUE
#endif

#include_next <halconf.h>
