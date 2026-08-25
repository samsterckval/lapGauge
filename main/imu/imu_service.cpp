#include "imu_service.hpp"

#include "imu_config.hpp"
#include "esp_lvgl_port.h"
#include "esp_log.h"

namespace imu {

namespace {
constexpr char kTag[] = "imu_service";
constexpr uint32_t kTaskStackWords = 4096;
constexpr UBaseType_t kTaskPriority = 5;
} // namespace

ImuService::ImuService(std::unique_ptr<IAccelerometerSensor> sensor, lv_obj_t *screen)
    : sensor_(std::move(sensor)), data_(config::kEmaWindowSamples), view_(screen) {}

ImuService::~ImuService()
{
    if (taskHandle_ != nullptr) {
        vTaskDelete(taskHandle_);
    }
}

bool ImuService::start()
{
    if (!sensor_->init()) {
        return false;
    }
    xTaskCreate(&ImuService::taskTrampoline, "imu_poll", kTaskStackWords, this, kTaskPriority, &taskHandle_);
    return true;
}

void ImuService::taskTrampoline(void *arg)
{
    static_cast<ImuService *>(arg)->taskLoop();
}

void ImuService::taskLoop()
{
    AccelSample raw;
    while (true) {
        if (sensor_->read(raw)) {
            data_.update(raw);
            lvgl_port_lock(0);
            view_.update(data_.filtered());
            lvgl_port_unlock();
        } else {
            ESP_LOGW(kTag, "sensor read failed");
        }
        vTaskDelay(pdMS_TO_TICKS(config::kSamplePeriodMs));
    }
}

} // namespace imu
