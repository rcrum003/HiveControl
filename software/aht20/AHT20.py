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


class RawI2CAdapter:
    """Adapter using raw file descriptor I/O. Works on all Pi I2C buses including bit-banged."""

    I2C_SLAVE = 0x0703

    def __init__(self, bus: int | None = None, address: int = AHT20_I2C_ADDR) -> None:
        import fcntl
        import glob
        import os
        self._os = os
        self._fcntl = fcntl
        self._address = address
        self._fd = None

        if bus is not None:
            self._open_bus(bus, address)
        else:
            self._find_and_open_bus(address)

    def _open_bus(self, bus_num: int, address: int) -> None:
        path = "/dev/i2c-{}".format(bus_num)
        self._fd = self._os.open(path, self._os.O_RDWR)
        self._fcntl.ioctl(self._fd, self.I2C_SLAVE, address)

    def _find_and_open_bus(self, address: int) -> None:
        import glob
        bus_paths = sorted(glob.glob('/dev/i2c-*'))
        errors = []
        for path in bus_paths:
            bus_num = int(path.split('-')[-1])
            try:
                self._open_bus(bus_num, address)
                # Verify device responds with a single-byte read
                self._os.read(self._fd, 1)
                return
            except Exception as e:
                errors.append("bus {}: {}".format(bus_num, e))
                if self._fd is not None:
                    try:
                        self._os.close(self._fd)
                    except Exception:
                        pass
                    self._fd = None
        raise FileNotFoundError(
            "AHT20 not found at address 0x{:02x} on any I2C bus. {}"
            .format(address, "; ".join(errors))
        )

    def write_bytes(self, data: list) -> None:
        self._os.write(self._fd, bytes(data))

    def read_bytes(self, count: int) -> list:
        return list(self._os.read(self._fd, count))

    def close(self) -> None:
        if self._fd is not None:
            self._os.close(self._fd)
            self._fd = None


if __name__ == "__main__":
    adapter = RawI2CAdapter()
    sensor = AHT20(adapter)
    temp_c, humidity = sensor.read_data()

    temp_f = (temp_c * 1.8) + 32.0
    print("{:.2f} {:.2f} {:.2f}".format(temp_f, temp_c, humidity))

    sensor.close()
