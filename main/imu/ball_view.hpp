#pragma once

#include "lvgl.h"
#include "accelerometer_data.hpp"

namespace imu {

// Renders an accelerometer reading as a ball centered on screen at 0g: X/Y
// acceleration moves it, Z acceleration resizes it. Knows nothing about
// where the data came from or how it was filtered.
class BallView {
public:
    explicit BallView(lv_obj_t *parent);

    // Caller must hold the LVGL port lock (lvgl_port_lock) when calling
    // this - it touches LVGL objects.
    void update(const AccelSample &accel);

private:
    lv_obj_t *ball_;
};

} // namespace imu
