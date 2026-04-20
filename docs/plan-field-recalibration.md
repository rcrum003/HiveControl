# Implementation Plan: Field Recalibration (Scale Under Load)

## The Problem

Once a beehive is on the scale, the beekeeper can't remove it to re-zero. But scale accuracy drifts over time (temperature, settling, sensor aging). The keeper needs to verify and adjust calibration by placing a known weight **on top of** the loaded hive.

## The Math

This is a differential calibration — we can only adjust the slope (scale factor), not the intercept (zero point), because we can't observe the zero state.

```
1. Read raw value with hive on scale:           raw_before
2. Place known weight ON TOP of hive, read:     raw_after  
3. raw_diff = raw_after - raw_before
4. new_slope = raw_diff / known_weight          (HX711)
   new_slope = known_weight / raw_diff          (Phidget)
```

The intercept stays unchanged. Only the slope is recalibrated.

## UI Flow (2-step inline wizard, same pattern as initial calibration)

1. **"Recalibrate Under Load"** button appears alongside the existing "Calibrate Scale" button in step 3
2. **Step 1 (Baseline):** "Leave the hive on the scale as-is. Click to take a baseline reading." -> Take Baseline Reading
3. **Step 2 (Add Weight):** "Place a known weight on top of the hive (e.g. a 10-lb bag of sugar). Enter the weight, then click to take a loaded reading." -> Shows the new slope, comparison to old slope, and a verification showing the delta reads correctly
4. **Save** writes the new slope to the database (intercept untouched)

## Implementation

### Changes to existing files only (no new files needed):

1. **`www/public_html/admin/setup-wizard.php`** — Add "Recalibrate Under Load" button and a `FieldCal` JS object (same pattern as `CalWiz`) with the 2-step flow. Reuses `calibrate_raw_reading.php` for raw readings and `calibrate_save.php` for saving (save will send the existing intercept + new slope).

2. **`www/public_html/admin/calibrate_save.php`** — No changes needed. The existing endpoint already accepts both slope and intercept values — field cal will just pass the existing intercept back along with the new slope.

No new backend scripts, endpoints, or sudoers changes needed — we reuse everything from the initial calibration wizard.

## Risks

| Risk | Severity | Mitigation |
|------|----------|------------|
| Hive weight shifts during readings (bees moving, wind) | Medium | Take readings a few seconds apart, warn user to wait for stable reading |
| Known weight too light relative to hive (poor signal-to-noise) | Medium | Recommend minimum 10 lbs, warn if diff is very small |
| Baseline and loaded readings are identical | Low | Detect and show "no weight change detected" error |

## Estimated Complexity: LOW
- ~1-2 hours: JS object + HTML within the existing wizard step
- Reuses all existing infrastructure
