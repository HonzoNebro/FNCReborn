# Display development

## Frame-rate choices

DISPLAY offers 30, 60, 120, 144, 165, 180 and 240 FPS. These are timer targets,
not guaranteed rendering rates. Restart after changing them. The hook already
accepted 10–240 Hz; no simulation-timer changes were needed for these choices.
There is no unlimited mode: zero falls back to the original timer behavior.

Keep the C++ row values and both Rust/Python Apt label lists in sync. The
installer now checks for all current labels before skipping a patched archive,
so an existing four-option menu is rebuilt from `fe.big.orig`. The original
backup remains untouched. Do not distribute the patched game archive.

Local verification: game and installer release builds passed; old-menu upgrade
and second-run idempotence passed on a local archive; Rust and Python produced
identical updated archives. Visual menu navigation, selecting the three new
values, persistence after restart and actual timing still require game tests.

## Ultrawide investigation

The initial monitor is 3440×1440 at 165 Hz (43:18). The SDK's default guest
mode is 1280×720 (16:9). Resolution scaling changes pixel density separately.
Both renderer backends use the configured guest video-mode dimensions as the
presenter's display aspect ratio. Changing those dimensions can therefore
remove side bars even if the game's projection remains unchanged: full-screen
coverage alone is not proof of correct ultrawide rendering.

The existing camera hook identifies aspect at camera +0x08 and FOV at +0x04.
Optional `--fnc_log_camera_aspect` logs these fields every 300 camera updates,
including the normal TV camera. It requires the existing `fnc_fp_enable` option
to remain enabled. It does not alter projection or camera position, and is off
by default. Field interpretation and any resulting projection change still need
runtime validation.

Compare the same fighters, arena, camera and scene using these command-line
overrides on the existing Proton launch command (after the exe):

```text
16:9: --video_mode_width=1280 --video_mode_height=720 --resolution_scale=1 --fnc_frame_rate=60 --fnc_log_camera_aspect
Wide: --video_mode_width=1720 --video_mode_height=720 --resolution_scale=1 --fnc_frame_rate=60 --fnc_log_camera_aspect
```

1720×720 matches 3440×1440's ratio at half its dimensions. These are diagnostic
requests, not a supported ultrawide preset or a promise about the internal render
target size. Copy each `FNCReborn.log` before the next launch. Command-line
overrides keep the saved TOML intact unless the corresponding setting is changed
and saved from the in-game menu during the experiment.

Check camera aspect in the logs, visible arena width, fighter proportions,
vertical framing, HUD, menus, cutscenes and videos. Desired behavior is extra
horizontal view without stretched fighters or cropped vertical content. The logged field remained 0.5625 in both local runs, consistent with the
inverse aspect (height/width = 9/16). A 43:18 inverse aspect would be about
0.418605. Investigate the game projection before adding any permanent
widescreen option. Even a
changed aspect field must be confirmed visually; it may not drive every camera.

The optional projection experiment below is disabled by default. It must be
tested separately for TV camera, player camera, replays and Champion cutscenes,
with HUD/video aspect handled independently.

## First runtime comparison (2026-10-30)

The user confirmed all seven FPS menu choices are visible; the new rates were
not selected. The reference and wide tests produced 32 and 31 camera samples
respectively, all with aspect field 0.5625. Wide combat ran normally, but videos
also widened and slight fighter distortion was possible. This supports a
presentation stretch rather than a verified wider projection. No production
ultrawide preset should be enabled on that evidence.

The reference run failed during combat: the kernel recorded an AMDGPU GPU
page fault in FNCReborn.exe / vkd3d_queue at 23:10:44, then a graphics ring
timeout and successful ring reset at 23:10:46. The application reported
DXGI_ERROR_DEVICE_REMOVED (0x887A0005). The wide run ended via normal window
closure. This identifies a GPU failure path, not its root cause; a single pair
of runs does not establish a relationship to aspect settings. Preserve both
logs before repeating tests. Do not share GPU coredumps containing game data.

## Experimental horizontal expansion

`fnc_widescreen_aspect` defaults to zero (off). Set it to the desired W/H ratio,
for example `2.388888888888889` for 3440×1440. It requires `fnc_fp_enable` and
the wider guest presentation mode from the earlier experiment.

Inspection of the supported executable's projection builder established that
the camera stores horizontal FOV in radians and H/W. The original hook writes
only those camera fields after the director update, using:

```text
target H/W = 1 / target aspect
new horizontal FOV = 2 atan(tan(original horizontal FOV / 2) × original H/W / target H/W)
```

This preserves vertical field of view and expands horizontal coverage. It only
accepts original camera H/W near 9/16, finite FOVs away from zero/pi and target
W/H greater than 16/9 and at most 4. Other camera formats are left alone.
The existing camera snapshot is restored before the next director update,
including when player-camera restoration was disabled, to avoid accumulating
the expansion through smoothing or across frames.

`[FNC-ASPECT]` logs the source camera before modification. `[FNC-WIDE]` logs
the applied inverse aspect and horizontal FOV. Expect H/W around 0.418605 for
this monitor. The ordinary TV camera and the player-camera branch are covered;
the latter needs valid boxer tracking before its override can run.

The local test helper supports `reference`, `wide` (presentation only) and
`corrected` (presentation plus projection). It explicitly disables the
projection correction for the first two modes and writes separate logs.
For the next comparison:

```sh
bash build/reports/test-ultrawide.sh corrected
```

The game was rebuilt and the pure C++ projection tests passed. The tests cover
vertical-view preservation at several FOVs and aspect ratios, disabled/16:9
behavior, non-standard source cameras and invalid inputs. They do not validate
the hook in running gameplay, culling, framing, HUD or videos. Run them without
game data using:

```sh
g++ -std=c++17 -Wall -Wextra -Werror project/tests/widescreen_test.cpp -o /tmp/fnc-widescreen-test
/tmp/fnc-widescreen-test
```

This prototype does not correct 2D HUD or pre-rendered video proportions; those
still follow the presentation-only experiment. Do not describe it as complete
ultrawide support. Check fighter proportions, newly visible arena sides,
unchanged vertical framing, edge culling, camera switches, knockdowns/replays,
pause/resume and Champion cutscenes before adopting it. Preserve logs on any
GPU failure and keep the known-working normal launch as the fallback.

### First corrected gameplay run

The user reports full monitor coverage and improved fighter proportions,
consistent with the intended correction. The recorded camera values confirm
source H/W 0.5625 restored before each sampled update and applied H/W
0.41860464. Sampled FOV pairs preserve the vertical tangent within 1e-5.
The session ended by normal window closure without DEVICE_REMOVED in its log.
This is preliminary single-machine gameplay evidence, not complete validation
of HUD/video proportions, edge culling, every camera, replays or Champion mode.
