#!/bin/bash
# Environmental compensation calibration script
# Collects weight + ambient data over time with a known static weight on the scale.
# After collection, run compute_compensation.py to calculate coefficients.
#
# Usage:
#   ./calibrate_env_compensation.sh start    — begin collecting calibration data
#   ./calibrate_env_compensation.sh stop     — stop collection and compute coefficients
#   ./calibrate_env_compensation.sh status   — show how many samples collected
#
# Place a known, unchanging weight on the scale before starting.
# Let it run for 48-72 hours to capture a full range of temp/humidity conditions.
#
# Version 2026041901

set -eo pipefail

HOMEDIR="${HOMEDIR:-/home/HiveControl}"
LOCALDATABASE="$HOMEDIR/data/hive-data.db"
WEIGHTRUNDIR="$HOMEDIR/scripts/weight"
CALIBRATION_FLAG="$HOMEDIR/scripts/weight/.calibrating_compensation"

source "$HOMEDIR/scripts/hiveconfig.inc"
source "$HOMEDIR/scripts/data/logger.inc"
source "$HOMEDIR/scripts/data/check.inc"

DATE=$(TZ=":$TIMEZONE" date '+%F %T')

# Ensure calibration_data table exists
sqlite3 "$LOCALDATABASE" "CREATE TABLE IF NOT EXISTS calibration_data (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    date TEXT,
    raw_weight REAL,
    net_weight REAL,
    ambient_temp_f REAL,
    ambient_humidity REAL,
    temp_source TEXT DEFAULT 'weather_cache'
);" 2>/dev/null

ACTION="${1:-status}"

case "$ACTION" in

    start)
        echo "============================================"
        echo "Environmental Compensation Calibration"
        echo "============================================"
        echo ""
        echo "IMPORTANT: Place a known, static weight on the scale."
        echo "Do NOT add or remove anything during calibration."
        echo ""
        echo "This will collect a sample every 15 minutes."
        echo "Let it run for at least 48 hours (ideally 72) to"
        echo "capture a full range of temperature and humidity."
        echo ""
        read -p "Is the static weight on the scale? (yes/no): " CONFIRM
        if [ "$CONFIRM" != "yes" ]; then
            echo "Aborted. Place the weight and try again."
            exit 1
        fi

        # Clear old calibration data
        sqlite3 "$LOCALDATABASE" "DELETE FROM calibration_data;"
        echo "Cleared previous calibration data."

        # Set the flag file
        touch "$CALIBRATION_FLAG"
        echo "Calibration started at $DATE"
        echo "$DATE" > "$CALIBRATION_FLAG"

        echo ""
        echo "Calibration is now running. The weight_monitor.sh cron job"
        echo "will automatically collect calibration samples."
        echo ""
        echo "Run './calibrate_env_compensation.sh status' to check progress."
        echo "Run './calibrate_env_compensation.sh stop' when done (48-72 hrs)."
        ;;

    collect)
        # Called internally by weight_monitor.sh when calibration flag is set
        if [ ! -f "$CALIBRATION_FLAG" ]; then
            exit 0
        fi

        RAWWEIGHT="$2"
        NETWEIGHT="$3"
        AMBIENT_TEMP="$4"
        AMBIENT_HUMIDITY="$5"
        TEMP_SOURCE="${6:-weather_cache}"

        if [[ -z "$RAWWEIGHT" ]] || [[ "$AMBIENT_TEMP" == "null" ]] || [[ "$AMBIENT_HUMIDITY" == "null" ]]; then
            echo "Skipping calibration sample — missing data"
            exit 0
        fi

        # Escape single quotes for SQL safety
        sql_esc() { echo "${1//\'/\'\'}"; }

        sqlite3 "$LOCALDATABASE" "INSERT INTO calibration_data (
            date, raw_weight, net_weight, ambient_temp_f, ambient_humidity, temp_source
        ) VALUES (
            '$(sql_esc "$DATE")', '$(sql_esc "$RAWWEIGHT")', '$(sql_esc "$NETWEIGHT")',
            '$(sql_esc "$AMBIENT_TEMP")', '$(sql_esc "$AMBIENT_HUMIDITY")', '$(sql_esc "$TEMP_SOURCE")'
        );"

        SAMPLE_COUNT=$(sqlite3 "$LOCALDATABASE" "SELECT COUNT(*) FROM calibration_data;")
        echo "Calibration sample $SAMPLE_COUNT recorded"
        ;;

    stop)
        if [ ! -f "$CALIBRATION_FLAG" ]; then
            echo "Calibration is not currently running."
            exit 1
        fi

        START_DATE=$(cat "$CALIBRATION_FLAG")
        SAMPLE_COUNT=$(sqlite3 "$LOCALDATABASE" "SELECT COUNT(*) FROM calibration_data;")

        echo "============================================"
        echo "Stopping Calibration"
        echo "============================================"
        echo "Started: $START_DATE"
        echo "Samples collected: $SAMPLE_COUNT"
        echo ""

        if [ "$SAMPLE_COUNT" -lt 20 ]; then
            echo "WARNING: Only $SAMPLE_COUNT samples collected."
            echo "At least 96 samples (24 hours at 15-min intervals) recommended."
            read -p "Continue anyway? (yes/no): " CONFIRM
            if [ "$CONFIRM" != "yes" ]; then
                echo "Calibration still running. Collect more data."
                exit 0
            fi
        fi

        rm -f "$CALIBRATION_FLAG"
        echo "Calibration data collection stopped."
        echo ""
        echo "Computing compensation coefficients..."

        /usr/bin/python3 "$WEIGHTRUNDIR/compute_compensation.py" "$LOCALDATABASE"
        ;;

    status)
        if [ -f "$CALIBRATION_FLAG" ]; then
            START_DATE=$(cat "$CALIBRATION_FLAG")
            SAMPLE_COUNT=$(sqlite3 "$LOCALDATABASE" "SELECT COUNT(*) FROM calibration_data;" 2>/dev/null || echo "0")
            HOURS_RUNNING=$(( ( $(date +%s) - $(date -d "$START_DATE" +%s 2>/dev/null || echo "0") ) / 3600 ))

            echo "Calibration is RUNNING"
            echo "  Started: $START_DATE"
            echo "  Samples: $SAMPLE_COUNT"
            echo "  Hours:   ~$HOURS_RUNNING"

            if [ "$SAMPLE_COUNT" -gt 0 ]; then
                echo ""
                echo "Weight range observed:"
                sqlite3 "$LOCALDATABASE" "SELECT
                    printf('  Min: %.2f lbs', MIN(net_weight)),
                    printf('  Max: %.2f lbs', MAX(net_weight)),
                    printf('  Drift: %.2f lbs', MAX(net_weight) - MIN(net_weight))
                FROM calibration_data;" 2>/dev/null | tr '|' '\n'
                echo ""
                echo "Temperature range:"
                sqlite3 "$LOCALDATABASE" "SELECT
                    printf('  Min: %.1f F', MIN(ambient_temp_f)),
                    printf('  Max: %.1f F', MAX(ambient_temp_f))
                FROM calibration_data WHERE ambient_temp_f IS NOT NULL;" 2>/dev/null | tr '|' '\n'
                echo ""
                echo "Humidity range:"
                sqlite3 "$LOCALDATABASE" "SELECT
                    printf('  Min: %.1f%%', MIN(ambient_humidity)),
                    printf('  Max: %.1f%%', MAX(ambient_humidity))
                FROM calibration_data WHERE ambient_humidity IS NOT NULL;" 2>/dev/null | tr '|' '\n'
            fi
        else
            echo "Calibration is NOT running."
            echo "Use './calibrate_env_compensation.sh start' to begin."
        fi
        ;;

    *)
        echo "Usage: $0 {start|stop|status}"
        exit 1
        ;;
esac
