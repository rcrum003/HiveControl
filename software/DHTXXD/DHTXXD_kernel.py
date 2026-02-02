#!/usr/bin/env python3
"""
DHT11/DHT22 sensor reader using Linux kernel IIO driver.

This script reads temperature and humidity from DHT sensors using the
kernel's dht11 driver via sysfs, which works on Raspberry Pi 5 and
doesn't require pigpio.

Prerequisites:
1. Enable the dht11 overlay in /boot/firmware/config.txt:
   dtoverlay=dht11,gpiopin=<gpio_number>

2. Reboot the Pi

The sensor data will be available at:
   /sys/devices/platform/dht11@<gpio>/iio:device*/in_temp_input
   /sys/devices/platform/dht11@<gpio>/iio:device*/in_humidityrelative_input

Usage:
    python3 DHTXXD_kernel.py -g <gpio>
    python3 DHTXXD_kernel.py -g 4       # Read from GPIO 4

Output format matches original DHTXXD:
    status temperature humidity
    0 = success, 1 = checksum error, 2 = bad data, 3 = timeout
"""

import argparse
import glob
import sys
import time


def find_dht_device(gpio):
    """
    Find the IIO device path for a DHT sensor on the specified GPIO.

    Args:
        gpio: GPIO pin number

    Returns:
        Path to the iio:device directory, or None if not found
    """
    # The device tree creates entries like /sys/devices/platform/dht11@4/
    patterns = [
        f"/sys/devices/platform/dht11@{gpio}/iio:device*",
        f"/sys/bus/iio/devices/iio:device*/name",  # Fallback search
    ]

    for pattern in patterns:
        matches = glob.glob(pattern)
        if matches:
            if "name" in pattern:
                # Check if any device is a dht11
                for name_file in matches:
                    try:
                        with open(name_file, 'r') as f:
                            if 'dht11' in f.read():
                                return name_file.replace('/name', '')
                    except Exception:
                        continue
            else:
                return matches[0]

    return None


def read_sensor(device_path, retries=3):
    """
    Read temperature and humidity from the DHT sensor.

    Args:
        device_path: Path to the IIO device
        retries: Number of read attempts

    Returns:
        Tuple of (status, temperature_c, humidity)
        status: 0=success, 3=timeout/error
    """
    temp_path = f"{device_path}/in_temp_input"
    humidity_path = f"{device_path}/in_humidityrelative_input"

    for attempt in range(retries):
        try:
            # Read temperature (value is in millidegrees Celsius)
            with open(temp_path, 'r') as f:
                temp_raw = int(f.read().strip())
                temperature = temp_raw / 1000.0

            # Small delay between reads
            time.sleep(0.1)

            # Read humidity (value is in millipercent)
            with open(humidity_path, 'r') as f:
                humidity_raw = int(f.read().strip())
                humidity = humidity_raw / 1000.0

            # Validate readings
            if -40 <= temperature <= 80 and 0 <= humidity <= 100:
                return 0, temperature, humidity

        except FileNotFoundError:
            # Device not found
            return 3, 0.0, 0.0
        except (ValueError, IOError) as e:
            # Read error, retry
            if attempt < retries - 1:
                time.sleep(0.5)
                continue
            return 3, 0.0, 0.0

    return 3, 0.0, 0.0


def main():
    parser = argparse.ArgumentParser(
        description='Read DHT11/DHT22 sensor using kernel driver'
    )
    parser.add_argument(
        '-g', '--gpio',
        type=int,
        default=4,
        help='GPIO pin number (default: 4)'
    )
    parser.add_argument(
        '-v', '--verbose',
        action='store_true',
        help='Verbose output'
    )

    args = parser.parse_args()

    # Find the DHT device
    device_path = find_dht_device(args.gpio)

    if device_path is None:
        if args.verbose:
            print(f"DHT device not found on GPIO {args.gpio}", file=sys.stderr)
            print("Make sure dtoverlay=dht11,gpiopin={} is in /boot/firmware/config.txt".format(args.gpio), file=sys.stderr)
        # Output error in same format as original DHTXXD
        print("3 0.0 0.0")
        sys.exit(1)

    if args.verbose:
        print(f"Found DHT device at: {device_path}", file=sys.stderr)

    # Read the sensor
    status, temperature, humidity = read_sensor(device_path)

    # Output in same format as original DHTXXD: status temperature humidity
    print(f"{status} {temperature:.1f} {humidity:.1f}")

    sys.exit(0 if status == 0 else 1)


if __name__ == "__main__":
    main()
