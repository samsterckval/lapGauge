#pragma once

// App-wide configuration flags - not specific to any one subsystem (IMU,
// display, etc). Subsystem-specific tunables belong in that subsystem's own
// config header instead (e.g. main/imu/imu_config.hpp).
namespace app::config {

// Shows a small on-screen debug overlay (currently a refresh-rate label).
// Flip to false to compile it out entirely.
inline constexpr bool kShowDebugInfo = true;

} // namespace app::config
