/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include <stdio.h>
#include "ws2812.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"

int main() {
    const uint BOOT_PIN = 21;
    gpio_init(BOOT_PIN);
    gpio_set_dir(BOOT_PIN, GPIO_IN);

    rp_init();
    turn_on_pixel();

    uint32_t rgb = 0x00000000;
    set_pixel_color(rgb);
    while (true) {
        if(!gpio_get(BOOT_PIN)) {
            rgb = 0x00123456;
            set_pixel_color(rgb);
        } else {
            rgb = 0x00000000;
            set_pixel_color(rgb);
        }
        sleep_ms(250);
    }
}
