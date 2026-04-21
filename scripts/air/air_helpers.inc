#!/bin/bash
# air_helpers.inc — EPA AQI conversion functions for air quality scripts
# Uses EPA breakpoint tables per 40 CFR Part 58, Appendix G
#
# AQI formula: AQI = ((AQI_hi - AQI_lo) / (BP_hi - BP_lo)) * (C - BP_lo) + AQI_lo

source /home/HiveControl/scripts/weather/wx_helpers.inc 2>/dev/null

# Compute AQI from concentration using breakpoint pairs
# Usage: compute_aqi <concentration> <bp_lo> <bp_hi> <aqi_lo> <aqi_hi>
compute_aqi() {
    local c="$1" bp_lo="$2" bp_hi="$3" aqi_lo="$4" aqi_hi="$5"
    if [ -z "$c" ] || [ "$c" = "null" ]; then
        echo "null"
        return
    fi
    local result
    result=$(echo "scale=0; (($aqi_hi - $aqi_lo) / ($bp_hi - $bp_lo)) * ($c - $bp_lo) + $aqi_lo" | bc -l 2>/dev/null)
    if [ -z "$result" ]; then
        echo "null"
    else
        printf "%.0f" "$result" 2>/dev/null || echo "null"
    fi
}

# PM2.5 (ug/m3, 24-hr) to AQI — EPA 2024 revised breakpoints
pm25_to_aqi() {
    local c="$1"
    if [ -z "$c" ] || [ "$c" = "null" ]; then echo "null"; return; fi

    local in_range
    in_range=$(echo "$c >= 0" | bc -l 2>/dev/null)
    if [ "$in_range" != "1" ]; then echo "null"; return; fi

    if [ 1 -eq "$(echo "$c <= 9.0" | bc)" ]; then
        compute_aqi "$c" 0 9.0 0 50
    elif [ 1 -eq "$(echo "$c <= 35.4" | bc)" ]; then
        compute_aqi "$c" 9.1 35.4 51 100
    elif [ 1 -eq "$(echo "$c <= 55.4" | bc)" ]; then
        compute_aqi "$c" 35.5 55.4 101 150
    elif [ 1 -eq "$(echo "$c <= 125.4" | bc)" ]; then
        compute_aqi "$c" 55.5 125.4 151 200
    elif [ 1 -eq "$(echo "$c <= 225.4" | bc)" ]; then
        compute_aqi "$c" 125.5 225.4 201 300
    elif [ 1 -eq "$(echo "$c <= 325.4" | bc)" ]; then
        compute_aqi "$c" 225.5 325.4 301 500
    else
        echo "500"
    fi
}

# PM10 (ug/m3, 24-hr) to AQI
pm10_to_aqi() {
    local c="$1"
    if [ -z "$c" ] || [ "$c" = "null" ]; then echo "null"; return; fi

    local in_range
    in_range=$(echo "$c >= 0" | bc -l 2>/dev/null)
    if [ "$in_range" != "1" ]; then echo "null"; return; fi

    if [ 1 -eq "$(echo "$c <= 54" | bc)" ]; then
        compute_aqi "$c" 0 54 0 50
    elif [ 1 -eq "$(echo "$c <= 154" | bc)" ]; then
        compute_aqi "$c" 55 154 51 100
    elif [ 1 -eq "$(echo "$c <= 254" | bc)" ]; then
        compute_aqi "$c" 155 254 101 150
    elif [ 1 -eq "$(echo "$c <= 354" | bc)" ]; then
        compute_aqi "$c" 255 354 151 200
    elif [ 1 -eq "$(echo "$c <= 424" | bc)" ]; then
        compute_aqi "$c" 355 424 201 300
    elif [ 1 -eq "$(echo "$c <= 604" | bc)" ]; then
        compute_aqi "$c" 425 604 301 500
    else
        echo "500"
    fi
}

# O3 (ppm, 8-hr average) to AQI
o3_to_aqi() {
    local c="$1"
    if [ -z "$c" ] || [ "$c" = "null" ]; then echo "null"; return; fi

    local in_range
    in_range=$(echo "$c >= 0" | bc -l 2>/dev/null)
    if [ "$in_range" != "1" ]; then echo "null"; return; fi

    if [ 1 -eq "$(echo "$c <= 0.054" | bc)" ]; then
        compute_aqi "$c" 0 0.054 0 50
    elif [ 1 -eq "$(echo "$c <= 0.070" | bc)" ]; then
        compute_aqi "$c" 0.055 0.070 51 100
    elif [ 1 -eq "$(echo "$c <= 0.085" | bc)" ]; then
        compute_aqi "$c" 0.071 0.085 101 150
    elif [ 1 -eq "$(echo "$c <= 0.105" | bc)" ]; then
        compute_aqi "$c" 0.086 0.105 151 200
    elif [ 1 -eq "$(echo "$c <= 0.200" | bc)" ]; then
        compute_aqi "$c" 0.106 0.200 201 300
    else
        echo "500"
    fi
}

# NO2 (ppb, 1-hr) to AQI
no2_to_aqi() {
    local c="$1"
    if [ -z "$c" ] || [ "$c" = "null" ]; then echo "null"; return; fi

    local in_range
    in_range=$(echo "$c >= 0" | bc -l 2>/dev/null)
    if [ "$in_range" != "1" ]; then echo "null"; return; fi

    if [ 1 -eq "$(echo "$c <= 53" | bc)" ]; then
        compute_aqi "$c" 0 53 0 50
    elif [ 1 -eq "$(echo "$c <= 100" | bc)" ]; then
        compute_aqi "$c" 54 100 51 100
    elif [ 1 -eq "$(echo "$c <= 360" | bc)" ]; then
        compute_aqi "$c" 101 360 101 150
    elif [ 1 -eq "$(echo "$c <= 649" | bc)" ]; then
        compute_aqi "$c" 361 649 151 200
    elif [ 1 -eq "$(echo "$c <= 1249" | bc)" ]; then
        compute_aqi "$c" 650 1249 201 300
    elif [ 1 -eq "$(echo "$c <= 2049" | bc)" ]; then
        compute_aqi "$c" 1250 2049 301 500
    else
        echo "500"
    fi
}

# AQI to category name
aqi_to_category() {
    local aqi="$1"
    if [ -z "$aqi" ] || [ "$aqi" = "null" ]; then echo "Unknown"; return; fi

    if [ "$aqi" -le 50 ] 2>/dev/null; then
        echo "Good"
    elif [ "$aqi" -le 100 ]; then
        echo "Moderate"
    elif [ "$aqi" -le 150 ]; then
        echo "Unhealthy for Sensitive Groups"
    elif [ "$aqi" -le 200 ]; then
        echo "Unhealthy"
    elif [ "$aqi" -le 300 ]; then
        echo "Very Unhealthy"
    else
        echo "Hazardous"
    fi
}

# AQI to hex color (EPA standard colors)
aqi_to_color() {
    local aqi="$1"
    if [ -z "$aqi" ] || [ "$aqi" = "null" ]; then echo "#999999"; return; fi

    if [ "$aqi" -le 50 ] 2>/dev/null; then
        echo "#00E400"
    elif [ "$aqi" -le 100 ]; then
        echo "#FFFF00"
    elif [ "$aqi" -le 150 ]; then
        echo "#FF7E00"
    elif [ "$aqi" -le 200 ]; then
        echo "#FF0000"
    elif [ "$aqi" -le 300 ]; then
        echo "#8F3F97"
    else
        echo "#7E0023"
    fi
}
