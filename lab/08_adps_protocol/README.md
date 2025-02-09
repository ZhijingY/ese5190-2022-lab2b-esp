### TODO:

Use the capabilities of your sequencer to implement the ADPS9960 protocol and control the sensor.

## Components used for protoboard work

- RP2040

## Peripheral used

- GPIO
- PIO

## How the prototype works

In this part the values of RGBC and proximity are read from APDS9960 via PIO I2C, and display the data on console periodically.

## Code

The [main C code](https://github.com/ZhijingY/ese5190-2022-lab2b-esp/blob/main/lab/08_adps_protocol/part8.c) was uploaded. One important trick (inspired by Dang0v) is that one line needs to be added to i2c.pio:

    mov isr, null
    
This line needs to be added at the end of bitloop to reset the input bit counter, or the values of RGBC and proximity will not be refreshed and recorded.

## Demo GIF

The demo GIF is shown below:

![a](https://github.com/ZhijingY/ese5190-2022-lab2b-esp/blob/main/lab/08_adps_protocol/part8.gif)

In this demo GIF, I mainly tested the proximity measurement performance.

