## Components used for protoboard work

- RP2040
- ADPS9960

## Peripheral used

- GPIO
- I2C

## How the prototype works

Once the program starts, we can press the BOOT button to monitor the data transfer over I2C on pin 22 and pin 23.

## Code

    #include <stdio.h>
    #include <stdlib.h>

    #include "pico/stdlib.h"
    #include "hardware/pio.h"
    #include "hardware/dma.h"
    #include "hardware/structs/bus_ctrl.h"

    #include <string.h>
    #include "pico/binary_info.h"
    #include "hardware/pio.h"
    #include "hardware/i2c.h"

    #include "ws2812.h"
    #include "hardware/clocks.h"
    #include "ws2812.pio.h"
    #include "registers.h"

    // Some logic to analyse:
    #include "hardware/structs/pwm.h"

    const uint CAPTURE_PIN_BASE = 22;
    const uint CAPTURE_PIN_COUNT = 2;
    const uint CAPTURE_N_SAMPLES = 256;
    const uint TRIGGER_PIN = 22;
    const uint BOOT_PIN = 21;

    static inline uint bits_packed_per_word(uint pin_count) {
        const uint SHIFT_REG_WIDTH = 32;
        return SHIFT_REG_WIDTH - (SHIFT_REG_WIDTH % pin_count);
    }

    void logic_analyser_init(PIO pio, uint sm, uint pin_base, uint pin_count, float div) {
        // Load a program to capture n pins. This is just a single `in pins, n`
        // instruction with a wrap.
        uint16_t capture_prog_instr = pio_encode_in(pio_pins, pin_count);
        struct pio_program capture_prog = {
                .instructions = &capture_prog_instr,
                .length = 1,
                .origin = -1
        };
        uint offset = pio_add_program(pio, &capture_prog);
        pio_sm_config c = pio_get_default_sm_config();
        sm_config_set_in_pins(&c, pin_base);
        sm_config_set_wrap(&c, offset, offset);
        sm_config_set_clkdiv(&c, div);
        sm_config_set_in_shift(&c, true, true, bits_packed_per_word(pin_count));
        sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_RX);
        pio_sm_init(pio, sm, offset, &c);
    }

    void logic_analyser_arm(PIO pio, uint sm, uint dma_chan, uint32_t *capture_buf, size_t capture_size_words,
                            uint trigger_pin, bool trigger_level) {
        pio_sm_set_enabled(pio, sm, false);
        // Need to clear _input shift counter_, as well as FIFO, because there may be
        // partial ISR contents left over from a previous run. sm_restart does this.
        pio_sm_clear_fifos(pio, sm);
        pio_sm_restart(pio, sm);

        dma_channel_config c = dma_channel_get_default_config(dma_chan);
        channel_config_set_read_increment(&c, false);
        channel_config_set_write_increment(&c, true);
        channel_config_set_dreq(&c, pio_get_dreq(pio, sm, false));

        dma_channel_configure(dma_chan, &c,
            capture_buf,        // Destination pointer
            &pio->rxf[sm],      // Source pointer
            capture_size_words, // Number of transfers
            true                // Start immediately
        );

        pio_sm_exec(pio, sm, pio_encode_wait_gpio(trigger_level, trigger_pin));
        pio_sm_set_enabled(pio, sm, true);
    }

    void print_capture_buf(const uint32_t *buf, uint pin_base, uint pin_count, uint32_t n_samples) {
        // Display the capture buffer in text form, like this:
        // 00: __--__--__--__--__--__--
        // 01: ____----____----____----
        printf("Capture:\n");
        // Each FIFO record may be only partially filled with bits, depending on
        // whether pin_count is a factor of 32.
        uint record_size_bits = bits_packed_per_word(pin_count);
        for (int pin = 0; pin < pin_count; ++pin) {
            printf("%02d: ", pin + pin_base);
            for (int sample = 0; sample < n_samples; ++sample) {
                uint bit_index = pin + sample * pin_count;
                uint word_index = bit_index / record_size_bits;
                // Data is left-justified in each FIFO entry, hence the (32 - record_size_bits) offset
                uint word_mask = 1u << (bit_index % record_size_bits + 32 - record_size_bits);
                printf(buf[word_index] & word_mask ? "-" : "_");
            }
            printf("\n");
        }
    }

    int main() {
        stdio_init_all();
        gpio_init(BOOT_PIN);
        gpio_set_dir(BOOT_PIN, GPIO_IN);
        rp_init();
        turn_on_pixel();

        uint32_t rgb = 0x00A4B3E6;
        set_pixel_color(rgb);
        sleep_ms(1000);
        set_pixel_color(0x00000000);

        printf("PIO logic analyser example\n");

        uint total_sample_bits = CAPTURE_N_SAMPLES * CAPTURE_PIN_COUNT;
        total_sample_bits += bits_packed_per_word(CAPTURE_PIN_COUNT) - 1;
        uint buf_size_words = total_sample_bits / bits_packed_per_word(CAPTURE_PIN_COUNT);
        uint32_t *capture_buf = malloc(buf_size_words * sizeof(uint32_t));
        hard_assert(capture_buf);

        bus_ctrl_hw->priority = BUSCTRL_BUS_PRIORITY_DMA_W_BITS | BUSCTRL_BUS_PRIORITY_DMA_R_BITS;

        PIO pio = pio0;
        uint sm = 0;
        uint dma_chan = 0;
        logic_analyser_init(pio, sm, CAPTURE_PIN_BASE, CAPTURE_PIN_COUNT, 256.f);

        while(true) {
            if(!gpio_get(BOOT_PIN)) {

                printf("Arming trigger\n");
                logic_analyser_arm(pio, sm, dma_chan, capture_buf, buf_size_words, CAPTURE_PIN_BASE + 1, false);

                printf("Starting I2C Detection\n");

                gpio_set_function(CAPTURE_PIN_BASE, GPIO_FUNC_I2C);
                gpio_set_function(CAPTURE_PIN_BASE + 1, GPIO_FUNC_I2C);

                // ------------------------------------------------------------------------

                // The logic analyser should have started capturing as soon as it saw the
                // first transition. Wait until the last sample comes in from the DMA.
                dma_channel_wait_for_finish_blocking(dma_chan);

                print_capture_buf(capture_buf, CAPTURE_PIN_BASE, CAPTURE_PIN_COUNT, CAPTURE_N_SAMPLES);

                sleep_ms(1000);
            }
        }
    }
    
The first set_pixel_color() is to confirm that the program is successfully running on the board.


## Demo Video

https://youtube.com/shorts/wq8ONv1sfkM?feature=share

In this demo, I showed two rounds of recording and replaying of the BOOT button pressing sequence, and I tested different replay speed.

