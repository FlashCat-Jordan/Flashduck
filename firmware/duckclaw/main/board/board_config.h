#pragma once

#include "driver/gpio.h"

/*
 * Flashduck / DuckClaw board configuration
 *
 * The two "hand" outputs on the official camera board are rewired on
 * Flashduck:
 *   left hand  -> mouth servo  (GPIO4)
 *   right hand -> neck servo   (GPIO7)
 * The leg/foot servos and the 240x240 ST7789 display keep the official
 * camera-board routing.
 */

/* ── Servo GPIO pins ───────────────────────────────────────── */
#define OTTO_PIN_LEFT_LEG    5
#define OTTO_PIN_RIGHT_LEG   43
#define OTTO_PIN_LEFT_FOOT   6
#define OTTO_PIN_RIGHT_FOOT  44
#define OTTO_PIN_LEFT_HAND   4   /* Flashduck: mouth */
#define OTTO_PIN_RIGHT_HAND  7   /* Flashduck: neck  */

/* ── LCD GPIO pins ─────────────────────────────────────────── */
#define OTTO_PIN_LCD_BL      38
#define OTTO_PIN_LCD_MOSI    45
#define OTTO_PIN_LCD_SCLK    48
#define OTTO_PIN_LCD_DC      47
#define OTTO_PIN_LCD_RST     1
#define OTTO_PIN_LCD_CS      (-1)

#define OTTO_LCD_SPI_HOST     SPI3_HOST
#define OTTO_LCD_PIXEL_CLK    (40 * 1000 * 1000)
#define OTTO_LCD_H_RES        240
#define OTTO_LCD_V_RES        240

/* BOOT button (same as the official board) */
#define OTTO_PIN_BOOT_BUTTON  GPIO_NUM_0
