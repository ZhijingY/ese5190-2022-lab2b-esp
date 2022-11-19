#ifndef _WS2812_H 
#define _WS2812_H 
 
#include <stdlib.h>

void turn_on_pixel();
void set_pixel_color(uint32_t rgb);
void rp_init();
uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b);

#endif