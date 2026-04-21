#!/usr/bin/python3
"""BroodMinder BLE scanner using bleak (modern async BLE library).

Replaces the legacy bluepy-based BM_Scan.py. Scans for BroodMinder devices
by manufacturer data (company ID 0x028D) and decodes temperature, humidity,
weight, and battery from the v2 advertising format.

Output format (with weight):
  deviceName,MAC,RSSI,datetime,sample,tempC,tempF,humidity,battery,weight
Output format (without weight):
  MAC,RSSI,datetime,sample,tempC,tempF,humidity,battery

Usage: sudo python3 BM_Scan_bleak.py [scan_seconds]
"""

__author__ = "Ryan Crum"
__version__ = "2.0"

import asyncio
import datetime
import sys

from bleak import BleakScanner

BROODMINDER_COMPANY_ID = 0x028D
DEFAULT_SCAN_SECONDS = 15


def decode_broodminder(addr: str, name: str, rssi: int, raw: bytes, observation_date: str) -> None:
    if len(raw) < 15:
        return

    model = raw[0]
    battery_percent = raw[4]
    sample_number = raw[5] | (raw[6] << 8)

    temp_raw = raw[7] | (raw[8] << 8)
    if model >= 47:
        temp_c = round((temp_raw - 5000) / 100.0, 1)
    else:
        temp_c = round((temp_raw / 65536.0 * 165.0) - 40.0, 1)
    temp_f = round(temp_c * 9.0 / 5.0 + 32.0, 1)

    has_humidity = model in (42, 56)
    humidity_percent = raw[14] if has_humidity else 0

    has_weight = model in (43, 49, 57, 58)
    weight_l = (raw[11] << 8 | raw[10]) - 32767
    weight_r = (raw[13] << 8 | raw[12]) - 32767
    weight_total = round((weight_l + weight_r) / 100.0, 2)

    device_id = name if name else addr

    if has_weight and weight_total > -1:
        print(f"Sample = {sample_number}, Weight = {weight_total}, "
              f"TemperatureF = {temp_f}, Humidity = {humidity_percent}, "
              f"Battery = {battery_percent}")
        print(f"{device_id},{addr},{rssi},{observation_date},{sample_number},"
              f"{temp_c},{temp_f},{humidity_percent},{battery_percent},{weight_total}")
    else:
        print(f"{addr},{rssi},{observation_date},{sample_number},"
              f"{temp_c},{temp_f},{humidity_percent},{battery_percent}")


async def main() -> None:
    scan_seconds = DEFAULT_SCAN_SECONDS
    if len(sys.argv) > 1:
        try:
            scan_seconds = int(sys.argv[1])
        except ValueError:
            pass

    observation_date = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    found: dict[str, bool] = {}

    def detection_callback(device, advertisement_data) -> None:
        if BROODMINDER_COMPANY_ID in advertisement_data.manufacturer_data:
            addr = device.address.lower()
            if addr not in found:
                found[addr] = True
                raw = advertisement_data.manufacturer_data[BROODMINDER_COMPANY_ID]
                name = advertisement_data.local_name or ""
                rssi = advertisement_data.rssi
                decode_broodminder(addr, name, rssi, raw, observation_date)

    scanner = BleakScanner(detection_callback=detection_callback)
    await scanner.start()
    await asyncio.sleep(scan_seconds)
    await scanner.stop()


if __name__ == "__main__":
    asyncio.run(main())
