#include "tca9554.h"
#include "pins.h"

#define REG_OUTPUT  0x01
#define REG_CONFIG  0x03  // 0 = output, 1 = input (power-on default: all inputs)

static i2c_master_dev_handle_t s_dev = NULL;
static uint8_t s_output_shadow = 0xFF; // matches chip's power-on output reg default

static esp_err_t write_reg(uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = { reg, val };
    return i2c_master_transmit(s_dev, buf, sizeof(buf), 1000);
}

esp_err_t tca9554_init(i2c_master_bus_handle_t bus)
{
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = TCA9554_I2C_ADDR,
        .scl_speed_hz = 400000,
    };
    esp_err_t err = i2c_master_bus_add_device(bus, &dev_cfg, &s_dev);
    if (err != ESP_OK) return err;

    // Set the three pins we drive (LCD_RST, TP_RST, SD_CS) as outputs.
    // Config register bit = 0 means output; leave everything else as input.
    uint8_t config = 0xFF;
    config &= ~(1 << EXIO_LCD_RST);
    config &= ~(1 << EXIO_TP_RST);
    config &= ~(1 << EXIO_SD_CS);

    err = write_reg(REG_CONFIG, config);
    if (err != ESP_OK) return err;

    return write_reg(REG_OUTPUT, s_output_shadow);
}

esp_err_t tca9554_set_level(uint8_t pin, bool level)
{
    if (level) {
        s_output_shadow |= (1 << pin);
    } else {
        s_output_shadow &= ~(1 << pin);
    }
    return write_reg(REG_OUTPUT, s_output_shadow);
}
