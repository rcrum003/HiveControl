#!/bin/bash

# ==================================================
# HiveControl Easy Installation Script
# v1.0 - Simplified for non-technical users
#
# This script can be run directly from curl without
# needing to chmod or run separately
#
# Usage:
#   curl -sSL https://raw.githubusercontent.com/rcrum003/HiveControl/master/easy-install.sh | sudo bash
#
# With options:
#   curl -sSL https://raw.githubusercontent.com/rcrum003/HiveControl/master/easy-install.sh | sudo bash -s -- -b
# ==================================================

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Progress indicator
print_header() {
    echo ""
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}========================================${NC}"
    echo ""
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

print_info() {
    echo -e "  $1"
}

# Check if running as root
if [[ $EUID -ne 0 ]]; then
   print_error "This script must be run as root (use sudo)"
   echo ""
   echo "Please run:"
   echo "  curl -sSL https://raw.githubusercontent.com/rcrum003/HiveControl/master/easy-install.sh | sudo bash"
   echo ""
   exit 1
fi

# Welcome message
clear
echo ""
echo -e "${GREEN}╔══════════════════════════════════════════════╗${NC}"
echo -e "${GREEN}║                                              ║${NC}"
echo -e "${GREEN}║          HiveControl Easy Installer          ║${NC}"
echo -e "${GREEN}║                                              ║${NC}"
echo -e "${GREEN}║      Automated Installation for Everyone     ║${NC}"
echo -e "${GREEN}║                                              ║${NC}"
echo -e "${GREEN}╚══════════════════════════════════════════════╝${NC}"
echo ""
print_info "This installer will set up HiveControl on your Raspberry Pi"
print_info "Installation will take approximately 30-60 minutes"
echo ""
print_warning "Make sure you have a stable internet connection"
echo ""
echo -n "Press ENTER to begin installation..."
read

# Parse options (same as original install.sh)
BEECOUNTER="false"
HIVECAM="false"
DEBUG="false"
XRDP="false"
KEYBOARD="false"

while getopts "h?bwdxk" opt; do
    case "$opt" in
    h|\?)
        echo "HiveControl Installation Options:"
        echo " -b  Use RaspiCam as a Beecounter Setup (Warning: Can take up to 8 hrs)"
        echo " -w  Use RaspiCam as Livestream"
        echo " -x  Install XRDP for Windows Users"
        echo " -k  Touch Screen Keyboard Install"
        echo " -h  This help message"
        echo " -d  Turn on Debugging"
        exit 0
        ;;
    b)  BEECOUNTER="true"
        ;;
    w)  HIVECAM="true"
        ;;
    d)  DEBUG="true"
        ;;
    x)  XRDP="true"
        ;;
    k)  KEYBOARD="true"
        ;;
    esac
done

if [[ $DEBUG == "true" ]]; then
    set -x
fi

# Start installation
print_header "Starting HiveControl Installation"

# Step 1: Download the main installer
print_info "Downloading HiveControl installer..."
cd /tmp
curl -sSL https://raw.githubusercontent.com/rcrum003/HiveControl/master/install.sh -o /tmp/hivecontrol-install.sh

if [ $? -ne 0 ]; then
    print_error "Failed to download installer"
    exit 1
fi

print_success "Installer downloaded"

# Step 2: Make it executable (handled automatically)
chmod +x /tmp/hivecontrol-install.sh

# Step 3: Build command with options
CMD="/tmp/hivecontrol-install.sh"
OPTIONS=""

if [[ $BEECOUNTER == "true" ]]; then
    OPTIONS="$OPTIONS -b"
fi

if [[ $HIVECAM == "true" ]]; then
    OPTIONS="$OPTIONS -w"
fi

if [[ $XRDP == "true" ]]; then
    OPTIONS="$OPTIONS -x"
fi

if [[ $KEYBOARD == "true" ]]; then
    OPTIONS="$OPTIONS -k"
fi

if [[ $DEBUG == "true" ]]; then
    OPTIONS="$OPTIONS -d"
fi

# Step 4: Run the installer
print_header "Running HiveControl Installer"

if [[ -n "$OPTIONS" ]]; then
    print_info "Installing with options:$OPTIONS"
fi

echo ""

# Run the actual installer
bash $CMD $OPTIONS

# Clean up
rm -f /tmp/hivecontrol-install.sh

print_success "Installation script completed!"
echo ""
print_info "Your Raspberry Pi will now reboot..."
echo ""
