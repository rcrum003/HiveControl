#!/bin/bash
# High-frequency weight monitoring script
# Runs independently of currconditions.sh on a shorter interval (e.g. every 15 min)
# Only reads weight sensor + cached ambient weather data — no API calls
#
# Version 2026041901

set -eo pipefail

HOMEDIR="${HOMEDIR:-/home/HiveControl}"
LOCALDATABASE="$HOMEDIR/data/hive-data.db"
WEIGHTRUNDIR="$HOMEDIR/scripts/weight"
LOCKFILE="/tmp/hivecontrol_weight.lock"

source "$HOMEDIR/scripts/hiveconfig.inc"
source "$HOMEDIR/scripts/data/logger.inc"
source "$HOMEDIR/scripts/data/check.inc"

DATE=$(TZ=":$TIMEZONE" date '+%F %T')
DATE_UTC=$(TZ=":${UTC:-UTC}" date '+%F %T')

# Escape single quotes for SQL by doubling them
function sql_escape() {
    echo "${1//\'/\'\'}"
}

# Check feature gate before doing any work
if [ "$ENABLE_HIVE_WEIGHT_CHK" != "yes" ]; then
    exit 0
fi

# Ensure weight_readings table exists
sqlite3 "$LOCALDATABASE" "CREATE TABLE IF NOT EXISTS weight_readings (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    hiveid TEXT,
    date TEXT,
    date_utc TEXT,
    raw_weight REAL,
    net_weight REAL,
    compensated_weight REAL,
    ambient_temp_f REAL,
    ambient_humidity REAL,
    temp_source TEXT DEFAULT 'weather_cache',
    sample_count INTEGER DEFAULT 50,
    sync INTEGER DEFAULT 0
);" 2>/dev/null

# Use flock to prevent concurrent reads on the HX711.
# currconditions.sh acquires the same lock in get_hive_weight(),
# so we skip if it is currently running a weight read.
exec 200>"$LOCKFILE"
if ! flock -n 200; then
    loglocal "$DATE" WEIGHT_MONITOR INFO "Skipping — another weight read is in progress"
    exit 0
fi

# Get weight reading via existing getweight.sh
WEIGHT_OUTPUT=$("$WEIGHTRUNDIR/getweight.sh" 2>&1) || {
    loglocal "$DATE" WEIGHT_MONITOR ERROR "Failed to get weight: $WEIGHT_OUTPUT"
    exit 1
}

read -r RAWWEIGHT NETWEIGHT <<< "$WEIGHT_OUTPUT"

check RAWWEIGHT
check NETWEIGHT

if [[ -z "$RAWWEIGHT" ]] || [[ "$RAWWEIGHT" == "0" && "$NETWEIGHT" == "0" ]]; then
    loglocal "$DATE" WEIGHT_MONITOR ERROR "Invalid weight reading: raw=$RAWWEIGHT net=$NETWEIGHT"
    exit 1
fi

# Get cached ambient weather data from most recent allhivedata record
AMBIENT_DATA=$(sqlite3 "$LOCALDATABASE" \
    "SELECT weather_tempf, weather_humidity FROM allhivedata
     WHERE weather_tempf IS NOT NULL AND weather_tempf != 'null'
       AND weather_humidity IS NOT NULL AND weather_humidity != 'null'
     ORDER BY date DESC LIMIT 1;" 2>/dev/null)

AMBIENT_TEMP="null"
AMBIENT_HUMIDITY="null"
TEMP_SOURCE="weather_cache"

if [[ -n "$AMBIENT_DATA" ]]; then
    IFS='|' read -r AMBIENT_TEMP AMBIENT_HUMIDITY <<< "$AMBIENT_DATA"
fi

# Apply environmental compensation if enabled
COMPENSATED_WEIGHT="$NETWEIGHT"
re_num='^-?[0-9]+([.][0-9]+)?$'

if [ "${WEIGHT_COMPENSATION_ENABLED:-no}" = "yes" ]; then
    TEMP_COEFF="${WEIGHT_TEMP_COEFF:-0}"
    HUMIDITY_COEFF="${WEIGHT_HUMIDITY_COEFF:-0}"
    REF_TEMP="${WEIGHT_REF_TEMP:-0}"
    REF_HUMIDITY="${WEIGHT_REF_HUMIDITY:-0}"

    # Validate all compensation inputs are numeric before calling bc
    [[ "$TEMP_COEFF" =~ $re_num ]] || TEMP_COEFF="0"
    [[ "$HUMIDITY_COEFF" =~ $re_num ]] || HUMIDITY_COEFF="0"
    [[ "$REF_TEMP" =~ $re_num ]] || REF_TEMP="0"
    [[ "$REF_HUMIDITY" =~ $re_num ]] || REF_HUMIDITY="0"

    if [[ "$AMBIENT_TEMP" != "null" ]] && [[ "$AMBIENT_HUMIDITY" != "null" ]] \
       && [[ "$AMBIENT_TEMP" =~ $re_num ]] && [[ "$AMBIENT_HUMIDITY" =~ $re_num ]]; then
        COMPENSATED_WEIGHT=$(echo "scale=4; $NETWEIGHT - ($TEMP_COEFF * ($AMBIENT_TEMP - $REF_TEMP)) - ($HUMIDITY_COEFF * ($AMBIENT_HUMIDITY - $REF_HUMIDITY))" | bc)
        COMPENSATED_WEIGHT=$(echo "scale=2; $COMPENSATED_WEIGHT / 1" | bc)
    fi
fi

# Store in weight_readings table with proper escaping
sqlite3 "$LOCALDATABASE" "INSERT INTO weight_readings (
    hiveid, date, date_utc, raw_weight, net_weight, compensated_weight,
    ambient_temp_f, ambient_humidity, temp_source, sample_count, sync
) VALUES (
    '$(sql_escape "$HIVEID")',
    '$(sql_escape "$DATE")',
    '$(sql_escape "$DATE_UTC")',
    '$(sql_escape "$RAWWEIGHT")',
    '$(sql_escape "$NETWEIGHT")',
    '$(sql_escape "$COMPENSATED_WEIGHT")',
    '$(sql_escape "$AMBIENT_TEMP")',
    '$(sql_escape "$AMBIENT_HUMIDITY")',
    '$(sql_escape "$TEMP_SOURCE")',
    50,
    0
);"

# Feed calibration data if calibration is running
CALIBRATION_FLAG="$HOMEDIR/scripts/weight/.calibrating_compensation"
if [ -f "$CALIBRATION_FLAG" ]; then
    "$WEIGHTRUNDIR/calibrate_env_compensation.sh" collect \
        "$RAWWEIGHT" "$NETWEIGHT" "$AMBIENT_TEMP" "$AMBIENT_HUMIDITY" "$TEMP_SOURCE"
fi

echo "$DATE | raw=$RAWWEIGHT net=$NETWEIGHT comp=$COMPENSATED_WEIGHT temp=$AMBIENT_TEMP hum=$AMBIENT_HUMIDITY"
