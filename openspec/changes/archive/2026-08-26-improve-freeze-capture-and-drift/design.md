## Context

The existing `LivingFreezeEngine` stores up to several seconds of recent stereo audio and currently captures all available history. Its read-position offset is only a few samples and its playback-speed variation is small, so the current Drift range is difficult to hear. Processing must remain allocation-free and non-blocking in the audio callback, and the three existing host parameters remain unchanged.

## Goals / Non-Goals

**Goals:**

- Capture a fixed approximately 600 ms stereo window ending at the Freeze edge, or the shorter available history during startup.
- Keep capture state immutable while frozen and produce a continuous, click-free loop.
- Make Drift 0% exactly stable and make positive Drift progressively more audible while remaining smooth, bounded, deterministic for a fixed seed, and non-periodic.
- Add measurable JUCE Unit Test coverage for the capture contract, continuity, and Drift scaling.

**Non-Goals:**

- No new parameters or changes to host automation, state serialization, UI, or MVP feature scope.
- No requirement to add multiple read heads unless validation shows the single-head design cannot meet the behavioral contract.

## Decisions

### Fixed bounded capture window

Use a named capture-duration constant of 0.6 seconds and derive its sample count from the prepared sample rate. At a Freeze rising edge, copy only the newest `min(availableSamples, captureSamples)` samples from the preallocated ring into the preallocated frozen buffers. The ring may remain larger for future extensibility, but capture length is no longer tied to ring capacity.

This preserves the existing ring-buffer approach and avoids allocations or locks. A configurable runtime capture parameter was considered and rejected because it expands the MVP and the host-facing contract.

### Boundary-safe continuous playback

Retain linear interpolation for fractional reads and use an overlap region at the captured window boundary. During the final crossfade span, blend the end read with the corresponding beginning read using a bounded equal-power or linear transition; ensure the span is clamped for short captures. Reset the playback cursor to the start of a newly captured fragment.

The crossfade is preferred over simply forcing a zero crossing because it works for arbitrary stereo material and keeps the output dense when the source has no silence. A hard modulo loop or a fade-to-zero was considered and rejected because both can expose repetition or create silence.

### Smoothed non-periodic Drift with one read head

Keep one read head initially. Generate a new signed random Drift target at irregular block/sample intervals using the existing seeded PRNG. Smooth the active modulation toward the target with a time-based ramp rather than jumping at update boundaries. Scale the modulation nonlinearly enough that low positive values remain subtle while the full range is clearly audible.

Express read-position movement in milliseconds and convert it to samples using the prepared sample rate. Use a maximum offset in the tens of milliseconds at 100% Drift, with opposite small channel offsets retained only if they do not create stereo discontinuities. Increase the maximum playback-speed deviation to a musically audible but safe bounded percentage at 100%; apply it continuously to the cursor increment. At Drift 0%, bypass both modulation paths and keep the cursor increment and read position stable.

A cyclic LFO was considered and rejected because it would make the texture predictably periodic. Multiple read heads were considered but deferred because a seeded irregular target process provides the required non-periodic motion with lower CPU and state complexity.

### Explicit preallocation and test observability

Prepare all ring and frozen buffers, smoothing state, and constants before processing. Do not resize vectors, allocate helper objects, or acquire synchronization primitives from `process`. Keep the existing finite-output guard. Tests may inspect captured length through the existing accessor and compare seeded engine outputs and continuity metrics; no new user-facing API is required.

## Risks / Trade-offs

- [Risk] A 600 ms window may omit a slow attack or release → capture the available preceding material during startup and keep the duration as a named internal constant for future adjustment.
- [Risk] Larger position and speed excursions can expose interpolation artifacts → use fractional linear reads, bounded modulation, and continuous ramps; assert finite output and sample-step continuity in tests.
- [Risk] A short fragment can still sound repetitive → use the boundary overlap and irregular Drift; revisit read-head count only if manual listening or measurements show the single-head approach is insufficient.
- [Risk] Changing capture length alters existing test assumptions → update tests to assert the new duration-based contract rather than total ring history.

## Migration Plan

No project or state migration is required. Implement the engine and JUCE Unit Test changes, run the existing test executable, then perform manual Ableton Live listening at Drift 0%, low, medium, and 100%. If a regression requires rollback, revert the engine/test change; serialized parameters and plugin identifiers remain compatible.
