// Copyright 2025 Christopher Courtney, aka Drashna Jael're  (@drashna) <drashna@live.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "drashna_runtime.h"
#include "timer.h"
#include "wpm.h"

uint8_t wpm_graph_samples[WPM_GRAPH_SAMPLES] = {0};

void update_wpm_avg(void) {
    static uint8_t wpm_samples[10] = {0};
    static uint8_t index           = 0;
    uint16_t       sum             = 0;

    wpm_samples[index] = get_current_wpm();
    index              = (index + 1) % 10;

    for (uint8_t i = 0; i < 10; i++) {
        sum += wpm_samples[i];
    }

    userspace_runtime_state.wpm.wpm_avg = sum / 10;
    if (index == 9) {
        // Shift the wpm_graph_samples array to the right
        for (uint8_t i = WPM_GRAPH_SAMPLES - 1; i > 0; i--) {
            wpm_graph_samples[i] = wpm_graph_samples[i - 1];
        }

        // Update the first element of the array with the new average WPM
        wpm_graph_samples[0] = userspace_runtime_state.wpm.wpm_avg;
    }
}

void keyboard_post_init_wpm(void) {
    userspace_runtime_state.wpm.wpm_avg   = 0;
    userspace_runtime_state.wpm.wpm_peak  = 0;
}

void housekeeping_task_wpm(void) {
    if (is_keyboard_master()) {
        static uint32_t timer    = 0;
        static uint16_t interval = 0;

        if (userspace_runtime_state.wpm.wpm_peak < get_current_wpm()) {
            userspace_runtime_state.wpm.wpm_peak = get_current_wpm();
            timer                                = timer_read32();
        } else if (timer_elapsed32(timer) > 10000) {
            userspace_runtime_state.wpm.wpm_peak = get_current_wpm();
        }
        if (timer_elapsed(interval) >= 100) {
            update_wpm_avg();
            interval = timer_read();
        }
    }
}
