#!/bin/bash
# One-time migration (HiveControl 2.46): send this hive's equipment to the
# cloud, which manages it from now on.
#
# Uploads what the Hive Body page stored locally — stack order, box counts,
# part weights, temperature sensor and frame feeder positions, syrup flag —
# to $POST_EQUIPMENT_URL. The cloud imports it only if it has no equipment
# for this hive yet (a hive already set up in the cloud keeps the cloud's).
# Afterwards hiveconfig.sh downloads the weights with the rest of the config,
# so getweight.sh subtracts the cloud's tare.
#
# Runs from upgrade.sh and, until it succeeds, from hiveconfig.sh.
# Leaves $EQUIPMENT_FLAG once the cloud has the equipment.

source /home/HiveControl/scripts/data/logger.inc
source /home/HiveControl/scripts/data/cloud.inc

LOCALDATABASE="/home/HiveControl/data/hive-data.db"
EQUIPMENT_FLAG="/home/HiveControl/data/.equipment_in_cloud"

if [ -f "$EQUIPMENT_FLAG" ]; then
	exit 0
fi

DATE=$(date '+%F %T')
q() { sqlite3 "$LOCALDATABASE" "$1" 2>/dev/null; }

HIVEAPI=$(q "SELECT HIVEAPI FROM hiveconfig;")
HIVEID=$(q "SELECT HIVEID FROM hiveconfig;")
HIVENAME=$(q "SELECT HIVENAME FROM hiveconfig;")
if [ -z "$HIVEAPI" ] || [ -z "$HIVENAME" ]; then
	echo "Not connected to hivecontrol.org; equipment stays local for now"
	exit 0
fi

# Older installs only have this table once the Hive Body page was opened.
q "CREATE TABLE IF NOT EXISTS hiveequipmentweight (
	HIVE_BASE_SOLID_BOTTOM_BOARD_WEIGHT REAL DEFAULT 0, HIVE_BASE_SCREENED_BOTTOM_BOARD_WEIGHT REAL DEFAULT 0,
	HIVE_FEEDER_WEIGHT REAL DEFAULT 0, HIVE_TOP_INNER_COVER_WEIGHT REAL DEFAULT 0,
	HIVE_TOP_TELE_COVER_WEIGHT REAL DEFAULT 0, HIVE_TOP_MIGRATORY_COVER_WEIGHT REAL DEFAULT 0,
	HIVE_BODY_MEDIUM_FOUNDATION_WEIGHT REAL DEFAULT 0, HIVE_BODY_MEDIUM_FOUNDATION_LESS_WEIGHT REAL DEFAULT 0,
	HIVE_BODY_DEEP_FOUNDATION_WEIGHT REAL DEFAULT 0, HIVE_BODY_DEEP_FOUNDATION_LESS_WEIGHT REAL DEFAULT 0,
	HIVE_BODY_SHAL_FOUNDATION_WEIGHT REAL DEFAULT 0, HIVE_BODY_SHAL_FOUNDATION_LESS_WEIGHT REAL DEFAULT 0,
	HIVE_TOP_WEIGHT REAL DEFAULT 0, HIVE_COMPUTER_WEIGHT REAL DEFAULT 0, HIVE_MISC_WEIGHT REAL DEFAULT 0, id INTEGER DEFAULT 1);"

CONFIG_FIELDS="NUM_HIVE_BASE_SOLID_BOTTOM_BOARD NUM_HIVE_BASE_SCREENED_BOTTOM_BOARD NUM_HIVE_FEEDER
	NUM_HIVE_TOP_INNER_COVER NUM_HIVE_TOP_TELE_COVER NUM_HIVE_TOP_MIGRATORY_COVER
	NUM_HIVE_BODY_DEEP_FOUNDATION NUM_HIVE_BODY_DEEP_FOUNDATION_LESS
	NUM_HIVE_BODY_MEDIUM_FOUNDATION NUM_HIVE_BODY_MEDIUM_FOUNDATION_LESS
	NUM_HIVE_BODY_SHAL_FOUNDATION NUM_HIVE_BODY_SHAL_FOUNDATION_LESS
	HIVE_STACK_ORDER SENSOR_TEMP_POSITION SENSOR_TEMP_LABEL FEEDER_HAS_SYRUP
	FRAME_FEEDER_POSITION FRAME_FEEDER_LABEL"
WEIGHT_FIELDS="HIVE_BASE_SOLID_BOTTOM_BOARD_WEIGHT HIVE_BASE_SCREENED_BOTTOM_BOARD_WEIGHT HIVE_FEEDER_WEIGHT
	HIVE_TOP_INNER_COVER_WEIGHT HIVE_TOP_TELE_COVER_WEIGHT HIVE_TOP_MIGRATORY_COVER_WEIGHT
	HIVE_BODY_DEEP_FOUNDATION_WEIGHT HIVE_BODY_MEDIUM_FOUNDATION_WEIGHT HIVE_BODY_SHAL_FOUNDATION_WEIGHT
	HIVE_BODY_DEEP_FOUNDATION_LESS_WEIGHT HIVE_BODY_MEDIUM_FOUNDATION_LESS_WEIGHT HIVE_BODY_SHAL_FOUNDATION_LESS_WEIGHT
	HIVE_TOP_WEIGHT HIVE_COMPUTER_WEIGHT HIVE_MISC_WEIGHT"

# curl -F arguments; a column an older install lacks is simply not sent.
ARGS=(-F "api_token=$HIVEAPI" -F "hive_id=$HIVEID" -F "hivename=$HIVENAME")
for f in $CONFIG_FIELDS; do
	v=$(q "SELECT $f FROM hiveconfig WHERE id=1;")
	[ -n "$v" ] && ARGS+=(-F "$f=$v")
done
for f in $WEIGHT_FIELDS; do
	v=$(q "SELECT $f FROM hiveequipmentweight WHERE id=1;")
	[ -n "$v" ] && ARGS+=(-F "$f=$v")
done

RESPONSE=$(/usr/bin/curl --silent --retry 5 -X POST -H "Content-Type: multipart/form-data" -H "X-Requested-With: XMLHttpRequest" "${ARGS[@]}" "$POST_EQUIPMENT_URL")
RESULT=$(echo "$RESPONSE" | jq -r '.result // empty' 2>/dev/null)

case "$RESULT" in
	SUCCESS)
		loglocal "$DATE" CONFIG INFO "Sent hive equipment and weights to hivecontrol.org; they are managed there now"
		touch "$EQUIPMENT_FLAG"
		;;
	EXISTS)
		loglocal "$DATE" CONFIG INFO "hivecontrol.org already has this hive's equipment; using the cloud's weights"
		touch "$EQUIPMENT_FLAG"
		;;
	INVALID_DATA)
		# Nothing configured on the Hive Body page: there's nothing to move.
		loglocal "$DATE" CONFIG INFO "No local hive equipment to send; manage it at hivecontrol.org"
		touch "$EQUIPMENT_FLAG"
		;;
	*)
		# Offline, bad token or stale hive id: try again on the next config check.
		# (loglocal writes a double-quoted SQL string, so the reply loses its quotes.)
		SAFE_REPLY=$(echo "${RESPONSE:-no response}" | tr -d '"\\' | cut -c1-200)
		loglocal "$DATE" CONFIG WARNING "Could not send hive equipment to hivecontrol.org (will retry): $SAFE_REPLY"
		exit 1
		;;
esac
exit 0
