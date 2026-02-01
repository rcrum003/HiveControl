#!/usr/bin/env python3
"""
Wrapper for lgpio library (Pi 5 and later).

Provides GPIO access using lgpio, and I2C/SPI via smbus2/spidev.
lgpio is the recommended library for Raspberry Pi 5's RP1 chip.

Install dependencies:
    sudo apt install python3-lgpio
    pip3 install smbus2 spidev
"""

import lgpio
from . import constants


class LgpioGPIO:
    """GPIO implementation using lgpio for Pi 5 and later."""

    def __init__(self, chip: int = 0):
        """
        Initialize GPIO chip.

        Args:
            chip: GPIO chip number (default 0 for /dev/gpiochip0)
        """
        self._chip = lgpio.gpiochip_open(chip)
        self._connected = self._chip >= 0
        self._callbacks = {}
        self._claimed_pins = set()

        # Map our unified constants to lgpio constants
        self._edge_map = {
            constants.RISING_EDGE: lgpio.RISING_EDGE,
            constants.FALLING_EDGE: lgpio.FALLING_EDGE,
            constants.EITHER_EDGE: lgpio.BOTH_EDGES,
        }

    @property
    def connected(self) -> bool:
        """Check if GPIO chip is open."""
        return self._connected

    def set_mode(self, gpio: int, mode: int) -> None:
        """
        Set pin mode (INPUT/OUTPUT).

        Args:
            gpio: GPIO pin number
            mode: INPUT or OUTPUT constant
        """
        if gpio in self._claimed_pins:
            lgpio.gpio_free(self._chip, gpio)

        if mode == constants.OUTPUT:
            lgpio.gpio_claim_output(self._chip, gpio)
        else:
            lgpio.gpio_claim_input(self._chip, gpio)

        self._claimed_pins.add(gpio)

    def read(self, gpio: int) -> int:
        """Read pin state (0 or 1)."""
        return lgpio.gpio_read(self._chip, gpio)

    def write(self, gpio: int, level: int) -> None:
        """Write pin state (0 or 1)."""
        lgpio.gpio_write(self._chip, gpio, level)

    def callback(self, gpio: int, edge: int, func):
        """
        Register edge detection callback.

        Note: lgpio callback signature differs from pigpio.
        lgpio callback receives (chip, gpio, level, timestamp)
        We wrap to provide (gpio, level, tick) like pigpio.

        Args:
            gpio: GPIO pin number
            edge: Edge type (RISING_EDGE, FALLING_EDGE, EITHER_EDGE)
            func: Callback function(gpio, level, tick)

        Returns:
            Callback wrapper object with cancel() method
        """
        lgpio_edge = self._edge_map.get(edge, lgpio.BOTH_EDGES)

        # Claim pin for alert if not already an input
        if gpio not in self._claimed_pins:
            lgpio.gpio_claim_input(self._chip, gpio)
            self._claimed_pins.add(gpio)

        # Set up alert on the pin
        lgpio.gpio_claim_alert(self._chip, gpio, lgpio_edge)

        # Create wrapper that adapts lgpio callback signature to pigpio style
        def wrapper(chip, pin, level, timestamp):
            # Convert timestamp to microseconds like pigpio tick
            tick = int(timestamp * 1000000) & 0xFFFFFFFF
            func(pin, level, tick)

        cb = lgpio.callback(self._chip, gpio, lgpio_edge, wrapper)
        self._callbacks[gpio] = cb
        return _CallbackWrapper(cb)

    def set_watchdog(self, gpio: int, timeout_ms: int) -> None:
        """
        Set watchdog timeout for pin.

        Note: lgpio doesn't have native watchdog support.
        This is a no-op; HX711_lgpio.py uses alternative timing.
        """
        # lgpio doesn't support watchdogs directly
        # The HX711_lgpio implementation handles timing differently
        pass

    def stop(self) -> None:
        """Clean up and release resources."""
        # Cancel all callbacks
        for cb in self._callbacks.values():
            try:
                cb.cancel()
            except Exception:
                pass
        self._callbacks.clear()

        # Free claimed pins
        for gpio in self._claimed_pins:
            try:
                lgpio.gpio_free(self._chip, gpio)
            except Exception:
                pass
        self._claimed_pins.clear()

        # Close chip
        if self._chip >= 0:
            try:
                lgpio.gpiochip_close(self._chip)
            except Exception:
                pass
            self._chip = -1
            self._connected = False


class _CallbackWrapper:
    """Wrapper to provide consistent cancel() interface."""

    def __init__(self, cb):
        self._cb = cb

    def cancel(self):
        """Cancel the callback."""
        if self._cb is not None:
            self._cb.cancel()
            self._cb = None


class LgpioI2C:
    """
    I2C implementation using smbus2 for Pi 5 compatibility.

    smbus2 uses the kernel I2C driver which works across all Pi versions.
    """

    def __init__(self):
        """Initialize I2C manager."""
        try:
            import smbus2
            self._smbus = smbus2
        except ImportError:
            raise ImportError(
                "smbus2 not installed. Install with: pip3 install smbus2"
            )
        self._handles = {}
        self._next_handle = 1

    def open(self, bus: int, address: int) -> int:
        """
        Open I2C connection.

        Args:
            bus: I2C bus number (usually 1)
            address: I2C device address

        Returns:
            Handle for subsequent operations
        """
        handle = self._next_handle
        self._next_handle += 1

        self._handles[handle] = {
            'bus': self._smbus.SMBus(bus),
            'address': address
        }
        return handle

    def write_device(self, handle: int, data) -> None:
        """
        Write data to I2C device.

        Args:
            handle: Handle from open()
            data: Bytes or list to write
        """
        h = self._handles[handle]
        data_list = list(data) if hasattr(data, '__iter__') else [data]

        if len(data_list) == 1:
            h['bus'].write_byte(h['address'], data_list[0])
        elif len(data_list) >= 2:
            # First byte is register, rest is data
            h['bus'].write_i2c_block_data(h['address'], data_list[0], data_list[1:])

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
        h = self._handles[handle]
        data = h['bus'].read_i2c_block_data(h['address'], register, count)
        return count, data

    def close(self, handle: int) -> None:
        """Close I2C connection."""
        if handle in self._handles:
            self._handles[handle]['bus'].close()
            del self._handles[handle]


class LgpioSPI:
    """
    SPI implementation using spidev for Pi 5 compatibility.

    spidev uses the kernel SPI driver which works across all Pi versions.
    """

    def __init__(self):
        """Initialize SPI manager."""
        try:
            import spidev
            self._spidev = spidev
        except ImportError:
            raise ImportError(
                "spidev not installed. Install with: pip3 install spidev"
            )
        self._handles = {}
        self._next_handle = 1

    def open(self, channel: int, baud: int, flags: int = 0) -> int:
        """
        Open SPI connection.

        Args:
            channel: SPI channel (CE0=0, CE1=1, etc.)
            baud: Clock speed in Hz
            flags: Mode flags (bit 8 = aux SPI, bits 0-1 = mode)

        Returns:
            Handle for subsequent operations
        """
        spi = self._spidev.SpiDev()

        # Check for auxiliary SPI flag (bit 8)
        bus = 1 if (flags & 256) else 0

        spi.open(bus, channel)
        spi.max_speed_hz = baud
        spi.mode = flags & 0x3  # Mode bits (0-3)

        handle = self._next_handle
        self._next_handle += 1
        self._handles[handle] = spi

        return handle

    def xfer(self, handle: int, data: list) -> tuple:
        """
        Transfer data over SPI (simultaneous read/write).

        Args:
            handle: Handle from open()
            data: List of bytes to send

        Returns:
            Tuple of (count, data_received)
        """
        spi = self._handles[handle]
        result = spi.xfer2(list(data))
        return len(result), result

    def close(self, handle: int) -> None:
        """Close SPI connection."""
        if handle in self._handles:
            self._handles[handle].close()
            del self._handles[handle]


# Export lgpio constants for compatibility
OUTPUT = constants.OUTPUT
INPUT = constants.INPUT
EITHER_EDGE = constants.EITHER_EDGE
FALLING_EDGE = constants.FALLING_EDGE
RISING_EDGE = constants.RISING_EDGE
