#!/bin/bash
#
# Validate hiveconfig table schema and add any missing columns.
#
# Called by upgrade.sh and upgrade-dev.sh after the patch chain runs.
# This is a safety net: if a hive skipped a version, a patch failed
# silently, or a new column was added without a sequential patch, this
# script ensures the DB matches what the code expects.
#
# Idempotent — safe to run multiple times.

DB="${1:-/home/HiveControl/data/hive-data.db}"

if [ ! -f "$DB" ]; then
    echo "schema_validate: database not found at $DB"
    exit 1
fi

EXISTING=$(sqlite3 "$DB" "PRAGMA table_info(hiveconfig);" | awk -F'|' '{print $2}')

ensure_column() {
    local col="$1"
    local typedef="$2"
    if ! echo "$EXISTING" | grep -qxi "$col"; then
        echo "  Adding missing column: $col"
        sqlite3 "$DB" "ALTER TABLE hiveconfig ADD COLUMN $col $typedef;"
    fi
}

echo "Validating hiveconfig schema..."

# Weight compensation (DB_PATCH_35–37 era)
ensure_column "WEIGHT_COMPENSATION_ENABLED" "TEXT DEFAULT 'no'"
ensure_column "WEIGHT_TEMP_COEFF"           "REAL DEFAULT 0"
ensure_column "WEIGHT_HUMIDITY_COEFF"       "REAL DEFAULT 0"
ensure_column "WEIGHT_REF_TEMP"             "REAL DEFAULT NULL"
ensure_column "WEIGHT_REF_HUMIDITY"         "REAL DEFAULT NULL"
ensure_column "WEIGHT_MONITOR_INTERVAL"     "INTEGER DEFAULT 15"

# Weather fallback chain (DB_PATCH_38–39 era)
ensure_column "WEATHER_FALLBACK"       "TEXT DEFAULT ''"
ensure_column "WEATHER_FALLBACK_2"     "TEXT DEFAULT ''"
ensure_column "WX_MAX_STALE_MINUTES"   "INTEGER DEFAULT 120"

# Multi-provider API keys (DB_PATCH_40 era)
ensure_column "KEY_OPENWEATHERMAP"  "TEXT DEFAULT ''"
ensure_column "KEY_WEATHERAPI"      "TEXT DEFAULT ''"
ensure_column "KEY_VISUALCROSSING"  "TEXT DEFAULT ''"
ensure_column "KEY_PIRATEWEATHER"   "TEXT DEFAULT ''"
ensure_column "KEY_TOMORROW"        "TEXT DEFAULT ''"
ensure_column "KEY_AMBEE"           "TEXT DEFAULT ''"

# EPA AirNow (DB_PATCH_40 era)
ensure_column "KEY_AIRNOW"      "TEXT DEFAULT ''"
ensure_column "AIRNOW_DISTANCE" "INTEGER DEFAULT 25"
ensure_column "ENABLE_AIRNOW"   "TEXT DEFAULT 'no'"

# Alert thresholds
ensure_column "alert_weight_loss_threshold" "REAL DEFAULT 1.0"
ensure_column "alert_weight_loss_hours"     "INTEGER DEFAULT 6"
ensure_column "alert_swarm_threshold"       "REAL DEFAULT 3.0"
ensure_column "alert_high_temp"             "REAL DEFAULT 100.0"
ensure_column "alert_low_temp"              "REAL DEFAULT 40.0"
ensure_column "alert_stale_minutes"         "INTEGER DEFAULT 30"
ensure_column "alert_flow_daily_gain"       "REAL DEFAULT 0.5"
ensure_column "alert_flow_days"             "INTEGER DEFAULT 3"
ensure_column "alerts_enabled"              "TEXT DEFAULT 'on'"
ensure_column "alert_pm25_threshold"        "NUMERIC DEFAULT 35.5"
ensure_column "alert_o3_threshold"          "NUMERIC DEFAULT 100"
ensure_column "alert_smoke_aqi_threshold"   "INTEGER DEFAULT 150"

# Hive body stack order / sensor placement (DB_PATCH_42)
ensure_column "HIVE_STACK_ORDER"       "TEXT DEFAULT ''"
ensure_column "SENSOR_TEMP_POSITION"   "INTEGER DEFAULT -1"
ensure_column "SENSOR_TEMP_LABEL"      "TEXT DEFAULT 'Hive Temp'"
ensure_column "FEEDER_HAS_SYRUP"       "INTEGER DEFAULT 0"

# WX Underground API key (DB_PATCH_43)
ensure_column "WXAPIKEY" "TEXT DEFAULT ''"

# Pollen enable/disable (DB_PATCH_44)
ensure_column "ENABLE_POLLEN" "TEXT DEFAULT 'yes'"

# Chart display columns
ensure_column "color_pollen"    "TEXT DEFAULT '#66BB6A'"
ensure_column "trend_pollen"    "TEXT DEFAULT 'off'"
ensure_column "color_pm10"      "TEXT DEFAULT '#FF8C00'"
ensure_column "color_o3"        "TEXT DEFAULT '#9370DB'"
ensure_column "color_no2"       "TEXT DEFAULT '#20B2AA'"
ensure_column "trend_pm10"      "TEXT DEFAULT 'off'"
ensure_column "trend_o3"        "TEXT DEFAULT 'off'"
ensure_column "trend_no2"       "TEXT DEFAULT 'off'"
ensure_column "color_pm2_5"     "TEXT DEFAULT '#FF6347'"
ensure_column "color_pm2_5_aqi" "TEXT DEFAULT '#CC3333'"
ensure_column "color_pm10_aqi"  "TEXT DEFAULT '#CC6600'"
ensure_column "trend_pm2_5"     "TEXT DEFAULT 'off'"
ensure_column "trend_pm2_5_aqi" "TEXT DEFAULT 'off'"
ensure_column "trend_pm10_aqi"  "TEXT DEFAULT 'off'"

echo "Schema validation complete."
