#!/bin/bash
# RTSP → HLS streaming proxy
# Converts an RTSP/RTSPS camera stream to HLS segments for browser playback
# Managed by /etc/init.d/rtsp_stream

source /home/HiveControl/scripts/hiveconfig.inc

STREAM_DIR="/home/HiveControl/www/public_html/stream"
PIDFILE="/var/run/rtsp_stream.pid"
PLAYLIST="$STREAM_DIR/live.m3u8"

if [ "$CAMERATYPE" != "RTSP" ] || [ -z "$CAMERA_RTSP_URL" ]; then
    exit 0
fi

mkdir -p "$STREAM_DIR"

# Validate URL scheme
case "$CAMERA_RTSP_URL" in
    rtsp://*|rtsps://*) ;;
    *) echo "ERROR: Invalid RTSP URL"; exit 1 ;;
esac

TLS_OPTS=""
if echo "$CAMERA_RTSP_URL" | grep -qi "^rtsps://"; then
    TLS_OPTS="-tls_verify 0"
fi

cleanup() {
    rm -f "$PIDFILE"
    rm -f "$STREAM_DIR"/live*.ts "$STREAM_DIR"/live.m3u8
    exit 0
}
trap cleanup SIGTERM SIGINT

echo $$ > "$PIDFILE"

RETRY_DELAY=5
MAX_RETRY_DELAY=60

while true; do
    /usr/bin/ffmpeg -hide_banner -loglevel warning \
        -rtsp_transport tcp \
        $TLS_OPTS \
        -i "$CAMERA_RTSP_URL" \
        -fflags flush_packets \
        -max_delay 500000 \
        -an \
        -vcodec copy \
        -hls_time 2 \
        -hls_list_size 3 \
        -hls_flags delete_segments \
        -hls_allow_cache 0 \
        "$PLAYLIST" 2>&1

    # ffmpeg exited — stream may have disconnected
    rm -f "$STREAM_DIR"/live*.ts "$STREAM_DIR"/live.m3u8

    # Check if we should still be running
    if [ "$1" = "--once" ]; then
        break
    fi

    sleep $RETRY_DELAY
    RETRY_DELAY=$((RETRY_DELAY * 2))
    [ $RETRY_DELAY -gt $MAX_RETRY_DELAY ] && RETRY_DELAY=$MAX_RETRY_DELAY
done

cleanup
