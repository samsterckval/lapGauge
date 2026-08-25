#pragma once

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

// Starts the debug overlay on `disp` if app::config::kShowDebugInfo is set
// (a no-op otherwise). Call once, after the LVGL display is ready.
void debug_ui_start(lv_display_t *disp);

#ifdef __cplusplus
}
#endif
