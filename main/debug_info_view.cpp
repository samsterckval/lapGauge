#include "debug_info_view.hpp"

namespace app {

namespace {
constexpr uint32_t kUpdateIntervalMs = 1000;
} // namespace

DebugInfoView::DebugInfoView(lv_display_t *disp)
{
    label_ = lv_label_create(lv_display_get_screen_active(disp));
    lv_label_set_text(label_, "-- Hz");
    // The panel is round, so a corner (e.g. TOP_LEFT) sits outside the
    // visible circle and never renders. Top-center, offset down from the
    // very top edge, stays well inside it.
    lv_obj_align(label_, LV_ALIGN_TOP_MID, 0, 40);

    // Fires once per completed LVGL refresh pass, i.e. once per actual
    // frame pushed toward the panel - independent of how often any
    // particular data source (IMU or otherwise) updates on-screen content.
    lv_display_add_event_cb(disp, &DebugInfoView::onRefreshReady, LV_EVENT_REFR_READY, this);
}

void DebugInfoView::onRefreshReady(lv_event_t *e)
{
    static_cast<DebugInfoView *>(lv_event_get_user_data(e))->recordRefresh();
}

void DebugInfoView::recordRefresh()
{
    ++frameCount_;

    const uint32_t now = lv_tick_get();
    if (windowStartMs_ == 0) {
        windowStartMs_ = now;
        return;
    }

    const uint32_t elapsed = now - windowStartMs_;
    if (elapsed >= kUpdateIntervalMs) {
        const uint32_t refreshRateHz = (frameCount_ * 1000) / elapsed;
        lv_label_set_text_fmt(label_, "%u Hz", static_cast<unsigned>(refreshRateHz));
        frameCount_ = 0;
        windowStartMs_ = now;
    }
}

} // namespace app
