#pragma once

#include "lvgl.h"

namespace app {

// Small on-screen overlay for debug info - currently just the display's
// actual refresh rate. Self-driving: it hooks LV_EVENT_REFR_READY on the
// display itself, so it counts real screen refreshes regardless of what's
// pushing content to it (IMU, future subsystems, whatever) - no one needs
// to feed it samples. Gated on app::config::kShowDebugInfo by whoever
// constructs it (see debug_ui_start); this class itself doesn't know about
// that flag.
class DebugInfoView {
public:
    explicit DebugInfoView(lv_display_t *disp);

    DebugInfoView(const DebugInfoView &) = delete;
    DebugInfoView &operator=(const DebugInfoView &) = delete;

private:
    static void onRefreshReady(lv_event_t *e);
    void recordRefresh();

    lv_obj_t *label_;
    uint32_t frameCount_ = 0;
    uint32_t windowStartMs_ = 0;
};

} // namespace app
