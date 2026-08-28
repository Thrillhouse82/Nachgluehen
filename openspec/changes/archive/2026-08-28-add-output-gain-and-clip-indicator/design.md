## Context

The processor currently ends at the LivingFreezeEngine's Dry/Wet output and the editor has two rotary controls. See proposal.md for the motivation and the delta specifications for the required observable behavior.

## Goals / Non-Goals

**Goals:**

- Add a final, automatable output trim with a musically useful and safe range.
- Make potentially clipped final output visible without changing its sound.
- Keep gain smoothing and metering real-time safe and allocation-free.

**Non-Goals:**

- Adding a limiter, compressor, automatic gain compensation, or a second texture-level algorithm.
- Changing existing parameter identifiers or their behavior.
- Making the editor resizable or redesigning its visual identity.

## Decisions

### Bounded final Output Gain

Output Gain is an AudioProcessorValueTreeState parameter applied after the engine's Dry/Wet mix. Its default is 0 dB, its lowest endpoint mutes output, and its upper endpoint is +12 dB. A bounded boost gives enough practical make-up gain while avoiding meaningless infinite amplification and constraining accidental overload.

The stage uses a short linear or dB-domain smoothing response independent from texture gain compensation. It is not folded into Dry/Wet, so automation and saved projects retain a clear separation between blend and final level.

### Passive post-output clip warning

The processor compares the final post-gain samples against a -0.1 dBFS warning threshold. It does not alter samples: adding a limiter would change the effect's sound and is outside this change. The audio thread records a lock-free hold state, while the editor polls and renders it as a compact red `CLIP` label. The hold duration is at least one second so brief peaks are visible at the editor's normal refresh rate.

### Fixed-layout extension

The existing fixed editor layout gains a third rotary control and the status label beside it. The control and label reuse the existing custom look-and-feel rather than introducing a meter component or a resizable layout.

## Risks / Trade-offs

- [Positive Output Gain can drive the host signal beyond full scale] → The bounded +12 dB maximum and post-output warning make the condition visible while preserving the unaltered signal.
- [Rapid automation could become audible] → Smooth the final gain independently and cover it with continuity tests.
- [Brief peaks could be missed by editor polling] → Maintain the audio-thread warning state for at least one second.
- [Additional parameter affects saved project state] → Use the existing parameter state serialization and add restore tests.

## Migration Plan

Existing saved states omit the new parameter and therefore use its declared 0 dB default. Removing the output stage cleanly restores the prior signal path because 0 dB is unity gain.
