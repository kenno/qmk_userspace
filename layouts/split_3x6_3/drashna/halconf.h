// Copyright 2025 Christopher Courtney, aka Drashna Jael're  (@drashna) <drashna@live.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#if defined(KEYBOARD_crkbd) || defined(KEYBOARD_splitkb_aurora_corne)
#    define HAL_USE_PWM    TRUE
#    define HAL_USE_SERIAL TRUE
#endif

#include_next <halconf.h>
