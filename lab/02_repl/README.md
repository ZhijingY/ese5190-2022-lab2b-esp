## Components used for protoboard work

- RP2040

## Peripheral used

- GPIO

## How the prototype works

Once the program starts, on the console it will ask the user to input the register address and decide whether to read or write. Then it will perform the expected reading or writing, and print a success message on the screen.

## Code

    #include "pico/stdlib.h"
    #include <stdio.h>
    #include "ws2812.h"
    #include "hardware/pio.h"
    #include "hardware/clocks.h"
    #include "ws2812.pio.h"
    #include "registers.h"
    #include "ws2812.pio.h"

    int main() {

        uint32_t addr_input = 0x00000000;
        ADDRESS addr = 0x00000000;
        VALUE data;
        int opt;

        stdio_init_all();

        // while (!stdio_usb_connected());

        rp_init();
        turn_on_pixel();

        uint32_t rgb = 0x00ffffff;
        set_pixel_color(rgb);

        while (true) {
            printf("New Program start: \n");
            printf("Input a register address you would like to read/write: \n");
            scanf("%x", &addr_input);
            printf("Confirmation of address: %x\n", addr_input);
            addr = (ADDRESS) addr_input;
        
            printf("Input 0 for reading and 1 for writing: \n");
            scanf("%d", &opt);
            if(opt == 0) {
                printf("The data read is: %x\n", register_read(addr));
            } else {
                printf("Type in the data you want to write into the address: \n");
                scanf("%x", &data);
                register_write(addr, data);
                printf("Confirmation of data written in: %x\n", register_read(addr));
            }
            sleep_ms(250);

        }

        return 0;
    }

    
The code is still in progress. The issue now is to figure out how to access registers inside the MCU and the function reponsible for fetching data/reading data.


## Demo GIF

![a](https://github.com/ZhijingY/ese5190-2022-lab2b-esp/blob/main/lab/02_repl/_part2.gif)
