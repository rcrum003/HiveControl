#!/usr/bin/env python3
"""
Raspberry Pi version detection module.

Reads /proc/device-tree/model to determine Pi version and select
the appropriate GPIO library.
"""

import re
from enum import Enum
from typing import Optional, Tuple


class PiGeneration(Enum):
    """Enumeration of Pi generations for GPIO compatibility."""
    LEGACY = "legacy"      # Pi 4 and earlier (uses pigpio)
    RP1 = "rp1"           # Pi 5+ (uses lgpio/gpiod)
    UNKNOWN = "unknown"


def read_pi_model() -> Optional[str]:
    """
    Read the Raspberry Pi model from device tree.

    Returns:
        Model string (e.g., "Raspberry Pi 4 Model B Rev 1.2")
        or None if not a Raspberry Pi or file not found.
    """
    model_path = "/proc/device-tree/model"
    try:
        with open(model_path, 'r') as f:
            return f.read().strip().rstrip('\x00')
    except (FileNotFoundError, IOError, PermissionError):
        return None


def parse_pi_version(model_string: Optional[str]) -> Tuple[Optional[int], Optional[str]]:
    """
    Parse the Pi version number from model string.

    Args:
        model_string: Full model string from device tree

    Returns:
        Tuple of (version_number, full_model) or (None, None) if not parseable

    Examples:
        "Raspberry Pi 4 Model B Rev 1.2" -> (4, "Raspberry Pi 4 Model B Rev 1.2")
        "Raspberry Pi 5 Model B Rev 1.0" -> (5, "Raspberry Pi 5 Model B Rev 1.0")
        "Raspberry Pi Zero 2 W Rev 1.0" -> (0, "Raspberry Pi Zero 2 W Rev 1.0")
    """
    if not model_string:
        return None, None

    # Check if it's a Raspberry Pi at all
    if "Raspberry Pi" not in model_string:
        return None, model_string

    # Pattern for standard Pi models (Pi 1, 2, 3, 4, 5, etc.)
    match = re.search(r"Raspberry Pi (\d+)", model_string)
    if match:
        return int(match.group(1)), model_string

    # Pattern for Compute Module
    match = re.search(r"Raspberry Pi Compute Module (\d+)", model_string)
    if match:
        return int(match.group(1)), model_string

    # Handle Pi Zero variants
    if "Zero 2" in model_string:
        # Pi Zero 2 W uses BCM2710 (same as Pi 3)
        return 3, model_string
    elif "Zero" in model_string:
        # Original Pi Zero uses BCM2835 (same as Pi 1)
        return 1, model_string

    # Handle Pi 1 Model A/B (doesn't have version number in name)
    if re.search(r"Raspberry Pi Model [AB]", model_string):
        return 1, model_string

    return None, model_string


def get_pi_generation() -> PiGeneration:
    """
    Determine the Pi generation for GPIO library selection.

    Returns:
        PiGeneration.LEGACY for Pi 4 and earlier (use pigpio)
        PiGeneration.RP1 for Pi 5 and later (use lgpio)
        PiGeneration.UNKNOWN if detection fails
    """
    model_string = read_pi_model()
    version, _ = parse_pi_version(model_string)

    if version is None:
        return PiGeneration.UNKNOWN

    # Pi 5 and later use the RP1 southbridge chip
    if version >= 5:
        return PiGeneration.RP1
    else:
        return PiGeneration.LEGACY


def is_pi5_or_later() -> bool:
    """
    Quick check if running on Pi 5 or later.

    Returns:
        True if Pi 5+, False otherwise (including unknown)
    """
    return get_pi_generation() == PiGeneration.RP1


def get_detailed_info() -> dict:
    """
    Get detailed Pi information for diagnostics.

    Returns:
        Dictionary with model, version, generation, and recommended library
    """
    model_string = read_pi_model()
    version, _ = parse_pi_version(model_string)
    generation = get_pi_generation()

    library_map = {
        PiGeneration.LEGACY: "pigpio",
        PiGeneration.RP1: "lgpio",
        PiGeneration.UNKNOWN: "pigpio (fallback)"
    }

    return {
        "model": model_string,
        "version": version,
        "generation": generation.value,
        "recommended_library": library_map[generation],
        "is_pi5_or_later": generation == PiGeneration.RP1
    }


if __name__ == "__main__":
    # Print detection results when run directly
    info = get_detailed_info()
    print("Raspberry Pi Detection Results:")
    print(f"  Model: {info['model']}")
    print(f"  Version: {info['version']}")
    print(f"  Generation: {info['generation']}")
    print(f"  Recommended GPIO Library: {info['recommended_library']}")
    print(f"  Is Pi 5 or later: {info['is_pi5_or_later']}")
