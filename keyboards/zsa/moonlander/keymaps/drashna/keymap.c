// Copyright 2023 Christopher Courtney, aka Drashna Jael're  (@drashna) <drashna@live.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "drashna.h"

enum more_custom_keycodes {
    KC_SWAP_NUM = USER_SAFE_RANGE,
};

// clang-format off
#define LAYOUT_moonlander_wrapper(...) LAYOUT_moonlander(__VA_ARGS__)

#define LAYOUT_moonlander_base( \
    K01, K02, K03, K04, K05, K06, K07, K08, K09, K0A, \
    K11, K12, K13, K14, K15, K16, K17, K18, K19, K1A, K1B, \
    K21, K22, K23, K24, K25, K26, K27, K28, K29, K2A  \
    ) \
    LAYOUT_moonlander_wrapper( \
        KC_ESC,  ________________NUMBER_LEFT________________, UC_FLIP,        UC_TABL, ________________NUMBER_RIGHT_______________, KC_MINS, \
        SH_TT,   K01,     K02,     K03,     K04,     K05,     TG_DBLO,        TG_DBLO, K06,     K07,     K08,     K09,     K0A,     SH_TT, \
        KC_C1R3, K11,     K12,     K13,     K14,     K15,     TG_GAME,        TG_GAME, K16,     K17,     K18,     K19,     K1A,     RALT_T(K1B), \
        KC_MLSF, CTL_T(K21), K22,  K23,     K24,     K25,                              K26,     K27,     K28,     K29,  RCTL_T(K2A),KC_MRSF, \
        KC_GRV,  OS_MEH,  OS_HYPR, KC_LBRC, KC_RBRC,          UC_CLUE,        KC_DEL,           KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, UC_IRNY, \
                                            KC_SPC,  BK_LWER, OS_LALT,        OS_RGUI, DL_RAIS, KC_ENT \
    )

#define LAYOUT_base_wrapper(...) LAYOUT_moonlander_base(__VA_ARGS__)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_DEFAULT_LAYER_1] = LAYOUT_base_wrapper(
        _________________QWERTY_L1_________________, _________________QWERTY_R1_________________,
        _________________QWERTY_L2_________________, _________________QWERTY_R2_________________,
        _________________QWERTY_L3_________________, _________________QWERTY_R3_________________
    ),

    [_DEFAULT_LAYER_2] = LAYOUT_base_wrapper(
        ______________COLEMAK_MOD_DH_L1____________, ______________COLEMAK_MOD_DH_R1____________,
        ______________COLEMAK_MOD_DH_L2____________, ______________COLEMAK_MOD_DH_R2____________,
        ______________COLEMAK_MOD_DH_L3____________, ______________COLEMAK_MOD_DH_R3____________
    ),
    [_DEFAULT_LAYER_3] = LAYOUT_base_wrapper(
        _________________COLEMAK_L1________________, _________________COLEMAK_R1________________,
        _________________COLEMAK_L2________________, _________________COLEMAK_R2________________,
        _________________COLEMAK_L3________________, _________________COLEMAK_R3________________
    ),

    [_DEFAULT_LAYER_4] = LAYOUT_base_wrapper(
        _________________DVORAK_L1_________________, _________________DVORAK_R1_________________,
        _________________DVORAK_L2_________________, _________________DVORAK_R2_________________,
        _________________DVORAK_L3_________________, _________________DVORAK_R3_________________
    ),


    [_GAMEPAD] = LAYOUT_moonlander_wrapper(
        KC_ESC,  XXXXXXX, KC_1,    KC_2,    KC_3,    KC_4, HYPR(KC_Q),                 _______, KC_F9,   KC_F10,  KC_F11,  KC_F12,  XXXXXXX, KC_NO,
        KC_F1,   KC_K,    KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,                    UC_SHRG, UC_DISA, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, KC_NO,
        KC_TAB,  ALT_T(KC_G),KC_A, KC_S,    KC_D,    KC_F,    _______,            TG_GAME, KC_I,    KC_O,    XXXXXXX, XXXXXXX, XXXXXXX, KC_NO,
        KC_LCTL, KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,                                      KC_N,    KC_M,    XXXXXXX, XXXXXXX, XXXXXXX, KC_NO,
        KC_GRV,  KC_U,    KC_I,    KC_Y,    KC_T,             KC_PSCR,                 _______,          KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, KC_NO,
                                            KC_V,    KC_SPC,  KC_H,                    KC_NO, XXXXXXX,KC_SWAP_NUM
    ),
    [_DIABLO] = LAYOUT_moonlander_wrapper(
        KC_ESC,  KC_V,    KC_D,    KC_LALT, XXXXXXX, XXXXXXX, XXXXXXX,                 _______, KC_F9,   KC_F10,   KC_F11,  KC_F12,  XXXXXXX, KC_NO,
        KC_TAB,  KC_S,    KC_I,    KC_F,    KC_M,    KC_T,    _______,                 _______, XXXXXXX, XXXXXXX,  XXXXXXX, XXXXXXX, XXXXXXX, KC_NO,
        XXXXXXX, KC_1,    KC_2,    KC_3,    KC_4,    KC_G,    XXXXXXX,                 _______, XXXXXXX, XXXXXXX,  XXXXXXX, XXXXXXX, XXXXXXX, KC_NO,
        KC_LCTL, KC_D3_1, KC_D3_2, KC_D3_3, KC_D3_4, KC_Z,                                      KC_N,    KC_M,     XXXXXXX, XXXXXXX, XXXXXXX, KC_NO,
        XXXXXXX, XXXXXXX, KC_L,    KC_J,    KC_F,             KC_PSCR,                 XXXXXXX,          XXXXXXX,  XXXXXXX, XXXXXXX, XXXXXXX, KC_NO,
                          KC_DIABLO_CLEAR,  SFT_T(KC_SPACE),  ALT_T(KC_Q),             KC_PGDN, KC_DEL,  KC_ENT
    ),
    [_DIABLOII] = LAYOUT_moonlander_wrapper(
        KC_ESC,  KC_1,    KC_2,    KC_3,    KC_4,    KC_GRV,  XXXXXXX,                 _______, KC_F9,   KC_F10,   KC_F11,  KC_F12,  XXXXXXX, KC_NO,
        KC_TAB,  KC_A,    KC_T,    KC_Q,    KC_I,    KC_M,    TG(_DIABLOII),           _______, XXXXXXX, XXXXXXX,  XXXXXXX, XXXXXXX, XXXXXXX, KC_NO,
        KC_S,    KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   XXXXXXX,                 _______, XXXXXXX, XXXXXXX,  XXXXXXX, XXXXXXX, XXXXXXX, KC_NO,
        KC_LCTL, KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,                                    KC_N,    KC_M,     XXXXXXX, XXXXXXX, XXXXXXX, KC_NO,
        XXXXXXX, XXXXXXX, KC_G,    KC_F,    KC_L,             KC_V,                    XXXXXXX,          XXXXXXX,  XXXXXXX, XXXXXXX, XXXXXXX, KC_NO,
                                            KC_G,    KC_LSFT, KC_LCTL,                 KC_PGDN, KC_DEL,  KC_ENT
    ),

    [_LOWER] = LAYOUT_moonlander_wrapper(
        KC_F12,  _________________FUNC_LEFT_________________, _______,        _______, _________________FUNC_RIGHT________________, KC_F11,
        _______, _________________LOWER_L1__________________, _______,        _______, _________________LOWER_R1__________________, KC_PIPE,
        _______, _________________LOWER_L2__________________, _______,        _______, _________________LOWER_R2__________________, KC_DQUO,
        _______, _________________LOWER_L3__________________,                          _________________LOWER_R3__________________, KC_PSCR,
        _______, _______, _______, _______, _______,          _______,        _______,          _______, _______, _______, _______, KC_PAUS,
                                            _______, _______, _______,        _______, _______, _______
    ),

    [_RAISE] = LAYOUT_moonlander_wrapper(
        KC_F12,  _________________FUNC_LEFT_________________, _______,        _______, _________________FUNC_RIGHT________________, KC_F11,
        _______, _________________RAISE_L1__________________, _______,        _______, _________________RAISE_R1__________________, _______,
        _______, _________________RAISE_L2__________________, _______,        _______, _________________RAISE_R2__________________, KC_BSLS,
        _______, _________________RAISE_L3__________________,                          _________________RAISE_R3__________________, KC_PSCR,
        _______, _______, _______, _______, _______,          _______,        _______,          KC_HOME, KC_PGDN, KC_PGUP, KC_END,  KC_PAUS,
                                            _______, _______, _______,        _______, _______, _______
    ),

    [_ADJUST] = LAYOUT_moonlander_wrapper(
        QK_MAKE, KC_WIDE,KC_AUSSIE,KC_SCRIPT,KC_ZALGO,KC_NOMODE,_______,   KC_NUKE,KC_NOMODE,KC_BLOCKS,KC_REGIONAL,_______,_______, QK_BOOT,
        VRSN,    _________________ADJUST_L1_________________, TG(_DIABLOII),  _______, _________________ADJUST_R1_________________, EE_CLR,
        KEYLOCK, _________________ADJUST_L2_________________, _______,        _______, _________________ADJUST_R2_________________, RGB_IDL,
        UC_NEXT, _________________ADJUST_L3_________________,                          _________________ADJUST_R3_________________, TG_MODS,
        _______, _______, _______, _______, _______,          _______,        US_MSRP,          _______, _______, _______, _______, AC_TOGG,
                                            QK_RBT,  _______, _______,        _______, _______, DB_TOGG
    ),
};

/*  Blank

    [_LAYER] = LAYOUT_moonlander_wrapper(
        _______, _______, _______, _______, _______, _______, _______,        _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______,        _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______,        _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______,                          _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______,          _______,        _______,          _______, _______, _______, _______, _______,
                                            _______, _______, _______,        _______, _______, _______
    ),

*/
// clang-format on

bool process_record_keymap(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case KC_1:
            if (layer_state_is(_GAMEPAD) && userspace_config.gaming.swapped_numbers) {
                if (record->event.pressed) {
                    register_code(KC_2);
                } else {
                    unregister_code(KC_2);
                }
                return false;
            }
            break;
        case KC_2:
            if (layer_state_is(_GAMEPAD) && userspace_config.gaming.swapped_numbers) {
                if (record->event.pressed) {
                    register_code(KC_1);
                } else {
                    unregister_code(KC_1);
                }
                return false;
            }
            break;
        case KC_SWAP_NUM:
            if (record->event.pressed) {
                userspace_config.gaming.swapped_numbers ^= 1;
                eeconfig_update_user_datablock_handler(&userspace_config, 0, EECONFIG_USER_DATA_SIZE);
                unregister_code(KC_1);
                unregister_code(KC_2);
            }
            break;
    }
    return true;
}

#ifdef RGB_MATRIX_ENABLE

#    if defined(RGBLIGHT_ENABLE) && defined(RGBLIGHT_CUSTOM)
const uint8_t led_mapping[RGBLIGHT_LED_COUNT] = {0,  1,  2,  3,  4,  9,  14, 19, 24, 29, 30, 31, 32, 33, 34, 35,
                                                 71, 70, 69, 68, 67, 66, 65, 60, 55, 50, 45, 40, 39, 38, 37, 36};
#    endif

#    ifdef COMMUNITY_MODULE_INDICATORS_ENABLE
#        include "elpekenin/indicators.h"

const indicator_t PROGMEM indicators[] = {
#        if __GNUC__ > 8
    // clang-format off
    KEYCODE_WITH_MOD_INDICATOR(OSM(MOD_LSFT), MOD_BIT_LSHIFT, HSV_COLOR(HSV_CYAN)),
    KEYCODE_WITH_MOD_INDICATOR(OSM(MOD_RSFT), MOD_BIT_RSHIFT, HSV_COLOR(HSV_CYAN)),
    KEYCODE_WITH_MOD_INDICATOR(KC_LSFT, MOD_BIT_LSHIFT, HSV_COLOR(HSV_CYAN)),
    KEYCODE_WITH_MOD_INDICATOR(KC_RSFT, MOD_BIT_RSHIFT, HSV_COLOR(HSV_CYAN)), // 4

    KEYCODE_WITH_MOD_INDICATOR(OSM(MOD_LCTL), MOD_BIT_LCTRL, HSV_COLOR(HSV_RED)),
    KEYCODE_WITH_MOD_INDICATOR(OSM(MOD_RCTL), MOD_BIT_RCTRL, HSV_COLOR(HSV_RED)),
    KEYCODE_WITH_MOD_INDICATOR(KC_LCTL, MOD_BIT_LCTRL, HSV_COLOR(HSV_RED)),
    KEYCODE_WITH_MOD_INDICATOR(KC_RCTL, MOD_BIT_RCTRL, HSV_COLOR(HSV_RED)),
    KEYCODE_WITH_MOD_INDICATOR(CTL_T(KC_Z), MOD_BIT_LCTRL, HSV_COLOR(HSV_RED)),
    KEYCODE_WITH_MOD_INDICATOR(RCTL_T(KC_SLSH), MOD_BIT_RCTRL, HSV_COLOR(HSV_RED)), // 10

    KEYCODE_WITH_MOD_INDICATOR(OSM(MOD_LALT), MOD_BIT_LALT, HSV_COLOR(HSV_YELLOW)),
    KEYCODE_WITH_MOD_INDICATOR(OSM(MOD_RALT), MOD_BIT_RALT, HSV_COLOR(HSV_YELLOW)),
    KEYCODE_WITH_MOD_INDICATOR(KC_LALT, MOD_BIT_LALT, HSV_COLOR(HSV_YELLOW)),
    KEYCODE_WITH_MOD_INDICATOR(KC_RALT, MOD_BIT_RALT, HSV_COLOR(HSV_YELLOW)), // 14

    KEYCODE_WITH_MOD_INDICATOR(OSM(MOD_LGUI), MOD_BIT_LGUI, HSV_COLOR(HSV_GREEN)),
    KEYCODE_WITH_MOD_INDICATOR(OSM(MOD_RGUI), MOD_BIT_RGUI, HSV_COLOR(HSV_GREEN)),
    KEYCODE_WITH_MOD_INDICATOR(KC_LGUI, MOD_BIT_LGUI, HSV_COLOR(HSV_GREEN)),
    KEYCODE_WITH_MOD_INDICATOR(KC_RGUI, MOD_BIT_RGUI, HSV_COLOR(HSV_GREEN)), // 18

    KEYCODE_INDICATOR(QK_BOOTLOADER, HSV_COLOR(HSV_RED)),
    KEYCODE_INDICATOR(QK_MAKE, HSV_COLOR(HSV_GREEN)),
    KEYCODE_INDICATOR(QK_REBOOT, HSV_COLOR(HSV_YELLOW)),
    KEYCODE_INDICATOR(QK_CLEAR_EEPROM, HSV_COLOR(HSV_BLUE)), // 22

    KEYCODE_IN_LAYER_INDICATOR(KC_1, _GAMEPAD, HSV_COLOR(HSV_WHITE)),
    KEYCODE_IN_LAYER_INDICATOR(KC_2, _GAMEPAD, HSV_COLOR(HSV_GREEN)),
    KEYCODE_IN_LAYER_INDICATOR(KC_3, _GAMEPAD, HSV_COLOR(HSV_PURPLE)),
    KEYCODE_IN_LAYER_INDICATOR(KC_Q, _GAMEPAD, HSV_COLOR(HSV_GREEN)),
    KEYCODE_IN_LAYER_INDICATOR(KC_W, _GAMEPAD, HSV_COLOR(HSV_CYAN)),
    KEYCODE_IN_LAYER_INDICATOR(KC_E, _GAMEPAD, HSV_COLOR(HSV_RED)),
    KEYCODE_IN_LAYER_INDICATOR(KC_R, _GAMEPAD, HSV_COLOR(HSV_ORANGE)),
    KEYCODE_IN_LAYER_INDICATOR(KC_A, _GAMEPAD, HSV_COLOR(HSV_CYAN)),
    KEYCODE_IN_LAYER_INDICATOR(KC_S, _GAMEPAD, HSV_COLOR(HSV_CYAN)),
    KEYCODE_IN_LAYER_INDICATOR(KC_D, _GAMEPAD, HSV_COLOR(HSV_CYAN)), // 32
// clang-format on
#        else
    NULL
#        endif
};
#    endif

bool rgb_matrix_indicators_advanced_keymap(uint8_t led_min, uint8_t led_max) {
#    ifdef COMMUNITY_MODULE_INDICATORS_ENABLE
    void rgb_matrix_indicators_render_layer(uint8_t led_min, uint8_t led_max);

    rgb_matrix_indicators_render_layer(led_min, led_max);
    return false;
#    endif
    return true;
}
#endif

#ifdef TAPPING_TERM_PER_KEY
uint16_t get_tapping_term_keymap(uint16_t keycode, keyrecord_t *record) {
    if (keycode >= QK_MOD_TAP && keycode <= QK_MOD_TAP_MAX) {
        if (mod_config(keycode & 0xf) & MOD_MASK_ALT) {
            return (2 * TAPPING_TERM);
        }
    }
    return TAPPING_TERM;
}
#endif
