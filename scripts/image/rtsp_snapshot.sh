#!/bin/bash
# Capture a single frame from an RTSP stream for dashboard snapshot

source /home/HiveControl/scripts/hiveconfig.inc

if [ "$CAMERATYPE" != "RTSP" ] || [ -z "$CAMERA_RTSP_URL" ]; then
    exit 0
fi

OUTPUT="$PUBLIC_HTML_DIR/images/hive_snapshot.jpg"
TMPFILE=$(mktemp /tmp/rtsp_snap_XXXXXX.jpg)
trap 'rm -f "$TMPFILE"' EXIT

/usr/bin/ffmpeg -hide_banner -loglevel error \
    -rtsp_transport tcp \
    -timeout 10000000 \
    -i "$CAMERA_RTSP_URL" \
    -frames:v 1 \
    -q:v 2 \
    -y "$TMPFILE" 2>&1

if [ -s "$TMPFILE" ]; then
    mv "$TMPFILE" "$OUTPUT"
    chown www-data:www-data "$OUTPUT" 2>/dev/null
fi
