# Contribution readiness

The game executable and runtime being tested under Proton are Windows PE
binaries. Changes to portable game hooks, menu assets and shared SDK logic
are intended to apply on Windows as well. Proton testing is not native Windows
OS validation. Linux launch scripts and host toolchain fixes have a different
platform scope.

| Change | Current evidence | Before declaring ready |
| --- | --- | --- |
| Linux setup / SDL options / host recompiler lookup | Installer and SDK-only build completed | Recheck clean setup and isolate the generic SDK change |
| FPS choices through 240 | Build, archive upgrade/idempotence, matching Rust/Python output, user saw all labels | Select new values, restart and measure; native Windows test |
| Wide 3D camera | Math tests, recorded camera values, positive user gameplay test | Edge culling, camera switches, replays, Champion scenes and native Windows |
| Console language query | SDK built, local asset inventory | In-game localization, subtitles and fallback tests |
| Resolution diagnostics | 2× session recorded a 2560×1440 active raster | Sample other scales and distinguish intermediate passes |
| Movie aspect fit | Playback/cleanup tests, Windows builds, user confirmed 16:9 video and restored ultrawide combat | More movie flows, skip/pause and native Windows; GPU crash unresolved |

Keep setup fixes, FPS choices, experimental camera/video work and language
changes in separate reviewable contributions. Do not present diagnostics or
single-machine experiments as universal bug fixes. Generic SDK corrections
can be proposed upstream separately from Fight Night-specific policy.

After these changes are reviewed, triage the repository issues using exact
reproductions and separate branches. Initial candidates remain round-transition
freezes/audio (#9), Legacy event crash (#8), invisible fighters (#16/#10),
replays (#7), and FPS limiter behavior (#11). Re-read current issue discussions
and PRs before starting each one to avoid duplicating ongoing work. The local
GPU page-fault/reset is a separate observation until its relationship to an
upstream issue is established.

Only original source, patches, configuration, tests and documentation belong
in contributions. Local disc data, patched archives, generated C++, built game
binaries, image dumps and GPU dumps remain local under the repository rules.

The final same-settings control also crashed with movie fitting disabled. See
SESSION_CHECKPOINT.md; no graphics stability fix is claimed.
