// Copyright 2025 Christopher Courtney, aka Drashna Jael're  (@drashna) <drashna@live.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <stdint.h>

void init_display_st7789_76x284(void);
void st7789_76x284_display_power(bool on);
void st7789_76x284_draw_user(void);
void st7789_76x284_display_shutdown(bool jump_to_bootloader);
void init_display_st7789_76x284_rotation(void);
void init_display_st7789_76x284_inversion(void);
