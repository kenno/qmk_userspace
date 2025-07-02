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
    // start at 257 so that non-reactive keys are out of bounds and treated as 0
    hsv.v = scale8(257 - offset, hsv.v);
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
