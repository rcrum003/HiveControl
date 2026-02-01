#!/usr/bin/env python3
"""
Factory for creating appropriate GPIO instances based on Pi version.

Automatically detects the Raspberry Pi version and returns the
appropriate wrapper implementation:
- Pi 4 and earlier: pigpio wrappers
- Pi 5 and later: lgpio wrappers with smbus2/spidev

Usage:
    from gpio_abstraction.gpio_factory import get_gpio, get_i2c, get_spi, cleanup

    gpio = get_gpio()
    if not gpio.connected:
        print("Failed to connect to GPIO")
        exit(1)

    # Use gpio...
    gpio.set_mode(18, OUTPUT)
    gpio.write(18, 1)

    # Clean up when done
    cleanup()
"""

import os
import logging
from .pi_detect import get_pi_generation, PiGeneration

logger = logging.getLogger(__name__)

# Singleton instances
_gpio_instance = None
_i2c_instance = None
_spi_instance = None
_current_generation = None


def _get_forced_library():
    """Check for environment variable override."""
    return os.environ.get('HIVECONTROL_GPIO_LIBRARY', '').lower()


def get_gpio():
    """
    Get the appropriate GPIO instance for this Pi.

    Uses singleton pattern - returns the same instance on subsequent calls.
    Can be overridden by setting HIVECONTROL_GPIO_LIBRARY env var to
    'pigpio' or 'lgpio'.

    Returns:
        GPIO wrapper instance (PigpioGPIO or LgpioGPIO)

    Raises:
        RuntimeError: If GPIO connection fails
        ImportError: If required library is not installed
    """
    global _gpio_instance, _current_generation

    if _gpio_instance is not None:
        return _gpio_instance

    forced = _get_forced_library()
    if forced:
        generation = PiGeneration.LEGACY if forced == 'pigpio' else PiGeneration.RP1
        logger.info(f"Using forced GPIO library: {forced}")
    else:
        generation = get_pi_generation()

    _current_generation = generation

    if generation == PiGeneration.RP1:
        logger.info("Detected Pi 5 or later, using lgpio")
        try:
            from .lgpio_wrapper import LgpioGPIO
            _gpio_instance = LgpioGPIO()
        except ImportError as e:
            raise ImportError(
                f"lgpio not available: {e}. "
                "Install with: sudo apt install python3-lgpio"
            )
    else:
        logger.info("Detected Pi 4 or earlier (or unknown), using pigpio")
        try:
            from .pigpio_wrapper import PigpioGPIO
            _gpio_instance = PigpioGPIO()
        except ImportError as e:
            raise ImportError(
                f"pigpio not available: {e}. "
                "Install pigpio and start daemon: sudo pigpiod"
            )

    if not _gpio_instance.connected:
        lib_name = 'lgpio' if generation == PiGeneration.RP1 else 'pigpio'
        if lib_name == 'pigpio':
            raise RuntimeError(
                f"Failed to connect to {lib_name}. "
                "Ensure pigpiod is running: sudo pigpiod"
            )
        else:
            raise RuntimeError(
                f"Failed to connect to {lib_name}. "
                "Check GPIO permissions and chip availability."
            )

    return _gpio_instance


def get_i2c():
    """
    Get the appropriate I2C instance for this Pi.

    For Pi 4 and earlier, uses pigpio I2C (requires pigpiod).
    For Pi 5 and later, uses smbus2 (kernel driver).

    Returns:
        I2C wrapper instance (PigpioI2C or LgpioI2C)
    """
    global _i2c_instance, _current_generation

    if _i2c_instance is not None:
        return _i2c_instance

    # Ensure GPIO is initialized first to detect generation
    if _current_generation is None:
        get_gpio()

    if _current_generation == PiGeneration.RP1:
        from .lgpio_wrapper import LgpioI2C
        _i2c_instance = LgpioI2C()
    else:
        from .pigpio_wrapper import PigpioI2C
        gpio = get_gpio()
        _i2c_instance = PigpioI2C(gpio.pi)

    return _i2c_instance


def get_spi():
    """
    Get the appropriate SPI instance for this Pi.

    For Pi 4 and earlier, uses pigpio SPI (requires pigpiod).
    For Pi 5 and later, uses spidev (kernel driver).

    Returns:
        SPI wrapper instance (PigpioSPI or LgpioSPI)
    """
    global _spi_instance, _current_generation

    if _spi_instance is not None:
        return _spi_instance

    # Ensure GPIO is initialized first to detect generation
    if _current_generation is None:
        get_gpio()

    if _current_generation == PiGeneration.RP1:
        from .lgpio_wrapper import LgpioSPI
        _spi_instance = LgpioSPI()
    else:
        from .pigpio_wrapper import PigpioSPI
        gpio = get_gpio()
        _spi_instance = PigpioSPI(gpio.pi)

    return _spi_instance


def get_generation():
    """
    Get the detected Pi generation.

    Returns:
        PiGeneration enum value
    """
    global _current_generation

    if _current_generation is None:
        forced = _get_forced_library()
        if forced:
            _current_generation = PiGeneration.LEGACY if forced == 'pigpio' else PiGeneration.RP1
        else:
            _current_generation = get_pi_generation()

    return _current_generation


def is_pigpio():
    """Check if using pigpio (Pi 4 and earlier)."""
    return get_generation() != PiGeneration.RP1


def is_lgpio():
    """Check if using lgpio (Pi 5 and later)."""
    return get_generation() == PiGeneration.RP1


def cleanup():
    """
    Clean up all GPIO resources.

    Call this when your application exits to properly release
    GPIO handles and stop the connection.
    """
    global _gpio_instance, _i2c_instance, _spi_instance, _current_generation

    if _gpio_instance is not None:
        try:
            _gpio_instance.stop()
        except Exception as e:
            logger.warning(f"Error during GPIO cleanup: {e}")
        _gpio_instance = None

    _i2c_instance = None
    _spi_instance = None
    _current_generation = None


# For use in "with" statements
class GPIOContext:
    """Context manager for GPIO operations."""

    def __enter__(self):
        return get_gpio()

    def __exit__(self, exc_type, exc_val, exc_tb):
        cleanup()
        return False


if __name__ == "__main__":
    # Test the factory
    from .pi_detect import get_detailed_info

    print("GPIO Factory Test")
    print("=" * 40)

    info = get_detailed_info()
    print(f"Pi Model: {info['model']}")
    print(f"Generation: {info['generation']}")
    print(f"Recommended: {info['recommended_library']}")
    print()

    try:
        gpio = get_gpio()
        print(f"GPIO connected: {gpio.connected}")
        print(f"Using: {'lgpio' if is_lgpio() else 'pigpio'}")
        cleanup()
        print("Cleanup successful")
    except Exception as e:
        print(f"Error: {e}")
