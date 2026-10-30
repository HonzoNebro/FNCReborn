# Linux development and validation

## Reference

Start from FNCReborn `7f68ee3c1e8d9f68be05b672dfd5ede379fb5521`
and SDK `c94f5ebdcb3c9d1a460ca48e04f9758448f8d518`, with the repository's
SDK patch. Do not update the SDK while comparing a game-specific fix.
Linux Setup cross-compiles a **Windows executable**; this is not a native port.
The native SDK and recompiler built on the host do not change that distinction.

The supported `default.xex` SHA-1 is
`498796AF783F0D598BBDA166B6BC8D518EDF8428`. For an extracted copy, check:

```sh
sha1sum /path/to/your/game/default.xex
```

Use your own disc. Keep game data, generated C++, executables and memory dumps
local, following CONTRIBUTING.md. Reports below belong in ignored `build/`.

## Build from this checkout

Run these commands from the repository root. Replace the game path with your
own extracted folder. `--accept-license` accepts Microsoft's terms for the
downloaded C++ headers and libraries; review those terms before using it.

```sh
cargo build --locked --release -j 2 --manifest-path installer/Cargo.toml
FNC_BUILD_JOBS=2 ./installer/target/release/FNCReborn-Setup \
  --cli --dir "$PWD" --game-dir /path/to/your/game \
  --accept-license --no-download --no-integration
```

For an ISO, replace `--game-dir /path/to/your/game` with
`--iso /path/to/your/game.iso`. Once `dist/game` exists, omit the source option
to rebuild. Always retain `--no-download` for local changes: normal update
downloads upstream source. Setup preserves an existing `dist/FNCReborn.toml`.

The SDK and host recompiler can be built without game data:

```sh
FNC_BUILD_JOBS=2 ./installer/target/release/FNCReborn-Setup \
  --sdk-only "$PWD" --accept-license
```

The SDK patch preserves explicit SDL backend cache options. Without this fix,
the pinned SDK forces Linux window/audio backends on, overriding Setup's
console-only options and requiring desktop development dependencies even for
the host recompiler. The Windows runtime still uses its normal backends.
Setup also checks the SDK source tree's `out` directory for the host
recompiler: the pinned SDK writes executables there rather than in the CMake
build directory. Otherwise a successful Linux build ends with "The recompiler
was built but not found".

Validation on Nobara 44 (2026-10-30): the locked release build of Setup and
`FNC_BUILD_JOBS=2 ... --sdk-only` completed successfully, producing the Windows
SDK DLLs and a working Linux `rexglue --help`. Native Vulkan CMake configuration
also succeeded with the optional SDL integrations disabled as described below.
No game executable, gameplay or Windows-host execution was tested: disc data
was not available. Build success does not validate the experimental timer fix.

## Proton reference

Use a dedicated prefix and a fixed installed Proton version. Set `PROTONPATH`
to its absolute directory, then launch from `dist`:

```sh
mkdir -p build/reports/baseline
cp dist/FNCReborn.toml build/reports/baseline/FNCReborn.toml
cd dist
WINEPREFIX="$PWD/../build/proton-prefix" \
  GAMEID=fncreborn PROTONPATH=/absolute/path/to/Proton \
  umu-run ./FNCReborn.exe
```

Record the exact Proton/UMU versions, OS, kernel, GPU/driver, source/SDK
commits, build options, XEX hash and TOML for every comparison. Copy logs and
any crash report after each run, before another launch replaces them. Back up
saves before testing. A fresh prefix needs to initialize before load-time
measurements are comparable.

Test 30 and 60 FPS separately, changing only `fnc_frame_rate` initially.
Use the F1 counter for game swaps: an external overlay may count repeated
frames. Measure elapsed wall time and the in-game round clock as well; the
round clock need not advance one displayed second per real second. Compare
the same round settings with a reliable reference.

On the initial test machine, `vulkaninfo` reported an invalid `lsfg-vk`
configuration. A separate query with `VK_LOADER_LAYERS_DISABLE='~implicit~'`
succeeded and identified RX 9070 XT, RADV/Mesa 26.2.5 and Vulkan 1.4.354.
For a clean baseline, add that variable to the launch command to disable
implicit Vulkan layers for that process only. Record it in the report; it
also disables implicit overlay layers. No global layer settings need changing.

For each original/patched pair record:

| Scenario | Result to record |
| --- | --- |
| Menus and loading | Image, audio, input, warm/cold load time |
| Complete multi-round fight | Internal FPS, wall time, round-clock advance |
| Pause and resume | No catch-up jump, audio remains synchronized |
| Knockdown, replay, round transition | Image, fighters, camera, audio |
| Champion and Legacy | Progression, final fight, freezes/crashes |
| Save, exit and reload | Same profile and progression restored |

## Candidate fixes and native Linux

The timer patch in issue #2 is experimental and must be reviewed and tested
before adoption: https://github.com/whompay/FNCReborn/issues/2
Its reported correct speed is not a rendering-performance improvement.
Validate both original and experimental modes and check pause, scene changes,
replays and frame-rate recovery before selecting defaults.

Next reproduce #9 (round transition/audio), #8 (event lifetime crash), then
#16/#10/#7 (invisible fighters/replays). Similar symptoms do not establish a
shared cause. Keep each fix in a separate contribution.

For a native experiment, keep SDK/build/install/dist directories separate from
the Windows reference, enable the SDK's `REXGLUE_USE_VULKAN` option and use
`gpu_backend = "vulkan"`. A successful ELF build is not proof of playability:
issue #13 reports black fighters and ring. Compare the same scene against
D3D12 under Proton. Check shaders, render targets, synchronization and readback
using captured evidence. The project's FPS lookup and crash profiler currently
have Windows-specific implementations and need separate Linux validation.

Native support is ready only after full fights, sound, controller input and
save/reload pass without WINE. Windows compatibility must be tested separately.

Initial Nobara 44 configuration with Clang 19.1.5 succeeded after disabling
optional SDL integrations with `-DSDL_X11_XSCRNSAVER=OFF -DSDL_X11_XTEST=OFF`.
However, SDL detected only the dummy audio driver: install development headers
for a real audio backend before testing sound. Do not confuse successful
configuration with a usable native runtime.

Use a separate SDK source checkout for native runtime builds as well as a
separate CMake build directory. The pinned SDK writes binaries into its source
tree's `out/linux-amd64`, so sharing that checkout with Setup's headless host
recompiler build would mix artifacts despite separate CMake build directories.
