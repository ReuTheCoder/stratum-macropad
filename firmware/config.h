#pragma once

/* Encoder resolution - one detent per step is common; adjust after testing */
#define ENCODER_RESOLUTION 4

/* RGB brightness cap - safety headroom, raise later if desired */
#define RGBLIGHT_LIMIT_VAL 200

/* OLED - SSD1306, 0.91" is typically 128x32 */
#define OLED_DISPLAY_128X32

/* I2C - override broken promicro_rp2040 board defaults with real pins */
#define I2C1_SDA_PIN GP6
#define I2C1_SCL_PIN GP7