#pragma once

#include <algorithm>
#include <cstdint>

namespace snes {
struct NightDisplayState {
  bool active{false};
  bool waiting_for_time{false};
  bool display_on{false};
  uint8_t brightness{0};
};

// Local wall-clock seconds. Start is inclusive; end is exclusive.
// Equal times disable the window, rather than switching the display off all day.
inline bool in_night_window(int now, int start, int end) {
  if (now < 0 || now >= 86400 || start < 0 || start >= 86400 ||
      end < 0 || end >= 86400 || start == end) return false;
  return start < end ? now >= start && now < end : now >= start || now < end;
}

inline NightDisplayState night_display(bool schedule_enabled, bool time_valid,
    int now, int start, int end, bool dim, bool manual_display,
    int day_brightness, int night_brightness) {
  NightDisplayState out;
  const bool configured = schedule_enabled && start != end;
  out.waiting_for_time = configured && !time_valid;
  out.active = configured && time_valid && in_night_window(now, start, end);
  out.brightness = static_cast<uint8_t>(std::clamp(day_brightness, 0, 100));
  if (out.active && dim)
    out.brightness = std::min(out.brightness,
        static_cast<uint8_t>(std::clamp(night_brightness, 0, 100)));
  out.display_on = manual_display && !out.waiting_for_time &&
                   !(out.active && !dim) && out.brightness > 0;
  if (!out.display_on) out.brightness = 0;
  return out;
}
}  // namespace snes
