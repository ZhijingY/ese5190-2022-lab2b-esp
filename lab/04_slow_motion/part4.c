/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "ws2812.h"

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"

#define IS_RGBW true

#define WS2812_PIN 12
#define WS2812_POWER_PIN 11
#define PIO pio0
#define SM 0
#define FREQ 800000

#define record 'r'
#define replay 'p'
#define BOOT_PIN 21


int main() {
    stdio_init_all();
    uint offset = pio_add_program(PIO, &ws2812_program);
    //rp_init();
    ws2812_program_init(PIO, SM, offset, WS2812_PIN, FREQ, IS_RGBW);
    turn_on_pixel();

    gpio_init(BOOT_PIN);
    gpio_set_dir(BOOT_PIN, GPIO_IN);

    uint32_t key = 0x00000000;
    uint32_t flag = 0x00000000;

    while(true){
        key = getchar_timeout_us(0);
        switch(key){
            case 'r':
                set_pixel_color(0X00FF0000);
                sleep_ms(1000);
                while(true){
                    flag = 0x00000000;
                    flag = getchar_timeout_us(0);
                    if(!gpio_get(BOOT_PIN)) {
                        printf("1\n");
                        set_pixel_color(0X0000FF00);
                    } 
                    else {
                        printf("0\n");
                        set_pixel_color(0x00000000);
                    }
                    if(flag == 'N'){
                        set_pixel_color(0X00000000);
                        sleep_ms(10);
                        break;
                    }
                    sleep_ms(10); 
                }
                break;
                
            case 'p':
                while(true){
                    flag = 0x00000000;
                    flag = getchar_timeout_us(0);
                    if(flag == '1'){
                        set_pixel_color(0X000000FF);
                    }
                    if(flag == '0'){
                        set_pixel_color(0x00000000);
                    }
                    if(flag == 'N'){
                        set_pixel_color(0x00000000);
                        sleep_ms(10);
                        break;
                    }
                    sleep_ms(10);
                }
                break;

        }
    }
}
