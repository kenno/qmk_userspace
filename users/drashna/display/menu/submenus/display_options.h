
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Display options

#ifdef QUANTUM_PAINTER_ENABLE
#    include "display/painter/painter.h"
extern painter_image_array_t screen_saver_image[];
extern const uint8_t         screensaver_image_size;

bool menu_handler_display_mode_master(menu_input_t input) {
    switch (input) {
        case menu_input_left:
            userspace_config.painter.display_mode_master = (userspace_config.painter.display_mode_master - 1) % 4;
            if (userspace_config.painter.display_mode_master > 3) {
                userspace_config.painter.display_mode_master = 3;
            }
            eeconfig_update_user_datablock(&userspace_config);
            return false;
        case menu_input_right:
            userspace_config.painter.display_mode_master = (userspace_config.painter.display_mode_master + 1) % 4;
            if (userspace_config.painter.display_mode_master > 3) {
                userspace_config.painter.display_mode_master = 0;
            }
            eeconfig_update_user_datablock(&userspace_config);
            return false;
        default:
            return true;
    }
}

__attribute__((weak)) void display_handler_display_mode_master(char *text_buffer, size_t buffer_len) {
    switch (userspace_config.painter.display_mode_master) {
        case 0:
            strncpy(text_buffer, "Console", buffer_len - 1);
            return;
        case 1:
            strncpy(text_buffer, "Fonts", buffer_len - 1);
            return;
        case 2:
            strncpy(text_buffer, "QMK Info", buffer_len - 1);
            return;
        case 3:
            strncpy(text_buffer, "Layer Map", buffer_len - 1);
            return;
    }

    strncpy(text_buffer, "Unknown", buffer_len);
}

bool menu_handler_display_mode_slave(menu_input_t input) {
    switch (input) {
        case menu_input_left:
            userspace_config.painter.display_mode_slave = (userspace_config.painter.display_mode_slave - 1) % 3;
            if (userspace_config.painter.display_mode_slave > 2) {
                userspace_config.painter.display_mode_slave = 2;
            }
            eeconfig_update_user_datablock(&userspace_config);
            return false;
        case menu_input_right:
            userspace_config.painter.display_mode_slave = (userspace_config.painter.display_mode_slave + 1) % 43;
            if (userspace_config.painter.display_mode_slave > 2) {
                userspace_config.painter.display_mode_slave = 0;
            }
            eeconfig_update_user_datablock(&userspace_config);
            return false;
        default:
            return true;
    }
}

__attribute__((weak)) void display_handler_display_mode_slave(char *text_buffer, size_t buffer_len) {
    switch (userspace_config.painter.display_mode_slave) {
        case 0:
            strncpy(text_buffer, "Console", buffer_len - 1);
            return;
        case 1:
            strncpy(text_buffer, "Fonts", buffer_len - 1);
            return;
        case 2:
            strncpy(text_buffer, "QMK Info", buffer_len - 1);
            return;
    }

    strncpy(text_buffer, "Unknown", buffer_len);
}

bool menu_handler_display_image(menu_input_t input) {
    switch (input) {
        case menu_input_left:
            userspace_config.painter.display_logo =
                (userspace_config.painter.display_logo - 1) % screensaver_image_size;
            if (userspace_config.painter.display_logo > (screensaver_image_size - 1)) {
                userspace_config.painter.display_logo = (screensaver_image_size - 1);
            }
            eeconfig_update_user_datablock(&userspace_config);
            return false;
        case menu_input_right:
            userspace_config.painter.display_logo =
                (userspace_config.painter.display_logo + 1) % screensaver_image_size;
            if (userspace_config.painter.display_logo > (screensaver_image_size - 1)) {
                userspace_config.painter.display_logo = 0;
            }
            eeconfig_update_user_datablock(&userspace_config);
            return false;
        default:
            return true;
    }
}

__attribute__((weak)) void display_handler_display_image(char *text_buffer, size_t buffer_len) {
    strncpy(text_buffer, screen_saver_image[userspace_config.painter.display_logo].name, buffer_len - 1);
}

#    ifdef SPLIT_KEYBOARD
bool menu_handler_display_menu_location(menu_input_t input) {
    switch (input) {
        case menu_input_left:
            userspace_config.painter.menu_render_side = (userspace_config.painter.menu_render_side - 1) % 4;
            if (userspace_config.painter.menu_render_side < 1) {
                userspace_config.painter.menu_render_side = 3;
            }
            eeconfig_update_user_datablock(&userspace_config);
            return false;
        case menu_input_right:
            userspace_config.painter.menu_render_side = (userspace_config.painter.menu_render_side + 1) % 4;
            if (userspace_config.painter.menu_render_side < 1) {
                userspace_config.painter.menu_render_side = 1;
            }
            xprintf("menu location: %d\n", userspace_config.painter.menu_render_side);
            eeconfig_update_user_datablock(&userspace_config);
            return false;
        default:
            return true;
    }
}

__attribute__((weak)) void display_handler_display_menu_location(char *text_buffer, size_t buffer_len) {
    switch (userspace_config.painter.menu_render_side) {
        case 1:
            strncpy(text_buffer, "Left", buffer_len - 1);
            return;
        case 2:
            strncpy(text_buffer, "Right", buffer_len - 1);
            return;
        case 3:
            strncpy(text_buffer, "Both", buffer_len - 1);
            return;
    }
    strncpy(text_buffer, "Everywhere", buffer_len);
}
#    endif // SPLIT_KEYBOARD
#endif // QUANTUM_PAINTER_ENABLE

bool menu_handler_display_rotation(menu_input_t input) {
#ifdef QUANTUM_PAINTER_ILI9341_ENABLE
    void init_display_ili9341_rotation(void);
#endif // QUANTUM_PAINTER_ILI9341_ENABLE
#ifdef QUANTUM_PAINTER_ILI9488_ENABLE
    void init_display_ili9488_rotation(void);
#endif // QUANTUM_PAINTER_ILI9341_ENABLE
    switch (input) {
#ifdef DISPLAY_FULL_ROTATION_ENABLE
        case menu_input_left:
            userspace_config.painter.rotation = (userspace_config.painter.rotation - 1) % 4;
            if (userspace_config.painter.rotation > 3) {
                userspace_config.painter.rotation = 3;
            }
            eeconfig_update_user_datablock(&userspace_config);
            return false;
        case menu_input_right:
            userspace_config.painter.rotation = (userspace_config.painter.rotation + 1) % 4;
            if (userspace_config.painter.rotation > 3) {
                userspace_config.painter.rotation = 0;
            }
            eeconfig_update_user_datablock(&userspace_config);
#    ifdef QUANTUM_PAINTER_ILI9341_ENABLE
            init_display_ili9341_rotation();
#    endif // QUANTUM_PAINTER_ILI9341_ENABLE
#    ifdef QUANTUM_PAINTER_ILI9488_ENABLE
            init_display_ili9488_rotation();
#    endif // QUANTUM_PAINTER_ILI9341_ENABLE
            return false;
#else
        case menu_input_left:
        case menu_input_right:
            userspace_config.painter.rotation ^= 1;
            eeconfig_update_user_datablock(&userspace_config);
#    ifdef QUANTUM_PAINTER_ILI9341_ENABLE
            init_display_ili9341_rotation();
#    endif // QUANTUM_PAINTER_ILI9341_ENABLE
#    ifdef QUANTUM_PAINTER_ILI9488_ENABLE
            init_display_ili9488_rotation();
#    endif // QUANTUM_PAINTER_ILI9341_ENABLE
            return false;
#endif
        default:
            return true;
    }
}

__attribute__((weak)) void display_handler_display_rotation(char *text_buffer, size_t buffer_len) {
#ifdef DISPLAY_FULL_ROTATION_ENABLE
    switch (userspace_config.painter.rotation) {
        case 0:
            strncpy(text_buffer, "0", buffer_len - 1);
            return;
        case 1:
            strncpy(text_buffer, "90", buffer_len - 1);
            return;
        case 2:
            strncpy(text_buffer, "180", buffer_len - 1);
            return;
        case 3:
            strncpy(text_buffer, "270", buffer_len - 1);
            return;
    }
#else
    strncpy(text_buffer, userspace_config.painter.rotation ? "Flipped" : "Normal", buffer_len - 1);
    return;
#endif

    strncpy(text_buffer, "Unknown", buffer_len);
}

bool menu_handler_display_inverted(menu_input_t input) {
#ifdef QUANTUM_PAINTER_ILI9341_ENABLE
    void init_display_ili9341_inversion(void);
#endif // QUANTUM_PAINTER_ILI9341_ENABLE
#ifdef QUANTUM_PAINTER_ILI9488_ENABLE
    void init_display_ili9488_inversion(void);
#endif // QUANTUM_PAINTER_ILI9341_ENABLE
    switch (input) {
        case menu_input_left:
        case menu_input_right:
            userspace_config.painter.inverted = !userspace_config.painter.inverted;
            eeconfig_update_user_datablock(&userspace_config);
#ifdef QUANTUM_PAINTER_ILI9341_ENABLE
            init_display_ili9341_inversion();
#endif // QUANTUM_PAINTER_ILI9341_ENABLE
#ifdef QUANTUM_PAINTER_ILI9488_ENABLE
            init_display_ili9488_inversion();
#endif // QUANTUM_PAINTER_ILI9341_ENABLE
            return false;
        default:
            return true;
    }
}

__attribute__((weak)) void display_handler_display_inverted(char *text_buffer, size_t buffer_len) {
    strncpy(text_buffer, userspace_config.painter.inverted ? "Inverted" : "Normal", buffer_len - 1);
}

bool menu_handler_display_hue(menu_input_t input, bool painter_is_primary) {
    switch (input) {
        case menu_input_left:
            painter_decrease_hue(painter_is_primary);
            return false;
        case menu_input_right:
            painter_increase_hue(painter_is_primary);
            return false;
        default:
            return true;
    }
}

#ifdef QUANTUM_PAINTER_ENABLE

__attribute__((weak)) void display_handler_display_hue(char *text_buffer, size_t buffer_len, bool painter_is_primary) {
    snprintf(text_buffer, buffer_len - 1, "%d", painter_get_hue(painter_is_primary));
}

bool menu_handler_display_sat(menu_input_t input, bool painter_is_primary) {
    switch (input) {
        case menu_input_left:
            painter_decrease_sat(painter_is_primary);
            return false;
        case menu_input_right:
            painter_increase_sat(painter_is_primary);
            return false;
        default:
            return true;
    }
}

__attribute__((weak)) void display_handler_display_sat(char *text_buffer, size_t buffer_len, bool painter_is_primary) {
    snprintf(text_buffer, buffer_len - 1, "%d", painter_get_sat(painter_is_primary));
}

bool menu_handler_display_val(menu_input_t input, bool painter_is_primary) {
    switch (input) {
        case menu_input_left:
            painter_decrease_val(painter_is_primary);
            return false;
        case menu_input_right:
            painter_increase_val(painter_is_primary);
            return false;
        default:
            return true;
    }
}

__attribute__((weak)) void display_handler_display_val(char *text_buffer, size_t buffer_len, bool painter_is_primary) {
    snprintf(text_buffer, buffer_len - 1, "%d", painter_get_val(painter_is_primary));
}

bool menu_handler_display_hue_primary(menu_input_t input) {
    return menu_handler_display_hue(input, true);
}

__attribute__((weak)) void display_handler_display_hue_primary(char *text_buffer, size_t buffer_len) {
    display_handler_display_hue(text_buffer, buffer_len, true);
}

bool menu_handler_display_sat_primary(menu_input_t input) {
    return menu_handler_display_sat(input, true);
}

__attribute__((weak)) void display_handler_display_sat_primary(char *text_buffer, size_t buffer_len) {
    display_handler_display_sat(text_buffer, buffer_len, true);
}

bool menu_handler_display_val_primary(menu_input_t input) {
    return menu_handler_display_val(input, true);
}

__attribute__((weak)) void display_handler_display_val_primary(char *text_buffer, size_t buffer_len) {
    display_handler_display_val(text_buffer, buffer_len, true);
}

bool menu_handler_display_hue_secondary(menu_input_t input) {
    return menu_handler_display_hue(input, false);
}

__attribute__((weak)) void display_handler_display_hue_secondary(char *text_buffer, size_t buffer_len) {
    display_handler_display_hue(text_buffer, buffer_len, false);
}

bool menu_handler_display_sat_secondary(menu_input_t input) {
    return menu_handler_display_sat(input, false);
}

__attribute__((weak)) void display_handler_display_sat_secondary(char *text_buffer, size_t buffer_len) {
    display_handler_display_sat(text_buffer, buffer_len, false);
}

bool menu_handler_display_val_secondary(menu_input_t input) {
    return menu_handler_display_val(input, false);
}

__attribute__((weak)) void display_handler_display_val_secondary(char *text_buffer, size_t buffer_len) {
    display_handler_display_val(text_buffer, buffer_len, false);
}
#endif // QUANTUM_PAINTER_ENABLE

menu_entry_t display_option_entries[] = {
#ifdef QUANTUM_PAINTER_ENABLE
    MENU_ENTRY_CHILD("Display (Master)", display_mode_master),
    MENU_ENTRY_CHILD("Display (Slave)", display_mode_slave),
    MENU_ENTRY_CHILD("Image", display_image),
#    ifdef SPLIT_KEYBOARD
    MENU_ENTRY_CHILD("Menu Location", display_menu_location),
#    endif // SPLIT_KEYBOARD
#endif // QUANTUM_PAINTER_ENABLE
    MENU_ENTRY_CHILD("Rotation", display_rotation),
    MENU_ENTRY_CHILD("Inverted", display_inverted),
#ifdef QUANTUM_PAINTER_ENABLE
    MENU_ENTRY_CHILD("Primary Hue", display_hue_primary),
    MENU_ENTRY_CHILD("Primary Saturation", display_sat_primary),
    MENU_ENTRY_CHILD("Primary Value", display_val_primary),
    MENU_ENTRY_CHILD("Secondary Hue", display_hue_secondary),
    MENU_ENTRY_CHILD("Secondary Saturation", display_sat_secondary),
    MENU_ENTRY_CHILD("Secondary Value", display_val_secondary),
#endif // QUANTUM_PAINTER_ENABLE
};
