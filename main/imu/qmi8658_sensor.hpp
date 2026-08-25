#pragma once

#include "accelerometer_sensor.hpp"
#include "driver/i2c_master.h"

namespace imu {

// QMI8658 driver, accelerometer axis only (gyro is left unconfigured and
// unread - add a symmetrical read path later if it's ever needed).
//
// Internally the chip is configured for a fixed +/-8g full-scale range and
// a fixed internal output data rate, independent of imu::config::kSampleRateHz:
// the chip just needs to refresh faster than we poll it, its own ODR isn't
// something callers need to reason about.
class Qmi8658Sensor final : public IAccelerometerSensor {
public:
    Qmi8658Sensor(i2c_master_bus_handle_t bus, uint8_t i2cAddress);

    bool init() override;
    bool read(AccelSample &out) override;

private:
    esp_err_t writeReg(uint8_t reg, uint8_t value);
    esp_err_t readReg(uint8_t reg, uint8_t &value);
    esp_err_t readRegs(uint8_t reg, uint8_t *buffer, size_t len);
    esp_err_t setRegBit(uint8_t reg, uint8_t bit);

    i2c_master_bus_handle_t bus_;
    uint8_t address_;
    i2c_master_dev_handle_t dev_ = nullptr;
    float accelScaleGPerLsb_ = 0.0f;
};

} // namespace imu
