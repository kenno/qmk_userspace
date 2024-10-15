// Copyright 2024 Christopher Courtney, aka Drashna Jael're  (@drashna) <drashna@live.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <stdint.h>

void init_display_ili9488(void);
void ili9488_display_power(bool on);
void ili9488_draw_user(void);
void ili9488_display_shutdown(bool jump_to_bootloader);
void init_display_ili9488_rotation(void);
void init_display_ili9488_inversion(void);
