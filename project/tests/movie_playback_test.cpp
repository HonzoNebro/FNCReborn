#include "../src/movie_playback.h"
#include <cassert>

int main() {
  using namespace fnc_movie;
  Playback state;
  assert(!state.active());
  assert(!state.Observe(kStopCallback, 1));
  assert(!state.Observe(0, 1));
  assert(state.Observe(kPlayingCallback, 1));
  assert(state.active());
  assert(!state.Observe(kPlayingCallback, 1));  // Repeated notification.
  assert(!state.Observe(kPlayingCallback, 2));
  assert(!state.Observe(kStopCallback, 99));    // Unknown player cannot stop another.
  assert(!state.Observe(kStopCallback, 1));
  assert(state.active());                     // The second player is still active.
  assert(!state.Observe(0, 2));                // No stop event while paused.
  assert(state.Observe(kStopCallback, 2));
  assert(!state.active());
  assert(state.Observe(kPlayingCallback, 2));  // Replay after stopping.
  assert(state.Observe(kStopCallback, 2));
  assert(!state.active());

  // A player may be destroyed while still marked playing, with no stop callback.
  state.Bind(1, 100);
  assert(state.Observe(kPlayingCallback, 1));
  assert(!state.ReleaseObject(999));
  assert(state.active());
  assert(state.ReleaseObject(100));
  assert(!state.active());
  assert(!state.ReleaseObject(100));

  // Destroying one player must not clear another live player's state.
  state.Bind(1, 100);
  state.Bind(2, 200);
  assert(state.Observe(kPlayingCallback, 1));
  assert(!state.Observe(kPlayingCallback, 2));
  assert(!state.ReleaseObject(100));
  assert(state.active());
  assert(state.ReleaseObject(200));

  // Reusing an ID for a new object must survive delayed cleanup of the old one.
  state.Bind(1, 100);
  assert(state.Observe(kPlayingCallback, 1));
  state.Bind(1, 200);
  assert(!state.ReleaseObject(100));
  assert(state.active());
  assert(state.ReleaseObject(200));
  assert(!state.active());
}
