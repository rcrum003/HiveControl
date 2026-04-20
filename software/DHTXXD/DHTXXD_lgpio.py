#!/usr/bin/env python3
"""
DHT22/DHT11 sensor reader using lgpio.

Uses busy-wait polling with perf_counter_ns() for microsecond-precision
edge detection. Works on all Raspberry Pi models with modern kernels
(Bookworm/Trixie) where pigpio may not function correctly.

Output format matches original DHTXXD: status temperature humidity
  status: 0=success, 1=checksum, 2=bad data, 3=timeout

Usage:
    python3 DHTXXD_lgpio.py -g <bcm_gpio>
"""

import argparse
import lgpio
import sys
import time


def read_dht(gpio: int, retries: int = 5) -> tuple[int, float, float]:
    chip = lgpio.gpiochip_open(0)
    try:
        for attempt in range(retries):
            result = _single_read(chip, gpio)
            if result[0] == 0:
                return result
            lgpio.gpio_free(chip, gpio)
            time.sleep(2 + attempt)
        return result
    finally:
        try:
            lgpio.gpio_free(chip, gpio)
        except Exception:
            pass
        lgpio.gpiochip_close(chip)


def _single_read(chip: int, gpio: int) -> tuple[int, float, float]:
    lgpio.gpio_claim_output(chip, gpio, 1)
    time.sleep(0.1)

    lgpio.gpio_write(chip, gpio, 0)
    time.sleep(0.020)

    lgpio.gpio_write(chip, gpio, 1)
    lgpio.gpio_free(chip, gpio)
    lgpio.gpio_claim_input(chip, gpio, lgpio.SET_PULL_UP)

    edges: list[tuple[int, int]] = []
    t_start = time.perf_counter_ns()
    timeout_ns = 20_000_000
    prev = lgpio.gpio_read(chip, gpio)

    while True:
        val = lgpio.gpio_read(chip, gpio)
        now = time.perf_counter_ns() - t_start
        if now > timeout_ns:
            break
        if val != prev:
            edges.append((now, val))
            prev = val
            if len(edges) >= 86:
                break

    high_pulses: list[int] = []
    for i in range(len(edges) - 1):
        ts, level = edges[i]
        ts_next = edges[i + 1][0]
        if level == 1:
            high_pulses.append(ts_next - ts)

    if len(high_pulses) < 40:
        return (3, 0.0, 0.0)

    data_pulses = high_pulses[-40:]

    threshold = sum(data_pulses) / len(data_pulses)

    bits: list[int] = []
    for dur in data_pulses:
        bits.append(1 if dur > threshold else 0)

    byte_vals: list[int] = []
    for i in range(0, 40, 8):
        byte_val = 0
        for j in range(8):
            byte_val = (byte_val << 1) | bits[i + j]
        byte_vals.append(byte_val)

    checksum = (byte_vals[0] + byte_vals[1] + byte_vals[2] + byte_vals[3]) & 0xFF
    if checksum != byte_vals[4]:
        return (1, 0.0, 0.0)

    humidity = ((byte_vals[0] << 8) | byte_vals[1]) / 10.0
    temp_raw = ((byte_vals[2] & 0x7F) << 8) | byte_vals[3]
    temperature = temp_raw / 10.0
    if byte_vals[2] & 0x80:
        temperature = -temperature

    if not (-40 <= temperature <= 80 and 0 <= humidity <= 100):
        return (2, 0.0, 0.0)

    return (0, temperature, humidity)


def main() -> None:
    parser = argparse.ArgumentParser(description="Read DHT22/DHT11 via lgpio")
    parser.add_argument("-g", "--gpio", type=int, required=True, help="BCM GPIO pin")
    args = parser.parse_args()

    status, temp_c, humidity = read_dht(args.gpio)
    print(f"{status} {temp_c:.1f} {humidity:.1f}")
    sys.exit(0 if status == 0 else 1)


if __name__ == "__main__":
    main()
