# Blower Detection — Threshold Tuning Guide

This sketch (`ESP_NOW_Blower_MPU6050.ino`) detects furnace blower ON/OFF
state from an MPU-6050 mounted on the blower housing, using the variance
of the accelerometer vector magnitude over a rolling sample window. It's
included here as a **reference/aid for tuning your own thresholds** —
the numeric values baked into this copy were measured on one specific
furnace and mount, and will almost certainly need to be re-measured for
yours.

## Why thresholds are per-installation

Variance readings depend on:

- **Mounting rigidity** — a loose or shifting sensor (e.g. duct tape
  starting to let go) introduces its own vibration/noise that has
  nothing to do with the blower, and will shift your baseline
  unpredictably. Mount the sensor as rigidly as you can before
  collecting tuning data, and re-tune if the mount changes.
- **Blower/furnace type** — direct-drive vs. belt-drive blowers,
  variable-speed vs. single-speed motors, and enclosure material all
  change the vibration signature and its magnitude.
- **Sensor orientation and placement** — where exactly on the housing
  it's mounted affects how strongly it picks up the blower's vibration
  vs. ambient building noise.

Because of this, treat every constant below as a **starting point to
verify on your own bench**, not a value to copy blindly.

## The four tunable constants

```cpp
const float ON_THRESHOLD    = 7000.0f;   // must rise above this to start counting toward ON
const float OFF_THRESHOLD   = 5000.0f;   // must drop below this to start counting toward OFF
const float SHOCK_CEILING   = 120000.0f; // sustained readings at/above this are treated as
                                          // shock/impact noise (stomping, drops), not blower
const int   ON_CONFIRM      = 3;         // consecutive over-threshold cycles before declaring ON
const int   OFF_CONFIRM     = 5;         // consecutive under-threshold cycles before declaring OFF
```

- **`ON_THRESHOLD` / `OFF_THRESHOLD`** form a hysteresis pair. Once the
  blower is OFF, variance must rise above `ON_THRESHOLD` to start
  counting toward ON; once ON, it must drop below `OFF_THRESHOLD` to
  start counting toward OFF. Readings between the two thresholds don't
  move either counter. The gap between them absorbs noise that would
  otherwise cause rapid ON/OFF/ON chatter if a single cutoff sat right
  at the boundary.
- **`SHOCK_CEILING`** excludes sustained high-variance events — a
  footstep, a stomp, something dropped near the sensor — from ever
  being counted as blower-ON, since those events can register far
  higher than the blower itself, and would otherwise satisfy
  `ON_CONFIRM` if they lasted long enough.
- **`ON_CONFIRM` / `OFF_CONFIRM`** are cycle counts, not seconds. One
  cycle = `SAMPLE_COUNT * SAMPLE_DELAY_MS` (default 64 samples × 5ms ≈
  320ms). Increase either if you see spurious transitions; a clean
  mechanical stop can usually use a low `OFF_CONFIRM`, while `ON_CONFIRM`
  may need to be a bit higher if your blower's spin-up is gradual
  rather than an abrupt jump in variance.

## How to collect your own numbers

1. Mount the sensor as rigidly as your install allows.
2. Flash this sketch as-is (the default thresholds are a reasonable
   starting point) and open the serial monitor, or poll `/status` over
   the network if WiFi is connected — both print live variance plus
   the current threshold/confirm values.
3. Log variance in each of these conditions for at least a minute
   each:
   - Blower fully OFF, ambient/quiet
   - Any nearby but unrelated vibration sources you expect in normal
     use (appliances, footsteps near the unit, etc.)
   - Blower ON, each speed/mode it runs at (e.g. heating fan-only vs.
     cooling, if your system has both)
   - A deliberate stomp or knock near the sensor, to see how high a
     real shock event reads on your hardware
4. Look for a **gap** between your highest OFF/ambient readings and
   your lowest ON readings. Put `OFF_THRESHOLD` a bit above the OFF
   ceiling and `ON_THRESHOLD` a bit below the ON floor, leaving margin
   on both sides.
5. Set `SHOCK_CEILING` above your highest legitimate ON reading but
   below your lowest observed shock reading. If those two ranges
   overlap on your hardware, the ceiling can't fully separate them —
   you may need a different mitigation (e.g. a longer `ON_CONFIRM`,
   since a stomp is usually much shorter than a real blower run).
6. Watch the serial log across a couple of real ON/OFF cycles once
   your furnace is actually running, and adjust `ON_CONFIRM` /
   `OFF_CONFIRM` if you see chatter or slow confirmation.

## Reference data (this install only)

Collected 07/01/2026, mounted furnace, not a general-purpose baseline:

| Condition | Variance range |
|---|---|
| OFF / quiet ambient | ~1,500 – 4,000 |
| Disposal running (10–15 ft) | ~2,000 – 4,600 (barely registers) |
| Footstep/stomp shocks | 130,000+ sustained |
| Blower ON, heating fan-only | ~4,500 – 9,500 |
| Blower ON, cooling mode | ~44,000 – 114,000 (higher fan speed) |

This is included only to show the *shape* of a working separation
(quiet vs. ON vs. shock) — not as a target to match on different
hardware.
