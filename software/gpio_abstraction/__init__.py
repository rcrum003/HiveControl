#!/usr/bin/env python3
"""
GPIO Abstraction Layer for HiveControl

Provides automatic selection of GPIO libraries based on Raspberry Pi version:
- Pi 4 and earlier: Uses pigpio
- Pi 5 and later: Uses lgpio + smbus2/spidev

Usage:
    from gpio_abstraction.gpio_factory import get_gpio, get_i2c, get_spi, cleanup
    from gpio_abstraction.pi_detect import get_pi_generation, get_detailed_info
"""

from .pi_detect import get_pi_generation, get_detailed_info, PiGeneration
from .gpio_factory import get_gpio, get_i2c, get_spi, cleanup

__all__ = [
    'get_pi_generation',
    'get_detailed_info',
    'PiGeneration',
    'get_gpio',
    'get_i2c',
    'get_spi',
    'cleanup'
]
