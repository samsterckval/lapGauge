#include "debug_ui.h"

#include "debug_info_view.hpp"
#include "app_config.hpp"
#include "esp_lvgl_port.h"
#include <memory>

namespace {
// Owns the view for the process lifetime; debug_ui_start is only expected
// to be called once.
std::unique_ptr<app::DebugInfoView> g_debug_view;
} // namespace

void debug_ui_start(lv_display_t *disp)
{
    if constexpr (app::config::kShowDebugInfo) {
        // DebugInfoView's constructor creates an LVGL label, so it needs
        // the port lock held just like any other LVGL access.
        lvgl_port_lock(0);
        g_debug_view = std::make_unique<app::DebugInfoView>(disp);
        lvgl_port_unlock();
    }
}
