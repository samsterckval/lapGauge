#pragma once

#include <cstdint>

namespace imu {

// A single 3-axis accelerometer reading, in g.
struct AccelSample {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

// Holds the latest raw and EMA-filtered accelerometer reading. Deliberately
// has no knowledge of any particular sensor driver, so the driver behind it
// can be swapped out without touching anything that consumes this data.
class AccelerometerData {
public:
    explicit AccelerometerData(uint32_t emaWindowSamples)
        : alpha_(2.0f / (static_cast<float>(emaWindowSamples) + 1.0f)) {}

    // Feeds in a new raw sample and updates the EMA-filtered value. The
    // first sample primes the filter directly, so it doesn't ramp up from
    // zero.
    void update(const AccelSample &raw) {
        raw_ = raw;
        if (!primed_) {
            filtered_ = raw;
            primed_ = true;
            return;
        }
        filtered_.x += alpha_ * (raw.x - filtered_.x);
        filtered_.y += alpha_ * (raw.y - filtered_.y);
        filtered_.z += alpha_ * (raw.z - filtered_.z);
    }

    const AccelSample &raw() const { return raw_; }
    const AccelSample &filtered() const { return filtered_; }

private:
    float alpha_;
    bool primed_ = false;
    AccelSample raw_{};
    AccelSample filtered_{};
};

} // namespace imu
