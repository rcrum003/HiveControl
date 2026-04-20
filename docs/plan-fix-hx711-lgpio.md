# Plan: Fix HX711 Reading on 64-bit OS (lgpio Driver)

## Problem

`HX711_lgpio.py` returns -1 (0xFFFFFF — all 24 data bits high) on a Raspberry Pi 3
running a 64-bit kernel with the lgpio GPIO library.

The same hardware reads correctly (raw value 252925) on the old 32-bit Raspbian
Buster with pigpio/pigpiod.

## Root Cause

**The HX711 enters power-down mode because the clock stays high too long.**

The HX711 datasheet specifies:
- Minimum clock-high pulse width: 0.1 μs
- **Maximum clock-high pulse width: 60 μs** — if CLOCK stays high longer, the
  HX711 enters power-down mode and the DATA output becomes undefined (floats high).

### Working code (pigpio HX711.py)
```python
pi.wave_add_generic([
    pigpio.pulse(1<<CLOCK, 0, 20),   # CLOCK high for exactly 20μs (DMA-timed)
    pigpio.pulse(0, 1<<CLOCK, 20)    # CLOCK low for exactly 20μs (DMA-timed)
])
```
- Uses **hardware DMA waveforms** — kernel-level, microsecond-precise timing
- All 25 clock pulses sent as a single atomic wave chain
- Data read via interrupt callbacks on CLOCK falling edge

### Broken code (lgpio HX711_lgpio.py)
```python
lgpio.gpio_write(chip, CLOCK, 1)      # ~10μs call overhead
time.sleep(0.000001)                   # intended 1μs, actual 50-100μs (kernel min)
value = (value << 1) | lgpio.gpio_read(chip, DATA)  # ~10μs
lgpio.gpio_write(chip, CLOCK, 0)      # CLOCK finally goes low
```
- Total CLOCK-high time: **~70-120μs** (exceeds the 60μs power-down threshold)
- `time.sleep()` on Linux has ~50-100μs minimum granularity (scheduler tick)
- Result: HX711 powers down on every bit → DATA floats high → all bits = 1 → -1

## Evidence

| Test | Old Server (32-bit) | New Server (64-bit) |
|------|-------------------|-------------------|
| OS | Raspbian Buster armv7l | Bookworm aarch64 |
| Kernel | 4.19.97-v7+ | 6.12.75-v8+ |
| GPIO lib | pigpio (DMA waveforms) | lgpio (software bit-bang) |
| pigpiod | v70, running, functional | v70, running, GPIO broken |
| HX711 DATA response | Goes low (ready) ✓ | Goes low (ready) ✓ |
| Raw reading | 252925 | -1 (0xFFFFFF) |
| Clock timing | Exactly 20μs (hardware) | ~70-120μs (software) |

## Fix Plan

### Phase 1: Remove `time.sleep()` — use Python overhead as timing

**Estimated effort: 30 minutes**

The simplest fix: remove the `time.sleep(0.000001)` calls entirely. The overhead
of `lgpio.gpio_write()` + `lgpio.gpio_read()` through Python/ctypes is ~5-20μs
per call, which:
- Exceeds the HX711 minimum (0.1μs) ✓
- Stays well under the 60μs power-down threshold ✓

Changes to `HX711_lgpio.py` `_read_raw()`:
```python
# Read 24 bits — NO sleep needed; lgpio call overhead (~10-20μs)
# provides sufficient pulse width without exceeding 60μs power-down limit
for _ in range(24):
    lgpio.gpio_write(chip, CLOCK, 1)
    value = (value << 1) | lgpio.gpio_read(chip, DATA)
    lgpio.gpio_write(chip, CLOCK, 0)

# Extra gain/channel pulses
for _ in range(extra_pulses):
    lgpio.gpio_write(chip, CLOCK, 1)
    lgpio.gpio_write(chip, CLOCK, 0)
```

**Risk**: If the Pi is under heavy CPU load and Python gets preempted mid-loop,
a single clock-high period could stretch beyond 60μs. Mitigation: retry logic
already exists in `getrawreading.sh` (up to 4 attempts).

### Phase 2 (if Phase 1 is flaky): C helper for bit-banging

**Estimated effort: 2-3 hours**

Write a small C program (`hx711_read.c`) that uses lgpio's C API directly for
the timing-critical 24-bit read. C eliminates Python interpreter overhead and
allows `nanosleep()` or busy-wait for precise sub-microsecond timing.

```c
// hx711_read.c — read raw HX711 value via lgpio C API
#include <lgpio.h>
#include <time.h>

static void busy_wait_us(int us) {
    struct timespec start, now;
    clock_gettime(CLOCK_MONOTONIC, &start);
    do {
        clock_gettime(CLOCK_MONOTONIC, &now);
    } while ((now.tv_nsec - start.tv_nsec + 
              (now.tv_sec - start.tv_sec) * 1000000000L) < us * 1000L);
}

int main(int argc, char *argv[]) {
    int data_pin = 23, clock_pin = 24;
    // ... open gpiochip, claim pins, read 24 bits with precise timing
    // print raw value to stdout
}
```

Call from `getrawreading.sh` or from Python via `subprocess`.

### Phase 3 (long-term, optional): Linux kernel IIO HX711 driver

**Estimated effort: 4-6 hours**

Linux has a built-in HX711 driver in the IIO subsystem (since kernel 4.16).
All timing handled in kernel space — the most reliable approach.

Setup:
1. Create device tree overlay specifying DATA and CLOCK GPIO pins
2. Load overlay at boot via `/boot/firmware/config.txt`
3. Read from `/sys/bus/iio/devices/iio:deviceX/in_voltage0_raw`

This eliminates all userspace timing concerns permanently. Would need a new
Python wrapper that reads from sysfs instead of bit-banging GPIO.

## Recommendation

**Start with Phase 1.** It's the smallest change (delete 4 lines) and should work
for the Pi 3's workload. The retry logic in `getrawreading.sh` handles the rare
case where kernel preemption causes a timing glitch.

If Phase 1 produces intermittent -1 readings under load, move to Phase 2 (C helper).

Phase 3 (kernel IIO driver) is the ideal long-term solution but requires device
tree overlay setup and testing across Pi 3/4/5 models.

## Files to Modify

| File | Change |
|------|--------|
| `scripts/weight/HX711_lgpio.py` | Remove `time.sleep()` calls in `_read_raw()` |
| `scripts/weight/getrawreading.sh` | No changes needed (retry logic already present) |
| `scripts/weight/hx711.sh` | No changes needed (calls correct driver based on arch) |

## Testing

1. Boot test server (192.168.110.240) with new 64-bit OS
2. Run `sudo python3 /home/HiveControl/scripts/weight/HX711_lgpio.py` directly
3. Verify raw reading is a valid number (not -1), close to 252925 (the known good value)
4. Run 10 consecutive readings to check for intermittent failures
5. Test via the wizard calibration flow (AJAX → getrawreading.sh → HX711_lgpio.py)
6. If any readings are -1, proceed to Phase 2
