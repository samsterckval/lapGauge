#pragma once

#include "accelerometer_data.hpp"

namespace imu {

// Driver-agnostic interface for a 3-axis accelerometer. Everything upstream
// (the EMA filter, the ball view, the polling task) talks to this interface
// only, so a concrete driver like Qmi8658Sensor can be swapped for another
// chip later without touching any of that code.
class IAccelerometerSensor {
public:
    virtual ~IAccelerometerSensor() = default;

    // One-time bring-up: bus/device registration, chip ID check, sensor
    // configuration. Returns true on success.
    virtual bool init() = 0;

    // Reads the latest accelerometer sample into `out`. Returns true on a
    // successful read (out is valid), false on a communication error (out
    // is left unchanged).
    virtual bool read(AccelSample &out) = 0;
};

} // namespace imu
