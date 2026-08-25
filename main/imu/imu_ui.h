#pragma once

#include <stdbool.h>
#include "lvgl.h"
#include "driver/i2c_master.h"

#ifdef __cplusplus
extern "C" {
#endif

// Brings up the QMI8658 on `i2c_bus` and starts a background task that
// renders its accelerometer output as a ball on `screen`. Call once, after
// both the I2C bus and the LVGL display are ready. Returns false if the
// sensor failed to initialize (screen is left untouched in that case).
bool imu_ui_start(i2c_master_bus_handle_t i2c_bus, lv_obj_t *screen);

#ifdef __cplusplus
}
#endif
