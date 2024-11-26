#include "display/menu/menu.h"
#include "drashna_runtime.h"
#include "oled_driver.h"

#define snprintf_nowarn(...) (snprintf(__VA_ARGS__) < 0 ? abort() : (void)0)

bool oled_render_menu(uint8_t col, uint8_t line, uint8_t num_of_lines) {
    uint8_t scroll_offset   = 0;
    char    text_buffer[21] = {0};
    static bool last_state      = false;

    if (userspace_runtime_state.menu_state.is_in_menu != last_state) {
        last_state                               = userspace_runtime_state.menu_state.is_in_menu;
        userspace_runtime_state.menu_state.dirty = true;
    }

    if (userspace_runtime_state.menu_state.dirty) {
        oled_set_cursor(col, line);
        for (uint8_t i = 0; i < num_of_lines; i++) {
            oled_advance_page(true);
        }
        userspace_runtime_state.menu_state.dirty = false;
    }

    if (!userspace_runtime_state.menu_state.is_in_menu) {
        return false;
    }

    menu_entry_t *menu     = get_current_menu();
    menu_entry_t *selected = get_selected_menu_item();

    scroll_offset = get_menu_scroll_offset(menu, num_of_lines - 1);

    snprintf(text_buffer, sizeof(text_buffer), "%-20s", menu->text);

    oled_set_cursor(col, line);
    for (uint8_t i = 0; i < num_of_lines; i++) {
        oled_advance_page(true);
    }
    oled_set_cursor(col, line);
    oled_write(" ", true);
    oled_write(text_buffer, true);
    for (uint8_t i = scroll_offset; i < menu->parent.child_count && i <= (scroll_offset + (num_of_lines - 1) - 1);
         i++) {
        menu_entry_t *child   = &menu->parent.children[i];
        char          buf[19] = {0}, val[19] = {0};

        oled_set_cursor(col, line + 1 + i - scroll_offset);
        oled_write(child == selected ? " *" : "  ", false);
        snprintf(buf, sizeof(buf), "%s", child->text);

        if (child->flags & menu_flag_is_value) {
            child->child.display_handler(val, sizeof(val));
            if (child->flags & menu_flag_is_parent) {
                snprintf_nowarn(text_buffer, 19, "%s [%s]", buf, val);
            } else {
                snprintf_nowarn(text_buffer, 19, "%s: %s", buf, val);
            }
        } else {
            snprintf_nowarn(text_buffer, 19, "%s", buf);
        }
        oled_write(text_buffer, false);
        oled_set_cursor(col + 19, line + 1 + i - scroll_offset);
        oled_write(child->flags & menu_flag_is_parent ? ">" : " ", false);
        oled_advance_page(true);
    }
    return true;
}
