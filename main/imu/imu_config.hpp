#pragma once

#include <cstdint>

// Tunables for the IMU readout pipeline and its ball display. Everything
// that should be adjustable without touching logic lives here.
namespace imu::config {

// How often the background task pulls a new sample from the sensor and
// pushes it through the filter. Independent of the sensor's own internal
// output data rate (the driver picks an internal ODR fast enough to always
// have a fresh sample ready at this poll rate).
inline constexpr uint32_t kSampleRateHz = 100;
inline constexpr uint32_t kSamplePeriodMs = 1000 / kSampleRateHz;

// EMA smoothing "window" in samples. Converted to a smoothing factor via
// alpha = 2 / (N + 1), the usual EMA/SMA equivalence: larger N = smoother
// but laggier response.
inline constexpr uint32_t kEmaWindowSamples = 8;

// Acceleration magnitude (per axis, in g) that maps to full ball travel /
// full size change. Values beyond this are clamped.
inline constexpr float kMaxAccelG = 2.0f;

// Ball appearance, in pixels. Diameter at 0g, and the min/max it can shrink
// / grow to as Z acceleration sweeps from -kMaxAccelG to +kMaxAccelG.
inline constexpr int32_t kBallDefaultDiameterPx = 40;
inline constexpr int32_t kBallMinDiameterPx = 24;
inline constexpr int32_t kBallMaxDiameterPx = 64;

// How far from screen center the ball can travel (in pixels) at
// +/-kMaxAccelG on X/Y. Kept well inside the round panel's visible area so
// the ball never clips the bezel even at kBallMaxDiameterPx.
inline constexpr int32_t kBallTravelRadiusPx = 160;

} // namespace imu::config
