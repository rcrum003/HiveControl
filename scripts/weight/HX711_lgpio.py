#!/usr/bin/env python3
"""
HX711 24-bit ADC driver using lgpio for Raspberry Pi 5 and later.

This is a reimplementation of HX711.py using lgpio instead of pigpio.
The pigpio version uses hardware waveform generation which isn't available
in lgpio, so this version uses software-timed GPIO operations.

For Pi 4 and earlier, use HX711.py with pigpio instead.

Based on HX711.py by joan@abyz.me.uk (Public Domain)
Modified for HiveControl lgpio compatibility.
"""

import time
import lgpio

CH_A_GAIN_64 = 0   # Channel A gain 64
CH_A_GAIN_128 = 1  # Channel A gain 128
CH_B_GAIN_32 = 2   # Channel B gain 32


class sensor:
    """
    A class to read the HX711 24-bit ADC using lgpio.
    """

    def __init__(self, chip, DATA, CLOCK, mode=CH_A_GAIN_128, callback=None):
        """
        Instantiate with the GPIO chip handle, data GPIO, and clock GPIO.

        Args:
            chip: lgpio chip handle from gpiochip_open()
            DATA: GPIO pin number for HX711 data line
            CLOCK: GPIO pin number for HX711 clock line
            mode: Channel and gain setting
                CH_A_GAIN_64  - Channel A gain 64
                CH_A_GAIN_128 - Channel A gain 128
                CH_B_GAIN_32  - Channel B gain 32
            callback: Optional callback function(count, mode, reading)
        """
        self._chip = chip
        self.DATA = DATA
        self.CLOCK = CLOCK
        self.callback = callback

        self._paused = True
        self._reading = None
        self._count = 0

        self.set_mode(mode)

        # Configure GPIO pins
        lgpio.gpio_claim_output(chip, CLOCK)
        lgpio.gpio_claim_input(chip, DATA)

        # Start with clock high to pause the HX711
        lgpio.gpio_write(chip, CLOCK, 1)

        self._valid_after = time.time() + 0.4

    def set_mode(self, mode):
        """
        Set the mode (channel and gain).

        Args:
            mode: CH_A_GAIN_64, CH_A_GAIN_128, or CH_B_GAIN_32
        """
        self._mode = mode

        if mode == CH_A_GAIN_128:
            self._pulses = 25
        elif mode == CH_B_GAIN_32:
            self._pulses = 26
        elif mode == CH_A_GAIN_64:
            self._pulses = 27
        else:
            raise ValueError(f"Invalid mode: {mode}")

        self._valid_after = time.time() + 0.4

    def get_mode(self):
        """Returns the current mode."""
        return self._mode

    def get_reading(self):
        """
        Returns the current count, mode, and reading.

        The count is incremented for each new reading.

        Returns:
            Tuple of (count, mode, reading)
        """
        return self._count, self._mode, self._reading

    def set_callback(self, callback):
        """
        Set the callback to be called for every new reading.

        Args:
            callback: Function(count, mode, reading) or None to disable
        """
        self.callback = callback

    def pause(self):
        """Pause readings by holding clock high."""
        self._paused = True
        lgpio.gpio_write(self._chip, self.CLOCK, 1)

    def start(self):
        """Start/resume readings by releasing clock."""
        lgpio.gpio_write(self._chip, self.CLOCK, 0)
        self._paused = False
        self._valid_after = time.time() + 0.4

    def _read_raw(self):
        """
        Read a raw 24-bit value from the HX711.

        This uses software timing which is less precise than pigpio's
        hardware waveforms, but works on Pi 5.

        Returns:
            24-bit signed reading, or None if timeout
        """
        chip = self._chip
        DATA = self.DATA
        CLOCK = self.CLOCK

        # Wait for DATA to go low (HX711 ready)
        timeout = time.time() + 1.0  # 1 second timeout
        while lgpio.gpio_read(chip, DATA) == 1:
            if time.time() > timeout:
                return None
            time.sleep(0.001)

        # Read 24 bits — no sleep needed; lgpio call overhead (~10-20μs)
        # provides sufficient pulse width without exceeding 60μs power-down limit
        value = 0
        for _ in range(24):
            lgpio.gpio_write(chip, CLOCK, 1)
            value = (value << 1) | lgpio.gpio_read(chip, DATA)
            lgpio.gpio_write(chip, CLOCK, 0)

        # Send additional pulses for gain/channel selection
        extra_pulses = self._pulses - 24
        for _ in range(extra_pulses):
            lgpio.gpio_write(chip, CLOCK, 1)
            lgpio.gpio_write(chip, CLOCK, 0)

        # Convert to signed 24-bit
        if value & 0x800000:
            value |= ~0xFFFFFF

        return value

    def read(self):
        """
        Perform a single reading.

        This is a blocking call that waits for the HX711 to be ready.

        Returns:
            Reading value or None if paused/error
        """
        if self._paused:
            return None

        if time.time() < self._valid_after:
            time.sleep(max(0, self._valid_after - time.time()))

        value = self._read_raw()

        if value is not None:
            self._reading = value
            self._count += 1
            if self.callback is not None:
                self.callback(self._count, self._mode, self._reading)

        return value

    def read_average(self, times=10):
        """
        Read multiple values and return the average.

        Args:
            times: Number of readings to average

        Returns:
            Average reading value
        """
        values = []
        for _ in range(times):
            val = self.read()
            if val is not None:
                values.append(val)
            time.sleep(0.01)

        if not values:
            return None

        return sum(values) / len(values)

    def read_trimmed_mean(self, times: int = 50, trim: float = 0.1) -> float | None:
        """
        Read multiple values, discard outliers, and return the mean.

        Sorts readings, removes the top and bottom `trim` fraction,
        and averages the remaining values. More robust than simple
        averaging against electrical noise spikes.

        Args:
            times: Number of readings to collect
            trim: Fraction to discard from each end (0.0 to <0.5)

        Returns:
            Trimmed mean reading value, or None if no valid readings
        """
        if not (0.0 <= trim < 0.5):
            raise ValueError(f"trim must be in [0.0, 0.5), got {trim}")

        values = []
        for _ in range(times):
            val = self.read()
            if val is not None:
                values.append(val)
            time.sleep(0.01)

        if not values:
            return None

        values.sort()
        cut = int(len(values) * trim)
        trimmed = values[cut:len(values) - cut] if cut > 0 else values

        if not trimmed:
            return None

        return sum(trimmed) / len(trimmed)

    def cancel(self):
        """
        Cancel the sensor and release resources.
        """
        self.pause()
        # Free the GPIO pins
        try:
            lgpio.gpio_free(self._chip, self.DATA)
        except Exception:
            pass
        try:
            lgpio.gpio_free(self._chip, self.CLOCK)
        except Exception:
            pass


if __name__ == "__main__":

    chip = lgpio.gpiochip_open(0)
    if chip < 0:
        print("Failed to open GPIO chip")
        exit(1)

    try:
        s = sensor(chip, DATA=23, CLOCK=24, mode=CH_A_GAIN_128)
        s.start()
        time.sleep(2)

        result = s.read_trimmed_mean(times=50, trim=0.1)
        if result is not None:
            print(int(result))
        else:
            print("No data to consider")

        s.cancel()

    finally:
        lgpio.gpiochip_close(chip)
