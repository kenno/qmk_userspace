// Copyright 2020 @ridingqwerty
// Copyright 2020 @tzarc
// Copyright 2021 Christopher Courtney, aka Drashna Jael're  (@drashna) <drashna@live.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "quantum/unicode/unicode.h"
#include "unicode.h"
#include "process_records.h"
#include "process_unicode_common.h"

/**
 * @brief Main handler for unicode input
 *
 * @param keycode Keycode from switch matrix
 * @param record keyrecord_t data struture
 * @return true Send keycode from matrix to host
 * @return false Stop processing and do not send to host
 */

bool process_record_unicode(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case UC_FLIP: // (ノಠ痊ಠ)ノ彡┻━┻
            if (record->event.pressed) {
                send_unicode_string("(ノಠ痊ಠ)ノ彡┻━┻");
            }
            break;

        case UC_TABL: // ┬─┬ノ( º _ ºノ)
            if (record->event.pressed) {
                send_unicode_string("┬─┬ノ( º _ ºノ)");
            }
            break;

        case UC_SHRG: // ¯\_(ツ)_/¯
            if (record->event.pressed) {
                send_unicode_string("¯\\_(ツ)_/¯");
            }
            break;

        case UC_DISA: // ಠ_ಠ
            if (record->event.pressed) {
                send_unicode_string("ಠ_ಠ");
            }
            break;

        case UC_IRNY: // ⸮
            if (record->event.pressed) {
                register_unicode(0x2E2E);
            }
            break;
        case UC_CLUE: // ‽
            if (record->event.pressed) {
                register_unicode(0x203D);
            }
            break;
    }
    return true;
}

/**
 * @brief Initialize the default unicode mode on firmware startup
 *
 */
void keyboard_post_init_unicode(void) {
    unicode_input_mode_init();
}

/**
 * @brief Set the unicode input mode without extra functionality
 *
 * @param input_mode
 */
void set_unicode_input_mode_soft(uint8_t input_mode) {
    unicode_config.input_mode = input_mode;
    unicode_input_mode_set_kb(input_mode);
}

#ifdef COMMUNITY_MODULE_UNICODE_TYPING_ENABLE
#    include "drashna_runtime.h"
#    include "unicode_typing.h"
void post_process_record_unicode_typing_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        userspace_runtime_state.unicode.typing_mode = get_unicode_typing_mode();
    }
}
#endif
