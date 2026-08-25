#pragma once

// --- LCD (QSPI, ST77916) ---
#define PIN_LCD_SDA0   46
#define PIN_LCD_SDA1   45
#define PIN_LCD_SDA2   42
#define PIN_LCD_SDA3   41
#define PIN_LCD_SCK    40
#define PIN_LCD_CS     21
#define PIN_LCD_TE     18   // tearing-effect signal, native GPIO
#define PIN_LCD_BL      5   // backlight, native GPIO
// LCD_RST is NOT a native GPIO - it's EXIO2 on the TCA9554 expander

// --- Touch (CST816, I2C) ---
#define PIN_TP_SDA      1
#define PIN_TP_SCL      3
#define PIN_TP_INT      4   // native GPIO
// TP_RST is EXIO1 on the TCA9554 expander

// --- Shared I2C bus (IMU + RTC + TCA9554 expander all live here) ---
#define PIN_I2C_SCL    10
#define PIN_I2C_SDA    11

// --- QMI8658 IMU (I2C, shares the bus above) ---
// 0x6B matches Waveshare's published address for this board family; SA0
// strapping isn't visible in the schematic pin table, so if the chip never
// ACKs, try 0x6A instead.
#define QMI8658_I2C_ADDR   0x6B

// --- TCA9554 I/O expander ---
#define TCA9554_I2C_ADDR   0x20
#define EXIO_TP_RST         1   // bit 1 -> touch reset
#define EXIO_LCD_RST         2   // bit 2 -> LCD reset
#define EXIO_SD_CS         3   // bit 3 -> SD card CS

// --- SD card (SPI) ---
#define PIN_SD_MISO    16
#define PIN_SD_MOSI    17
#define PIN_SD_SCLK    14
// SD_CS is EXIO3 on the expander

#define LCD_H_RES  360
#define LCD_V_RES  360
