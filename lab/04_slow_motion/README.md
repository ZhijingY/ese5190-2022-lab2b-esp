## Components used for protoboard work

- RP2040

## Peripheral used

- GPIO

## How the prototype works

Once the program starts, on the console it will ask the user to type c to start recording the BOOT button pressing for 10 seconds, then the sequence will be replayed in the speed chosen by the user.

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
        status.light_color =       0x00A2B49E;

        set_pixel_color(status.light_color);
        sleep_ms(1000);
        set_pixel_color(0x00000000);

        while(true) {
        int max = 10*1000;
        int i = 0;
        int j = 0;
        int speed = 1;
        uint32_t arr[10*100];
        char input = 0;
        while(input != 99) {
            printf("Press c to start.\n");
            scanf("%c",&input);
        }
        printf("Start to press button.\n");
        while (i < max/10) {
            if (gpio_get(QTPY_BOOT_PIN)) { 
                status.button_is_pressed = 0x00000000;
                arr[i] = 0x00000000;
            } else {
                status.button_is_pressed = 0x00000001;
                arr[i] = 0x00000001;
            }
            if (status.button_is_pressed) {
                set_pixel_color(status.light_color);
            } else {
                set_pixel_color(0x00000000);
            }
            sleep_ms(10);
            i += 1;
        }
        printf("Recording Done.\n");
        sleep_ms(100);
        while(true){
            printf("Enter 1 to replay in normal speed. Enter 2 to slow down replay. Enter 3 to speed up replay.\n");
            scanf("%d",&speed);
            if(speed == 3){
                printf("Start Replay fast!\n");
                speed = 5;
            }else if(speed == 1){
                printf("Start Replay in normal speed!\n");
                speed = 10;
            }else{
                printf("Start Replay slow!\n");
                speed = 20;
            }
            while(j < max/10){
                if(arr[j]){
                    set_pixel_color(status.light_color);
                }else{
                    set_pixel_color(0x00000000);
                }
                sleep_ms(speed); 
                j += 1;
            }
            j = 0;
            printf("Replay Done!\n");
        }
        }
        return 0;
    }

The first set_pixel_color() is to confirm that the program is successfully running on the board.


## Demo Video

https://youtu.be/LhURo-ZxE8I

In this demo, I showed two rounds of recording and replaying of the BOOT button pressing sequence, and I tested different replay speed.

