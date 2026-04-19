#!/bin/bash
# Regenerate hiveconfig.inc from the database
# Lightweight version of dump_config_to_file() from hiveconfig.sh
# Called by the web UI when config changes need to be reflected in sensor scripts immediately

LOCALDATABASE="/home/HiveControl/data/hive-data.db"
CONFIGOUT="/home/HiveControl/scripts/hiveconfig.inc"
TEMPFILE="/tmp/hiveconfig_dump_$$"

sqlite3 -header -line "$LOCALDATABASE" "SELECT * from hiveconfig LEFT JOIN hiveequipmentweight on hiveconfig.id=hiveequipmentweight.id;" | sort | uniq > "$TEMPFILE"

if [ ! -s "$TEMPFILE" ]; then
    rm -f "$TEMPFILE"
    exit 1
fi

awk '{ gsub(/ = /, "=\""); print }' "$TEMPFILE" | sed 's/^ *//g' | awk '{print $0"\""}' > "$CONFIGOUT"
rm -f "$TEMPFILE"

if [ ! -s "$CONFIGOUT" ]; then
    exit 1
fi
