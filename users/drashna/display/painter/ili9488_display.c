// Copyright 2018-2024 Nick Brassel (@tzarc)
// Copyright 2025 Christopher Courtney, aka Drashna Jael're  (@drashna) <drashna@live.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "drashna_runtime.h"
#include "drashna_names.h"
#include "drashna_layers.h"
#include "drashna_util.h"
#include "version.h"
#include "qp_ili9xxx_opcodes.h"
#include "qp_comms.h"
#include "display/painter/painter.h"
#include "display/painter/ili9488_display.h"
#ifdef QUANTUM_PAINTER_DRIVERS_ILI9488_SURFACE
#    include "qp_surface.h"
#endif // QUANTUM_PAINTER_DRIVERS_ILI9488_SURFACE
#ifdef SPLIT_KEYBOARD
#    include "split_util.h"
#endif // SPLIT_KEYBOARD
#if defined(RGB_MATRIX_ENABLE)
#    include "rgb/rgb_matrix_stuff.h"
#endif // defined(RGB_MATRIX_ENABLE)
#if defined(RGBLIGHT_ENABLE)
#    include "rgb/rgb_stuff.h"
#endif // defined(RGBLIGHT_ENABLE)
#ifdef COMMUNITY_MODULE_RTC_ENABLE
#    include "rtc.h"
#endif // COMMUNITY_MODULE_RTC_ENABLE
#ifdef COMMUNITY_MODULE_LAYER_MAP_ENABLE
#    include "layer_map.h"
#endif // COMMUNITY_MODULE_LAYER_MAP_ENABLE
#ifdef COMMUNITY_MODULE_KEYBOARD_LOCK_ENABLE
#    include "keyboard_lock.h"
#endif // COMMUNITY_MODULE_KEYBOARD_LOCK_ENABLE
#ifdef COMMUNITY_MODULE_UNICODE_TYPING_ENABLE
#    include "unicode_typing.h"
#endif // COMMUNITY_MODULE_UNICODE_TYPING_ENABLE

static painter_device_t display;

extern painter_image_handle_t windows_logo, apple_logo, linux_logo;
extern painter_image_handle_t mouse_icon, gamepad_icon;
extern painter_image_handle_t akira_explosion;

#ifndef ILI9488_CS_PIN
#    define ILI9488_CS_PIN DISPLAY_CS_PIN
#endif // ILI9488_CS_PIN
#ifndef ILI9488_DC_PIN
#    define ILI9488_DC_PIN DISPLAY_DC_PIN
#endif // ILI9488_DC_PIN
#ifndef ILI9488_RST_PIN
#    ifndef DISPLAY_RST_PIN
#        define ILI9488_RST_PIN NO_PIN
#    else // DISPLAY_RST_PIN
#        define ILI9488_RST_PIN DISPLAY_RST_PIN
#    endif // DISPLAY_RST_PIN
#endif     // ILI9488_RST_PIN
#ifndef ILI9488_SPI_DIVIDER
#    ifndef DISPLAY_SPI_DIVIDER
#        define ILI9488_SPI_DIVIDER 1
#    else // DISPLAY_SPI_DIVIDER
#        define ILI9488_SPI_DIVIDER DISPLAY_SPI_DIVIDER
#    endif // DISPLAY_SPI_DIVIDER
#endif     // ILI9488_SPI_DIVIDER
#ifndef ILI9488_SPI_MODE
#    ifndef DISPLAY_SPI_MODE
#        define ILI9488_SPI_MODE 0
#    else // DISPLAY_SPI_MODE
#        define ILI9488_SPI_MODE DISPLAY_SPI_MODE
#    endif // DISPLAY_SPI_MODE
#endif     // ILI9488_SPI_MODE

#ifdef QUANTUM_PAINTER_DRIVERS_ILI9488_SURFACE
#    if SURFACE_NUM_DEVICES < 2
#        error Not enough surfaces for the ILI9488 display
#    endif
#    define SURFACE_MENU_WIDTH  236
#    define SURFACE_MENU_HEIGHT 120
#    if HAL_USE_SDRAM == TRUE
__attribute__((section(".ram7")))
#    endif
static uint8_t   menu_buffer[2][SURFACE_REQUIRED_BUFFER_BYTE_SIZE(SURFACE_MENU_WIDTH, SURFACE_MENU_HEIGHT, 16)];
painter_device_t menu_surface[2];
#endif

static bool has_run = false, forced_reinit = false;

void init_display_ili9488_inversion(void) {
    qp_comms_start(display);
    qp_comms_command(display, userspace_config.display.inverted ? ILI9XXX_CMD_INVERT_OFF : ILI9XXX_CMD_INVERT_ON);
    qp_comms_stop(display);
    if (has_run) {
        forced_reinit = true;
    }
}

void init_display_ili9488_rotation(void) {
    uint16_t width;
    uint16_t height;

    qp_init(display, userspace_config.display.rotation ? QP_ROTATION_90 : QP_ROTATION_270);
    qp_get_geometry(display, &width, &height, NULL, NULL, NULL);
    qp_clear(display);
    qp_rect(display, 0, 0, width - 1, height - 1, 0, 0, 0, true);

    // if needs inversion, run it only after the clear and rect functions or otherwise it won't work
    init_display_ili9488_inversion();
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Initial render of frame/logo

    painter_render_frame(display, font_thintel, true, 0, true);
    painter_render_frame(display, font_thintel, false, 240, false);
    qp_power(display, true);
    qp_flush(display);
    if (has_run) {
        forced_reinit = true;
    }
    has_run = true;
}

/**
 * @brief Initializes the display, clears it and sets frame and title
 *
 */
void init_display_ili9488(void) {
    display = qp_ili9488_make_spi_device(320, 480, ILI9488_CS_PIN, ILI9488_DC_PIN, ILI9488_RST_PIN, ILI9488_SPI_DIVIDER,
                                         ILI9488_SPI_MODE);

#ifdef QUANTUM_PAINTER_DRIVERS_ILI9488_SURFACE
    menu_surface[0] = qp_make_rgb565_surface(SURFACE_MENU_WIDTH, SURFACE_MENU_HEIGHT, menu_buffer[0]);
    menu_surface[1] = qp_make_rgb565_surface(SURFACE_MENU_WIDTH, SURFACE_MENU_HEIGHT, menu_buffer[1]);
#endif // QUANTUM_PAINTER_DRIVERS_ILI9488_SURFACE

    wait_ms(50);

#ifdef QUANTUM_PAINTER_DRIVERS_ILI9488_SURFACE
    qp_init(menu_surface[0], QP_ROTATION_0);
    qp_init(menu_surface[1], QP_ROTATION_0);
#endif // QUANTUM_PAINTER_DRIVERS_ILI9488_SURFACE

    init_display_ili9488_rotation();
}

void ili9488_display_power(bool on) {
    qp_power(display, on);
}

__attribute__((weak)) void ili9488_draw_user(void) {
    bool hue_redraw = forced_reinit;

    static dual_hsv_t curr_hsv = {0};
    if (memcmp(&curr_hsv, &userspace_config.display.painter.hsv, sizeof(dual_hsv_t)) != 0) {
        curr_hsv   = painter_get_dual_hsv();
        hue_redraw = true;
    }
    const uint8_t disabled_val = curr_hsv.primary.v / 2;

    uint16_t width;
    uint16_t height;
    qp_get_geometry(display, &width, &height, NULL, NULL, NULL);
    width /= 2;

    char buf[50] = {0};
    (void)buf;

    uint16_t ypos = 20;
    uint16_t xpos = 5;
    // left side
    {
        if (hue_redraw) {
            painter_render_frame(display, font_thintel, true, 0, true);
            qp_drawimage_recolor(display, width - mouse_icon->width - 6, 5, mouse_icon, curr_hsv.primary.h,
                                 curr_hsv.primary.s, curr_hsv.primary.v, 0, 0, 0);
        }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Matrix Scan rate

        painter_render_scan_rate(display, font_oled, xpos, ypos, hue_redraw, &curr_hsv);
        ypos += font_oled->line_height + 4;

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //  WPM

#ifdef WPM_ENABLE
        painter_render_wpm(display, font_oled, 5, ypos, hue_redraw, &curr_hsv);
#endif // WPM_ENABLE

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // RGB Matrix Settings

        ypos = 20;
        xpos = 83;
#if defined(RGB_MATRIX_ENABLE)
        painter_render_rgb(display, font_oled, xpos, ypos, hue_redraw, &curr_hsv,
                           "RGB Matrix Config:", rgb_matrix_get_effect_name, rgb_matrix_get_hsv,
                           rgb_matrix_is_enabled(), RGB_MATRIX_MAXIMUM_BRIGHTNESS);
#endif // RGB_MATRIX_ENABLE

        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // LED Lock indicator(text)

        ypos = 24;
        xpos = 212;
        painter_render_lock_state(display, font_oled, xpos, ypos, hue_redraw, &curr_hsv, disabled_val);

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Keymap config (nkro, autocorrect, oneshots)

        ypos                                        = 54 + 4;
        static keymap_config_t last_keymap_config   = {0};
        bool                   keymap_config_redraw = false;
        if (last_keymap_config.raw != keymap_config.raw) {
            last_keymap_config.raw = keymap_config.raw;
            keymap_config_redraw   = true;
        }
#ifdef CAPS_WORD_ENABLE
        static bool caps_words = false;
        if (caps_words != is_caps_word_on()) {
            caps_words           = is_caps_word_on();
            keymap_config_redraw = true;
        }
#endif // CAPS_WORD_ENABLE
        if (hue_redraw || keymap_config_redraw) {
            xpos = 80 + 4;
            qp_drawimage(display, xpos, ypos + 2, last_keymap_config.swap_lctl_lgui ? apple_logo : windows_logo);
            xpos += windows_logo->width + 5;
            xpos += qp_drawtext_recolor(display, xpos, ypos, font_oled, (const char *)"NKRO",
                                        last_keymap_config.nkro ? curr_hsv.secondary.h : curr_hsv.primary.h,
                                        last_keymap_config.nkro ? curr_hsv.secondary.s : curr_hsv.primary.s,
                                        last_keymap_config.nkro ? curr_hsv.primary.v : disabled_val, 0, 0, 0) +
                    5;
            xpos += qp_drawtext_recolor(
                        display, xpos, ypos, font_oled, (const char *)"CRCT",
                        last_keymap_config.autocorrect_enable ? curr_hsv.secondary.h : curr_hsv.primary.h,
                        last_keymap_config.autocorrect_enable ? curr_hsv.secondary.s : curr_hsv.primary.s,
                        last_keymap_config.autocorrect_enable ? curr_hsv.primary.v : disabled_val, 0, 0, 0) +
                    5;
            xpos +=
                qp_drawtext_recolor(display, xpos, ypos, font_oled, (const char *)"1SHT",
                                    last_keymap_config.oneshot_enable ? curr_hsv.secondary.h : curr_hsv.primary.h,
                                    last_keymap_config.oneshot_enable ? curr_hsv.secondary.s : curr_hsv.primary.s,
                                    last_keymap_config.oneshot_enable ? curr_hsv.primary.v : disabled_val, 0, 0, 0) +
                5;
#ifdef CAPS_WORD_ENABLE
            xpos += qp_drawtext_recolor(display, xpos, ypos, font_oled, "CAPS",
                                        is_caps_word_on() ? curr_hsv.secondary.h : curr_hsv.primary.h,
                                        is_caps_word_on() ? curr_hsv.secondary.s : curr_hsv.primary.s,
                                        is_caps_word_on() ? curr_hsv.primary.v : disabled_val, 0, 0, 0);
#endif // CAPS_WORD_ENABLE
        }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Device Config (Audio, Audio Clicky, Host Driver lock, Swap Hands)

        ypos += font_oled->line_height + 4;
        static userspace_runtime_state_t last_user_state = {0};
        if (hue_redraw || memcmp(&userspace_runtime_state, &last_user_state, sizeof(userspace_runtime_state))) {
            memcpy(&last_user_state, &userspace_runtime_state, sizeof(userspace_runtime_state));
            xpos = 80 + 4 + windows_logo->width + 5;
            xpos += qp_drawtext_recolor(display, xpos, ypos, font_oled, (const char *)"AUDIO",
                                        last_user_state.audio.enable ? curr_hsv.secondary.h : curr_hsv.primary.h,
                                        last_user_state.audio.enable ? curr_hsv.secondary.s : curr_hsv.primary.s,
                                        last_user_state.audio.enable ? curr_hsv.primary.v : disabled_val, 0, 0, 0) +
                    5;
            xpos +=
                qp_drawtext_recolor(display, xpos, ypos, font_oled, (const char *)"CLCK",
                                    last_user_state.audio.clicky_enable ? curr_hsv.secondary.h : curr_hsv.primary.h,
                                    last_user_state.audio.clicky_enable ? curr_hsv.secondary.s : curr_hsv.primary.s,
                                    last_user_state.audio.clicky_enable ? curr_hsv.primary.v : disabled_val, 0, 0, 0) +
                5;
            xpos += qp_drawtext_recolor(
                        display, xpos, ypos, font_oled, (const char *)"HOST",
                        last_user_state.internals.host_driver_disabled ? curr_hsv.secondary.h : curr_hsv.primary.h,
                        last_user_state.internals.host_driver_disabled ? curr_hsv.secondary.s : curr_hsv.primary.s,
                        last_user_state.internals.host_driver_disabled ? curr_hsv.primary.v : disabled_val, 0, 0, 0) +
                    5;
            xpos +=
                qp_drawtext_recolor(display, xpos, ypos, font_oled, (const char *)"SWAP",
                                    last_user_state.internals.swap_hands ? curr_hsv.secondary.h : curr_hsv.primary.h,
                                    last_user_state.internals.swap_hands ? curr_hsv.secondary.s : curr_hsv.primary.s,
                                    last_user_state.internals.swap_hands ? curr_hsv.primary.v : disabled_val, 0, 0, 0);
        }
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Pointing Device CPI

        ypos = 43 + 4;
#if defined(POINTING_DEVICE_ENABLE)
#    if (defined(KEYBOARD_bastardkb_charybdis) || defined(KEYBOARD_handwired_tractyl_manuform))
#        include QMK_KEYBOARD_H

        static uint16_t last_cpi = {0xFFFF};
        uint16_t        curr_cpi = charybdis_get_pointer_sniping_enabled() ? charybdis_get_pointer_sniping_dpi()
                                                                           : charybdis_get_pointer_default_dpi();
        if (hue_redraw || last_cpi != curr_cpi) {
            last_cpi = curr_cpi;
            xpos     = 5;
            xpos += qp_drawtext_recolor(display, xpos, ypos, font_oled, "CPI:   ", curr_hsv.primary.h,
                                        curr_hsv.primary.s, curr_hsv.primary.v, 0, 0, 0);
            snprintf(buf, sizeof(buf), "%5u", curr_cpi);
            xpos += qp_drawtext_recolor(display, xpos, ypos, font_oled, buf, curr_hsv.secondary.h, curr_hsv.secondary.s,
                                        curr_hsv.secondary.v, 0, 0, 0);
        }
        ypos += font_oled->line_height + 4;
#    endif
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Pointing Device Auto Mouse Layer

#    ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
        static uint8_t last_am_state     = 0xFF;
        bool           auto_mouse_redraw = false;
        if (last_am_state != get_auto_mouse_enable()) {
            last_am_state     = get_auto_mouse_enable();
            auto_mouse_redraw = true;
        }
        if (hue_redraw || auto_mouse_redraw) {
            xpos = 5;
            qp_drawtext_recolor(display, xpos, ypos, font_oled, "Auto Layer:", curr_hsv.primary.h, curr_hsv.primary.s,
                                curr_hsv.primary.v, 0, 0, 0);
        }
        ypos += font_oled->line_height + 4;

        static uint8_t last_am_layer = 0xFF;
        if (hue_redraw || last_am_layer != get_auto_mouse_layer() || auto_mouse_redraw) {
            last_am_state = get_auto_mouse_layer();
            xpos          = 5;
            snprintf(buf, sizeof(buf), "%12s", get_layer_name_string(get_auto_mouse_layer()));
            qp_drawtext_recolor(display, xpos, ypos, font_oled, truncate_text(buf, 80 - 5 - 2, font_oled, false, false),
                                get_auto_mouse_enable() ? curr_hsv.secondary.h : curr_hsv.primary.h,
                                get_auto_mouse_enable() ? curr_hsv.secondary.s : curr_hsv.primary.s,
                                get_auto_mouse_enable() ? curr_hsv.primary.v : disabled_val, 0, 0, 0);
        }
        ypos += font_oled->line_height + 4;

#    endif

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Pointing Device Drag Scroll

#    if (defined(KEYBOARD_bastardkb_charybdis) || defined(KEYBOARD_handwired_tractyl_manuform))
        static uint32_t last_ds_state = 0xFFFFFFFF;
        if (hue_redraw || last_ds_state != charybdis_get_pointer_dragscroll_enabled()) {
            last_ds_state = charybdis_get_pointer_dragscroll_enabled();
            xpos          = 5;
            xpos += qp_drawtext_recolor(
                display, xpos, ypos, font_oled, "Drag-Scroll",
                charybdis_get_pointer_dragscroll_enabled() ? curr_hsv.secondary.h : curr_hsv.primary.h,
                charybdis_get_pointer_dragscroll_enabled() ? curr_hsv.secondary.s : curr_hsv.primary.s,
                charybdis_get_pointer_dragscroll_enabled() ? curr_hsv.primary.v : disabled_val, 0, 0, 0);
        }
        ypos += font_oled->line_height + 4;

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Pointing Device Sniping mode

        static uint8_t last_sp_state = 0xFF;

        if (hue_redraw || last_sp_state != charybdis_get_pointer_sniping_enabled()) {
            last_sp_state = charybdis_get_pointer_sniping_enabled();
            xpos          = 5;
            xpos += qp_drawtext_recolor(
                display, xpos, ypos, font_oled, "Sniping",
                charybdis_get_pointer_sniping_enabled() ? curr_hsv.secondary.h : curr_hsv.primary.h,
                charybdis_get_pointer_sniping_enabled() ? curr_hsv.secondary.s : curr_hsv.primary.s,
                charybdis_get_pointer_sniping_enabled() ? curr_hsv.primary.v : disabled_val, 0, 0, 0);
        }
#    endif
#endif // POINTING_DEVICE_ENABLE

#ifdef CUSTOM_UNICODE_ENABLE
        ypos                             = 80 + 4;
        static uint8_t last_unicode_mode = UNICODE_MODE_COUNT;
        if (hue_redraw || last_unicode_mode != get_unicode_input_mode()) {
            last_unicode_mode   = get_unicode_input_mode();
            xpos                = 80 + 4;
            uint8_t xpos_offset = xpos +
                                  qp_drawtext_recolor(display, xpos, ypos, font_oled, "Unicode", curr_hsv.primary.h,
                                                      curr_hsv.primary.s, curr_hsv.primary.v, 0, 0, 0) +
                                  4;
            switch (last_unicode_mode) {
                case UNICODE_MODE_WINCOMPOSE:
                case UNICODE_MODE_WINDOWS:
                    qp_drawimage(display, xpos_offset, ypos + 2, windows_logo);
                    break;
                case UNICODE_MODE_MACOS:
                    qp_drawimage(display, xpos_offset, ypos + 2, apple_logo);
                    break;
                case UNICODE_MODE_LINUX:
                case UNICODE_MODE_BSD:
                case UNICODE_MODE_EMACS:
                    qp_drawimage(display, xpos_offset, ypos + 2, linux_logo);
                    break;
            }
            ypos += font_oled->line_height + 4;
            qp_drawtext_recolor(display, xpos + 8, ypos, font_oled, "Mode", curr_hsv.primary.h, curr_hsv.primary.s,
                                curr_hsv.primary.v, 0, 0, 0);
        }

        ypos                                    = 80 + 4;
        static uint8_t last_unicode_typing_mode = 0;
        if (hue_redraw || last_unicode_typing_mode != userspace_runtime_state.unicode.typing_mode) {
            last_unicode_typing_mode = userspace_runtime_state.unicode.typing_mode;
            xpos                     = 149 + 4;
            qp_drawtext_recolor(display, xpos, ypos, font_oled, "Typing Mode:", curr_hsv.primary.h, curr_hsv.primary.s,
                                curr_hsv.primary.v, 0, 0, 0);
            ypos += font_oled->line_height + 4;
            snprintf(buf, sizeof(buf), "%14s", get_unicode_typing_mode_str(last_unicode_typing_mode));
            qp_drawtext_recolor(display, xpos, ypos, font_oled, buf, curr_hsv.secondary.h, curr_hsv.secondary.s,
                                curr_hsv.secondary.v, 0, 0, 0);
        }
#endif // CUSTOM_UNICODE_ENABLE

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Mods

        ypos = 107 + 3;
        xpos = 5;

        painter_render_modifiers(display, font_oled, xpos, ypos, width, hue_redraw || keymap_config_redraw, &curr_hsv,
                                 disabled_val);

#ifdef OS_DETECTION_ENABLE
        ypos = 107 + 4;
        xpos = 159;
        painter_render_os_detection(display, font_oled, xpos, ypos, width, hue_redraw, &curr_hsv);
#endif // OS_DETECTION_ENABLE

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //  Default layer state

        ypos                                    = 122 + 4;
        xpos                                    = 125;
        bool                 layer_state_redraw = false, dl_state_redraw = false;
        static layer_state_t last_layer_state = 0, last_dl_state = 0;
        if (last_layer_state != layer_state) {
            last_layer_state   = layer_state;
            layer_state_redraw = true;
        }
        if (last_dl_state != default_layer_state) {
            last_dl_state   = default_layer_state;
            dl_state_redraw = true;
        }

        if (hue_redraw || dl_state_redraw || layer_state_redraw) {
            qp_drawtext_recolor(display, xpos, ypos, font_oled, "Layout: ", curr_hsv.primary.h, curr_hsv.primary.s,
                                curr_hsv.primary.v, 0, 0, 0);
            ypos += font_oled->line_height + 4;
            snprintf(buf, sizeof(buf), "%10s", get_layer_name_string(default_layer_state, false, true));
            qp_drawtext_recolor(display, xpos, ypos, font_oled, buf, curr_hsv.secondary.h, curr_hsv.secondary.s,
                                curr_hsv.secondary.v, 0, 0, 0);
        } else {
            ypos += font_oled->line_height + 4;
        }
        ypos += font_oled->line_height + 4;

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Layer State

        if (hue_redraw || layer_state_redraw) {
            qp_drawtext_recolor(display, xpos, ypos, font_oled, "Layer: ", curr_hsv.primary.h, curr_hsv.primary.s,
                                curr_hsv.primary.v, 0, 0, 0);
            ypos += font_oled->line_height + 4;
            layer_state_t temp = last_layer_state;
            if (is_gaming_layer_active(last_layer_state)) {
                temp = last_layer_state & ~((layer_state_t)1 << _MOUSE);
            }
            snprintf(buf, sizeof(buf), "%10s", get_layer_name_string(temp, false, false));
            qp_drawtext_recolor(display, xpos, ypos, font_oled, buf, curr_hsv.secondary.h, curr_hsv.secondary.s,
                                curr_hsv.secondary.v, 0, 0, 0);
            ypos = 122 + 4;
            xpos = 190;
            qp_drawimage_recolor(display, xpos, ypos, gamepad_icon, curr_hsv.primary.h, curr_hsv.primary.s,
                                 layer_state_cmp(last_layer_state, _GAMEPAD) ? curr_hsv.primary.v : disabled_val, 0, 0,
                                 0);
            qp_drawimage_recolor(display, xpos + gamepad_icon->width + 6, ypos + 4, mouse_icon, curr_hsv.primary.h,
                                 curr_hsv.primary.s,
                                 layer_state_cmp(layer_state, _MOUSE) ? curr_hsv.primary.v : disabled_val, 0, 0, 0);
            ypos += gamepad_icon->height + 2;
            qp_drawtext_recolor(display, xpos, ypos, font_oled, "Diablo",
                                layer_state_cmp(last_layer_state, _DIABLO) ? 0 : curr_hsv.primary.h, curr_hsv.primary.s,
                                layer_state_cmp(last_layer_state, _DIABLO) ? curr_hsv.primary.v : disabled_val, 0, 0,
                                0);
            ypos += font_oled->line_height + 2;
            qp_drawtext_recolor(
                display, xpos, ypos, font_oled, "Diablo 2",
                layer_state_cmp(last_layer_state, _DIABLOII) ? 0 : curr_hsv.primary.h, curr_hsv.primary.s,
                layer_state_cmp(last_layer_state, _DIABLOII) ? curr_hsv.primary.v : disabled_val, 0, 0, 0);
        }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Autocorrection values

#ifdef AUTOCORRECT_ENABLE
        ypos = 122 + 4;

        painter_render_autocorrect(display, font_oled, 5, ypos, width, hue_redraw, &curr_hsv);
        ypos += (font_oled->line_height + 4) * 3;

#endif // AUTOCORRECT_ENABLE

        ypos += font_oled->line_height + 1;

        // Keylogger
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DISPLAY_KEYLOGGER_ENABLE // keep at very end
        ypos = height - (font_mono->line_height + 2);

        painter_render_keylogger(display, font_mono, 27, ypos, width - 27, hue_redraw, &curr_hsv);
#endif // DISPLAY_KEYLOGGER_ENABLE

        // RTC
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        ypos -= (font_oled->line_height + 3);
#ifdef COMMUNITY_MODULE_RTC_ENABLE
        static uint16_t rtc_timer = 0;

        painter_render_rtc_time(display, font_oled, 5, ypos, width, hue_redraw, &rtc_timer, &curr_hsv.primary);
#else
        if (hue_redraw) {
            snprintf(buf, sizeof(buf), "Built on: %s", QMK_BUILDDATE);

            uint8_t title_width = qp_textwidth(font_oled, buf);
            if (title_width > (width - 6)) {
                title_width = width - 6;
            }
            uint8_t title_xpos = (width - title_width) / 2;

            xpos += qp_drawtext_recolor(display, title_xpos, ypos, font_oled, buf, curr_hsv.primary.h,
                                        curr_hsv.primary.s, curr_hsv.primary.v, 0, 0, 0);
        }

#endif // COMMUNITY_MODULE_RTC_ENABLE
#ifdef QUANTUM_PAINTER_DRIVERS_ILI9488_SURFACE
        painter_render_menu_block(menu_surface[0], font_oled, 0, 0, SURFACE_MENU_WIDTH, SURFACE_MENU_HEIGHT, hue_redraw,
                                  &curr_hsv, true, true);
        qp_surface_draw(menu_surface[0], display, 2, 172, false);
#else  // QUANTUM_PAINTER_DRIVERS_ILI9488_SURFACE
        painter_render_menu_block(display, font_oled, 2, 172, 237, 291, hue_redraw, &curr_hsv, true, true);
#endif // QUANTUM_PAINTER_DRIVERS_ILI9488_SURFACE
    } // else
    // right side
    {
        ypos = 20;
        xpos = 5 + width;

        if (hue_redraw) {
            painter_render_frame(display, font_thintel, false, 240, false);
            qp_drawimage_recolor(display, width * 2 - mouse_icon->width - 6, 5, mouse_icon, curr_hsv.primary.h,
                                 curr_hsv.primary.s, curr_hsv.primary.v, 0, 0, 0);
        }

        static uint16_t last_scan_rate = 0;
        if (last_scan_rate != get_matrix_scan_rate() || hue_redraw) {
            last_scan_rate = get_matrix_scan_rate();

            painter_render_scan_rate(display, font_oled, xpos, ypos, true, &curr_hsv);
        }

#if defined(RGBLIGHT_ENABLE)

        ypos = 20;
        xpos = 83 + width;
        painter_render_rgb(display, font_oled, xpos, ypos, hue_redraw, &curr_hsv,
                           "RGB Light Config:", rgblight_get_effect_name, rgblight_get_hsv, rgblight_is_enabled(),
                           RGBLIGHT_LIMIT_VAL);
#endif // RGBLIGHT_ENABLE
        static bool                  is_showing_nuke = true;
        __attribute__((unused)) bool nuke_redraw     = false;
        if (is_showing_nuke != userspace_config.nuke_switch) {
            is_showing_nuke = userspace_config.nuke_switch;
            nuke_redraw     = true;
        }
        if (hue_redraw || nuke_redraw) {
            if (is_showing_nuke) {
                qp_rect(display, 2, 32, 79, 170, 0, 0, 0, true);
            } else {
                qp_drawimage(display, 2 + width, 32, akira_explosion);
            }
        }
#if defined(HAPTIC_ENABLE)
        painter_render_haptic(display, font_oled, 83 + width, 58, hue_redraw, &curr_hsv);
#endif // HAPTIC_ENABLE
        ypos                           = height - (16 + font_oled->line_height);
        static uint16_t last_rtc_timer = 0;
        painter_render_rtc_time(display, font_oled, 5 + width, ypos, width + width, hue_redraw, &last_rtc_timer,
                                &curr_hsv.primary);

#ifdef QUANTUM_PAINTER_DRIVERS_ILI9488_SURFACE
        painter_render_menu_block(menu_surface[1], font_oled, 0, 0, SURFACE_MENU_WIDTH, SURFACE_MENU_HEIGHT, hue_redraw,
                                  &curr_hsv, true, true);
        qp_surface_draw(menu_surface[1], display, 2 + width, 172, false);
#else  // QUANTUM_PAINTER_DRIVERS_ILI9488_SURFACE
        painter_render_menu_block(display, font_oled, 2 + width, 172, 237 + width, 291, hue_redraw, &curr_hsv, true,
                                  true);
#endif // QUANTUM_PAINTER_DRIVERS_ILI9488_SURFACE
    }
    forced_reinit = false;
    qp_flush(display);
}

void ili9488_display_shutdown(bool jump_to_bootloader) {
    ili9488_display_power(true);
    painter_render_shutdown(display, jump_to_bootloader);
}
