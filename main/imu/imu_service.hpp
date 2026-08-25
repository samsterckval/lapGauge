#pragma once

#include "accelerometer_sensor.hpp"
#include "accelerometer_data.hpp"
#include "ball_view.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <memory>

namespace imu {

// Owns the background task that polls the sensor at
// imu::config::kSampleRateHz, runs each sample through the EMA filter, and
// pushes the filtered result to the ball view under the LVGL port lock.
class ImuService {
public:
    ImuService(std::unique_ptr<IAccelerometerSensor> sensor, lv_obj_t *screen);
    ~ImuService();

    ImuService(const ImuService &) = delete;
    ImuService &operator=(const ImuService &) = delete;

    // Initializes the sensor and starts the polling task. Returns false
    // (and does not start the task) if sensor init fails.
    bool start();

private:
    static void taskTrampoline(void *arg);
    void taskLoop();

    std::unique_ptr<IAccelerometerSensor> sensor_;
    AccelerometerData data_;
    BallView view_;
    TaskHandle_t taskHandle_ = nullptr;
};

} // namespace imu
