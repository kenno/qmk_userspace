// Copyright 2020 Christopher Courtney, aka Drashna Jael're  (@drashna) <drashna@live.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "transport_sync.h"
#include "_wait.h"
#include "drashna.h"
#include "transactions.h"
#include <string.h>
#include "split_util.h"
#ifdef COMMUNITY_MODULE_KEYBOARD_LOCK_ENABLE
#    include "keyboard_lock.h"
#endif // COMMUNITY_MODULE_KEYBOARD_LOCK_ENABLE
#if defined(COMMUNITY_MODULE_DISPLAY_MENU_ENABLE)
#    include "display_menu.h"
#endif // COMMUNITY_MODULE_DISPLAY_MENU_ENABLE

#ifdef UNICODE_COMMON_ENABLE
#    include "process_unicode_common.h"
extern unicode_config_t unicode_config;
#    include "keyrecords/unicode.h"
#endif
#ifdef AUDIO_ENABLE
#    include "audio.h"
extern audio_config_t audio_config;
extern bool           delayed_tasks_run;
#endif
#ifdef SWAP_HANDS_ENABLE
extern bool swap_hands;
#endif
#ifdef WPM_ENABLE
extern uint8_t wpm_graph_samples[WPM_GRAPH_SAMPLES];
#endif // WPM_ENABLE
#ifdef DISPLAY_DRIVER_ENABLE
#    include "display/display.h"
#endif // DISPLAY_DRIVER_ENABLEj
#ifndef FORCED_SYNC_THROTTLE_MS
#    define FORCED_SYNC_THROTTLE_MS 100
#endif // FORCED_SYNC_THROTTLE_MS

bool has_first_run = false;

typedef enum PACKED extended_id_t {
    RPC_ID_EXTENDED_WPM_GRAPH_DATA = 0,
    RPC_ID_EXTENDED_AUTOCORRECT_STR,
    RPC_ID_EXTENDED_DISPLAY_KEYLOG_STR,
    RPC_ID_EXTENDED_KEYMAP_CONFIG,
    RPC_ID_EXTENDED_DEBUG_CONFIG,
    RPC_ID_EXTENDED_USERSPACE_CONFIG,
    RPC_ID_EXTENDED_USERSPACE_RUNTIME_STATE,
    RPC_ID_EXTENDED_SUSPEND_STATE,
    NUM_EXTENDED_IDS,
} extended_id_t;

_Static_assert(sizeof(extended_id_t) == 1, "extended_id_t is not 1 byte!");

#define RPC_EXTENDED_TRANSACTION_OVERHEAD    (sizeof(extended_id_t) + sizeof(uint8_t))
#define RPC_EXTENDED_TRANSACTION_BUFFER_SIZE (RPC_M2S_BUFFER_SIZE - RPC_EXTENDED_TRANSACTION_OVERHEAD)

typedef struct PACKED extended_msg_t {
    extended_id_t id;
    uint8_t       size;
    uint8_t       data[RPC_EXTENDED_TRANSACTION_BUFFER_SIZE];
} extended_msg_t;

_Static_assert(sizeof(extended_msg_t) == RPC_M2S_BUFFER_SIZE, "extended_rpc_t is larger than split buffer size!");
_Static_assert(sizeof(userspace_config_t) <= RPC_EXTENDED_TRANSACTION_BUFFER_SIZE,
               "userspace_config_t is larger than split buffer size!");
_Static_assert(sizeof(userspace_runtime_state_t) <= RPC_EXTENDED_TRANSACTION_BUFFER_SIZE,
               "userspace_runtime_state_t is larger than split buffer size!");

typedef void (*handler_fn_t)(const uint8_t* data, uint8_t size);

void recv_wpm_graph_data(const uint8_t* data, uint8_t size) {
#ifdef WPM_ENABLE
    if (memcmp(data, wpm_graph_samples, size) != 0) {
        memcpy(wpm_graph_samples, data, size);
    }
#endif
}

#ifdef AUTOCORRECT_ENABLE
extern char autocorrected_str[2][21];
extern char autocorrected_str_raw[2][21];
extern bool autocorrect_str_has_changed;
_Static_assert(sizeof(autocorrected_str) <= RPC_EXTENDED_TRANSACTION_BUFFER_SIZE,
               "Autocorrect array larger than buffer size!");
_Static_assert(sizeof(autocorrected_str_raw) <= RPC_EXTENDED_TRANSACTION_BUFFER_SIZE,
               "Autocorrect array larger than buffer size!");
#endif

void recv_autocorrect_string(const uint8_t* data, uint8_t size) {
#ifdef AUTOCORRECT_ENABLE
    if (memcmp(data, &autocorrected_str_raw, size) != 0) {
        memcpy(autocorrected_str_raw, data, size);
        center_text(autocorrected_str_raw[0], autocorrected_str[0], sizeof(autocorrected_str[0]) - 1);
        center_text(autocorrected_str_raw[1], autocorrected_str[1], sizeof(autocorrected_str[1]) - 1);
        autocorrect_str_has_changed = true;
    }
#endif
}

void recv_keylogger_string_sync(const uint8_t* data, uint8_t size) {
#if defined(DISPLAY_DRIVER_ENABLE) && defined(DISPLAY_KEYLOGGER_ENABLE)
    if (memcmp(data, &display_keylogger_string, size) != 0) {
        memcpy(&display_keylogger_string, data, size);
        keylogger_has_changed = true;
    }
#endif // DISPLAY_DRIVER_ENABLE && DISPLAY_KEYLOGGER_ENABLE
}

void recv_keymap_config(const uint8_t* data, uint8_t size) {
    if (memcmp(data, &keymap_config, size) != 0) {
        memcpy(&keymap_config, data, size);
        eeconfig_update_keymap(keymap_config.raw);
    }
}

void recv_debug_config(const uint8_t* data, uint8_t size) {
    if (memcmp(data, &debug_config, size) != 0) {
        memcpy(&debug_config, data, size);
        eeconfig_update_debug(debug_config.raw);
    }
}

void recv_userspace_config(const uint8_t* data, uint8_t size) {
    if (memcmp(data, &userspace_config, size) != 0) {
        memcpy(&userspace_config, data, size);
        eeconfig_update_user_datablock_handler(&userspace_config, 0, EECONFIG_USER_DATA_SIZE);
#if defined(DISPLAY_DRIVER_ENABLE)
        static uint8_t last_inverted = 0xFF, last_rotation = 0xFF;
        if (userspace_config.display.inverted != last_inverted || userspace_config.display.rotation != last_rotation) {
            last_inverted = userspace_config.display.inverted;
            last_rotation = userspace_config.display.rotation;
#    if defined(QUANTUM_PAINTER_ILI9341_ENABLE)
            void init_display_ili9341_rotation(void);
            init_display_ili9341_rotation();
#    endif // QUANTUM_PAINTER_ILI9341_ENABLE
#    if defined(OLED_ENABLE)
            void oled_post_init(void);
            oled_post_init();
#    endif // OLED_ENABLE
        }
#endif // DISPLAY_DRIVER_ENABLE
    }
}

void recv_userspace_runtime_state(const uint8_t* data, uint8_t size) {
    if (memcmp(data, &userspace_runtime_state, size) != 0) {
        memcpy(&userspace_runtime_state, data, size);

#ifdef AUDIO_ENABLE
        bool has_audio_changed = false;
        if (userspace_runtime_state.audio.enable != is_audio_on()) {
            audio_config.enable = userspace_runtime_state.audio.enable;
            has_audio_changed   = true;
        }
#    ifdef AUDIO_CLICKY
        if (userspace_runtime_state.audio.clicky_enable != is_clicky_on()) {
            audio_config.clicky_enable = userspace_runtime_state.audio.clicky_enable;
            has_audio_changed          = true;
        }
        extern float clicky_freq;
        extern float clicky_rand;
        clicky_freq = userspace_runtime_state.audio.clicky_freq;
        clicky_rand = userspace_runtime_state.audio.clicky_rand;
#    endif // AUDIO_CLICK
#    ifdef MUSIC_ENABLE
        if (userspace_runtime_state.audio.music_enable != is_music_on()) {
            userspace_runtime_state.audio.music_enable ? music_on() : music_off();
        }
#    endif // MUSIC_ENABLE
        if (has_audio_changed) {
            static audio_config_t last_audio_config = {0};
            last_audio_config.raw = eeconfig_read_audio();
            if (last_audio_config.raw != audio_config.raw) {
                eeconfig_update_audio(audio_config.raw);
            }
        }
#endif // AUDIO_ENABLE

#ifdef UNICODE_COMMON_ENABLE
        if (get_unicode_input_mode() != userspace_runtime_state.unicode.mode) {
            unicode_config.input_mode = userspace_runtime_state.unicode.mode;
            eeprom_update_byte(EECONFIG_UNICODEMODE, unicode_config.input_mode);
        }
#endif // UNICODE_COMMON_ENABLE
#if defined(POINTING_DEVICE_ENABLE) && defined(POINTING_DEVICE_AUTO_MOUSE_ENABLE)
        if (get_auto_mouse_toggle() != userspace_runtime_state.internals.tap_toggling) {
            auto_mouse_toggle();
        }
#endif // POINTING_DEVICE_ENABLE && POINTING_DEVICE_AUTO_MOUSE_ENABLE
#ifdef SWAP_HANDS_ENABLE
        swap_hands = userspace_runtime_state.internals.swap_hands;
#endif // SWAP_HANDS_ENABLE
#ifdef CAPS_WORD_ENABLE
        if (is_caps_word_on() != userspace_runtime_state.internals.is_caps_word) {
            caps_word_toggle();
        }
#endif // CAPS_WORD_ENABLE
#ifdef COMMUNITY_MODULE_KEYBOARD_LOCK_ENABLE
        set_keyboard_lock(userspace_runtime_state.internals.host_driver_disabled);
#endif // COMMUNITY_MODULE_KEYBOARD_LOCK_ENABLE
#if defined(DISPLAY_DRIVER_ENABLE)
#    if defined(COMMUNITY_MODULE_DISPLAY_MENU_ENABLE)
        extern menu_state_t menu_state;
        if (!userspace_runtime_state.display.menu_state_runtime.has_rendered &&
            userspace_runtime_state.display.menu_state_runtime.dirty) {
            userspace_runtime_state.display.menu_state_runtime.has_rendered = true;
            userspace_runtime_state.display.menu_state_runtime.dirty        = false;
        }

        if (memcmp(&menu_state, &userspace_runtime_state.display.menu_state, sizeof(menu_state_t)) != 0) {
            memcpy(&menu_state, &userspace_runtime_state.display.menu_state, sizeof(menu_state_t));
        }
        if (userspace_runtime_state.display.menu_state_runtime.dirty) {
            display_menu_set_dirty(true);
        }
#    endif
#endif // QUANTUM_PAINTER_ENABLE && CUSTOM_QUANTUM_PAINTER_ENABLE
    }
}

void recv_device_suspend_state(const uint8_t* data, uint8_t size) {
    static bool is_device_suspended = false;
    if (memcmp(data, &is_device_suspended, size) != 0) {
        memcpy(&is_device_suspended, data, size);
        set_is_device_suspended(is_device_suspended);
    }
}

static const handler_fn_t handlers[NUM_EXTENDED_IDS] = {
    [RPC_ID_EXTENDED_WPM_GRAPH_DATA]          = recv_wpm_graph_data,
    [RPC_ID_EXTENDED_AUTOCORRECT_STR]         = recv_autocorrect_string,
    [RPC_ID_EXTENDED_DISPLAY_KEYLOG_STR]      = recv_keylogger_string_sync,
    [RPC_ID_EXTENDED_KEYMAP_CONFIG]           = recv_keymap_config,
    [RPC_ID_EXTENDED_DEBUG_CONFIG]            = recv_debug_config,
    [RPC_ID_EXTENDED_USERSPACE_CONFIG]        = recv_userspace_config,
    [RPC_ID_EXTENDED_USERSPACE_RUNTIME_STATE] = recv_userspace_runtime_state,
    [RPC_ID_EXTENDED_SUSPEND_STATE]           = recv_device_suspend_state,
};

void extended_message_handler(uint8_t initiator2target_buffer_size, const void* initiator2target_buffer,
                              uint8_t target2initiator_buffer_size, void* target2initiator_buffer) {
    extended_msg_t msg = {0};
    memcpy(&msg, initiator2target_buffer, initiator2target_buffer_size);
    if (msg.id >= NUM_EXTENDED_IDS) {
        xprintf("Invalid extended message ID: %d\n", msg.id);
        return;
    }
    if (msg.size > RPC_EXTENDED_TRANSACTION_BUFFER_SIZE) {
        xprintf("Invalid extended message size: %d (ID: %d)\n", msg.size, msg.id);
    }

    handler_fn_t handler = handlers[msg.id];
    // xprintf("Extended Transaction received:\nID: %d, Size: %d, data:\n  ", msg.id, msg.size);
    // for (uint8_t i = 0; i < msg.size; ++i) {
    //     xprintf("%d ", msg.data[i]);
    // }
    // xprintf("\n");

    handler(msg.data, msg.size);
}

bool send_extended_message_handler(enum extended_id_t id, const void* data, uint8_t size) {
    if (size > RPC_EXTENDED_TRANSACTION_BUFFER_SIZE) {
        xprintf("Invalid extended message size: %d (ID: %d)\n", size, id);
        return false;
    }
    extended_msg_t msg = {
        .id   = id,
        .size = size,
        .data = {0},
    };
    memcpy(&msg.data, data, size);
    // xprintf("Extended Transaction sent:\nID: %d, Size: %d, data:\n  ", msg.id, msg.size);
    // for (uint8_t i = 0; i < size; ++i) {
    //     xprintf("%d ", msg.data[i]);
    // }
    // xprintf("\n");
    return transaction_rpc_send(RPC_ID_EXTENDED_SYNC_TRANSPORT, sizeof(extended_msg_t), &msg);
}

/**
 * @brief Send the suspend state to the other half of the split keyboard
 *
 * @param status
 */
void send_device_suspend_state(bool status) {
    if (is_device_suspended() != status && is_keyboard_master()) {
        if (send_extended_message_handler(RPC_ID_EXTENDED_SUSPEND_STATE, &status, sizeof(bool))) {
            wait_ms(5);
        }
    }
}

/**
 * @brief Synchronizes the userspace runtime state.
 *
 * This function checks if the userspace runtime state needs to be synchronized
 * and performs the synchronization if necessary. It updates the last synchronization
 * time and the last known user runtime state.
 *
 * @param needs_sync Pointer to a boolean indicating if synchronization is needed.
 * @param last_sync Pointer to the timestamp of the last synchronization.
 * @param last_user_state Pointer to the last known user runtime configuration.
 */
void sync_userspace_runtime_state(void) {
    bool                         needs_sync      = false;
    static uint16_t              last_sync       = 0;
    static userspace_runtime_state_t last_user_state = {0};

#ifdef COMMUNITY_MODULE_KEYBOARD_LOCK_ENABLE
    userspace_runtime_state.internals.host_driver_disabled = get_keyboard_lock();
#endif // COMMUNITY_MODULE_KEYBOARD_LOCK_ENABLE
#if defined(POINTING_DEVICE_ENABLE) && defined(POINTING_DEVICE_AUTO_MOUSE_ENABLE)
    userspace_runtime_state.internals.tap_toggling = get_auto_mouse_toggle();
#endif // POINTING_DEVICE_ENABLE && POINTING_DEVICE_AUTO_MOUSE_ENABLE
#ifdef UNICODE_COMMON_ENABLE
    userspace_runtime_state.unicode.mode = get_unicode_input_mode();
#endif // UNICODE_COMMON_ENABLE
#ifdef SWAP_HANDS_ENABLE
    userspace_runtime_state.internals.swap_hands = swap_hands;
#endif // SWAP_HANDS_ENABLE
#ifdef CAPS_WORD_ENABLE
    userspace_runtime_state.internals.is_caps_word = is_caps_word_on();
#endif // CAPS_WORD_ENABLE

#ifdef DISPLAY_DRIVER_ENABLE
#    ifdef COMMUNITY_MODULE_DISPLAY_MENU_ENABLE
    extern menu_state_t menu_state;
    if (memcmp(&menu_state, &userspace_runtime_state.display.menu_state, sizeof(menu_state_t)) != 0) {
        memcpy(&userspace_runtime_state.display.menu_state, &menu_state, sizeof(menu_state_t));
    }
#    endif // COMMUNITY_MODULE_DISPLAY_MENU_ENABLE
#endif     // DISPLAY_DRIVER_ENABLE

    if (memcmp(&userspace_runtime_state, &last_user_state, sizeof(userspace_runtime_state_t))) {
        needs_sync = true;
        memcpy(&last_user_state, &userspace_runtime_state, sizeof(userspace_runtime_state_t));
    }
    if (timer_elapsed(last_sync) > FORCED_SYNC_THROTTLE_MS) {
        needs_sync = true;
    }
    if (needs_sync) {
        if (send_extended_message_handler(RPC_ID_EXTENDED_USERSPACE_RUNTIME_STATE, &userspace_runtime_state,
                                          sizeof(userspace_runtime_state_t))) {
            last_sync = timer_read32();
        }
        needs_sync = false;
    }
}

/**
 * @brief Synchronizes the userspace configuration if needed.
 *
 * This function checks if the userspace configuration needs to be synchronized.
 * If synchronization is required, it updates the last synchronization time and
 * the last known configuration.
 *
 * @param needs_sync Pointer to a boolean indicating if synchronization is needed.
 * @param last_sync Pointer to the last synchronization timestamp.
 * @param last_config Pointer to the last known userspace configuration.
 */
void sync_userspace_config(void) {
    bool                      needs_sync  = false;
    static uint16_t           last_sync   = 0;
    static userspace_config_t last_config = {0};

    if (memcmp(&userspace_config, &last_config, sizeof(userspace_config_t))) {
        needs_sync = true;
        memcpy(&last_config, &userspace_config, sizeof(userspace_config_t));
    }
    if (timer_elapsed(last_sync) > FORCED_SYNC_THROTTLE_MS) {
        needs_sync = true;
    }
    if (needs_sync) {
        if (send_extended_message_handler(RPC_ID_EXTENDED_USERSPACE_CONFIG, &userspace_config,
                                          sizeof(userspace_config_t))) {
            last_sync = timer_read32();
        }
        needs_sync = false;
    }
}

#if defined(DISPLAY_DRIVER_ENABLE) && defined(DISPLAY_KEYLOGGER_ENABLE)
/**
 * @brief Synchronizes the keylogger string between split keyboards.
 *
 * This function ensures that the keylogger string is synchronized across split keyboards.
 * It checks if synchronization is needed and performs the synchronization if required.
 *
 * @param needs_sync Pointer to a boolean indicating if synchronization is needed.
 * @param last_sync Pointer to a timestamp of the last synchronization.
 * @param keylog_temp Pointer to the temporary keylogger string to be synchronized.
 */
void sync_keylogger_string(void) {
    bool            needs_sync                                = false;
    static uint16_t last_sync                                 = 0;
    static char     keylog_temp[DISPLAY_KEYLOGGER_LENGTH + 1] = {0};

    if (memcmp(&display_keylogger_string, keylog_temp, (DISPLAY_KEYLOGGER_LENGTH + 1))) {
        needs_sync = true;
        memcpy(keylog_temp, &display_keylogger_string, (DISPLAY_KEYLOGGER_LENGTH + 1));
    }
    if (timer_elapsed(last_sync) > FORCED_SYNC_THROTTLE_MS) {
        needs_sync = true;
    }
    if (needs_sync) {
        if (send_extended_message_handler(RPC_ID_EXTENDED_DISPLAY_KEYLOG_STR, display_keylogger_string,
                                          (DISPLAY_KEYLOGGER_LENGTH + 1))) {
            last_sync = timer_read();
        }
        needs_sync = false;
    }
}
#endif // DISPLAY_DRIVER_ENABLE && DISPLAY_KEYLOGGER_ENABLE

#if defined(AUTOCORRECT_ENABLE)
static char temp_autocorrected_str[2][21] = {0};
_Static_assert(sizeof(temp_autocorrected_str) == sizeof(autocorrected_str_raw),
               "Size mismatch for autocorrect string syncing!");
/**
 * @brief Synchronizes the autocorrect strings between split keyboards.
 *
 * This function ensures that the autocorrect strings are synchronized between
 * the two halves of a split keyboard setup. It checks if synchronization is
 * needed and updates the last synchronization timestamp.
 *
 * @param needs_sync Pointer to a boolean flag indicating if synchronization is needed.
 * @param last_sync Pointer to the timestamp of the last synchronization.
 * @param temp_autocorrected_str 2D array holding the autocorrect strings for both halves.
 */
void sync_autocorrect_string(void) {
    bool            needs_sync = false;
    static uint16_t last_sync  = 0;
    if (memcmp(&autocorrected_str_raw, temp_autocorrected_str, sizeof(autocorrected_str_raw))) {
        needs_sync = true;
        memcpy(temp_autocorrected_str, &autocorrected_str_raw, sizeof(autocorrected_str_raw));
    }
    if (timer_elapsed(last_sync) > FORCED_SYNC_THROTTLE_MS) {
        needs_sync = true;
    }
    if (needs_sync) {
        if (send_extended_message_handler(RPC_ID_EXTENDED_AUTOCORRECT_STR, autocorrected_str_raw,
                                          sizeof(autocorrected_str_raw))) {
            last_sync = timer_read();
        }
        needs_sync = false;
    }
}
#endif // AUTOCORRECT_ENABLE

#ifdef WPM_ENABLE
void sync_wpm_graph_data(void) {
    bool            needs_sync                                 = false;
    static uint16_t last_sync                                  = 0;
    static uint8_t  local_wpm_graph_samples[WPM_GRAPH_SAMPLES] = {0};

    if (memcmp(wpm_graph_samples, local_wpm_graph_samples, sizeof(local_wpm_graph_samples))) {
        needs_sync = true;
        memcpy(local_wpm_graph_samples, wpm_graph_samples, sizeof(local_wpm_graph_samples));
    }
    if (timer_elapsed(last_sync) > (FORCED_SYNC_THROTTLE_MS * 100)) {
        needs_sync = true;
    }
    if (needs_sync) {
        if (send_extended_message_handler(RPC_ID_EXTENDED_WPM_GRAPH_DATA, local_wpm_graph_samples,
                                          sizeof(local_wpm_graph_samples))) {
            last_sync = timer_read32();
        }
        needs_sync = false;
    }
}
#endif // WPM_ENABLE

void sync_keymap_config(void) {
    bool                   needs_sync         = false;
    static uint16_t        last_sync          = 0;
    static keymap_config_t last_keymap_config = {0};

    if (memcmp(&keymap_config, &last_keymap_config, sizeof(keymap_config_t))) {
        needs_sync = true;
        memcpy(&last_keymap_config, &keymap_config, sizeof(keymap_config_t));
    }
    if (timer_elapsed(last_sync) > FORCED_SYNC_THROTTLE_MS) {
        needs_sync = true;
    }
    if (needs_sync) {
        if (send_extended_message_handler(RPC_ID_EXTENDED_KEYMAP_CONFIG, &keymap_config, sizeof(keymap_config_t))) {
            last_sync = timer_read();
        }
        needs_sync = false;
    }
}

void sync_debug_config(void) {
    bool                  needs_sync        = false;
    static uint16_t       last_sync         = 0;
    static debug_config_t last_debug_config = {0};

    if (memcmp(&debug_config, &last_debug_config, sizeof(debug_config_t))) {
        needs_sync = true;
        memcpy(&last_debug_config, &debug_config, sizeof(debug_config_t));
    }
    if (timer_elapsed(last_sync) > FORCED_SYNC_THROTTLE_MS) {
        needs_sync = true;
    }
    if (needs_sync) {
        if (send_extended_message_handler(RPC_ID_EXTENDED_DEBUG_CONFIG, &debug_config, sizeof(debug_config_t))) {
            last_sync = timer_read();
        }
        needs_sync = false;
    }
}

/**
 * @brief Initialize the transport sync
 *
 */
void keyboard_post_init_transport_sync(void) {
    // Register keyboard state sync split transaction
    transaction_register_rpc(RPC_ID_EXTENDED_SYNC_TRANSPORT, extended_message_handler);
}

/**
 * @brief Performs housekeeping tasks related to transport synchronization.
 *
 * This function is responsible for managing and executing any necessary
 * housekeeping tasks to ensure proper synchronization of transport mechanisms
 * in a split keyboard setup.
 */
void housekeeping_task_transport_sync(void) {
    if (!is_transport_connected()) {
        return;
    }

    if (!has_first_run) {
        if (timer_elapsed(0) > 1500) {
            has_first_run = true;
        } else {
            return;
        }
    }

    // Data sync from master to slave
    if (is_keyboard_master()) {
#ifdef WPM_ENABLE
        sync_wpm_graph_data();
#endif // WPM_ENABLE
#ifdef AUTOCORRECT_ENABLE
        sync_autocorrect_string();
#endif // AUTOCORRECT_ENABLE
#if defined(DISPLAY_DRIVER_ENABLE) && defined(DISPLAY_KEYLOGGER_ENABLE)
        sync_keylogger_string();
#endif // DISPLAY_DRIVER_ENABLE && DISPLAY_KEYLOGGER_ENABLE
        sync_keymap_config();
        sync_debug_config();
        sync_userspace_runtime_state();
        sync_userspace_config();
    }

    if (!is_keyboard_master()) {
#if 0
        // Data sync from slave to master
        static bool is_first_run = true;
        (void)is_first_run;
#    if defined(RGB_MATRIX_ENABLE)
        static rgb_config_t last_rgb_matrix_config = {0};
        if (is_first_run) {
            eeprom_read_block(&last_rgb_matrix_config, EECONFIG_RGB_MATRIX, sizeof(last_rgb_matrix_config));
        }
        if (last_rgb_matrix_config.raw != rgb_matrix_config.raw) {
            last_rgb_matrix_config = rgb_matrix_config;
            eeconfig_update_rgb_matrix();
            rgb_matrix_reload_from_eeprom();
            xprintf("RGB Matrix config updated\n");
        }
#    endif // RGB_MATRIX_ENABLE
#    if defined(RGBLIGHT_ENABLE) && (defined(RGB_MATRIX_ENABLE) && !defined(RGBLIGHT_CUSTOM))
        static rgblight_config_t last_rgblight_config = {0};
        extern rgblight_config_t rgblight_config;

        if (is_first_run) {
            last_rgblight_config.raw = eeconfig_read_rgblight();
        }
        if (last_rgblight_config.raw != rgblight_config.raw) {
            last_rgblight_config = rgblight_config;
            eeconfig_update_rgblight(rgblight_config.raw);
            rgblight_reload_from_eeprom();
            xprintf("RGB Light config updated\n");
        }
#    endif // RGBLIGHT_ENABLE && (RGB_MATRIX_ENABLE && !RGBLIGHT_CUSTOM)
#    if defined(HAPTIC_ENABLE) && defined(SPLIT_HAPTIC_ENABLE)
        static haptic_config_t last_haptic_config = {0};
        extern haptic_config_t haptic_config;

        if (is_first_run) {
            last_haptic_config.raw = eeconfig_read_haptic();
        }
        if (last_haptic_config.raw != haptic_config.raw) {
            last_haptic_config = haptic_config;
            eeconfig_update_haptic(haptic_config.raw);
            xprintf("Haptic config updated\n");
        }
#    endif
#    if defined(BACKLIGHT_ENABLE)
        static backlight_config_t last_backlight_config = {0};
        extern backlight_config_t backlight_config;

        if (is_first_run) {
            last_backlight_config.raw = eeconfig_read_backlight();
        }
#        if defined(QUANTUM_PAINTER_ENABLE)
        if (last_backlight_config.level != backlight_config.level ||
            last_backlight_config.breathing != backlight_config.breathing)
#        else
        if (last_backlight_config.raw != backlight_config.raw)
#        endif
        {
            last_backlight_config = backlight_config;
            eeconfig_update_backlight(backlight_config.raw);
            xprintf("Backlight config updated\n");
        }
#    endif // BACKLIGHT_ENABLE
        is_first_run = false;
#endif
    }
}
