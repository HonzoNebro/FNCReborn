# Experimental movie presentation

The existing ultrawide camera correction does not change pre-rendered VP6
movies. The local disc's 45 movies have 1280×720 video. Their complete image
should fit 16:9, with side bars on the 3440×1440 monitor.

## Detection and presentation

The supported executable's movie-state bridge sends `_MoviePlayingCallback`
on state 2 and `_MovieStopCallback` on state 4. An original hook observes only
that bridge's call site and the two known callback keys before forwarding the
original dispatch. No original game implementation is copied. Pausing does
not send a stop event through this bridge, so the active state is retained.

Active player IDs are tracked as a set: duplicate starts are idempotent, an
unknown stop cannot cancel another player, and presentation returns to normal
when the final active player stops. Updating before dispatch preserves event
order if a callback causes a nested stop or another start. Runtime validation
must confirm coverage of the game's different movie flows and cleanup paths.

The first gameplay test confirmed 16:9 video but exposed a missing-stop path:
after a second start, no stop notification arrived before combat. The tracker
now binds callback IDs to the concrete player objects in the state bridge and
also removes IDs when the movie-player destruction helper releases their
object. A delayed destruction of an old object cannot remove a reused ID
already bound to a new object. Tests cover destruction without stop, unknown
objects, multiple active objects and ID reuse. The next gameplay test must
establish whether this cleanup path resolves the observed retained state.

`fnc_video_letterbox` is off by default. `fnc_movie_active` is an internal
runtime flag driven by the hook; do not configure it manually. When both are
true, the D3D12 and Vulkan swap paths publish a normalized 16:9 display aspect
for that frame. The shared presenter recognizes enabled movie frames and
forces aspect fit without overscan cutoff, even if normal presentation has
letterboxing disabled. This uses the existing per-frame aspect properties,
so a queued movie frame can still fit after its player reports stopping.

The guest display mode, internal raster, camera configuration and source
video files are unchanged by the movie presentation switch. On 3440×1440,
uncropped 16:9 output should occupy 2560×1440 with 440-pixel side bars.
During gameplay the original configured presentation aspect is used again.
This does not distinguish multiple video panels inside a composed frame or
adjust in-engine cinematics, HUD geometry or embedded black bars in a video.

## Build and validation

Windows SDK and game builds passed. Playback-state tests passed with the host
C++ compiler. The Vulkan source path is updated but has not been compiled or
tested in this iteration. No native Windows OS gameplay test has been run.
An attempted Proton `--help` smoke test started the engine instead of printing
help and was terminated; it is not evidence that movie presentation works.

```sh
g++ -std=c++17 -Wall -Wextra -Werror project/tests/movie_playback_test.cpp -o /tmp/fnc-movie-test
/tmp/fnc-movie-test
```

The local runtime test is:

```sh
bash build/reports/test-movies.sh
```

It uses the existing dedicated Proton prefix, 2× scale, 60 FPS and the known
ultrawide camera correction, and enables the video experiment via CLI. It
writes `build/reports/movie-research/video-fit.log`; saved TOML defaults are
not changed by the script. First watch a complete pre-rendered introduction,
then enter a fight. Also test skipping a video and pausing/resuming if offered.

Expected logs are `[FNC-VIDEO] playback started`, `presentation movie fit`,
then `playback stopped` and `presentation game restored`. Verify complete,
undistorted video with side bars and full-width gameplay afterward. If there
are no transitions, report which video was used; do not enable a timeout or
file-open heuristic as a substitute. Preserve the log before repeating.

## Cleanup runtime result

The subsequent user test confirmed 16:9 video followed by ultrawide combat.
At 00:24:25 on 2026-10-31 the log records destruction of the active player and
restoration to 1720:720 presentation. The observed failure to restore from the
first implementation is therefore covered in this tested flow.

The same session later ended with DEVICE_REMOVED at 00:25:34 during combat.
Kernel logs identify a GPU memory protection fault in FNCReborn.exe/vkd3d_queue
at 00:25:32, followed by timeout and graphics-ring reset. A similar failure was
observed before the movie feature existed. This is not proof that current
changes are unrelated; the graphics failure still needs controlled reproduction.

The local helper's `control` argument disables only movie presentation fitting,
keeping the executable, Proton version, camera, guest video mode, 2× scale,
60 FPS and other settings the same. It logs to `video-fit-control.log`:

```sh
bash build/reports/test-movies.sh control
```

For comparison use the same fighters, arena and round settings. Stretched
videos are expected in this control. A successful single run does not establish
stability, and a failed one does not identify the underlying renderer/driver bug.
