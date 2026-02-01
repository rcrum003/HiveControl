#!/usr/bin/env python3
"""
Wrapper for pigpio library (Pi 4 and earlier).

Provides GPIO, I2C, and SPI access using the pigpio daemon.
Requires pigpiod to be running: sudo pigpiod
"""

import pigpio
from . import constants


class PigpioGPIO:
    """GPIO implementation using pigpio for Pi 4 and earlier."""

    def __init__(self):
        """Initialize connection to pigpio daemon."""
        self._pi = pigpio.pi()
        # Map our unified constants to pigpio constants
        self._mode_map = {
            constants.INPUT: pigpio.INPUT,
            constants.OUTPUT: pigpio.OUTPUT,
        }
        self._edge_map = {
            constants.RISING_EDGE: pigpio.RISING_EDGE,
            constants.FALLING_EDGE: pigpio.FALLING_EDGE,
            constants.EITHER_EDGE: pigpio.EITHER_EDGE,
        }

    @property
    def pi(self):
        """Access underlying pigpio.pi() instance for advanced operations."""
        return self._pi

    @property
    def connected(self) -> bool:
        """Check if connected to pigpio daemon."""
        return self._pi.connected

    def set_mode(self, gpio: int, mode: int) -> None:
        """Set pin mode (INPUT/OUTPUT)."""
        self._pi.set_mode(gpio, self._mode_map.get(mode, mode))

    def read(self, gpio: int) -> int:
        """Read pin state (0 or 1)."""
        return self._pi.read(gpio)

    def write(self, gpio: int, level: int) -> None:
        """Write pin state (0 or 1)."""
        self._pi.write(gpio, level)

    def callback(self, gpio: int, edge: int, func):
        """
        Register edge detection callback.

        Args:
            gpio: GPIO pin number
            edge: Edge type (RISING_EDGE, FALLING_EDGE, EITHER_EDGE)
            func: Callback function(gpio, level, tick)

        Returns:
            pigpio callback object (call .cancel() to remove)
        """
        return self._pi.callback(gpio, self._edge_map.get(edge, edge), func)

    def set_watchdog(self, gpio: int, timeout_ms: int) -> None:
        """
        Set watchdog timeout for pin.

        Args:
            gpio: GPIO pin number
            timeout_ms: Timeout in milliseconds (0 to disable)
        """
        self._pi.set_watchdog(gpio, timeout_ms)

    def stop(self) -> None:
        """Clean up and release resources."""
        if self._pi is not None:
            self._pi.stop()

    # Advanced pigpio-specific methods for HX711 waveform generation
    def wave_add_generic(self, pulses: list) -> None:
        """Add pulses to waveform."""
        self._pi.wave_add_generic(pulses)

    def wave_create(self) -> int:
        """Create waveform from added pulses, return wave ID."""
        return self._pi.wave_create()

    def wave_chain(self, chain: list) -> None:
        """Execute chain of waveforms."""
        self._pi.wave_chain(chain)

    def wave_delete(self, wave_id: int) -> None:
        """Delete a waveform."""
        self._pi.wave_delete(wave_id)

    def wave_tx_stop(self) -> None:
        """Stop any ongoing waveform transmission."""
        self._pi.wave_tx_stop()


class PigpioI2C:
    """I2C implementation using pigpio."""

    def __init__(self, pi_instance):
        """
        Initialize I2C using existing pigpio connection.

        Args:
            pi_instance: pigpio.pi() instance
        """
        self._pi = pi_instance

    def open(self, bus: int, address: int) -> int:
        """
        Open I2C connection.

        Args:
            bus: I2C bus number (usually 1)
            address: I2C device address

        Returns:
            Handle for subsequent operations
        """
        return self._pi.i2c_open(bus, address)

    def write_device(self, handle: int, data) -> None:
        """
        Write data to I2C device.

        Args:
            handle: Handle from open()
            data: Bytes or list to write
        """
        self._pi.i2c_write_device(handle, data)

    def read_block_data(self, handle: int, register: int, count: int) -> tuple:
        """
        Read block of data from register.

        Args:
            handle: Handle from open()
            register: Register address to read from
            count: Number of bytes to read

        Returns:
            Tuple of (count, data_list)
        """
        return self._pi.i2c_read_i2c_block_data(handle, register, count)

    def close(self, handle: int) -> None:
        """Close I2C connection."""
        self._pi.i2c_close(handle)


class PigpioSPI:
    """SPI implementation using pigpio."""

    def __init__(self, pi_instance):
        """
        Initialize SPI using existing pigpio connection.

        Args:
            pi_instance: pigpio.pi() instance
        """
        self._pi = pi_instance

    def open(self, channel: int, baud: int, flags: int = 0) -> int:
        """
        Open SPI connection.

        Args:
            channel: SPI channel (CE0=0, CE1=1, etc.)
            baud: Clock speed in Hz
            flags: Mode flags (default 0 for main SPI, mode 0)

        Returns:
            Handle for subsequent operations
        """
        return self._pi.spi_open(channel, baud, flags)

    def xfer(self, handle: int, data: list) -> tuple:
        """
        Transfer data over SPI (simultaneous read/write).

        Args:
            handle: Handle from open()
            data: List of bytes to send

        Returns:
            Tuple of (count, data_received)
        """
        return self._pi.spi_xfer(handle, data)

    def close(self, handle: int) -> None:
        """Close SPI connection."""
        self._pi.spi_close(handle)


# Export pigpio constants and pulse for HX711 compatibility
pulse = pigpio.pulse
OUTPUT = pigpio.OUTPUT
INPUT = pigpio.INPUT
EITHER_EDGE = pigpio.EITHER_EDGE
FALLING_EDGE = pigpio.FALLING_EDGE
RISING_EDGE = pigpio.RISING_EDGE
