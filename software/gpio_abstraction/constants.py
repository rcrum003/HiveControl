#!/usr/bin/env python3
"""
Unified GPIO constants that work across pigpio and lgpio.

These constants provide a library-agnostic interface for GPIO operations.
The wrapper classes translate these to the appropriate library-specific values.
"""

# Pin modes
INPUT = 0
OUTPUT = 1

# Edge detection types
RISING_EDGE = 0
FALLING_EDGE = 1
EITHER_EDGE = 2

# Pull resistor configuration
PUD_OFF = 0
PUD_DOWN = 1
PUD_UP = 2

# Logic levels
LOW = 0
HIGH = 1
