## 1. Parameter and processor integration

- [x] 1.1 Add the normalized `smooth` APVTS parameter with 0–100% display, default 50%, and the existing percentage parsing/formatting behavior.
- [x] 1.2 Pass the raw Smooth value from the processor to the Living Freeze engine and retain compatible APVTS state serialization/restoration.
- [x] 1.3 Add parameter-contract tests for Smooth range, default, formatting, automation updates, and saved-state restoration.

## 2. Smooth texture DSP

- [x] 2.1 Extend prepared engine state and processing APIs to smooth the Smooth target without callback allocations, locks, or non-finite output.
- [x] 2.2 Implement a Smooth-dependent sine-derived voice window that preserves non-negative continuous zero endpoints and current behavior at Smooth 0.
- [x] 2.3 Couple high Smooth to unobtrusive safe-boundary restarts and a slower, bounded texture-gain-compensation response while retaining all safe read/start logic.
- [x] 2.4 Add and tune a lightweight wet-only transient reducer if endpoint rendering shows that window and gain behavior alone do not achieve measurable attack reduction.
- [x] 2.5 Add deterministic DSP tests for endpoint window continuity, Smooth-zero baseline behavior, high-Smooth transient reduction, gain stability, Smooth automation continuity, long no-click renders, and high-Smooth/high-Drift independence.

## 3. Editor control and layout

- [x] 3.1 Add the `SMOOTH` slider, label, and APVTS attachment using the existing rotary look-and-feel.
- [x] 3.2 Reflow the fixed-size editor to four equal rotary-control bounds while keeping Freeze, labels, Output Gain, and the independently bounded CLIP indicator fully visible.
- [x] 3.3 Add or update UI-focused tests as supported by the existing test harness to verify parameter attachment and equal control sizing.

## 4. Verification and listening checks

- [x] 4.1 Build and run the JUCE unit-test suite; resolve all Smooth and existing regression failures.
- [x] 4.2 Manually audition transient-rich chord/pluck and percussion captures at Smooth 0%, 50%, and 100%, confirming the intended rhythmic-to-pad character continuum.
- [x] 4.3 Manually audition soft material and the Drift/Smooth endpoint combinations to confirm Smooth does not unnecessarily dull the texture or suppress motion.
