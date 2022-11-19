### TODO:

You can now actually measure the timings you modeled in the last lab. Add APDS9960 support to your sensor, then set your system up to capture the following timing info:
- color packet delivered to PIO module
- bit delivered to WS2812 (24x/packet)
- full packet delivered to WS2812
- brightness changed on APDS

Run this experiment in both dark and light room settings (record initial ambient brightness in each case). The Neopixel should start 'off' and the ADPS9960 should be initialized with your preferred sampling rate (you may want to try a few different rates). Run the experiment for at least 100 samples at brightness settings of 0%, 25%, 50%, 75%, 100% (making sure to give the ADPS reading enough time to 'settle' each time Neopixel is turned off).

Report the observed 'jitter' based on misalignment with the free-running PWM module on the WS2012.

## Components used for protoboard work

- RP2040
- APDS9960

## Peripheral used

- GPIO
- PIO

## How the prototype works

The sensor will keep monitoring the brightness, proximity and RGB values on one core. On the other core, the pattern of PIO I2C bus transaction will be shown upon the BOOT button pressing and SCL pull-down triggering. Also when the BOOT button is pressed, the Neopixel will light up in the color detected by the sensor, displaying the color packet delivered to PIO module. Meanwhile it will blink as the pattern of RGB bits, showing the bit delivered to WS2812, full packet delivered to WS2812, and brightness changed on APDS.

## Code

The main C code is presented [here](https://github.com/ZhijingY/ese5190-2022-lab2b-esp/blob/main/lab/09_lab_on_a_chip/part9.c). The other relevant files have also been uploaded.

## Visualization of I2C bus

One clip of the PIO I2C bus trasaction of RGBC and proximity data between RP2040 and APDS9960 is shown below:
![a](https://github.com/ZhijingY/ese5190-2022-lab2b-esp/blob/main/lab/09_lab_on_a_chip/part9_1.png)

## Demo Video

The demo Video is shown below:

https://youtu.be/amoXaX4-qGw

In this demo video, I showed:

- The logical analyzer can be controlled by BOOT button, and triggered by the pull-down on SCL bus.
- The visualization of SDA and SCL bus upon BOOT button pressing.
- The Neopixel light lights up in the color sensed by APDS9960, and blinks as the pattern of the bit values in the RGB value sensed by the sensor, showing the color packet, as well as visualizing the bit delivered to WS2812 (24x/packet).
- The change in brightness is shown as the changing pattern of Neopixel blinking.


