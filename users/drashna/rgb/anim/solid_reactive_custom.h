// Copyright 2025 Christopher Courtney, aka Drashna Jael're  (@drashna) <drashna@live.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifdef RGB_MATRIX_KEYREACTIVE_ENABLED
RGB_MATRIX_EFFECT(SOLID_REACTIVE_SIMPLE_INCREASE)
RGB_MATRIX_EFFECT(SOLID_REACTIVE_SIMPLE_DECREASE)
#    ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

static hsv_t SOLID_REACTIVE_SIMPLE_INC_math(hsv_t hsv, uint16_t offset) {
    hsv.v = scale8(offset - 1, hsv.v);
    return hsv;
}
static hsv_t SOLID_REACTIVE_SIMPLE_DEC_math(hsv_t hsv, uint16_t offset) {
    uint8_t temp = hsv.v;
    hsv.v        = scale8(255 - offset, hsv.v);
    if (hsv.v == (temp - 1)) {
        hsv.v = 0;
    }
    return hsv;
}

bool SOLID_REACTIVE_SIMPLE_INCREASE(effect_params_t* params) {
    return effect_runner_reactive(params, &SOLID_REACTIVE_SIMPLE_INC_math);
}
bool SOLID_REACTIVE_SIMPLE_DECREASE(effect_params_t* params) {
    return effect_runner_reactive(params, &SOLID_REACTIVE_SIMPLE_DEC_math);
}

#    endif // RGB_MATRIX_CUSTOM_EFFECT_IMPLS
#endif     // RGB_MATRIX_KEYREACTIVE_ENABLED
