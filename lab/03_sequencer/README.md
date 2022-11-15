## Components used for protoboard work

- RP2040

## Peripheral used

- GPIO

## How the prototype works

Once the program starts, on the console it will ask the user to input the register address and decide whether to read or write. Then it will perform the expected reading or writing, and print a confirmation message on the screen.

## Code

    #include "pico/stdlib.h"
    #include <stdio.h>
    #include "ws2812.h"
    #include "hardware/pio.h"
    #include "hardware/clocks.h"
    #include "ws2812.pio.h"
    #include "registers.h"
    #include "ws2812.pio.h"
    #define PIO         pio0
    #define SM          0
    #define FREQ        800000
    #define PIN         12
    #define POWER_PIN   11
    #define IS_RGBW     true  

    #define QTPY_BOOT_PIN 21

    typedef struct {
        uint32_t last_serial_byte;
        uint32_t button_is_pressed;
        uint32_t light_color;
    } Flashlight; 


    int main() {

        stdio_init_all();
        gpio_init(QTPY_BOOT_PIN);
        gpio_set_dir(QTPY_BOOT_PIN, GPIO_IN);
        rp_init();
        turn_on_pixel();
    
        Flashlight status;
        status.last_serial_byte =  0x00000000;
        status.button_is_pressed = 0x00000000;
        status.light_color =       0x0012B4A6;

        set_pixel_color(status.light_color);
        sleep_ms(1000);
        set_pixel_color(0x00000000);

        while(true) {
        int i = 0;
        int j = 0;
        uint32_t arr[1000];
        char input = 0;
        while(input != 99) {
            printf("Press c to start.\n");
            scanf("%c",&input);
        }
        printf("Start to press button.\n");
        while (i < 1000) {
            if (gpio_get(QTPY_BOOT_PIN)) { // poll every cycle, 0 = "pressed"
                status.button_is_pressed = 0x00000000;
                arr[i] = 0x00000000;
            } else {
                status.button_is_pressed = 0x00000001;
                arr[i] = 0x00000001;
            }
            if (status.button_is_pressed) { // poll every cycle
                set_pixel_color(status.light_color);
            } else {
                set_pixel_color(0x00000000);
            }
            sleep_ms(10); // don't DDOS the serial console
            i += 1;
            printf("%d\n",i*10);
        }
        printf("Start Replay!\n");
        while(true){
            while(j < 1000){
                if(arr[j]){
                    set_pixel_color(status.light_color);
                }else{
                    set_pixel_color(0x00000000);
                }
                sleep_ms(10); // don't DDOS the serial console
                j += 1;
            }
            j = 0;
            printf("Replay Done!\n");
            break;
        }
        }
        return 0;
    }

The first set_pixel_color() is to confirm that the program is successfully running on the board.


## Demo Video

https://youtu.be/LhURo-ZxE8I

In this demo, I showed two rounds of recording and replaying of the BOOT button pressing sequence.

