#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/pio.h"
#include "hardware/i2c.h"
#include "pio_i2c.h"
#include "hardware/dma.h"
#include "pico/multicore.h"

#include "ws2812.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"
#include "registers.h"

// #include "sequencer.h"

#include <stdlib.h>
#include "hardware/structs/bus_ctrl.h"

const uint CAPTURE_PIN_BASE = 22;
const uint CAPTURE_PIN_COUNT = 2;
const uint CAPTURE_N_SAMPLES = 300;
const uint TRIGGER_PIN = 22;
const uint BOOT_PIN = 21;

const int addr = 0x39;
PIO pio = pio0;
uint sm = 1;

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

    // Configure state machine to loop over this `in` instruction forever,
    // with autopush enabled.
    pio_sm_config c = pio_get_default_sm_config();
    sm_config_set_in_pins(&c, pin_base);
    sm_config_set_wrap(&c, offset, offset);
    sm_config_set_clkdiv(&c, div);
    // Note that we may push at a < 32 bit threshold if pin_count does not
    // divide 32. We are using shift-to-right, so the sample data ends up
    // left-justified in the FIFO in this case, with some zeroes at the LSBs.
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
            //printf(buf[word_index] & word_mask ? "-" : "_");
            //printf(buf[word_index] & word_mask ? "1" : "0");
            printf(buf[word_index] & word_mask ? "-" : "_");
        }
        printf("\n");
    }
}

static void APDS9960_reset() {
    // Two byte reset. First byte register, second byte data
    // There are a load more options to set up the device in different ways that could be added here
    uint8_t buf[] = {0x80, 0x27};
    pio_i2c_write_blocking(pio, sm, addr, buf, 2, false);
}

static void mpu6050_read_raw(uint8_t *temp) {
    // For this particular device, we send the device the register we want to read
    // first, then subsequently read from the device. The register is auto incrementing
    // so we don't need to keep sending the register we want, just the first.

    uint8_t buffer[1];

    // Start reading acceleration registers from register 0x3B for 6 bytes
    uint8_t val = 0x9C;
    pio_i2c_write_blocking(pio, sm, addr, &val, 1, true); // true to keep master control of bus
    pio_i2c_read_blocking(pio, sm, addr, buffer, 1, false);

    *temp = buffer[0];
}

static void APDS_read_color(uint16_t *r, uint16_t *g, uint16_t *b, uint16_t *c) {

    uint8_t buffer[2];
    // Start reading acceleration registers from register 0x3B for 6 bytes
    uint8_t c_addr = 0x94;
    pio_i2c_write_blocking(pio, sm, addr, &c_addr, 1, true); // true to keep master control of bus
    pio_i2c_read_blocking(pio, sm, addr, buffer, 2, false);

    *c = buffer[1];
    *c = *c << 8;
    *c |= buffer[0];

    uint8_t r_addr = 0x96;
    pio_i2c_write_blocking(pio, sm, addr, &r_addr, 1, true); // true to keep master control of bus
    pio_i2c_read_blocking(pio, sm, addr, buffer, 2, false);

    *r = buffer[0] | (buffer[1] << 8);

    uint8_t g_addr = 0x98;
    pio_i2c_write_blocking(pio, sm, addr, &g_addr, 1, true); // true to keep master control of bus
    pio_i2c_read_blocking(pio, sm, addr, buffer, 2, false);

    *g = buffer[0] | (buffer[1] << 8);

    uint8_t b_addr = 0x9A;
    pio_i2c_write_blocking(pio, sm, addr, &b_addr, 1, true); // true to keep master control of bus
    pio_i2c_read_blocking(pio, sm, addr, buffer, 2, false);

    *b = buffer[0] | (buffer[1] << 8);
}

void main_core1() {
    uint8_t proximity = 0;
    uint16_t r,g,b,c;

    while (true) {     
        // set_pixel_color(0x00000000);
        proximity = 0;
        r = 0; g = 1; b = 0; c = 0;
        uint32_t rgb;
        mpu6050_read_raw(&proximity);
        APDS_read_color(&r, &g, &b, &c);

        if(!gpio_get(BOOT_PIN)) {
            for(int i = 0; i < 32; i++) {
                rgb = urgb_u32((uint8_t)(r), (uint8_t)(g), (uint8_t)(b));
                if((rgb >> i) & 0x1) {
                    sleep_ms(100);
                } else {
                    rgb = 0x00000000;
                    sleep_ms(100);
                }
                set_pixel_color(rgb);
            }
        }
        rgb = 0x00000000;
        set_pixel_color(rgb);

        sleep_ms(1000);
    }
}

int main() {
    const uint SDA_PIN = 22;
    const uint SCL_PIN = 23;

    stdio_init_all();
    gpio_init(BOOT_PIN);
    gpio_set_dir(BOOT_PIN, GPIO_IN);
    // i2c_init(i2c1, 200*1000);
    rp_init();
    turn_on_pixel();

    uint32_t rgb = 0x00A4B3E6;
    set_pixel_color(rgb);
    sleep_ms(1000);
    set_pixel_color(0x00000000);

    printf("PIO logic analyser set\n");

    uint total_sample_bits = CAPTURE_N_SAMPLES * CAPTURE_PIN_COUNT;
    total_sample_bits += bits_packed_per_word(CAPTURE_PIN_COUNT) - 1;
    uint buf_size_words = total_sample_bits / bits_packed_per_word(CAPTURE_PIN_COUNT);
    uint32_t *capture_buf = malloc(buf_size_words * sizeof(uint32_t));
    hard_assert(capture_buf);

    bus_ctrl_hw->priority = BUSCTRL_BUS_PRIORITY_DMA_W_BITS | BUSCTRL_BUS_PRIORITY_DMA_R_BITS;

    PIO pio_1 = pio1;
    uint sm_pio_1 = 0;
    uint dma_chan = 0;
    logic_analyser_init(pio_1, sm_pio_1, CAPTURE_PIN_BASE, CAPTURE_PIN_COUNT, 128.f);

    uint offset = pio_add_program(pio, &i2c_program);
    i2c_program_init(pio, sm, offset, SDA_PIN, SCL_PIN);
    // pio_i2c_repstart(pio, sm);
    // gpio_pull_up(SDA_PIN);
    // gpio_pull_up(SCL_PIN);
    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(SDA_PIN, SCL_PIN, GPIO_FUNC_I2C));
    APDS9960_reset();

    multicore_launch_core1(main_core1);

    while (true) {     
        while (gpio_get(BOOT_PIN)) {
            printf("Waiting for boot button\n");
            sleep_ms(500);
        };
        printf("Arming trigger\n");
        logic_analyser_arm(pio_1, sm_pio_1, dma_chan, capture_buf, buf_size_words, CAPTURE_PIN_BASE + 1, false);
        printf("Starting I2C Detection\n");
        dma_channel_wait_for_finish_blocking(dma_chan);
        print_capture_buf(capture_buf, CAPTURE_PIN_BASE, CAPTURE_PIN_COUNT, CAPTURE_N_SAMPLES);
        sleep_ms(100);
    }
}
