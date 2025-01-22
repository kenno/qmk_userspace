// Copyright 2023 Christopher Courtney, aka Drashna Jael're  (@drashna) <drashna@live.com>
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @brief Because this is a stupid idea, why not.   Adds support for watchdog monitoring of keyboard.
 *
 */

#include "watchdog.h"
#ifdef __arm__
#    include <hal.h>
#    include <math.h>
#    include <stddef.h>

// STM32-specific watchdog config calculations
// timeout = tick * PR * (RL + 1)
// tick = 1000000 / (lsi) clock
#    if !defined(WATCHDOG_CLOCK)
#        if defined(MCU_STM32)
#            define WATCHDOG_CLOCK STM32_LSICLK
#        elif defined(MCU_RP)
#            define WATCHDOG_CLOCK RP_XOSCCLK
#        endif
#    endif // !defined(WATCHDOG_CLOCK)

#    define _IWDG_LSI(us)    ((us) * WATCHDOG_CLOCK / 1000000)
#    define _IWDG_PR_US(us)  (uint8_t)(log(_IWDG_LSI(us)) / log(2) - 11) // 3
#    define _IWDG_PR_S(s)    _IWDG_PR_US(s * 1000000.0)
#    define _IWDG_SCALAR(us) (2 << ((uint8_t)_IWDG_PR_US(us) + 1))
#    define _IWDG_RL_US(us)  (uint64_t)(_IWDG_LSI(us)) / _IWDG_SCALAR(us)
#    define _IWDG_RL_S(s)    _IWDG_RL_US(s * 1000000.0)

#    if !defined(WATCHDOG_TIMEOUT)
#        define WATCHDOG_TIMEOUT 5.0f
#    endif // !defined(WATCHDOG_TIMEOUT)

static WDGConfig wdgcfg = {
#    if STM32_HAS_IWDG == TRUE
    .pr = _IWDG_PR_S(WATCHDOG_TIMEOUT),
#    endif // STM32_HAS_IWDG
    .rlr = _IWDG_RL_S(WATCHDOG_TIMEOUT),
#    if STM32_IWDG_IS_WINDOWED == TRUE
    .winr = STM32_IWDG_WIN_DISABLED,
#    endif // STM32_IWDG_IS_WINDOWED
};

void watchdog_init(void) {
    wdgInit();

    wdgStart(&WDGD1, &wdgcfg);
}

void watchdog_task(void) {
    wdgReset(&WDGD1);
}

void watchdog_shutdown(void) {
    wdgStop(&WDGD1);
}

void suspend_power_down_watchdog(void) {
    wdgReset(&WDGD1);
}

void suspend_wakeup_init_watchdog(void) {
    wdgReset(&WDGD1);
}

#elif defined(__AVR__)
#    include <avr/wdt.h>
#    pragma message "AVR Watchdog causes issues, likely due to the way the matrix scan is done.  Disable this."

void watchdog_init(void) {
    wdt_enable(WDTO_2S);
}

void watchdog_task(void) {
    wdt_reset();
}

void watchdog_shutdown(void) {
    wdt_disable();
}

void suspend_power_down_watchdog(void) {
    wdt_disable();
}

void suspend_wakeup_init_watchdog(void) {
    wdt_enable(WDTO_2S);
    clear_keyboard();
}

#endif // __AVR__
