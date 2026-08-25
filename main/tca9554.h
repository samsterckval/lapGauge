#pragma once
#include "driver/i2c_master.h"
#include "esp_err.h"

// Call once, after the shared I2C bus handle has been created.
esp_err_t tca9554_init(i2c_master_bus_handle_t bus);

// Drive a single expander output pin (0-7) high or low.
esp_err_t tca9554_set_level(uint8_t pin, bool level);
