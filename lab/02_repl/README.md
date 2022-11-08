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

    int main() {

        uint32_t addr = 0x00000000;

        while (true) {
            printf("Input a register address you would like to read/write: ")
            scanf("%ld", &addr);

            // Implement functions for reading/writing the input register address
        }

        return 0;
    }
    
The code is still in progress. The issue now is to figure out how to access registers inside the MCU and the function reponsible for fetching data/reading data.


## Demo GIF

![a](https://github.com/ZhijingY/ese5190-2022-lab2b-esp/blob/main/lab/02_repl/_part2.gif)
