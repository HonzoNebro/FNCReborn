// Playback notifications for the supported disc's movie-player integration.
#pragma once

#include <cstdint>
#include <map>
#include <set>

namespace fnc_movie {
constexpr uint32_t kPlayingCallback = 0x8209F680;
constexpr uint32_t kStopCallback = 0x8209F698;
constexpr uint32_t kNotificationReturn = 0x82726800;

class Playback {
 public:
  bool Bind(uint32_t player, uint32_t object) {
    if (!object) return false;
    auto it = objects_.find(player);
    if (it != objects_.end() && it->second == object) return false;
    objects_[player] = object;
    return true;
  }
  bool ReleaseObject(uint32_t object) {
    bool was_active = active();
    for (auto it = objects_.begin(); it != objects_.end();) {
      if (it->second == object) {
        players_.erase(it->first);
        it = objects_.erase(it);
      } else {
        ++it;
      }
    }
    return was_active != active();
  }
  bool Observe(uint32_t callback, uint32_t player) {
    bool was_active = active();
    if (callback == kPlayingCallback) players_.insert(player);
    if (callback == kStopCallback) players_.erase(player);
    return was_active != active();
  }
  bool active() const { return !players_.empty(); }

 private:
  std::set<uint32_t> players_;
  std::map<uint32_t, uint32_t> objects_;
};
}  // namespace fnc_movie
