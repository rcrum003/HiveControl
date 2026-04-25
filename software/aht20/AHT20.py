#!/usr/bin/env python3

import time
import sys

# AHT20 Temperature/Humidity Sensor Driver
# I2C Address: 0x38 (fixed)
# Protocol: Send trigger command, wait ~80ms, read 6 bytes
# Compatible with Raspberry Pi 4 (pigpio) and Pi 5 (lgpio/smbus2)

AHT20_I2C_ADDR = 0x38
AHT20_CMD_CALIBRATE = 0xBE
AHT20_CMD_TRIGGER = 0xAC
AHT20_CMD_SOFTRESET = 0xBA
AHT20_STATUS_BUSY = 0x80
AHT20_STATUS_CALIBRATED = 0x08


class AHT20:
    """AHT20 sensor using a simple I2C adapter with write_bytes/read_bytes."""

    def __init__(self, i2c_adapter) -> None:
        self._i2c = i2c_adapter
        time.sleep(0.1)
        self._reset()
        self._calibrate()

    def _reset(self) -> None:
        self._i2c.write_bytes([AHT20_CMD_SOFTRESET])
        time.sleep(0.02)

    def _calibrate(self) -> None:
        self._i2c.write_bytes([AHT20_CMD_CALIBRATE, 0x08, 0x00])
        self._wait_for_idle()
        status = self._read_status()
        if not (status & AHT20_STATUS_CALIBRATED):
            raise RuntimeError("AHT20 calibration failed")

    def _read_status(self) -> int:
        data = self._i2c.read_bytes(1)
        return data[0]

    def _wait_for_idle(self, timeout: float = 0.5) -> None:
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            status = self._read_status()
            if not (status & AHT20_STATUS_BUSY):
                return
            time.sleep(0.01)
        raise RuntimeError("AHT20 timeout waiting for idle")

    def read_data(self) -> tuple:
        """Returns (temperature_c, humidity_pct)."""
        self._i2c.write_bytes([AHT20_CMD_TRIGGER, 0x33, 0x00])
        time.sleep(0.08)
        self._wait_for_idle()

        data = self._i2c.read_bytes(7)

        raw_humidity = ((data[1] << 12) | (data[2] << 4) | (data[3] >> 4))
        raw_temperature = (((data[3] & 0x0F) << 16) | (data[4] << 8) | data[5])

        humidity = (raw_humidity / 1048576.0) * 100.0
        temperature_c = (raw_temperature / 1048576.0) * 200.0 - 50.0

        return temperature_c, humidity

    def close(self) -> None:
        self._i2c.close()


class PigpioI2CAdapter:
    """Adapter for pigpio raw I2C (Pi 4 and earlier)."""

    def __init__(self, pi, bus: int = 1, address: int = AHT20_I2C_ADDR) -> None:
        self._pi = pi
        self._handle = pi.i2c_open(bus, address)

    def write_bytes(self, data: list) -> None:
        self._pi.i2c_write_device(self._handle, data)

    def read_bytes(self, count: int) -> list:
        _count, data = self._pi.i2c_read_device(self._handle, count)
        return list(data)

    def close(self) -> None:
        if self._handle is not None:
            self._pi.i2c_close(self._handle)
            self._handle = None


class SMBus2I2CAdapter:
    """Adapter for smbus2. Uses raw I2C transactions via i2c_rdwr. Works on all Pi versions."""

    def __init__(self, bus: int | None = None, address: int = AHT20_I2C_ADDR) -> None:
        from smbus2 import SMBus, i2c_msg
        self._i2c_msg = i2c_msg
        self._address = address

        if bus is not None:
            self._bus = SMBus(bus)
        else:
            self._bus = self._find_bus(SMBus, i2c_msg, address)

    def _find_bus(self, SMBus, i2c_msg, address: int):
        """Scan available I2C buses for the AHT20 at the expected address."""
        import glob
        import os
        bus_paths = sorted(glob.glob('/dev/i2c-*'))
        errors = []
        for path in bus_paths:
            bus_num = int(path.split('-')[-1])
            try:
                bus = SMBus(bus_num)
                # Probe with a write (status request) — AHT20 may NACK bare reads
                msg = i2c_msg.write(address, [0x71])
                bus.i2c_rdwr(msg)
                return bus
            except OSError as e:
                errors.append("bus {}: {}".format(bus_num, e))
                try:
                    bus.close()
                except Exception:
                    pass
            except Exception as e:
                errors.append("bus {}: {}".format(bus_num, e))
                try:
                    bus.close()
                except Exception:
                    pass

        # Last resort: use ioctl I2C_SLAVE directly (bypasses smbus2 transaction layer)
        import fcntl
        I2C_SLAVE = 0x0703
        for path in bus_paths:
            try:
                fd = os.open(path, os.O_RDWR)
                fcntl.ioctl(fd, I2C_SLAVE, address)
                os.close(fd)
                bus_num = int(path.split('-')[-1])
                return SMBus(bus_num)
            except Exception:
                try:
                    os.close(fd)
                except Exception:
                    pass

        raise FileNotFoundError(
            "AHT20 not found at address 0x{:02x} on any I2C bus. {}"
            .format(address, "; ".join(errors))
        )

    def write_bytes(self, data: list) -> None:
        msg = self._i2c_msg.write(self._address, data)
        self._bus.i2c_rdwr(msg)

    def read_bytes(self, count: int) -> list:
        msg = self._i2c_msg.read(self._address, count)
        self._bus.i2c_rdwr(msg)
        return list(msg)

    def close(self) -> None:
        if self._bus is not None:
            self._bus.close()
            self._bus = None


if __name__ == "__main__":
    sys.path.insert(0, '/home/HiveControl/software')

    adapter = None
    cleanup_fn = None

    try:
        from gpio_abstraction.gpio_factory import get_gpio, cleanup
        from gpio_abstraction.pi_detect import is_pi5_or_later

        gpio = get_gpio()
        if not gpio.connected:
            raise RuntimeError("GPIO not connected")

        if is_pi5_or_later():
            adapter = SMBus2I2CAdapter()
        else:
            adapter = PigpioI2CAdapter(gpio.pi)
        cleanup_fn = cleanup

    except Exception:
        # gpio_abstraction failed (pigpio not installed, daemon not running, etc.)
        # Fall back to smbus2 which works on all Pi versions via kernel I2C driver
        adapter = SMBus2I2CAdapter()

    try:
        sensor = AHT20(adapter)
        temp_c, humidity = sensor.read_data()

        temp_f = (temp_c * 1.8) + 32.0
        print("{:.2f} {:.2f} {:.2f}".format(temp_f, temp_c, humidity))

        sensor.close()
    finally:
        if cleanup_fn:
            cleanup_fn()
