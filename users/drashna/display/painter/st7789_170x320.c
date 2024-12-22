// Copyright 2024 splitkb.com (support@splitkb.com)
// SPDX-License-Identifier: GPL-2.0-or-later

#include "drashna_runtime.h"

#include "qp.h"
#include "qp_comms.h"
#include "qp_st77xx_opcodes.h"
#include "display/painter/painter.h"
#include "display/painter/st7789_170x320.h"

#ifndef ST7789_CS_PIN
#    define ST7789_CS_PIN DISPLAY_CS_PIN
#endif // ST7789_CS_PIN
#ifndef ST7789_DC_PIN
#    define ST7789_DC_PIN DISPLAY_DC_PIN
#endif // ST7789_DC_PIN
#ifndef ST7789_RST_PIN
#    define ST7789_RST_PIN DISPLAY_RST_PIN
#endif // ST7789_RST_PIN
#ifndef ST7789_SPI_DIVIDER
#    define ST7789_SPI_DIVIDER DISPLAY_SPI_DIVIDER
#endif // ST7789_SPI_DIVIDER

static painter_device_t st7789_display;

static bool has_run = false, forced_reinit = false;

void init_display_st7789_170x320_inversion(void) {
    qp_comms_start(st7789_display);
    qp_comms_command(st7789_display, userspace_config.display.inverted ? ST77XX_CMD_INVERT_OFF : ST77XX_CMD_INVERT_ON);
    qp_comms_stop(st7789_display);
    if (has_run) {
        forced_reinit = true;
    }
}

void init_display_st7789_170x320_rotation(void) {
    uint16_t width;
    uint16_t height;

    qp_init(st7789_display, userspace_config.display.rotation ? QP_ROTATION_0 : QP_ROTATION_180);
    qp_set_viewport_offsets(st7789_display, 35, 0);
    qp_get_geometry(st7789_display, &width, &height, NULL, NULL, NULL);
    qp_clear(st7789_display);
    qp_rect(st7789_display, 0, 0, width - 1, height - 1, 0, 0, 0, true);

    // if needs inversion, run it only afetr the clear and rect functions or otherwise it won't work
    init_display_st7789_170x320_inversion();

    qp_power(st7789_display, true);
    qp_flush(st7789_display);
    if (has_run) {
        forced_reinit = true;
    }
    has_run = true;
}

void init_display_st7789_170x320(void) {
    st7789_display =
        qp_st7789_make_spi_device(170, 320, ST7789_CS_PIN, ST7789_DC_PIN, ST7789_RST_PIN, ST7789_SPI_DIVIDER, 3);
    init_display_st7789_170x320_rotation();

    qp_flush(st7789_display);
}

void st7789_170x320_display_power(bool on) {
    qp_power(st7789_display, on);
}

__attribute__((weak)) void st7789_170x320_draw_user(void) {
    uint16_t width;
    uint16_t height;
    qp_get_geometry(st7789_display, &width, &height, NULL, NULL, NULL);

    painter_render_menu(st7789_display, font_oled, 0, 0, width, height, false);
    qp_flush(st7789_display);
}

void st7789_170x320_display_shutdown(bool jump_to_bootloader) {
    st7789_170x320_display_power(true);
    painter_render_shutdown(st7789_display, jump_to_bootloader);
}
