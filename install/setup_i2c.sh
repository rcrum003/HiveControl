#!/bin/bash
# file: setup_i2c.sh
#
# This script will enable I2C and install i2c-tools on your Raspberry Pi
#

# check if sudo is used
if [ "$(id -u)" != 0 ]; then
  echo 'Sorry, you need to run this script with sudo'
  exit 1
fi

# Detect config.txt location (Pi 5 / Bookworm uses /boot/firmware/config.txt)
if [ -f /boot/firmware/config.txt ]; then
  CONFIG_TXT="/boot/firmware/config.txt"
else
  CONFIG_TXT="/boot/config.txt"
fi
echo ">>> Using config: $CONFIG_TXT"

# enable I2C on Raspberry Pi
echo '>>> Enable I2C'
if grep -q 'i2c-bcm2708' /etc/modules; then
  echo 'Seems i2c-bcm2708 module already exists, skip this step.'
else
  echo 'i2c-bcm2708' >> /etc/modules
fi
if grep -q 'i2c-dev' /etc/modules; then
  echo 'Seems i2c-dev module already exists, skip this step.'
else
  echo 'i2c-dev' >> /etc/modules
fi
if grep -q 'dtparam=i2c1=on' "$CONFIG_TXT"; then
  echo 'Seems i2c1 parameter already set, skip this step.'
else
  echo 'dtparam=i2c1=on' >> "$CONFIG_TXT"
fi
if grep -q 'dtparam=i2c1_baudrate=10000' "$CONFIG_TXT"; then
  echo "Seems dtparam=i2c1_baudrate=10000 is already set, skip this step."
else
  echo 'dtparam=i2c1_baudrate=10000' >> "$CONFIG_TXT"
fi
if grep -q 'dtparam=i2c_arm=on' "$CONFIG_TXT"; then
  echo 'Seems i2c_arm parameter already set, skip this step.'
else
  echo 'dtparam=i2c_arm=on' >> "$CONFIG_TXT"
fi
if [ -f /etc/modprobe.d/raspi-blacklist.conf ]; then
  sed -i 's/^blacklist spi-bcm2708/#blacklist spi-bcm2708/' /etc/modprobe.d/raspi-blacklist.conf
  sed -i 's/^blacklist i2c-bcm2708/#blacklist i2c-bcm2708/' /etc/modprobe.d/raspi-blacklist.conf
else
  echo 'File raspi-blacklist.conf does not exist, skip this step.'
fi

# Load i2c-dev module immediately (takes effect without reboot)
modprobe i2c-dev 2>/dev/null || true

# install i2c-tools
echo '>>> Install i2c-tools'
if hash i2cget 2>/dev/null; then
  echo 'Seems i2c-tools is installed already, skip this step.'
else
  apt-get install -y i2c-tools libi2c-dev
fi

echo '>>> I2C setup complete. A reboot is required for dtparam changes to take effect.'
