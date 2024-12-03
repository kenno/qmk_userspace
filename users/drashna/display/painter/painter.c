// Copyright 2024 Christopher Courtney, aka Drashna Jael're  (@drashna) <drashna@live.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display/painter/painter.h"
#include "display/menu/menu.h"
#include <stdio.h>
#include "drashna_names.h"
#include "drashna_runtime.h"
#include "drashna_util.h"
#include "version.h"

#ifdef SPLIT_KEYBOARD
#    include "split_util.h"
#endif // SPLIT_KEYBOARD
#if defined(QUANTUM_PAINTER_ILI9341_ENABLE) && defined(CUSTOM_QUANTUM_PAINTER_ILI9341)
#    include "display/painter/ili9341_display.h"
#endif // QUANTUM_PAINTER_ILI9341_ENABLE && CUSTOM_QUANTUM_PAINTER_ILI9341
#ifdef RTC_ENABLE
#    include "features/rtc/rtc.h"
#endif // RTC_ENABLE
#ifdef LAYER_MAP_ENABLE
#    include "features/layer_map.h"
#endif

painter_font_handle_t font_thintel, font_mono, font_oled;

painter_image_handle_t frame_top, frame_bottom;
painter_image_handle_t lock_caps_on, lock_caps_off;
painter_image_handle_t lock_num_on, lock_num_off;
painter_image_handle_t lock_scrl_on, lock_scrl_off;
painter_image_handle_t windows_logo, apple_logo, linux_logo;
painter_image_handle_t shift_icon, control_icon, alt_icon, command_icon, windows_icon;
painter_image_handle_t mouse_icon;
painter_image_handle_t gamepad_icon;
painter_image_handle_t qmk_banner;
painter_image_handle_t akira_explosion;
painter_image_handle_t nyan_cat;
deferred_token         nyan_token = INVALID_DEFERRED_TOKEN;

painter_image_array_t screen_saver_image[] = {
    [__COUNTER__] = {gfx_samurai_cyberpunk_minimal_dark_8k_b3_240x320, "Samurai Cyberpunk"},
    [__COUNTER__] = {gfx_anime_girl_jacket_240x320, "Anime Girl"},
    [__COUNTER__] = {gfx_asuka_240x320, "Asuka"},
    [__COUNTER__] = {gfx_neon_genesis_evangelion_initial_machine_00_240x320, "Eva Unit 00"},
    [__COUNTER__] = {gfx_neon_genesis_evangelion_initial_machine_01_240x320, "Eva Unit 01"},
    [__COUNTER__] = {gfx_neon_genesis_evangelion_initial_machine_02_240x320, "Eva Unit 02"},
    [__COUNTER__] = {gfx_neon_genesis_evangelion_initial_machine_03_240x320, "Eva Unit 03"},
    [__COUNTER__] = {gfx_neon_genesis_evangelion_initial_machine_04_240x320, "Eva Unit 04"},
    [__COUNTER__] = {gfx_neon_genesis_evangelion_initial_machine_05_240x320, "Eva Unit 05"},
    [__COUNTER__] = {gfx_neon_genesis_evangelion_initial_machine_06_240x320, "Eva Unit 06"},
};
const uint8_t screensaver_image_size = __COUNTER__;

__attribute__((weak)) bool painter_render_side(void) {
    return is_keyboard_master();
}

/**
 * @brief Initializes the assets used by the painter module.
 *
 * This function loads various fonts and images into memory for use in the
 * painter module. The assets include fonts, frames, logos, and icons.
 *
 */
void painter_init_assets(void) {
    font_thintel = qp_load_font_mem(font_thintel15);
    font_mono    = qp_load_font_mem(font_ProggyTiny15);
    font_oled    = qp_load_font_mem(font_oled_font);

    frame_top    = qp_load_image_mem(gfx_frame_top);
    frame_bottom = qp_load_image_mem(gfx_frame_bottom);

    windows_logo = qp_load_image_mem(gfx_windows_logo);
    apple_logo   = qp_load_image_mem(gfx_apple_logo);
    linux_logo   = qp_load_image_mem(gfx_linux_logo);

    shift_icon   = qp_load_image_mem(gfx_shift_icon);
    control_icon = qp_load_image_mem(gfx_control_icon);
    alt_icon     = qp_load_image_mem(gfx_alt_icon);
    command_icon = qp_load_image_mem(gfx_command_icon);
    windows_icon = qp_load_image_mem(gfx_windows_icon);

    mouse_icon      = qp_load_image_mem(gfx_mouse_icon);
    gamepad_icon    = qp_load_image_mem(gfx_gamepad_24x24);
    qmk_banner      = qp_load_image_mem(gfx_qmk_powered_by);
    akira_explosion = qp_load_image_mem(gfx_akira_explosion);

    nyan_cat = qp_load_image_mem(gfx_nyan_cat);
}

/**
 * @brief Renders RTC Time to display
 *
 * @param device screen to render to
 * @param font font to use
 * @param x x position to start rendering
 * @param y y position to start rendering
 * @param display_width maximum width for rendering
 * @param force_redraw do we need to redraw regardless of time
 * @param rtc_timer timer to check for redraw
 * @param hsv hsv value to render with
 */
void painter_render_rtc_time(painter_device_t device, painter_font_handle_t font, uint16_t x, uint16_t y,
                             uint16_t display_width, bool force_redraw, uint16_t* rtc_timer, hsv_t* hsv) {
#ifdef RTC_ENABLE

    bool rtc_redraw = false;
    if (timer_elapsed(*rtc_timer) > 125 && rtc_is_connected()) {
        *rtc_timer = timer_read();
        rtc_redraw = true;
    }
    if (force_redraw || rtc_redraw) {
        char buf[40] = {0};
        if (rtc_is_connected()) {
            snprintf(buf, sizeof(buf), "RTC Date/Time: %s", rtc_read_date_time_str());
        } else {
            snprintf(buf, sizeof(buf), "RTC Device Not Connected");
        }

        uint16_t total_width = display_width - 6 - x;
        uint16_t title_width = qp_textwidth(font, buf);
        if (title_width > (total_width)) {
            title_width = total_width;
        }
        uint16_t title_xpos = (total_width - title_width) / 2 + x;

        qp_drawtext_recolor(device, title_xpos, y, font, truncate_text(buf, total_width, font, false, false), hsv->h,
                            hsv->s, hsv->v, 0, 0, 0);
    }
#endif // RTC_ENABLE
}

/**
 * @brief Render the console log to the display
 *
 * @param device device to render to
 * @param font font to render with
 * @param x x position to start rendering
 * @param y y position to start rendering
 * @param display_width maximum width for rendering
 * @param force_redraw do we forcibly redraw the console log
 * @param hsv colors to render with
 * @param start start line to render
 * @param end last line to render
 */
void painter_render_console(painter_device_t device, painter_font_handle_t font, uint16_t x, uint16_t y,
                            uint16_t display_width, bool force_redraw, hsv_t* hsv, uint8_t start, uint8_t end) {
    if (console_log_needs_redraw || force_redraw) {
        for (uint8_t i = start; i < end; i++) {
            uint16_t xpos =
                x + qp_drawtext_recolor(device, x, y, font, logline_ptrs[i], hsv->h, hsv->s, hsv->v, 0, 0, 0);
            qp_rect(device, xpos, y, display_width, y + font->line_height, 0, 0, 0, true);
            y += font->line_height + 4;
        }
        console_has_redrawn = true;
    }
}

/**
 * @brief Render the matrix scan rate to the display
 *
 * @param device device to render to
 * @param font font to render with
 * @param x x position to start rendering
 * @param y y position to start rendering
 * @param force_redraw do we forcibly redraw the scan rate
 * @param curr_hsv painter colors
 */
void painter_render_scan_rate(painter_device_t device, painter_font_handle_t font, uint16_t x, uint16_t y,
                              bool force_redraw, dual_hsv_t* curr_hsv) {
    static uint32_t last_scan_rate = 0;
    if (last_scan_rate != get_matrix_scan_rate() || force_redraw) {
        last_scan_rate = get_matrix_scan_rate();
        char buf[6]    = {0};
        x += qp_drawtext_recolor(device, x, y, font, "SCANS: ", curr_hsv->primary.h, curr_hsv->primary.s,
                                 curr_hsv->primary.v, 0, 0, 0);
        snprintf(buf, sizeof(buf), "%5lu", get_matrix_scan_rate());
        qp_drawtext_recolor(device, x, y, font, buf, curr_hsv->secondary.h, curr_hsv->secondary.s,
                            curr_hsv->secondary.v, 0, 0, 0);
    }
}

/**
 * @brief Render the RGB Mode and HSV to the display, with a fancy swatch for the HSV color
 *
 * @param device device to render to
 * @param font font to render with
 * @param x x position to start rendering
 * @param y y position to start rendering
 * @param force_redraw do we forcibly redraw the RGB config
 * @param curr_hsv painter colors
 * @param get_rgb_mode
 * @param get_rgb_hsv
 * @param matrix
 */
void painter_render_rgb(painter_device_t device, painter_font_handle_t font, uint16_t x, uint16_t y, bool force_redraw,
                        dual_hsv_t* curr_hsv, const char* title, const char* (*get_rgb_mode)(void),
                        hsv_t (*get_rgb_hsv)(void), bool is_enabled, uint8_t max_val) {
#if defined(RGB_MATRIX_ENABLE) || defined(RGBLIGHT_ENABLE)
    char buf[22] = {0};
    if (force_redraw) {
        hsv_t rgb_hsv = get_rgb_hsv();
        qp_drawtext_recolor(device, x, y, font, title, curr_hsv->primary.h, curr_hsv->primary.s, curr_hsv->primary.v, 0,
                            0, 0);
        y += font->line_height + 4;
        snprintf(buf, sizeof(buf), "%21s", truncate_text(is_enabled ? get_rgb_mode() : "Off", 125, font, false, false));
        qp_drawtext_recolor(device, x + 125 - qp_textwidth(font, buf), y, font, buf, curr_hsv->secondary.h,
                            curr_hsv->secondary.s, curr_hsv->secondary.v, 0, 0, 0);

        y += font->line_height + 4;
        x += qp_drawtext_recolor(device, x, y, font, "HSV: ", curr_hsv->primary.h, curr_hsv->primary.s,
                                 curr_hsv->primary.v, 0, 0, 0);
        if (is_enabled) {
            snprintf(buf, sizeof(buf), "%3d, %3d, %3d", rgb_hsv.h, rgb_hsv.s, rgb_hsv.v);
        } else {
            snprintf(buf, sizeof(buf), "%13s", "Off");
        }
        x += qp_drawtext_recolor(device, x, y, font, buf, curr_hsv->secondary.h, curr_hsv->secondary.s,
                                 curr_hsv->secondary.v, 0, 0, 0);
        qp_rect(device, x + 6, y - 1, x + 6 + 10, y + 9, rgb_hsv.h, rgb_hsv.s,
                is_enabled ? (uint8_t)(rgb_hsv.v * 0xFF / max_val) : 0, true);
    }
#endif // defined(RGB_MATRIX_ENABLE)
}

/**
 * @brief Render the Caps Lock, Scroll Lock and Num Lock states to the display
 *
 * @param device device to render to
 * @param font font to draw with
 * @param x x position to start rendering
 * @param y y position to start rendering
 * @param force_redraw do we forcible redraw the lock states
 * @param curr_hsv painter hsv values
 * @param disabled_val disabled entry render value
 */
void painter_render_lock_state(painter_device_t device, painter_font_handle_t font, uint16_t x, uint16_t y,
                               bool force_redraw, dual_hsv_t* curr_hsv, uint8_t disabled_val) {
    static led_t last_led_state = {0};
    if (force_redraw || last_led_state.raw != host_keyboard_led_state().raw) {
        last_led_state.raw = host_keyboard_led_state().raw;
        qp_drawtext_recolor(device, x, y, font, "CAPS",
                            last_led_state.caps_lock ? curr_hsv->secondary.h : curr_hsv->primary.h,
                            last_led_state.caps_lock ? curr_hsv->secondary.s : curr_hsv->primary.s,
                            last_led_state.caps_lock ? curr_hsv->primary.v : disabled_val, 0, 0, 0);
        y += font->line_height + 2;
        qp_drawtext_recolor(device, x, y, font, "SCRL",
                            last_led_state.scroll_lock ? curr_hsv->secondary.h : curr_hsv->primary.h,
                            last_led_state.scroll_lock ? curr_hsv->secondary.s : curr_hsv->primary.s,
                            last_led_state.scroll_lock ? curr_hsv->primary.v : disabled_val, 0, 0, 0);
        y += font->line_height + 2;
        qp_drawtext_recolor(device, x, y, font, " NUM",
                            last_led_state.num_lock ? curr_hsv->secondary.h : curr_hsv->primary.h,
                            last_led_state.num_lock ? curr_hsv->secondary.s : curr_hsv->primary.s,
                            last_led_state.num_lock ? curr_hsv->primary.v : disabled_val, 0, 0, 0);
    }
}

/**
 * @brief Render the keylogger to the display
 *
 * @param device device to render to
 * @param font font to render with
 * @param x x position to start rendering
 * @param y y position to start rendering
 * @param force_redraw do we forcibly redraw the keylogger
 * @param curr_hsv painter colors
 */
void painter_render_wpm(painter_device_t device, painter_font_handle_t font, uint16_t x, uint16_t y, bool force_redraw,
                        dual_hsv_t* curr_hsv) {
#ifdef WPM_ENABLE
    static uint32_t last_wpm_update = 0;
    static char     buf[4]          = {0};
    if (force_redraw || last_wpm_update != get_current_wpm()) {
        last_wpm_update = get_current_wpm();
        x += qp_drawtext_recolor(device, x, y, font, "WPM:     ", curr_hsv->primary.h, curr_hsv->primary.s,
                                 curr_hsv->primary.v, 0, 0, 0);
        snprintf(buf, sizeof(buf), "%3u", get_current_wpm());
        qp_drawtext_recolor(device, x, y, font, buf, curr_hsv->secondary.h, curr_hsv->secondary.s,
                            curr_hsv->secondary.v, 0, 0, 0);
    }
#endif // WPM_ENABLE
}

/**
 * @brief Render the haptic feedback settings to the display
 *
 * @param device screen to render to
 * @param font font to render with
 * @param x x position to start rendering
 * @param y y position to start rendering
 * @param force_redraw do we forcibly redraw the haptic feedback settings
 * @param curr_hsv colors to render with
 */
void painter_render_haptic(painter_device_t device, painter_font_handle_t font, uint16_t x, uint16_t y,
                           bool force_redraw, dual_hsv_t* curr_hsv) {
#if defined(HAPTIC_ENABLE)
    char                   buf[22]     = {0};
    static haptic_config_t temp_config = {0};
    extern haptic_config_t haptic_config;
    if (force_redraw || haptic_config.raw != temp_config.raw) {
        temp_config.raw = haptic_config.raw;
        uint16_t temp_x = x;
        temp_x += qp_drawtext_recolor(device, temp_x, y, font, "Haptic Feedback: ", curr_hsv->primary.h,
                                      curr_hsv->primary.s, curr_hsv->primary.v, 0, 0, 0) +
                  4;
        if (haptic_get_enable()) {
            switch (haptic_get_feedback()) {
                case 0:
                    snprintf(buf, sizeof(buf), "%8s", "Press");
                    break;
                case 1:
                    snprintf(buf, sizeof(buf), "%8s", "Both");
                    break;
                case 2:
                    snprintf(buf, sizeof(buf), "%8s", "Release");
                    break;
            }
        } else {
            snprintf(buf, sizeof(buf), "%8s", "Off");
        }
        qp_drawtext_recolor(device, temp_x, y, font, buf, curr_hsv->secondary.h, curr_hsv->secondary.s,
                            curr_hsv->secondary.v, 0, 0, 0);

        y += font->line_height + 4;
        temp_x = x +
                 qp_drawtext_recolor(device, x, y, font, "Mode:", curr_hsv->primary.h, curr_hsv->primary.s,
                                     curr_hsv->primary.v, 0, 0, 0) +
                 4;
        snprintf(buf, sizeof(buf), "%20s",
                 truncate_text(get_haptic_drv2605l_effect_name(haptic_get_mode()), 120, font, true, false));
        qp_drawtext_recolor(device, temp_x, y, font, buf, curr_hsv->secondary.h, curr_hsv->secondary.s,
                            curr_hsv->secondary.v, 0, 0, 0);
    }
#endif // defined(HAPTIC_ENABLE)
}

/**
 * @brief Renders a TOTP (Time-based One-Time Password) on the specified painter device.
 *
 * @param device The painter device to render on.
 * @param font The font handle to use for rendering the text.
 * @param x The x-coordinate where the rendering should start.
 * @param y The y-coordinate where the rendering should start.
 * @param width The width of the area to render the TOTP.
 * @param force_redraw A boolean flag indicating whether to force a redraw of the TOTP.
 * @param curr_hsv A pointer to the current HSV color values.
 * @param wide_load Render as  "name XXXXXX" if false, or "name: XXX XXX" if true
 */
void painter_render_totp(painter_device_t device, painter_font_handle_t font, uint16_t x, uint16_t y, uint16_t width,
                         bool force_redraw, dual_hsv_t* curr_hsv, bool wide_load) {
#if defined(RTC_ENABLE) && defined(RTC_TOTP_ENABLE) && __has_include("rtc_secrets.h")
#    include "rtc_secrets.h"
    uint32_t    get_totp_code(const uint8_t* hmackey, const uint8_t keylength, const uint32_t timestep);
    static bool is_rtc_connected = false;
    bool        totp_redraw = false, draw_red_redraw = false;
    if (rtc_is_connected() != is_rtc_connected) {
        is_rtc_connected = rtc_is_connected();
        totp_redraw      = true;
    }
    if (rtc_read_time_struct().unixtime % 30 == 1) {
        totp_redraw = true;
    }
    if (rtc_read_time_struct().unixtime % 30 == 23) {
        totp_redraw = draw_red_redraw = true;
    }
    if (force_redraw || totp_redraw) {
        uint16_t temp_x     = x;
        char     buf[20]    = {0};
        uint16_t text_width = qp_textwidth(font, wide_load ? "WWWWWW: WWW WWW" : "WWWWWW WWWWWW") + 10;
        for (uint8_t i = 0; i < ARRAY_SIZE(totp_pairs); i++) {
            uint32_t code = rtc_is_connected()
                                ? get_totp_code(totp_pairs[i].hmacKey, totp_pairs[i].key_length, totp_pairs[i].timestep)
                                : 0;
            snprintf(buf, sizeof(buf), "%6s%s", totp_pairs[i].name, wide_load ? ": " : " ");
            if ((temp_x + text_width) > (width)) {
                temp_x = x;
                y += font->line_height + 3;
            }
            temp_x += qp_drawtext_recolor(device, temp_x, y, font, buf, curr_hsv->primary.h, curr_hsv->primary.s,
                                          curr_hsv->primary.v, 0, 0, 0);

            if (wide_load) {
                snprintf(buf, sizeof(buf), "%03ld %03ld", code / 1000, code % 1000);
            } else {
                snprintf(buf, sizeof(buf), "%06ld", code);
            }
            temp_x +=
                qp_drawtext_recolor(device, temp_x, y, font, buf, draw_red_redraw ? 0 : curr_hsv->secondary.h,
                                    draw_red_redraw ? 170 : curr_hsv->secondary.s, curr_hsv->secondary.v, 0, 0, 0) +
                10;
        }
    }
#endif
}

/**
 * @brief Renders a frame on the painter device.
 *
 * This function is responsible for rendering a frame on the specified painter device.
 * It uses the provided font for the title and can render on either the right or left side
 * of the display, with an optional offset.
 *
 * @param device The painter device to render the frame on.
 * @param font_title The font handle to use for the title.
 * @param right_side A boolean indicating whether to render on the right side (true) or left side (false).
 * @param offset An optional offset value for the rendering position.
 * @param color_side A boolean indicating whether to render the frame in primary (true) or secondary (false) color.
 */
void painter_render_frame(painter_device_t device, painter_font_handle_t font_title, bool right_side, uint16_t offset,
                          bool color_side) {
    painter_image_handle_t frame_top    = qp_load_image_mem(gfx_frame_top),
                           frame_bottom = qp_load_image_mem(gfx_frame_bottom);

    const uint16_t max_width = 240;
    uint16_t       xpos      = offset;
    uint16_t       width     = offset + max_width;
    uint16_t       height    = 320;

    hsv_t hsv = painter_get_hsv(color_side);

    // frame top
    qp_drawimage_recolor(device, xpos + 1, 2, frame_top, hsv.h, hsv.s, hsv.v, 0, 0, 0);
    // lines for frame sides
    qp_line(device, xpos + 1, frame_top->height, xpos + 1, height - frame_bottom->height, hsv.h, hsv.s, hsv.v);
    qp_line(device, width - 2, frame_top->height, width - 2, height - frame_bottom->height, hsv.h, hsv.s, hsv.v);

    // horizontal line below rgb
    qp_line(device, xpos + 80, 54, xpos + 237, 54, hsv.h, hsv.s, hsv.v);

    // caps lock horizontal line
    qp_line(device, xpos + 208, 16, xpos + 208, 54, hsv.h, hsv.s, hsv.v);

    if (right_side) {
        // vertical lines next to scan rate + wpm + pointing
        qp_line(device, xpos + 80, 16, xpos + 80, 106, hsv.h, hsv.s, hsv.v);
        // horizontal line below scan rate + wpm
        qp_line(device, xpos + 2, 43, xpos + 80, 43, hsv.h, hsv.s, hsv.v);

        // lines for unicode typing mode and mode
        qp_line(device, xpos + 80, 80, xpos + 237, 80, hsv.h, hsv.s, hsv.v);
        qp_line(device, xpos + 149, 80, xpos + 149, 106, hsv.h, hsv.s, hsv.v);

        // lines for mods and OS detection
        qp_line(device, xpos + 2, 107, xpos + 237, 107, hsv.h, hsv.s, hsv.v);
        qp_line(device, xpos + 155, 107, xpos + 155, 122, hsv.h, hsv.s, hsv.v);
        // lines for autocorrect and layers
        qp_line(device, xpos + 2, 122, xpos + 237, 122, hsv.h, hsv.s, hsv.v);
        qp_line(device, xpos + 121, 122, xpos + 121, 171, hsv.h, hsv.s, hsv.v);
        qp_line(device, xpos + 186, 122, xpos + 186, 171, hsv.h, hsv.s, hsv.v);
    } else {
        // horizontal line below scan rate + wpm
        qp_line(device, xpos + 2, 31, xpos + 80, 31, hsv.h, hsv.s, hsv.v);
        // vertical line next to pointing device block
        qp_line(device, xpos + 80, 16, xpos + 80, 170, hsv.h, hsv.s, hsv.v);

        // lines for unicode typing mode and mode
        qp_line(device, xpos + 80, 80, xpos + 237, 80, hsv.h, hsv.s, hsv.v);
    }
    // line above menu block
    qp_line(device, xpos + 2, 171, xpos + 237, 171, hsv.h, hsv.s, hsv.v);
    // line above rtc
    qp_line(device, xpos + 2, 292, xpos + 237, 292, hsv.h, hsv.s, hsv.v);
    // frame bottom
    qp_drawimage_recolor(device, xpos + 1, height - frame_bottom->height, frame_bottom, hsv.h, hsv.s, hsv.v, 0, 0, 0);

    char title[50] = {0};
    snprintf(title, sizeof(title), "%s", PRODUCT);
    uint16_t title_width = qp_textwidth(font_title, title);
    if (title_width > (max_width - 55)) {
        title_width = max_width;
    }
    uint16_t title_xpos = (max_width - title_width) / 2;
    qp_drawtext_recolor(device, xpos + title_xpos, 4, font_title,
                        truncate_text(title, title_width, font_title, false, false), 0, 0, 0, hsv.h, hsv.s, hsv.v);

    qp_close_image(frame_top);
    qp_close_image(frame_bottom);
}

/**
 * @brief Renders a menu block on the display.
 *
 * @param device The painter device to render on.
 * @param font The font handle to use for rendering text.
 * @param x The x-coordinate of the top-left corner of the menu block.
 * @param y The y-coordinate of the top-left corner of the menu block.
 * @param width The width of the menu block.
 * @param height The height of the menu block.
 * @param force_redraw If true, forces the menu block to be redrawn.
 * @param curr_hsv Pointer to the current HSV color values.
 * @param is_left If true, indicates that the menu block is on the left side rendering.
 */
void painter_render_menu_block(painter_device_t device, painter_font_handle_t font, uint16_t x, uint16_t y,
                               uint16_t width, uint16_t height, bool force_redraw, dual_hsv_t* curr_hsv, bool is_left) {
    static bool force_full_block_redraw = false;
#ifdef SPLIT_KEYBOARD
    bool           should_render_this_side = userspace_config.display.menu_render_side & (1 << (uint8_t)!is_left);
    static uint8_t last_menu_side          = 0xFF;
    if (last_menu_side != userspace_config.display.menu_render_side) {
        last_menu_side          = userspace_config.display.menu_render_side;
        force_full_block_redraw = true;
    }
    // if slave side can't be detected, we need to force render, just to be safe
    if (!is_transport_connected()) {
        should_render_this_side = true;
    }
#else  // SPLIT_KEYBOARD
    const bool should_render_this_side = true;
#endif // SPLIT_KEYBOARD

    if (should_render_this_side && painter_render_menu(device, font, x, y, width, height)) {
        force_full_block_redraw = true;
        if (nyan_token != INVALID_DEFERRED_TOKEN) {
            qp_stop_animation(nyan_token);
            nyan_token = INVALID_DEFERRED_TOKEN;
        }
    } else {
        bool     block_redraw = false;
        uint16_t surface_ypos = y + 2, surface_xpos = x + 3;

        uint8_t current_display_mode = is_left ? userspace_config.display.painter.display_mode_master
                                               : userspace_config.display.painter.display_mode_slave;

        static uint8_t last_display_mode[2] = {0xFF};
        if (last_display_mode[is_left] != current_display_mode) {
            last_display_mode[is_left] = current_display_mode;
            force_full_block_redraw    = true;
        }

        if (force_full_block_redraw || force_redraw) {
            qp_rect(device, x, y, width - 1, height - 1, 0, 0, 0, true);
            force_full_block_redraw = false;
            block_redraw            = true;
        }
        if (nyan_token != INVALID_DEFERRED_TOKEN && current_display_mode != 3) {
            qp_stop_animation(nyan_token);
            nyan_token = INVALID_DEFERRED_TOKEN;
        }

        switch (current_display_mode) {
            case 0:
                painter_render_console(device, font, x + 2, surface_ypos, width, force_redraw || block_redraw,
                                       &curr_hsv->primary, DISPLAY_CONSOLE_LOG_LINE_START,
                                       DISPLAY_CONSOLE_LOG_LINE_NUM);
                break;
            case 1:
                if (force_redraw || block_redraw) {
                    static uint16_t              max_font_xpos[3][4] = {0};
                    extern painter_font_handle_t font_thintel, font_mono, font_oled;
                    render_character_set(device, &surface_xpos, max_font_xpos[0], &surface_ypos, font_thintel,
                                         curr_hsv->primary.h, curr_hsv->primary.s, curr_hsv->primary.v, 0, 0, 0);
                    render_character_set(device, &surface_xpos, max_font_xpos[1], &surface_ypos, font_mono,
                                         curr_hsv->primary.h, curr_hsv->primary.s, curr_hsv->primary.v, 0, 0, 0);
                    render_character_set(device, &surface_xpos, max_font_xpos[2], &surface_ypos, font_oled,
                                         curr_hsv->primary.h, curr_hsv->primary.s, curr_hsv->primary.v, 0, 0, 0);
                }
                break;
            case 2:
                painter_render_qmk_info(device, font, x, y + 5, width, force_redraw || block_redraw, curr_hsv);
                break;
            case 3:
                if (nyan_token == INVALID_DEFERRED_TOKEN) {
                    nyan_token = qp_animate(device, surface_xpos + (width - nyan_cat->width) / 2,
                                            surface_ypos + (height - nyan_cat->height) / 2, nyan_cat);
                }
                break;
            case 4:
                //  Layer Map render
                ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                if (is_left) {
                    painter_render_layer_map(device, font, x, y, width, force_redraw || block_redraw, curr_hsv);
                } else {
                    painter_render_totp(device, font, x + 4, y + 3, width, force_redraw || block_redraw, curr_hsv,
                                        true);
                }

                break;
            default:
                break;
        }
    }
}

/**
 * @brief Renders the keylogger information on the display.
 *
 * This function renders the keylogger information on the specified device using the given font and coordinates.
 * It checks if the keylogger state has changed or if a forced redraw is requested, and updates the display accordingly.
 *
 * @param device The painter device to render on.
 * @param font The font handle to use for rendering text.
 * @param x The x-coordinate to start rendering.
 * @param y The y-coordinate to start rendering.
 * @param width The width of the area to render within.
 * @param force_redraw A boolean indicating whether to force a redraw regardless of keylogger state change.
 * @param curr_hsv A pointer to the current HSV color values to use for rendering.
 */
void painter_render_keylogger(painter_device_t device, painter_font_handle_t font, uint16_t x, uint16_t y,
                              uint16_t width, bool force_redraw, dual_hsv_t* curr_hsv) {
#ifdef DISPLAY_KEYLOGGER_ENABLE
    char buf[50] = {0};

    if (keylogger_has_changed || force_redraw) {
        snprintf(buf, sizeof(buf), "Keylogger: %s", display_keylogger_string);

        x += qp_drawtext_recolor(device, x, y, font, truncate_text(buf, width - x, font, false, false), 0, 255, 0,
                                 curr_hsv->primary.h, curr_hsv->primary.s, curr_hsv->primary.v);

        qp_rect(device, x, y, width, y + font->line_height, curr_hsv->primary.h, curr_hsv->primary.s,
                curr_hsv->primary.v, true);
        keylogger_has_changed = false;
    }
#endif
}

/**
 * @brief Renders the autocorrected and original text on the display.
 *
 * This function displays the autocorrected text and the original text on the specified device
 * using the provided font. It updates the display if the text has changed or if a forced redraw
 * is requested.
 *
 * @param device The painter device to render on.
 * @param font The font handle to use for rendering text.
 * @param x The x-coordinate where the text rendering starts.
 * @param y The y-coordinate where the text rendering starts.
 * @param width The width of the area to render the text.
 * @param force_redraw A boolean flag indicating whether to force a redraw of the text.
 * @param curr_hsv A pointer to a dual_hsv_t structure containing the primary and secondary HSV colors for text
 * rendering.
 */
void painter_render_autocorrect(painter_device_t device, painter_font_handle_t font, uint16_t x, uint16_t y,
                                uint16_t width, bool force_redraw, dual_hsv_t* curr_hsv) {
#ifdef AUTOCORRECT_ENABLE
    extern bool     autocorrect_str_has_changed;
    extern char     autocorrected_str_raw[2][21];
    char            buf[50]           = {0};
    static uint32_t autocorrect_timer = 0;
    if (timer_elapsed(autocorrect_timer) > 125) {
        autocorrect_timer           = timer_read();
        autocorrect_str_has_changed = true;
    }

    if (force_redraw || autocorrect_str_has_changed) {
        qp_drawtext_recolor(device, x, y, font, "Autocorrected: ", curr_hsv->primary.h, curr_hsv->primary.s,
                            curr_hsv->primary.v, 0, 0, 0);
        y += font->line_height + 4;
        snprintf(buf, sizeof(buf), "%19s", autocorrected_str_raw[0]);
        qp_drawtext_recolor(device, x, y, font, buf, curr_hsv->secondary.h, curr_hsv->secondary.s,
                            curr_hsv->secondary.v, 0, 0, 0);

        y += font->line_height + 4;
        qp_drawtext_recolor(device, x, y, font, "Original Text: ", curr_hsv->primary.h, curr_hsv->primary.s,
                            curr_hsv->primary.v, 0, 0, 0);
        y += font->line_height + 4;
        snprintf(buf, sizeof(buf), "%19s", autocorrected_str_raw[1]);

        qp_drawtext_recolor(device, x, y, font, buf, curr_hsv->secondary.h, curr_hsv->secondary.s,
                            curr_hsv->secondary.v, 0, 0, 0);
        autocorrect_str_has_changed = false;
    }
#endif // AUTOCORRECT_ENABLE
}

/**
 * @brief Renders the detected operating system on the display.
 *
 * This function checks the currently detected operating system and displays it on the specified device.
 *
 * @param device The painter device to render on.
 * @param font The font handle to use for rendering text.
 * @param x The x-coordinate to start rendering.
 * @param y The y-coordinate to start rendering.
 * @param width The width of the area to render.
 * @param force_redraw A boolean flag to force redraw even if the OS has not changed.
 * @param curr_hsv A pointer to a dual_hsv_t structure containing the current HSV color values for primary and secondary
 * colors.
 */
void painter_render_os_detection(painter_device_t device, painter_font_handle_t font, uint16_t x, uint16_t y,
                                 uint16_t width, bool force_redraw, dual_hsv_t* curr_hsv) {
#ifdef OS_DETECTION_ENABLE
    static os_variant_t last_detected_os    = {0};
    char                buf[50]             = {0};
    os_variant_t        current_detected_os = detected_host_os();
    if (force_redraw || last_detected_os != current_detected_os) {
        last_detected_os = current_detected_os;

        x += qp_drawtext_recolor(device, x, y, font, "OS: ", curr_hsv->primary.h, curr_hsv->primary.s,
                                 curr_hsv->primary.v, 0, 0, 0);

        snprintf(buf, sizeof(buf), "%9s", os_variant_to_string(current_detected_os));
        qp_drawtext_recolor(device, x, y, font, buf, curr_hsv->secondary.h, curr_hsv->secondary.s,
                            curr_hsv->secondary.v, 0, 0, 0);
    }
#endif
}

/**
 * @brief Renders the modifier icons on the display.
 *
 * This function draws the current state of the modifier keys (Shift, Control, Alt, Command, Windows) on the specified
 * device using the provided font and coordinates. It recolors the icons based on whether the modifiers are active or
 * not.
 *
 * @param device The painter device to render on.
 * @param font The font handle to use for rendering text.
 * @param x The x-coordinate to start rendering.
 * @param y The y-coordinate to start rendering.
 * @param width The width of the area to render.
 * @param force_redraw If true, forces the redraw of the modifiers regardless of state change.
 * @param curr_hsv The current HSV color values to use for active and inactive states.
 * @param disabled_val The value to use for the disabled state of the modifiers.
 */

void painter_render_modifiers(painter_device_t device, painter_font_handle_t font, uint16_t x, uint16_t y,
                              uint16_t width, bool force_redraw, dual_hsv_t* curr_hsv, uint8_t disabled_val) {
    extern painter_image_handle_t shift_icon, control_icon, alt_icon, command_icon, windows_icon;
    static uint8_t                last_mods    = 0;
    uint8_t                       current_mods = get_mods() | get_weak_mods() | get_oneshot_mods();
    if (force_redraw || last_mods != current_mods) {
        last_mods = current_mods;
        x += qp_drawtext_recolor(device, x, y + 1, font, "Modifiers:", curr_hsv->primary.h, curr_hsv->primary.s,
                                 curr_hsv->primary.v, 0, 0, 0) +
             2;

        if (qp_drawimage_recolor(
                device, x, y, shift_icon, last_mods & MOD_BIT_LSHIFT ? curr_hsv->secondary.h : curr_hsv->primary.h,
                curr_hsv->primary.s, last_mods & MOD_BIT_LSHIFT ? curr_hsv->primary.v : disabled_val, 0, 0, 0)) {
            x += shift_icon->width + 2;
        }
        if (qp_drawimage_recolor(device, x, y, keymap_config.swap_lctl_lgui ? command_icon : windows_icon,
                                 last_mods & MOD_BIT_LGUI ? curr_hsv->secondary.h : curr_hsv->primary.h,
                                 last_mods & MOD_BIT_LGUI ? curr_hsv->secondary.s : curr_hsv->primary.s,
                                 last_mods & MOD_BIT_LGUI ? curr_hsv->primary.v : disabled_val, 0, 0, 0)) {
            x += windows_icon->width + 2;
        }
        if (qp_drawimage_recolor(device, x, y, alt_icon,
                                 last_mods & MOD_BIT_LALT ? curr_hsv->secondary.h : curr_hsv->primary.h,
                                 last_mods & MOD_BIT_LALT ? curr_hsv->secondary.s : curr_hsv->primary.s,
                                 last_mods & MOD_BIT_LALT ? curr_hsv->primary.v : disabled_val, 0, 0, 0)) {
            x += alt_icon->width + 2;
        }
        if (qp_drawimage_recolor(device, x, y, control_icon,
                                 last_mods & MOD_BIT_LCTRL ? curr_hsv->secondary.h : curr_hsv->primary.h,
                                 last_mods & MOD_BIT_LCTRL ? curr_hsv->secondary.s : curr_hsv->primary.s,
                                 last_mods & MOD_BIT_LCTRL ? curr_hsv->primary.v : disabled_val, 0, 0, 0)) {
            x += control_icon->width + 2;
        }
        if (qp_drawimage_recolor(device, x, y, control_icon,
                                 last_mods & MOD_BIT_RCTRL ? curr_hsv->secondary.h : curr_hsv->primary.h,
                                 last_mods & MOD_BIT_RCTRL ? curr_hsv->secondary.s : curr_hsv->primary.s,
                                 last_mods & MOD_BIT_RCTRL ? curr_hsv->primary.v : disabled_val, 0, 0, 0)) {
            x += control_icon->width + 2;
        }
        if (qp_drawimage_recolor(device, x, y, alt_icon,
                                 last_mods & MOD_BIT_RALT ? curr_hsv->secondary.h : curr_hsv->primary.h,
                                 last_mods & MOD_BIT_RALT ? curr_hsv->secondary.s : curr_hsv->primary.s,
                                 last_mods & MOD_BIT_RALT ? curr_hsv->primary.v : disabled_val, 0, 0, 0)) {
            x += alt_icon->width + 2;
        }
        if (qp_drawimage_recolor(device, x, y, keymap_config.swap_rctl_rgui ? command_icon : windows_icon,
                                 last_mods & MOD_BIT_RGUI ? curr_hsv->secondary.h : curr_hsv->primary.h,
                                 last_mods & MOD_BIT_RGUI ? curr_hsv->secondary.s : curr_hsv->primary.s,
                                 last_mods & MOD_BIT_RGUI ? curr_hsv->primary.v : disabled_val, 0, 0, 0)) {
            x += windows_icon->width + 2;
        }
        if (qp_drawimage_recolor(device, x, y, shift_icon,
                                 last_mods & MOD_BIT_RSHIFT ? curr_hsv->secondary.h : curr_hsv->primary.h,
                                 last_mods & MOD_BIT_RSHIFT ? curr_hsv->secondary.s : curr_hsv->primary.s,
                                 last_mods & MOD_BIT_RSHIFT ? curr_hsv->primary.v : disabled_val, 0, 0, 0)) {
            x += shift_icon->width + 2;
        }
    }
}

/**
 * @brief Renders QMK information on the display.
 *
 * This function displays the build date, QMK version, and compiler version on the specified device.
 * It also draws a QMK banner image.
 *
 * @param device        The painter device to render on.
 * @param font          The font handle to use for text rendering.
 * @param x             The x-coordinate to start rendering.
 * @param y             The y-coordinate to start rendering.
 * @param width         The width of the rendering area.
 * @param force_redraw  A flag to force redraw.
 * @param curr_hsv      Pointer to a dual_hsv_t structure containing the current HSV color values.
 */
void painter_render_qmk_info(painter_device_t device, painter_font_handle_t font, uint16_t x, uint16_t y,
                             uint16_t width, bool force_redraw, dual_hsv_t* curr_hsv) {
    char     buf[50] = {0};
    uint16_t xpos    = x + 5;

    if (force_redraw) {
        snprintf(buf, sizeof(buf), "%s", QMK_BUILDDATE);
        xpos += qp_drawtext_recolor(device, xpos, y, font, "Built on: ", curr_hsv->primary.h, curr_hsv->primary.s,
                                    curr_hsv->primary.v, 0, 0, 0);
        qp_drawtext_recolor(device, xpos, y, font, buf, curr_hsv->secondary.h, curr_hsv->secondary.s,
                            curr_hsv->secondary.v, 0, 0, 0);
        xpos = x + 5;
        y += font->line_height + 4;
        snprintf(buf, sizeof(buf), "%s", QMK_VERSION);
        xpos += qp_drawtext_recolor(device, xpos, y, font, "Built from: ", curr_hsv->primary.h, curr_hsv->primary.s,
                                    curr_hsv->primary.v, 0, 0, 0);
        qp_drawtext_recolor(device, xpos, y, font, buf, curr_hsv->secondary.h, curr_hsv->secondary.s,
                            curr_hsv->secondary.v, 0, 0, 0);
        xpos = x + 5;
        y += font->line_height + 4;
        xpos += qp_drawtext_recolor(device, xpos, y, font, "Built with: ", curr_hsv->primary.h, curr_hsv->primary.s,
                                    curr_hsv->primary.v, 0, 0, 0);
        qp_drawtext_recolor(device, xpos, y, font, __VERSION__, curr_hsv->secondary.h, curr_hsv->secondary.s,
                            curr_hsv->secondary.v, 0, 0, 0);
        y += font->line_height + 4;
        qp_drawimage(device, x, y, qmk_banner);
    }
}

/**
 * @brief Renders the layer map on the display.
 *
 * This function renders the current keymap based on the active layers on the specified painter device.
 *
 * @param device The display device to render on.
 * @param font The font handle to use for rendering text.
 * @param x The x-coordinate to start rendering from.
 * @param y The y-coordinate to start rendering from.
 * @param width The width of the area to render within.
 * @param force_redraw A boolean indicating whether to force a redraw of the layer map.
 * @param curr_hsv A pointer to a dual_hsv_t structure containing the primary and secondary HSV color values to use for
 * rendering.
 */
void painter_render_layer_map(painter_device_t device, painter_font_handle_t font, uint16_t x, uint16_t y,
                              uint16_t width, bool force_redraw, dual_hsv_t* curr_hsv) {
#ifdef LAYER_MAP_ENABLE
    if (force_redraw || layer_map_has_updated) {
        y += font->line_height + 4;
        uint16_t xpos = x, ypos = y;
        for (uint8_t lm_y = 0; lm_y < LAYER_MAP_ROWS; lm_y++) {
            xpos = x + 20;
            for (uint8_t lm_x = 0; lm_x < LAYER_MAP_COLS; lm_x++) {
                uint16_t keycode = extract_basic_keycode(layer_map[lm_y][lm_x], NULL, false);
                wchar_t  code[2] = {0};

                // if (keycode == UC_IRNY) {
                //     code[0] = L'⸮';
                // } else if (keycode == UC_CLUE) {
                //     code[0] = L'‽'
                // } else
                if (keycode > 0xFF) {
                    keycode = KC_SPC;
                }
                if (keycode < ARRAY_SIZE(code_to_name)) {
                    code[0] = pgm_read_byte(&code_to_name[keycode]);
                }
                xpos += qp_drawtext_recolor(
                    device, xpos, ypos, font, (char*)code, curr_hsv->primary.h, curr_hsv->primary.s,
                    peek_matrix_layer_map(lm_y, lm_x) ? 0 : curr_hsv->primary.v, curr_hsv->secondary.h,
                    curr_hsv->secondary.s, peek_matrix_layer_map(lm_y, lm_x) ? curr_hsv->secondary.v : 0);
                xpos += qp_drawtext_recolor(device, xpos, ypos, font, " ", 0, 0, 0, 0, 0, 0);
            }
            ypos += font->line_height + 4;
        }
        layer_map_has_updated = false;
    }
#endif // LAYER_MAP_ENABLE
}

/**
 * @brief Renders the shutdown screen for the painter device.
 *
 * This function is responsible for rendering the shutdown screen on the specified
 * painter device. It can also optionally jump to the bootloader if specified.
 *
 * @param device The painter device to render the shutdown screen on.
 * @param jump_to_bootloader If true, the device will jump to the bootloader after rendering the shutdown screen.
 * @return true if the shutdown screen was rendered successfully, false otherwise.
 */
bool painter_render_shutdown(painter_device_t device, bool jump_to_bootloader) {
    painter_font_handle_t  font_proggy = qp_load_font_mem(font_proggy_clean_15);
    painter_image_handle_t logo        = qp_load_image_mem(gfx_qmk_logo_220x220);
    uint16_t               width, height, ypos = 240;
    qp_get_geometry(device, &width, &height, NULL, NULL, NULL);
    qp_rect(device, 0, 0, width - 1, height - 1, 0, 0, 0, true);
    qp_drawimage_recolor(device, (width - logo->width) / 2, 10, logo, 0, 0, 255, 0, 0, 0);

    char title[50] = {0};
    snprintf(title, sizeof(title), "%s", "Please Stand By...");
    uint16_t title_width = qp_textwidth(font_proggy, title);
    uint16_t title_xpos  = (width - title_width) / 2;
    qp_drawtext_recolor(device, title_xpos, ypos, font_proggy,
                        truncate_text(title, title_width, font_proggy, false, false), 0, 0, 255, 0, 0, 0);
    ypos += font_proggy->line_height + 4;
    snprintf(title, sizeof(title), "%s", jump_to_bootloader ? "Jumping to Bootloader..." : "Shutting Down...");
    title_width = qp_textwidth(font_proggy, title);
    title_xpos  = (width - title_width) / 2;
    qp_drawtext_recolor(device, title_xpos, ypos, font_proggy,
                        truncate_text(title, title_width, font_proggy, false, false), 0, 0, 255, 0, 0, 0);
    ypos += font_proggy->line_height + 4;
    if (!eeconfig_is_enabled()) {
        snprintf(title, sizeof(title), "%s", "Reinitialiing EEPROM...");
        title_width = qp_textwidth(font_proggy, title);
        title_xpos  = (width - title_width) / 2;
        qp_drawtext_recolor(device, title_xpos, ypos, font_proggy,
                            truncate_text(title, title_width, font_proggy, false, false), 0, 0, 255, 0, 0, 0);
        ypos += font_proggy->line_height + 4;
    }
    qp_flush(device);
    return false;
}

/**
 * @brief Truncates text to fit within a certain width
 *
 * @param text original text
 * @param max_width max width in pixels
 * @param font font being used
 * @param from_start truncate from start or end
 * @param add_ellipses add ellipses to truncated text
 * @return char* truncated text
 */
char* truncate_text(const char* text, uint16_t max_width, painter_font_handle_t font, bool from_start,
                    bool add_ellipses) {
    static char truncated_text[50];
    strncpy(truncated_text, text, sizeof(truncated_text) - 1);
    truncated_text[sizeof(truncated_text) - 1] = '\0';

    uint16_t text_width = qp_textwidth(font, truncated_text);
    if (text_width <= max_width) {
        return truncated_text;
    }

    size_t      len            = strlen(truncated_text);
    const char* ellipses       = "...";
    uint16_t    ellipses_width = add_ellipses ? qp_textwidth(font, ellipses) : 0;

    if (from_start) {
        size_t start_index = 0;
        while (start_index < len && text_width > max_width - ellipses_width) {
            start_index++;
            text_width = qp_textwidth(font, truncated_text + start_index);
        }

        if (add_ellipses) {
            char temp[75];
            snprintf(temp, sizeof(temp), "%s%s", ellipses, truncated_text + start_index);
            strncpy(truncated_text, temp, sizeof(truncated_text) - 1);
            truncated_text[sizeof(truncated_text) - 1] = '\0';
        } else {
            memmove(truncated_text, truncated_text + start_index, len - start_index + 1);
        }
    } else {
        while (len > 0 && text_width > max_width - ellipses_width) {
            len--;
            truncated_text[len] = '\0';
            text_width          = qp_textwidth(font, truncated_text);
        }

        if (add_ellipses) {
            snprintf(truncated_text + len, sizeof(truncated_text) - len, "%s", ellipses);
        }
    }

    return truncated_text;
}

/**
 * @brief Renders full character set of characters that can be displayed in 4 lines:
 *
 *   abcdefghijklmnopqrstuvwxyz
 *   ABCDEFGHIJKLMNOPQRSTUVWXYZ
 *   01234567890 !@#$%^&*()
 *   __+-=[]{}\\|;:'",.<>/?
 *
 * @param display quantum painter device to write to
 * @param x_offset x offset to start rendering at
 * @param max_pos array to store max x position of each line for clearing after
 * rerendering
 * @param ypos y position to start rendering at
 * @param font font to use
 * @param hue_fg text hue
 * @param sat_fg text saturation
 * @param val_fg text value
 * @param hue_bg background hue
 * @param sat_bg background saturation
 * @param val_bg background value
 */
static const char* test_text[] = {
    // did intentionally skip PROGMEM here :)
    "abcdefghijklmnopqrstuvwxyz",
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
    "01234567890 !@#$%%^&*()",
    "__+-=[]{}\\|;:'\",.<>/?",
};

void render_character_set(painter_device_t display, uint16_t* x_offset, uint16_t* max_pos, uint16_t* ypos,
                          painter_font_handle_t font, uint8_t hue_fg, uint8_t sat_fg, uint8_t val_fg, uint8_t hue_bg,
                          uint8_t sat_bg, uint8_t val_bg) {
    for (uint8_t i = 0; i < 4; ++i) {
        uint16_t width = qp_drawtext_recolor(display, *x_offset, *ypos, font, test_text[i], hue_fg, sat_fg, val_fg,
                                             hue_bg, sat_bg, val_bg);

        // clean up after the line (?)
        if (width < max_pos[i]) {
            qp_rect(display, *x_offset + width, *ypos, *x_offset + width + max_pos[i], *ypos + font->line_height, 0, 0,
                    0, true);
        }

        // move down for next line
        *ypos += font->line_height;
    }
}

#ifdef BACKLIGHT_ENABLE
static uint8_t last_backlight = 255;
#endif

void qp_backlight_enable(void) {
#ifdef BACKLIGHT_ENABLE
    if (last_backlight != 255) {
        backlight_level_noeeprom(last_backlight);
        last_backlight = 255;
    }
#elif defined(BACKLIGHT_PIN)
    gpio_write_pin_high(BACKLIGHT_PIN);
#endif // BACKLIGHT_ENABLE
}

void qp_backlight_disable(void) {
#ifdef BACKLIGHT_ENABLE
    if (last_backlight == 255) {
        last_backlight = get_backlight_level();
        backlight_level_noeeprom(0);
    }
#elif defined(BACKLIGHT_PIN)
    gpio_write_pin_low(BACKLIGHT_PIN);
#endif // BACKLIGHT_ENABLE
}

void housekeeping_task_quantum_painter(void) {
#ifdef SPLIT_KEYBOARD
    if (!is_keyboard_master()) {
        static bool suspended = false;
        bool        is_device_suspended(void);
        if (suspended != is_device_suspended()) {
            suspended = is_device_suspended();
            if (suspended) {
#    ifdef QUANTUM_PAINTER_ILI9341_ENABLE
                ili9341_display_power(false);
#    endif // QUANTUM_PAINTER_ILI9341_ENABLE
            } else {
#    ifdef QUANTUM_PAINTER_ILI9341_ENABLE
                ili9341_display_power(true);
#    endif // QUANTUM_PAINTER_ILI9341_ENABLE
            }
        }
    }
#endif
#ifdef RTC_ENABLE
    if (rtc_is_connected()) {
        static uint8_t last_second = 0xFF;
        if (rtc_read_time_struct().second != last_second) {
            last_second = rtc_read_time_struct().second;
            display_menu_set_dirty();
        }
    }
#endif // RTC_ENABLE
#ifdef QUANTUM_PAINTER_ILI9341_ENABLE
    ili9341_draw_user();
#endif // QUANTUM_PAINTER_ILI9341_ENABLE
#if (QUANTUM_PAINTER_DISPLAY_TIMEOUT) > 0
    if (is_keyboard_master() && (last_input_activity_elapsed() > QUANTUM_PAINTER_DISPLAY_TIMEOUT)) {
        qp_backlight_disable();
    } else {
        qp_backlight_enable();
    }
#endif
    if (console_has_redrawn) {
        console_log_needs_redraw = false;
    }
}

void keyboard_post_init_quantum_painter(void) {
    painter_init_assets();
#if defined(BACKLIGHT_ENABLE)
    if (!is_backlight_enabled()) {
        backlight_enable();
        backlight_level_noeeprom(get_backlight_level());
    }
    if (get_backlight_level() == 0) {
        backlight_level(BACKLIGHT_LEVELS);
    }
#elif !defined(BACKLIGHT_ENABLE) && defined(BACKLIGHT_PIN)
    gpio_set_pin_output_push_pull(BACKLIGHT_PIN);
    gpio_write_pin_high(BACKLIGHT_PIN);
#endif
#ifdef QUANTUM_PAINTER_ILI9341_ENABLE
    init_display_ili9341();
#endif // QUANTUM_PAINTER_ILI9341_ENABLE
}

void suspend_power_down_quantum_painter(void) {
    qp_backlight_disable();
#ifdef QUANTUM_PAINTER_ILI9341_ENABLE
    ili9341_display_power(false);
#endif // QUANTUM_PAINTER_ILI9341_ENABLE
}

void suspend_wakeup_init_quantum_painter(void) {
#ifdef QUANTUM_PAINTER_ILI9341_ENABLE
    ili9341_display_power(true);
#endif // QUANTUM_PAINTER_ILI9341_ENABLE
    qp_backlight_enable();
}

void shutdown_quantum_painter(bool jump_to_bootloader) {
#ifdef QUANTUM_PAINTER_ILI9341_ENABLE
    ili9341_display_shutdown(jump_to_bootloader);
#endif // QUANTUM_PAINTER_ILI9341_ENABLE
#ifdef BACKLIGHT_ENABLE
    qp_backlight_enable();
#elif defined(BACKLIGHT_PIN)
    gpio_write_pin_low(BACKLIGHT_PIN);
#endif
}

#ifndef PAINTER_HUE_STEP
#    define PAINTER_HUE_STEP 4
#endif // PAINTER_HUE_STEP

#ifndef PAINTER_SAT_STEP
#    define PAINTER_SAT_STEP 16
#endif // PAINTER_SAT_STEP

#ifndef PAINTER_VAL_STEP
#    define PAINTER_VAL_STEP 16
#endif // PAINTER_VAL_STEP

#ifndef PAINTER_SPD_STEP
#    define PAINTER_SPD_STEP 16
#endif // PAINTER_SPD_STEP
#include <lib/lib8tion/lib8tion.h>

/**
 * @brief Get the current HSV values for the painter
 *
 * @return HSV
 */
dual_hsv_t painter_get_dual_hsv(void) {
    return userspace_config.display.painter.hsv;
}

/**
 * @brief Set the HSV for painter rendering
 *
 * @param hue hue value to use (0-255, qmk's modified hue range)
 * @param sat satuartion value to use
 * @param val value/brightness value to use
 * @param write_to_eeprom save changes to eeprom?
 */
void painter_sethsv_eeprom_helper(uint8_t hue, uint8_t sat, uint8_t val, bool write_to_eeprom, bool primary) {
    hsv_t* hsv =
        primary ? &userspace_config.display.painter.hsv.primary : &userspace_config.display.painter.hsv.secondary;
    hsv->h = hue;
    hsv->s = sat;
    hsv->v = val;
    if (write_to_eeprom) {
        eeconfig_update_user_datablock(&userspace_config);
    }
    dprintf("painter set %s hsv [%s]: %u,%u,%u\n", primary ? "primary" : "secondary",
            write_to_eeprom ? "EEPROM" : "NOEEPROM", hsv->h, hsv->s, hsv->v);
}
/**
 * @brief Sets the HSV for painter rendering, without saving to eeprom
 *
 * @param hue hue value to use (0-255, qmk's modified hue range)
 * @param sat satuartion value to use
 * @param val value/brightness value to use
 */
void painter_sethsv_noeeprom(uint8_t hue, uint8_t sat, uint8_t val, bool primary) {
    painter_sethsv_eeprom_helper(hue, sat, val, false, primary);
}
/**
 * @brief Sets the HSV for painter rendering, and saves to eeprom
 *
 * @param hue hue value to use (0-255, qmk's modified hue range)
 * @param sat satuartion value to use
 * @param val value/brightness value to use
 */
void painter_sethsv(uint8_t hue, uint8_t sat, uint8_t val, bool primary) {
    painter_sethsv_eeprom_helper(hue, sat, val, true, primary);
}

/**
 * @brief Get the current HSV values for the painter
 *
 * @return HSV
 */
hsv_t painter_get_hsv(bool primary) {
    return primary ? userspace_config.display.painter.hsv.primary : userspace_config.display.painter.hsv.secondary;
}

/**
 * @brief Get the current hue value for the painter
 *
 * @return uint8_t
 */
uint8_t painter_get_hue(bool primary) {
    return primary ? userspace_config.display.painter.hsv.primary.h : userspace_config.display.painter.hsv.secondary.h;
}

/**
 * @brief Get the current saturation value for the painter
 *
 * @return uint8_t
 */
uint8_t painter_get_sat(bool primary) {
    return primary ? userspace_config.display.painter.hsv.primary.s : userspace_config.display.painter.hsv.secondary.s;
}

/**
 * @brief Get the current value/brightness value for the painter
 *
 * @return uint8_t
 */
uint8_t painter_get_val(bool primary) {
    return primary ? userspace_config.display.painter.hsv.primary.v : userspace_config.display.painter.hsv.secondary.v;
}

/**
 * @brief Increments hue up by the step value
 *
 * @param write_to_eeprom Save to eeprom?
 */
void painter_increase_hue_helper(bool write_to_eeprom, bool primary) {
    hsv_t* hsv =
        primary ? &userspace_config.display.painter.hsv.primary : &userspace_config.display.painter.hsv.secondary;
    painter_sethsv_eeprom_helper(qadd8(hsv->h, PAINTER_HUE_STEP), hsv->s, hsv->v, write_to_eeprom, primary);
}

/**
 * @brief Increments hue up by the step value, without saving to eeprom
 */
void painter_increase_hue_noeeprom(bool primary) {
    painter_increase_hue_helper(false, primary);
}

/**
 * @brief Increments hue up by the step value, and saves to eeprom
 */
void painter_increase_hue(bool primary) {
    painter_increase_hue_helper(true, primary);
}

/**
 * @brief Decrements hue down by the step value
 *
 * @param write_to_eeprom Save to eeprom?
 */
void painter_decrease_hue_helper(bool write_to_eeprom, bool primary) {
    hsv_t* hsv =
        primary ? &userspace_config.display.painter.hsv.primary : &userspace_config.display.painter.hsv.secondary;
    painter_sethsv_eeprom_helper(qsub8(hsv->h, PAINTER_HUE_STEP), hsv->s, hsv->v, write_to_eeprom, primary);
}

/**
 * @brief Decrements hue down by the step value, without saving to eeprom
 */
void painter_decrease_hue_noeeprom(bool primary) {
    painter_decrease_hue_helper(false, primary);
}

/**
 * @brief Decrements hue down by the step value, and saves to eeprom
 */
void painter_decrease_hue(bool primary) {
    painter_decrease_hue_helper(true, primary);
}

/**
 * @brief Increments saturation up by the step value
 *
 * @param write_to_eeprom Save to eeprom?
 */
void painter_increase_sat_helper(bool write_to_eeprom, bool primary) {
    hsv_t* hsv =
        primary ? &userspace_config.display.painter.hsv.primary : &userspace_config.display.painter.hsv.secondary;
    painter_sethsv_eeprom_helper(hsv->h, qadd8(hsv->s, PAINTER_SAT_STEP), hsv->v, write_to_eeprom, primary);
}

/**
 * @brief Increments saturation up by the step value, without saving to eeprom
 */
void painter_increase_sat_noeeprom(bool primary) {
    painter_increase_sat_helper(false, primary);
}

/**
 * @brief Increments saturation up by the step value, and saves to eeprom
 */
void painter_increase_sat(bool primary) {
    painter_increase_sat_helper(true, primary);
}

/**
 * @brief Decrements saturation down by the step value
 *
 * @param write_to_eeprom Save to eeprom?
 */
void painter_decrease_sat_helper(bool write_to_eeprom, bool primary) {
    hsv_t* hsv =
        primary ? &userspace_config.display.painter.hsv.primary : &userspace_config.display.painter.hsv.secondary;
    painter_sethsv_eeprom_helper(hsv->h, qsub8(hsv->s, PAINTER_SAT_STEP), hsv->v, write_to_eeprom, primary);
}

/**
 * @brief Decrements saturation down by the step value, without saving to eeprom
 */
void painter_decrease_sat_noeeprom(bool primary) {
    painter_decrease_sat_helper(false, primary);
}

/**
 * @brief Decrements saturation down by the step value, and saves to eeprom
 */
void painter_decrease_sat(bool primary) {
    painter_decrease_sat_helper(true, primary);
}

/**
 * @brief Increments value/brightness up by the step value
 *
 * @param write_to_eeprom Save to eeprom?
 */
void painter_increase_val_helper(bool write_to_eeprom, bool primary) {
    hsv_t* hsv =
        primary ? &userspace_config.display.painter.hsv.primary : &userspace_config.display.painter.hsv.secondary;
    painter_sethsv_eeprom_helper(hsv->h, hsv->s, qadd8(hsv->v, PAINTER_VAL_STEP), write_to_eeprom, primary);
}

/**
 * @brief Increments value/brightness up by the step value, without saving to eeprom
 */
void painter_increase_val_noeeprom(bool primary) {
    painter_increase_val_helper(false, primary);
}

/**
 * @brief Increments value/brightness up by the step value, and saves to eeprom
 */
void painter_increase_val(bool primary) {
    painter_increase_val_helper(true, primary);
}

/**
 * @brief Decrements value/brightness down by the step value
 *
 * @param write_to_eeprom Save to eeprom?
 */
void painter_decrease_val_helper(bool write_to_eeprom, bool primary) {
    hsv_t* hsv =
        primary ? &userspace_config.display.painter.hsv.primary : &userspace_config.display.painter.hsv.secondary;
    painter_sethsv_eeprom_helper(hsv->h, hsv->s, qsub8(hsv->v, PAINTER_VAL_STEP), write_to_eeprom, primary);
}

/**
 * @brief Decrements value/brightness down by the step value, without saving to eeprom
 */
void painter_decrease_val_noeeprom(bool primary) {
    painter_decrease_val_helper(false, primary);
}

/**
 * @brief Decrements value/brightness down by the step value, and saves to eeprom
 */
void painter_decrease_val(bool primary) {
    painter_decrease_val_helper(true, primary);
}
