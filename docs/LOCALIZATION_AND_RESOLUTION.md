# Display and localization investigation

Reference: the supported local disc, FNCReborn 7f68ee3 and patched SDK c94f5eb.
Asset observations are an inventory, not confirmation that each language or
subtitle path works in gameplay. Do not distribute extracted game content.

## Resolution versus presentation

Three distinct quantities matter: guest render-target dimensions, the SDK's
integer draw-resolution scale, and the display aspect/output window. A wider
video mode and a corrected camera do not establish a wider internal raster.
The SDK can present a 16:9 texture over an ultrawide surface.

`resolution_scale` sets both drawing axes unless explicit axis overrides are
present. Doubling both axes multiplies pixel count by four; tripling multiplies
it by nine. Some resources and passes can remain unscaled. Final swap output
alone does not establish the dimensions of every 3D pass.

Conceptual dimensions at a given height (not yet measured game targets):

| Height | 16:9 | Monitor 43:18 | 4:3 |
| --- | --- | --- | --- |
| 720 | 1280×720 | 1720×720 | 960×720 |
| 1440 | 2560×1440 | 3440×1440 | 1920×1440 |
| 2160 | 3840×2160 | 5160×2160 | 2880×2160 |

The D3D12-only `fnc_log_resolution` diagnostic is off by default. When enabled,
`[FNC-RES]` logs changes in packet dimensions, unscaled source texture,
allocation dimensions, active sampled region, presentation dimensions and draw
scale. Allocation padding must not be mistaken for active image dimensions.
Test at least menu, combat and a video at 1×/2×/3× before assigning pixel labels
to the selector. The local helper supports:

```sh
bash build/reports/test-display-research.sh resolution 2
```

This requests the established ultrawide camera experiment at 2× and 60 FPS,
with temporary CLI overrides and a separate log. No default TOML is changed.

Once measured, a selector should show aspect and internal scale separately,
with actual dimensions only where verified. Proposed aspect choices are the
original format, 16:9, monitor aspect and 4:3. The current camera experiment
supports wider views only; 4:3 needs its own projection/framing implementation
and HUD/video validation. Fractional scales or arbitrary internal resolutions
are not established capabilities of the current integer-scaling path.

## Local language inventory

- `boot.big` contains eng_us, fre_fr and ger_de text databases, plus a generic
  language database. `fe.big` contains a language-select movie and language
  attribute/selection handlers. The boot flow and any remembered choice need
  runtime investigation before introducing a replacement selector.
- Local speech banks exist under `loc/eng` and `loc/fre`. English includes
  speech, trainer and announcer banks; French includes speech and trainer
  banks. Roles, coverage and fallback must be checked by listening in game.
- `spanish_stream.aems` exists outside the localized speech directories. Its
  name alone does not prove Spanish commentary or a Spanish game localization.
- The extracted disc has 72 binary `ST00` `.sub` files: 36 with a French suffix
  and 36 with a German suffix. No Spanish or English `.sub` files were found
  in the extracted FMV directory. This does not exclude text stored elsewhere.
- All 45 VP6 movie files report 1280×720 video. Only `boot_movie_eng.vp6`
  exposes an embedded audio stream to ffprobe; story-movie audio must therefore
  be investigated through the separate speech/movie audio resources as well.

The existing SDK exposes `user_language` through console configuration, but
`XGetLanguage` ignored it and always selected English through a fixed NTSC-U
region. The SDK patch now uses valid configured Xbox language IDs, defaulting
to English for invalid IDs. Default English behavior is unchanged. Relevant
IDs are English 1, German 3, French 4 and Spanish 5; an ID does not create
missing assets. Only present language candidates are offered by the local
test helper:

```sh
bash build/reports/test-display-research.sh language fre
bash build/reports/test-display-research.sh language ger
```

These tests request the console language; game-specific language selection,
saved preferences or fallback may still determine the actual result. Check
menus, commentary, trainer, announcer, movie audio and subtitles separately.
Do not claim the SDK fix alone enables full French/German localization.

## Subtitles and videos

Start by restoring or enabling existing subtitle paths and understanding the
ST00 timing/text format and language selection. No transcription is necessary
for existing caption files, although availability and coverage vary. Spanish
captions would require translation or other content work if absent. A new
overlay would additionally need movie playback timing, pause/seek handling,
font layout and subtitle availability policies. It is a separate feature,
not a simple toggle, and should not precede testing the native system.

Pre-rendered videos should retain their source 16:9 aspect and receive side
bars on ultrawide. Detecting active movie playback and separating its output
from gameplay presentation remains unimplemented. The camera correction does
not fix that. HUD proportions are currently acceptable to the user and are
not a reason to rescale all 2D draws globally.

## Validation status

The updated Windows SDK compiled and installed; the two runtime DLLs were
updated locally with backups. Resolution logging is disabled by default, and
the configured language still defaults to English. New language selections and
render-dimension logging require gameplay tests. No assets, textures, movie
files or saved game preferences were modified by this investigation.

## First resolution measurement

The 2× ultrawide run logged a single unchanged swap configuration throughout
the session: packet and unscaled source 1280×720, allocation and active sampled
image 2560×1440, presentation aspect 1720×720, draw scale 2×2. Thus the observed
active swap raster is 2560×1440, not 3440×1440: the camera correction and wide
presentation do not increase the base raster width to 1720. This does not
measure every intermediate pass. Other scales remain to be sampled.

The user reports correct combat and potentially zoomed/cropped video. The log
records normal window closure without DEVICE_REMOVED. Swap dimensions alone
do not establish whether the video is cropped, stretched, or framed differently
by an in-engine cinematic camera. Distinguish pre-rendered VP6 playback from
real-time cutscenes before applying a global movie fix. Subtitles remain untested.
