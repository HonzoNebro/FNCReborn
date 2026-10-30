# Session checkpoint — 2026-10-31

Base: FNCReborn 7f68ee3, SDK c94f5eb, supported XEX verified locally.
Target machine: Nobara x86-64, RX 9070 XT/RADV, 3440×1440 at 165 Hz.

## Implemented and observed

- Linux setup: explicit SDL options respected, native host recompiler located
  in the correct output directory, configurable parallel build limit.
- FPS menu: 30/60/120/144/165/180/240, including upgrade of older DISPLAY tabs.
  Labels confirmed in game; selection/persistence of every new rate is pending.
- Experimental ultrawide camera: horizontal expansion with vertical FOV
  preserved. Math tests and positive local gameplay observations recorded.
- Experimental movie fit: 16:9 video and return to ultrawide combat confirmed
  after tracking player destruction as well as stop callbacks.
- Console-language query respects the existing configured language ID.
  Compilation passed; actual language/subtitle behavior remains untested.
- Resolution logging established that 2× has a 2560×1440 active swap raster,
  presented wide; it is not a 3440×1440 internal raster.

## Unresolved GPU failure

DEVICE_REMOVED / 0x887A0005 occurred during combat. Kernel logs for observed
failures show GPU page faults in FNCReborn.exe/vkd3d_queue followed by ring
timeouts and resets. It also recurred in the same-settings control with movie
presentation fitting disabled. This rules out treating movie fitting as a
necessary trigger, but does not identify the bug or clear all current changes.
A similar failure predates the movie feature. No crash fix is claimed.

Next: reproduce with the upstream game/runtime baseline and the same fight,
settings and Proton version, then isolate renderer/vkd3d/driver behavior with
appropriate diagnostics. Preserve existing local logs. Do not publish game or
GPU dumps. Do not add further visual changes while isolating this regression.

## Publication and follow-up

Linux setup fixes and FPS menu choices are separate draft contributions. This
checkpoint preserves the broader experiments for continued work, not as a
stable Linux release. Native Windows gameplay and native Linux/Vulkan remain
unverified. The next work session should start from this checkpoint and read
CONTRIBUTION_STATUS.md, MOVIE_PRESENTATION.md and the local test reports.
