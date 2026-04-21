#!/usr/bin/python3
"""BLE debugging tool for HiveControl.

Usage:
  sudo python3 ble_debug.py scan                      # Discover all BLE devices
  sudo python3 ble_debug.py scan -t 30                 # Scan for 30 seconds
  sudo python3 ble_debug.py scan --mfr                 # Only show devices with manufacturer data
  sudo python3 ble_debug.py scan --name brood          # Filter by name (case-insensitive)
  sudo python3 ble_debug.py scan --mac 06:09           # Filter by MAC prefix
  sudo python3 ble_debug.py listen                     # Continuous callback scan (catches intermittent ads)
  sudo python3 ble_debug.py listen -t 60               # Listen for 60 seconds
  sudo python3 ble_debug.py listen --mac 06:09         # Listen for specific MAC
  sudo python3 ble_debug.py info B0:38:29:7B:FB:60     # Connect and list GATT services
  sudo python3 ble_debug.py read B0:38:29:7B:FB:60     # Connect and read all readable characteristics
  sudo python3 ble_debug.py bm                         # Scan for BroodMinder devices specifically
  sudo python3 ble_debug.py bm -t 30                   # BroodMinder scan for 30 seconds
"""

import argparse
import asyncio
import sys

from bleak import BleakClient, BleakScanner

BROODMINDER_COMPANY_ID = 0x028D

BM_MODELS = {
    41: "T (legacy)",
    42: "TH (legacy)",
    43: "W (scale, legacy)",
    47: "T2 / T2SM",
    49: "W2 / XLR",
    52: "SubHub",
    56: "TH2 / TH2SM",
    57: "W3 / XLR",
    58: "W4 / XLR",
}


async def cmd_scan(args: argparse.Namespace) -> None:
    print(f"Scanning for {args.timeout}s...")
    devices = await BleakScanner.discover(timeout=args.timeout, return_adv=True)

    results = []
    for addr, (dev, adv) in devices.items():
        name = adv.local_name or ""
        if args.name and args.name.lower() not in name.lower():
            continue
        if args.mac and not addr.lower().startswith(args.mac.lower()):
            continue
        if args.mfr and not adv.manufacturer_data:
            continue
        results.append((addr, name, adv.rssi, adv.manufacturer_data, adv.service_uuids))

    results.sort(key=lambda x: x[2], reverse=True)

    print(f"\n{'MAC':<20} {'RSSI':>5}  {'Name':<25} {'Mfr Data'}")
    print("-" * 85)
    for addr, name, rssi, mfr, svcs in results:
        mfr_str = ""
        if mfr:
            mfr_str = " ".join(f"[{hex(k)}]={v.hex()}" for k, v in mfr.items())
        print(f"{addr:<20} {rssi:>5}  {name:<25} {mfr_str}")

    print(f"\n{len(results)} device(s) found")


async def cmd_listen(args: argparse.Namespace) -> None:
    seen: dict[str, int] = {}

    def callback(device, adv) -> None:
        addr = device.address
        name = adv.local_name or ""

        if args.name and args.name.lower() not in name.lower():
            return
        if args.mac and not addr.lower().startswith(args.mac.lower()):
            return
        if args.mfr and not adv.manufacturer_data:
            return

        count = seen.get(addr, 0) + 1
        seen[addr] = count

        mfr_str = ""
        if adv.manufacturer_data:
            mfr_str = " ".join(f"[{hex(k)}]={v.hex()}" for k, v in adv.manufacturer_data.items())

        if count == 1:
            print(f"NEW  {addr}  RSSI={adv.rssi:>4}  name={name:<25} mfr={mfr_str}")
        elif count <= 5 or count % 10 == 0:
            print(f" #{count:<3} {addr}  RSSI={adv.rssi:>4}  name={name:<25} mfr={mfr_str}")

    print(f"Listening for {args.timeout}s (Ctrl+C to stop)...")
    scanner = BleakScanner(detection_callback=callback)
    await scanner.start()
    try:
        await asyncio.sleep(args.timeout)
    except asyncio.CancelledError:
        pass
    await scanner.stop()
    print(f"\n{len(seen)} unique device(s) heard")


async def cmd_info(args: argparse.Namespace) -> None:
    print(f"Connecting to {args.address}...")
    try:
        async with BleakClient(args.address, timeout=args.timeout) as client:
            print(f"Connected: {client.is_connected}\n")
            for svc in client.services:
                print(f"Service: {svc.uuid}  ({svc.description})")
                for char in svc.characteristics:
                    props = ", ".join(char.properties)
                    print(f"  {char.uuid}  [{props}]  {char.description}")
                    for desc in char.descriptors:
                        print(f"    Descriptor: {desc.uuid}  {desc.description}")
                print()
    except Exception as e:
        print(f"Error: {e}")


async def cmd_read(args: argparse.Namespace) -> None:
    print(f"Connecting to {args.address}...")
    try:
        async with BleakClient(args.address, timeout=args.timeout) as client:
            print(f"Connected: {client.is_connected}\n")
            for svc in client.services:
                print(f"Service: {svc.uuid}  ({svc.description})")
                for char in svc.characteristics:
                    props = ", ".join(char.properties)
                    val_str = ""
                    if "read" in char.properties:
                        try:
                            val = await asyncio.wait_for(
                                client.read_gatt_char(char.uuid), timeout=3
                            )
                            ascii_repr = "".join(chr(b) if 32 <= b < 127 else "." for b in val)
                            val_str = f"\n         hex: {val.hex()}\n         ascii: {ascii_repr}"
                        except Exception as e:
                            val_str = f"\n         read error: {e}"
                    print(f"  {char.uuid}  [{props}]{val_str}")
                print()
    except Exception as e:
        print(f"Error: {e}")


async def cmd_bm(args: argparse.Namespace) -> None:
    found: dict[str, bool] = {}

    def callback(device, adv) -> None:
        addr = device.address.lower()
        name = adv.local_name or ""
        has_bm = BROODMINDER_COMPANY_ID in adv.manufacturer_data
        is_bm_name = any(p in name for p in ("BM-", "HB-", "BroodMinder"))
        is_bm_mac = addr.startswith("06:09")

        if not (has_bm or is_bm_name or is_bm_mac):
            return

        tag = "DATA" if has_bm else "NAME"
        if addr not in found:
            found[addr] = True
            print(f"\n[{tag}] {addr}  name={name}  RSSI={adv.rssi}")
            if has_bm:
                raw = adv.manufacturer_data[BROODMINDER_COMPANY_ID]
                print(f"  manufacturer bytes ({len(raw)}): {raw.hex()}")
                print(f"  bytes: {' '.join(f'{b:02x}' for b in raw)}")
                decode_bm(raw)
            else:
                print(f"  (no manufacturer data — may be a hub or sleeping sensor)")
                mfr_keys = [hex(k) for k in adv.manufacturer_data.keys()]
                if mfr_keys:
                    print(f"  other mfr keys: {mfr_keys}")

    print(f"Scanning for BroodMinder devices for {args.timeout}s...")
    scanner = BleakScanner(detection_callback=callback)
    await scanner.start()
    await asyncio.sleep(args.timeout)
    await scanner.stop()

    if not found:
        print("\nNo BroodMinder devices found.")
        print("Tips:")
        print("  - Press/hold button on sensor for 5+ seconds until LED blinks")
        print("  - Check CR2032 battery (replace if older than 1 year)")
        print("  - Use BroodMinder Bees app to wake device")
        print("  - Run: sudo rfkill unblock bluetooth && sudo hciconfig hci0 up")
    else:
        print(f"\n{len(found)} BroodMinder device(s) found")


def decode_bm(raw: bytes) -> None:
    if len(raw) < 15:
        print(f"  payload too short ({len(raw)} bytes) to decode")
        return

    model = raw[0]
    version_minor = raw[1]
    version_major = raw[2]
    battery = raw[4]
    sample = raw[5] | (raw[6] << 8)
    temp_raw = raw[7] | (raw[8] << 8)

    if model >= 47:
        temp_c = round((temp_raw - 5000) / 100.0, 2)
    else:
        temp_c = round((temp_raw / 65536.0 * 165.0) - 40.0, 2)
    temp_f = round(temp_c * 9.0 / 5.0 + 32.0, 2)

    model_name = BM_MODELS.get(model, f"unknown ({model})")
    has_weight = model in (43, 49, 57, 58)
    has_humidity = model in (42, 56)
    is_sm = model in (47, 49, 56, 57, 58)

    print(f"  Decoded:")
    print(f"    Model:       {model} — {model_name}")
    print(f"    Firmware:    {version_major}.{version_minor}")
    print(f"    Battery:     {battery}%")
    print(f"    Sample #:    {sample}")
    print(f"    Temperature: {temp_f}°F / {temp_c}°C")

    if is_sm and len(raw) > 9:
        rt_raw = raw[9] * 256 + raw[3]
        if model >= 47:
            rt_c = round((rt_raw - 5000) / 100.0, 2)
        else:
            rt_c = round((rt_raw / 65536.0 * 165.0) - 40.0, 2)
        rt_f = round(rt_c * 9.0 / 5.0 + 32.0, 2)
        print(f"    RT Temp:     {rt_f}°F / {rt_c}°C")

    if has_humidity and len(raw) > 14:
        print(f"    Humidity:    {raw[14]}%")

    if has_weight and len(raw) > 13:
        weight_l = (raw[11] << 8 | raw[10]) - 32767
        weight_r = (raw[13] << 8 | raw[12]) - 32767
        weight = round((weight_l + weight_r) / 100.0, 2)
        if weight > -1:
            print(f"    Weight:      {weight} lbs")
        else:
            print(f"    Weight:      N/A (raw: L={weight_l}, R={weight_r})")
    elif not has_weight:
        print(f"    Weight:      N/A (no scale on this model)")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="BLE debugging tool for HiveControl",
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    sub = parser.add_subparsers(dest="command")

    p_scan = sub.add_parser("scan", help="Discover all BLE devices")
    p_scan.add_argument("-t", "--timeout", type=int, default=10, help="Scan seconds (default: 10)")
    p_scan.add_argument("--mfr", action="store_true", help="Only show devices with manufacturer data")
    p_scan.add_argument("--name", help="Filter by name (case-insensitive)")
    p_scan.add_argument("--mac", help="Filter by MAC prefix")

    p_listen = sub.add_parser("listen", help="Continuous callback scan")
    p_listen.add_argument("-t", "--timeout", type=int, default=30, help="Listen seconds (default: 30)")
    p_listen.add_argument("--mfr", action="store_true", help="Only show devices with manufacturer data")
    p_listen.add_argument("--name", help="Filter by name")
    p_listen.add_argument("--mac", help="Filter by MAC prefix")

    p_info = sub.add_parser("info", help="Connect and list GATT services")
    p_info.add_argument("address", help="Device MAC address")
    p_info.add_argument("-t", "--timeout", type=int, default=10, help="Connect timeout")

    p_read = sub.add_parser("read", help="Connect and read all characteristics")
    p_read.add_argument("address", help="Device MAC address")
    p_read.add_argument("-t", "--timeout", type=int, default=10, help="Connect timeout")

    p_bm = sub.add_parser("bm", help="Scan for BroodMinder devices")
    p_bm.add_argument("-t", "--timeout", type=int, default=20, help="Scan seconds (default: 20)")

    args = parser.parse_args()

    if args.command is None:
        parser.print_help()
        sys.exit(1)

    commands = {
        "scan": cmd_scan,
        "listen": cmd_listen,
        "info": cmd_info,
        "read": cmd_read,
        "bm": cmd_bm,
    }

    asyncio.run(commands[args.command](args))


if __name__ == "__main__":
    main()
