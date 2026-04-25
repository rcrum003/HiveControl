#!/usr/bin/env python3

import glob
import os
import sys
import termios
import time

BAUD_RATE = 115200
BAUD_CONST = termios.B115200
READ_TIMEOUT = 5.0
MAX_LINES = 20


def find_trinkey_port():
    candidates = sorted(glob.glob('/dev/ttyACM*'))
    if not candidates:
        raise FileNotFoundError("No /dev/ttyACM* devices found. Is the SHT41 Trinkey plugged in?")
    return candidates[0]


def open_serial(port):
    fd = os.open(port, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)

    attrs = termios.tcgetattr(fd)
    # cflag: 8N1, local, enable receiver
    attrs[2] = termios.CS8 | termios.CLOCAL | termios.CREAD
    # iflag: no parity check, no flow control, no CR/NL translation
    attrs[0] = 0
    # oflag: raw output
    attrs[1] = 0
    # lflag: non-canonical, no echo
    attrs[3] = 0
    # cc: VMIN=1 byte, VTIME=1 (0.1s inter-char timeout)
    attrs[6][termios.VMIN] = 1
    attrs[6][termios.VTIME] = 1
    # set baud rate
    attrs[4] = BAUD_CONST  # ispeed
    attrs[5] = BAUD_CONST  # ospeed

    termios.tcsetattr(fd, termios.TCSANOW, attrs)
    termios.tcflush(fd, termios.TCIOFLUSH)

    # clear nonblock now that termios is configured
    import fcntl
    flags = fcntl.fcntl(fd, fcntl.F_GETFL)
    fcntl.fcntl(fd, fcntl.F_SETFL, flags & ~os.O_NONBLOCK)

    return fd


def read_line(fd, timeout):
    buf = bytearray()
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        try:
            ch = os.read(fd, 1)
        except OSError:
            time.sleep(0.01)
            continue
        if not ch:
            time.sleep(0.01)
            continue
        if ch == b'\n':
            return buf.decode('utf-8', errors='replace').strip()
        buf.extend(ch)
    raise TimeoutError("Timed out reading from Trinkey serial port")


def parse_reading(line):
    parts = [p.strip() for p in line.split(',')]

    if len(parts) == 4:
        # factory firmware: serial_number, temp_c, humidity%, touch
        temp_c = float(parts[1])
        humidity = float(parts[2])
    elif len(parts) == 2:
        # simple firmware: temp_c, humidity%
        temp_c = float(parts[0])
        humidity = float(parts[1])
    else:
        raise ValueError("Unexpected CSV format: {} fields".format(len(parts)))

    return temp_c, humidity


def main():
    port = find_trinkey_port()
    fd = open_serial(port)

    try:
        # discard stale partial line
        try:
            read_line(fd, 2.0)
        except TimeoutError:
            pass

        for _ in range(MAX_LINES):
            line = read_line(fd, READ_TIMEOUT)

            if not line or line.startswith('#'):
                continue

            try:
                temp_c, humidity = parse_reading(line)
            except (ValueError, IndexError):
                continue

            temp_f = (temp_c * 1.8) + 32.0
            print("{:.2f} {:.2f} {:.2f}".format(temp_f, temp_c, humidity))
            return

        raise RuntimeError("No valid reading after {} lines".format(MAX_LINES))

    finally:
        os.close(fd)


if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        print("Error: {}".format(e), file=sys.stderr)
        sys.exit(1)
