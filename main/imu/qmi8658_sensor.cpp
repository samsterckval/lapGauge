#include "qmi8658_sensor.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

namespace imu {

namespace {
constexpr char kTag[] = "qmi8658";

// Register map subset needed for accelerometer-only readout. Values are
// taken from the QMI8658 datasheet / vendor reference drivers, not
// discovered by trial and error on this specific board - double check
// against the datasheet if accel readings ever look wrong.
namespace reg {
constexpr uint8_t WHO_AM_I = 0x00;
constexpr uint8_t CTRL1 = 0x02;
constexpr uint8_t CTRL2 = 0x03;
constexpr uint8_t CTRL7 = 0x08;
constexpr uint8_t STATUS0 = 0x2E;
constexpr uint8_t AX_L = 0x35; // 6-byte burst: AX_L,AX_H,AY_L,AY_H,AZ_L,AZ_H
constexpr uint8_t RESET = 0x60;
constexpr uint8_t RST_RESULT = 0x4D;
} // namespace reg

constexpr uint8_t kWhoAmIValue = 0x05;
constexpr uint8_t kResetCommand = 0xB0;
constexpr uint8_t kResetResultMask = 0x80;

constexpr uint8_t kCtrl1AddrAutoIncrementBit = 6;
constexpr uint8_t kCtrl7AccelEnableBit = 0;

// CTRL2 = (range << 4) | odr. +/-8g full scale, ~224 Hz internal ODR -
// comfortably above imu::config::kSampleRateHz so a fresh sample is always
// ready when we poll.
constexpr uint8_t kAccelRange8G = 0x02;
constexpr uint8_t kAccelOdr224Hz = 0x05;
constexpr float kAccelScale8G = 8.0f / 32768.0f;

constexpr uint32_t kResetTimeoutMs = 500;
constexpr uint32_t kResetPollIntervalMs = 10;
constexpr uint32_t kI2cTimeoutMs = 1000;
} // namespace

Qmi8658Sensor::Qmi8658Sensor(i2c_master_bus_handle_t bus, uint8_t i2cAddress)
    : bus_(bus), address_(i2cAddress) {}

esp_err_t Qmi8658Sensor::writeReg(uint8_t reg, uint8_t value)
{
    uint8_t buf[2] = {reg, value};
    return i2c_master_transmit(dev_, buf, sizeof(buf), kI2cTimeoutMs);
}

esp_err_t Qmi8658Sensor::readReg(uint8_t reg, uint8_t &value)
{
    return i2c_master_transmit_receive(dev_, &reg, 1, &value, 1, kI2cTimeoutMs);
}

esp_err_t Qmi8658Sensor::readRegs(uint8_t reg, uint8_t *buffer, size_t len)
{
    return i2c_master_transmit_receive(dev_, &reg, 1, buffer, len, kI2cTimeoutMs);
}

esp_err_t Qmi8658Sensor::setRegBit(uint8_t reg, uint8_t bit)
{
    uint8_t value = 0;
    esp_err_t err = readReg(reg, value);
    if (err != ESP_OK) return err;
    return writeReg(reg, value | (1 << bit));
}

bool Qmi8658Sensor::init()
{
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = address_,
        .scl_speed_hz = 400000,
    };
    if (i2c_master_bus_add_device(bus_, &dev_cfg, &dev_) != ESP_OK) {
        ESP_LOGE(kTag, "failed to add I2C device at 0x%02X", address_);
        return false;
    }

    if (writeReg(reg::RESET, kResetCommand) != ESP_OK) {
        ESP_LOGE(kTag, "reset write failed");
        return false;
    }

    bool resetOk = false;
    for (uint32_t waited = 0; waited < kResetTimeoutMs; waited += kResetPollIntervalMs) {
        uint8_t result = 0;
        if (readReg(reg::RST_RESULT, result) == ESP_OK &&
            result != 0xFF && (result & kResetResultMask)) {
            resetOk = true;
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(kResetPollIntervalMs));
    }
    if (!resetOk) {
        ESP_LOGE(kTag, "reset timed out");
        return false;
    }

    if (setRegBit(reg::CTRL1, kCtrl1AddrAutoIncrementBit) != ESP_OK) {
        ESP_LOGE(kTag, "failed to enable address auto-increment");
        return false;
    }

    uint8_t whoAmI = 0;
    if (readReg(reg::WHO_AM_I, whoAmI) != ESP_OK || whoAmI != kWhoAmIValue) {
        ESP_LOGE(kTag, "unexpected WHO_AM_I: 0x%02X (expected 0x%02X)", whoAmI, kWhoAmIValue);
        return false;
    }

    const uint8_t ctrl2 = static_cast<uint8_t>((kAccelRange8G << 4) | kAccelOdr224Hz);
    if (writeReg(reg::CTRL2, ctrl2) != ESP_OK) {
        ESP_LOGE(kTag, "failed to configure accelerometer range/ODR");
        return false;
    }
    accelScaleGPerLsb_ = kAccelScale8G;

    if (setRegBit(reg::CTRL7, kCtrl7AccelEnableBit) != ESP_OK) {
        ESP_LOGE(kTag, "failed to enable accelerometer");
        return false;
    }

    ESP_LOGI(kTag, "QMI8658 ready at 0x%02X", address_);
    return true;
}

bool Qmi8658Sensor::read(AccelSample &out)
{
    uint8_t buffer[6];
    if (readRegs(reg::AX_L, buffer, sizeof(buffer)) != ESP_OK) {
        return false;
    }

    const int16_t rawX = static_cast<int16_t>(buffer[0] | (buffer[1] << 8));
    const int16_t rawY = static_cast<int16_t>(buffer[2] | (buffer[3] << 8));
    const int16_t rawZ = static_cast<int16_t>(buffer[4] | (buffer[5] << 8));

    out.x = static_cast<float>(rawX) * accelScaleGPerLsb_;
    out.y = static_cast<float>(rawY) * accelScaleGPerLsb_;
    out.z = static_cast<float>(rawZ) * accelScaleGPerLsb_;
    return true;
}

} // namespace imu
