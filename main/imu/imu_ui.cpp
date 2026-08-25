#include "imu_ui.h"

#include "imu_service.hpp"
#include "qmi8658_sensor.hpp"
#include "pins.h"
#include "esp_lvgl_port.h"
#include <memory>

namespace {
// Owns the service for the process lifetime; imu_ui_start is only expected
// to be called once.
std::unique_ptr<imu::ImuService> g_imu_service;
} // namespace

bool imu_ui_start(i2c_master_bus_handle_t i2c_bus, lv_obj_t *screen)
{
    auto sensor = std::make_unique<imu::Qmi8658Sensor>(i2c_bus, QMI8658_I2C_ADDR);

    // ImuService's constructor creates the ball's LVGL object, so it needs
    // the port lock held just like any other LVGL access.
    lvgl_port_lock(0);
    g_imu_service = std::make_unique<imu::ImuService>(std::move(sensor), screen);
    lvgl_port_unlock();

    return g_imu_service->start();
}
