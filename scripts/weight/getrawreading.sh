#!/bin/bash
# Return a raw (uncalibrated) sensor reading for scale calibration
# Usage: getrawreading.sh <scaletype>
# Output: single numeric value (raw ADC reading)

SCALETYPE=$1

if [ -z "$SCALETYPE" ]; then
    echo "ERROR: scale type required (hx711, phidget1046)"
    exit 1
fi

# Detect Raspberry Pi version and OS architecture for HX711 script selection
PI_MODEL=$(tr -d '\0' < /proc/device-tree/model 2>/dev/null || echo "Unknown")
PI_VERSION=$(echo "$PI_MODEL" | grep -oP 'Raspberry Pi \K\d+' || echo "0")
OS_ARCH=$(uname -m)

case "$SCALETYPE" in
    hx711)
        # Pi 5+ requires lgpio; 64-bit OS on any Pi also needs lgpio (pigpiod can't access GPIO on 64-bit kernels)
        if [ "$PI_VERSION" -ge 5 ] || [ "$OS_ARCH" = "aarch64" ] || [ "$OS_ARCH" = "arm64" ]; then
            HX711_SCRIPT="/home/HiveControl/scripts/weight/HX711_lgpio.py"
        else
            HX711_SCRIPT="/home/HiveControl/scripts/weight/HX711.py"
        fi

        DATA_GOOD=0
        COUNTER=1
        while [ $COUNTER -lt 5 ] && [ $DATA_GOOD -eq 0 ]; do
            RAW=$(/usr/bin/python3 "$HX711_SCRIPT" 2>/dev/null)
            if [[ $RAW =~ ^-?[0-9]+$ ]]; then
                DATA_GOOD=1
            fi
            let "COUNTER += 1"
        done

        if [ $DATA_GOOD -eq 1 ]; then
            echo "$RAW"
        else
            echo "ERROR: could not get a valid reading after multiple attempts"
            exit 1
        fi
        ;;

    phidget1046)
        SCALE=$(/usr/bin/python3 /home/HiveControl/scripts/weight/getrawphidget.py 2>/dev/null)
        INPUT_0=$(echo "$SCALE" | grep -o "Weight0 = \-*[0-9]*\.[0-9]*" | grep -o "\-*[0-9]*\.[0-9]*")
        INPUT_1=$(echo "$SCALE" | grep -o "Weight1 = \-*[0-9]*\.[0-9]*" | grep -o "\-*[0-9]*\.[0-9]*")
        INPUT_2=$(echo "$SCALE" | grep -o "Weight2 = \-*[0-9]*\.[0-9]*" | grep -o "\-*[0-9]*\.[0-9]*")
        INPUT_3=$(echo "$SCALE" | grep -o "Weight3 = \-*[0-9]*\.[0-9]*" | grep -o "\-*[0-9]*\.[0-9]*")

        if [ -z "$INPUT_0" ] && [ -z "$INPUT_1" ] && [ -z "$INPUT_2" ] && [ -z "$INPUT_3" ]; then
            echo "ERROR: no data from Phidget sensor"
            exit 1
        fi

        INPUT_0=${INPUT_0:-0}
        INPUT_1=${INPUT_1:-0}
        INPUT_2=${INPUT_2:-0}
        INPUT_3=${INPUT_3:-0}

        RAWVALUE=$(echo "scale=5; ($INPUT_0 + $INPUT_1 + $INPUT_2 + $INPUT_3)" | bc)
        echo "$RAWVALUE"
        ;;

    *)
        echo "ERROR: unsupported scale type for calibration: $SCALETYPE"
        exit 1
        ;;
esac
