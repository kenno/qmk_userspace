// Copyright 2022 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#include "hardware_id.h"
#include <stdlib.h>
#include <stdio.h>
#if defined(PROTOCOL_CHIBIOS)
#    include <ch.h>
#elif defined(PROTOCOL_LUFA)
#    ifndef SIGRD
#        define SIGRD 5
#    endif // SIGRD
#    include <avr/boot.h>
#else
#    error "How did you get here?"
#endif

hardware_id_t get_hardware_id(void) {
    hardware_id_t id = {0};
#if defined(__AVR__)
    for (uint8_t i = 0; i < 10; i += 1) {
        ((uint8_t *)&id)[i] = boot_signature_byte_get(i + 0x0E);
    }
#elif defined(RP2040)
    // Forward declare as including "hardware/flash.h" here causes more issues...
    void flash_get_unique_id(uint8_t *);

    flash_get_unique_id((uint8_t *)&id);
#elif defined(UID_BASE)
    uint32_t base[3];
    base[0] = (uint32_t)(*((uint32_t *)UID_BASE));
    base[1] = (uint32_t)(*((uint32_t *)(UID_BASE + 4)));
    base[2] = (uint32_t)(*((uint32_t *)(UID_BASE + 8)));

    // https://github.com/libopencm3/libopencm3/blob/master/lib/stm32/common/desig_common_all.c

    uint8_t uid[12], data[6];
    for (uint8_t index = 0; index < 12; index++) {
        uid[index] = (uint8_t)(base[index / 4] >> ((index % 4) * 8));
    }

    data[0] = uid[11];
    data[1] = uid[10] + uid[2];
    data[2] = uid[9];
    data[3] = uid[8] + uid[0];
    data[4] = uid[7];
    data[5] = uid[6];

    id.data[0] =
        (uint32_t)(data[1]) << 8 | (uint32_t)(data[0]) << 0 | (uint32_t)(data[3]) << 24 | (uint32_t)(data[2]) << 16;
    id.data[1] = (uint32_t)(data[5]) << 8 | (uint32_t)(data[4]) << 0;

    // id.data[0] = id.data[1] = id.data[2] = 0x12345678;
    // converts to 78563412 78563412 78563412
#endif
    return id;
}

#ifndef SERIAL_NUMBER_LENGTH
#    define SERIAL_NUMBER_LENGTH (sizeof(hardware_id_t) * 2)
#endif

const char *get_hardware_id_string(void) {
    static char buf[SERIAL_NUMBER_LENGTH + 1] = {0};
#if defined(UID_BASE)
    uint32_t base[3];
    base[0] = (uint32_t)(*((uint32_t *)UID_BASE));
    base[1] = (uint32_t)(*((uint32_t *)(UID_BASE + 4)));
    base[2] = (uint32_t)(*((uint32_t *)(UID_BASE + 8)));
    uint8_t uid[12];
    for (uint8_t index = 0; index < 12; index++) {
        uid[index] = (uint8_t)(base[index / 4] >> ((index % 4) * 8));
    }

    snprintf(buf, sizeof(buf), "%02X%02X%02X%02X%02X%02X", uid[11], uid[10] + uid[2], uid[9], uid[8] + uid[0], uid[7],
             uid[6]);
#else
    hardware_id_t id = get_hardware_id();
    snprintf(buf, sizeof(buf), "%08lX%08lX%08lX%08lX", id.data[0], id.data[1], id.data[2], id.data[3]);
#endif

    return buf;
}
