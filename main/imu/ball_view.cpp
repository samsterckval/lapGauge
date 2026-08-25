#include "ball_view.hpp"

#include "imu_config.hpp"
#include <algorithm>

namespace imu {

namespace {
float clampG(float value)
{
    return std::clamp(value, -config::kMaxAccelG, config::kMaxAccelG);
}
} // namespace

BallView::BallView(lv_obj_t *parent)
{
    ball_ = lv_obj_create(parent);
    lv_obj_remove_style_all(ball_);
    lv_obj_set_size(ball_, config::kBallDefaultDiameterPx, config::kBallDefaultDiameterPx);
    lv_obj_set_style_radius(ball_, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(ball_, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_set_style_bg_opa(ball_, LV_OPA_COVER, 0);
    lv_obj_clear_flag(ball_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(ball_, LV_ALIGN_CENTER, 0, 0);
}

void BallView::update(const AccelSample &accel)
{
    const float normX = clampG(accel.x) / config::kMaxAccelG;
    const float normY = clampG(accel.y) / config::kMaxAccelG;
    const float normZ = clampG(accel.z) / config::kMaxAccelG;

    const int32_t offsetX = static_cast<int32_t>(normX * config::kBallTravelRadiusPx);
    const int32_t offsetY = static_cast<int32_t>(normY * config::kBallTravelRadiusPx);
    lv_obj_align(ball_, LV_ALIGN_CENTER, -offsetY, offsetX);  // X and Y of IMU are 90deg from display

    const int32_t halfRange = normZ >= 0.0f
        ? (config::kBallMaxDiameterPx - config::kBallDefaultDiameterPx)
        : (config::kBallDefaultDiameterPx - config::kBallMinDiameterPx);
    const int32_t diameter = config::kBallDefaultDiameterPx + static_cast<int32_t>(normZ * halfRange);
    lv_obj_set_size(ball_, diameter, diameter);
}

} // namespace imu
