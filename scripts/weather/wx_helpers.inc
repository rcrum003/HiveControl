#!/bin/bash
# Shared weather helper functions for all weather API scripts
# Provides unit conversion, wind direction, and WunderGround JSON output

# Convert wind degrees to cardinal direction
degrees_to_cardinal() {
	local deg=$1
	if [ -z "$deg" ] || [ "$deg" = "null" ]; then
		echo "null"
		return
	fi
	if [ 1 -eq "$(echo "$deg >= 0" | bc)" ] && [ 1 -eq "$(echo "$deg < 22.5" | bc)" ]; then
		echo "North"
	elif [ 1 -eq "$(echo "$deg >= 22.5" | bc)" ] && [ 1 -eq "$(echo "$deg < 67.5" | bc)" ]; then
		echo "NE"
	elif [ 1 -eq "$(echo "$deg >= 67.5" | bc)" ] && [ 1 -eq "$(echo "$deg < 112.5" | bc)" ]; then
		echo "East"
	elif [ 1 -eq "$(echo "$deg >= 112.5" | bc)" ] && [ 1 -eq "$(echo "$deg < 157.5" | bc)" ]; then
		echo "SE"
	elif [ 1 -eq "$(echo "$deg >= 157.5" | bc)" ] && [ 1 -eq "$(echo "$deg < 202.5" | bc)" ]; then
		echo "South"
	elif [ 1 -eq "$(echo "$deg >= 202.5" | bc)" ] && [ 1 -eq "$(echo "$deg < 247.5" | bc)" ]; then
		echo "SW"
	elif [ 1 -eq "$(echo "$deg >= 247.5" | bc)" ] && [ 1 -eq "$(echo "$deg < 292.5" | bc)" ]; then
		echo "West"
	elif [ 1 -eq "$(echo "$deg >= 292.5" | bc)" ] && [ 1 -eq "$(echo "$deg < 337.5" | bc)" ]; then
		echo "NW"
	else
		echo "North"
	fi
}

# Unit conversions - safe with empty/null inputs
safe_bc() {
	local expr=$1
	local result
	result=$(echo "$expr" | bc -l 2>/dev/null)
	if [ -z "$result" ]; then
		echo "0"
	else
		echo "$result"
	fi
}

f_to_c() { safe_bc "scale=1;(($1-32)*5)/9"; }
c_to_f() { safe_bc "scale=1;($1*9/5)+32"; }
mph_to_kph() { safe_bc "scale=2;($1 * 1.609344)/1"; }
kph_to_mph() { safe_bc "scale=2;($1 / 1.609344)/1"; }
ms_to_mph() { safe_bc "scale=2;($1 * 2.23694)/1"; }
ms_to_kph() { safe_bc "scale=2;($1 * 3.6)/1"; }
in_to_mb() { safe_bc "scale=0;($1 * 33.8637526)/1"; }
mb_to_in() { safe_bc "scale=2;($1 / 33.8637526)/1"; }
pa_to_mb() { safe_bc "scale=1;($1 / 100)/1"; }
pa_to_in() { safe_bc "scale=2;($1 / 3386.39)/1"; }
in_to_mm() { safe_bc "scale=2;($1 / 0.039370)/1"; }
mm_to_in() { safe_bc "scale=4;($1 * 0.039370)/1"; }

calc_dewpoint_f() {
	local temp_f=$1
	local humidity=$2
	safe_bc "scale=2;(${temp_f} - (9 * (100 - ${humidity})) / 25)"
}

# Check if an observation timestamp is too old (stale)
# Args: $1 = observation time string, $2 = max age in minutes (default 120)
# Returns: 0 if fresh, 1 if stale or unparseable
validate_observation_time() {
	local obs_time="$1"
	local max_age="${2:-120}"

	if [ -z "$obs_time" ]; then
		return 1
	fi

	local obs_epoch
	obs_epoch=$(date -d "$obs_time" +%s 2>/dev/null)
	if [ -z "$obs_epoch" ]; then
		obs_epoch=$(date --date="$obs_time" +%s 2>/dev/null)
	fi
	if [ -z "$obs_epoch" ]; then
		return 1
	fi

	local now_epoch
	now_epoch=$(date +%s)
	local age_minutes=$(( (now_epoch - obs_epoch) / 60 ))

	if ! [[ "$age_minutes" =~ ^-?[0-9]+$ ]]; then
		echo "0"
		return 1
	fi

	# Negative age means future timestamp — treat as 0 age (accept)
	if [ "$age_minutes" -lt 0 ]; then
		age_minutes=0
	fi

	if [ "$age_minutes" -gt "$max_age" ]; then
		echo "$age_minutes"
		return 1
	fi

	echo "$age_minutes"
	return 0
}

# Output WunderGround-format JSON
# Set these variables before calling:
#   WX_STATION_ID, WX_OBS_TIME, WX_TEMP_F, WX_TEMP_C, WX_HUMIDITY,
#   WX_WIND_DIR_TEXT, WX_WIND_DIR_DEG, WX_WIND_MPH, WX_WIND_GUST_MPH,
#   WX_WIND_KPH, WX_WIND_GUST_KPH, WX_PRESSURE_MB, WX_PRESSURE_IN,
#   WX_PRESSURE_TREND, WX_DEWPOINT_F, WX_DEWPOINT_C, WX_SOLARRADIATION,
#   WX_UV, WX_PRECIP_1HR_IN, WX_PRECIP_1HR_MM, WX_PRECIP_TODAY_IN,
#   WX_PRECIP_TODAY_MM
output_wx_json() {
	cat <<WXEOF
{
"response": {
"version":"0.1",
"features": {
"conditions": 1
  }
	}
  ,	"current_observation": {
		"station_id":"$WX_STATION_ID",
		"observation_time":"$WX_OBS_TIME",
		"temperature_string":"$WX_TEMP_F F ($WX_TEMP_C C)",
		"temp_f":"$WX_TEMP_F",
		"temp_c":"$WX_TEMP_C",
		"relative_humidity":"$WX_HUMIDITY%",
		"wind_string":" $WX_WIND_DIR_TEXT at $WX_WIND_MPH MPH, Gust to $WX_WIND_GUST_MPH MPH",
		"wind_dir":"$WX_WIND_DIR_TEXT",
		"wind_degrees":"$WX_WIND_DIR_DEG",
		"wind_mph":"$WX_WIND_MPH",
		"wind_gust_mph":"$WX_WIND_GUST_MPH",
		"wind_kph":"$WX_WIND_KPH",
		"wind_gust_kph":"$WX_WIND_GUST_KPH",
		"pressure_mb":"$WX_PRESSURE_MB",
		"pressure_in":"$WX_PRESSURE_IN",
		"pressure_trend":"$WX_PRESSURE_TREND",
		"dewpoint_f":"$WX_DEWPOINT_F",
		"dewpoint_c":"$WX_DEWPOINT_C",
		"windchill_string":"NA",
		"windchill_f":"NA",
		"windchill_c":"NA",
		"solarradiation":"$WX_SOLARRADIATION",
		"UV":"$WX_UV",
		"precip_1hr_in":"$WX_PRECIP_1HR_IN",
		"precip_1hr_metric":"$WX_PRECIP_1HR_MM",
		"precip_today_string":"$WX_PRECIP_TODAY_IN in ($WX_PRECIP_TODAY_MM mm)",
		"precip_today_in":"$WX_PRECIP_TODAY_IN",
		"precip_today_metric":"$WX_PRECIP_TODAY_MM"
 }
}
WXEOF
}
