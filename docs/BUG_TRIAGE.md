# Initial bug triage

Source reference: FNCReborn `7f68ee3`, patched ReXGlue `c94f5eb`.
These are investigation notes, not verified root causes or fixes.

## Fight speed (#2)

The repository hook changes the 30 Hz frame timer only when its caller returns
to `0x822A9594`. The separate 60 Hz fight timer returns to `0x822AA580` and
is intentionally untouched in the reference source.

[titooo7's candidate](https://github.com/whompay/FNCReborn/issues/2#issuecomment-6322016689)
changes the fight timer mode and clears a cached-step flag in `sub_822AA690`.
It introduces `fnc_sim_timer_mode` and `fnc_sim_timer_cap`, plus a speed readout.
Review and preserve attribution before adapting it.

Checks required before adoption:

- Validate the object's offsets and timer-mode interpretation against the
  supported executable, including each caller of the new hook.
- Use byte-safe loads/stores for guest memory; the published patch casts guest
  bytes to `uint64_t*` and `uint32_t*`. Verify alignment, address translation,
  bounds and byte order rather than relying on host behavior.
- Confirm the first update, pause/resume, slow motion intentionally requested
  by the game, replay, scene change and long stalls preserve intended time.
- Compare fixed and real-time modes at stable 30/60 FPS and under frame drops;
  correct simulation speed does not imply better rendering performance.
- The reported cap of four 1/60-second periods covers roughly 15 FPS. Below
  that, slowdown is expected; do not remove the cap without physics validation.
- An atomic accumulated-step overlay is diagnostic only: also measure wall
  time and visible gameplay against the same reference round settings.

No experimental timer behavior is enabled by this development branch yet.

## Legacy crash (#8)

The report enters `xeKeSetEvent` and then `XEvent::Set`. The pinned SDK already
checks whether `GetNativeObject<XEvent>` returned an object before calling
`Set`. Adding the same null check would not establish a fix.

`XEvent::Set` dereferences its host `event_`. `GetNativeObject` retrieves
previously initialized objects by a handle stored in the guest dispatch
header, and contains a TODO to verify the retrieved object's type. Investigate
the handle, actual object type, initialization and lifetime at the failing call.
Do not infer a null host event or a type mismatch from the stack trace alone.
Compare with `fnc_sysheap_pool` enabled/disabled only as a controlled experiment;
there is no evidence yet that the pool causes the crash.

## Rendering and transitions

- [#9](https://github.com/whompay/FNCReborn/issues/9): capture the first round
  transition and check whether guest, GPU and audio threads continue making
  progress. A frozen image alone does not distinguish a deadlock from missing
  presentation.
- [#16](https://github.com/whompay/FNCReborn/issues/16),
  [#10](https://github.com/whompay/FNCReborn/issues/10),
  [#7](https://github.com/whompay/FNCReborn/issues/7): reproduce final Champion
  fight and knockout replays separately. Prior FMA and EDRAM changes are useful
  context, not proof of the current failure mechanism.
- [#13](https://github.com/whompay/FNCReborn/issues/13): reported black fighters
  and ring with Vulkan must be compared with the same D3D12 scene. Do not
  generalize an Adreno-specific driver workaround to the AMD test machine.
