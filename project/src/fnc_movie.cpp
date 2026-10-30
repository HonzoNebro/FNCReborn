// Original hook: observe movie state notifications without changing their dispatch.
#include "movie_playback.h"

#include <mutex>
#include <rex/cvar.h>
#include <rex/logging.h>
#include <rex/ppc.h>

REXCVAR_DEFINE_BOOL(fnc_video_letterbox, false, "Game/Video",
                    "Experimental: preserve full 16:9 movie frames during wide gameplay");
REXCVAR_DEFINE_BOOL(fnc_movie_active, false, "Game/Video",
                    "Internal: a movie player is playing and has not stopped or been destroyed")
    .lifecycle(rex::cvar::Lifecycle::kHotReload);

namespace {
std::mutex g_movie_mutex;
fnc_movie::Playback g_playback;

void PublishState(uint32_t id, const char* reason) {
  bool active = g_playback.active();
  bool set = rex::cvar::SetFlagByName("fnc_movie_active", active ? "true" : "false");
  if (REXCVAR_GET(fnc_video_letterbox)) {
    REXLOG_INFO("[FNC-VIDEO] playback {} ({} {:08X}, state bridge {})",
                active ? "started" : "stopped", reason, id, set ? "ok" : "FAILED");
  }
}
}

// Associate callback IDs with the concrete player before its state is dispatched.
extern "C" void __imp__sub_82726770(PPCContext& __restrict ctx, uint8_t* base);
extern "C" void sub_82726770(PPCContext& __restrict ctx, uint8_t* base) {
  {
    std::lock_guard lock(g_movie_mutex);
    if (g_playback.Bind(ctx.r4.u32, ctx.r5.u32) && REXCVAR_GET(fnc_video_letterbox)) {
      REXLOG_INFO("[FNC-VIDEO] tracking player {:08X}, object {:08X}", ctx.r4.u32, ctx.r5.u32);
    }
  }
  __imp__sub_82726770(ctx, base);
}

// Removal can destroy a player without sending the normal stopped callback.
extern "C" void __imp__sub_82729890(PPCContext& __restrict ctx, uint8_t* base);
extern "C" void sub_82729890(PPCContext& __restrict ctx, uint8_t* base) {
  {
    std::lock_guard lock(g_movie_mutex);
    if (g_playback.ReleaseObject(ctx.r3.u32)) PublishState(ctx.r3.u32, "destroyed object");
  }
  __imp__sub_82729890(ctx, base);
}

extern "C" void __imp__sub_825DF420(PPCContext& __restrict ctx, uint8_t* base);
extern "C" void sub_825DF420(PPCContext& __restrict ctx, uint8_t* base) {
  // The movie-state bridge dispatches these keys on state 2 (playing) and 4
  // (stopped). Pausing does not send a stop notification.
  if (ctx.lr == fnc_movie::kNotificationReturn &&
      (ctx.r3.u32 == fnc_movie::kPlayingCallback || ctx.r3.u32 == fnc_movie::kStopCallback)) {
    std::lock_guard lock(g_movie_mutex);
    bool changed = g_playback.Observe(ctx.r3.u32, ctx.r5.u32);
    if (changed) PublishState(ctx.r5.u32, "player");
  }
  __imp__sub_825DF420(ctx, base);
}
