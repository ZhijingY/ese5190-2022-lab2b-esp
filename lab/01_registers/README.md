## Components used for protoboard work

- RP2040

## Peripheral used

- GPIO

## How the prototype works

The BOOT button is GPIO21. Once BOOT button is pressed, the status of the input will be obtained by calling gpio_get(). If it returns 0, it means that the BOOT button is pressed. Then the Neopixel LED will be on and its color will be set.

## Code

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
            if(!gpio_get(BOOT_PIN)) { // gpio_get returns 0 if the the input status of PIN is set
                rgb = 0x00123456;
                set_pixel_color(rgb);
            } else {
                rgb = 0x00000000;
                set_pixel_color(rgb);
            }
            sleep_ms(250);
        }
    }

## Demo GIF

When the BOOT button is pressed, the Neopixel LED will be turned on.
![a](https://github.com/ZhijingY/ese5190-2022-lab2b-esp/blob/main/lab/01_registers/ESE519_Lab2B_part1.gif)
