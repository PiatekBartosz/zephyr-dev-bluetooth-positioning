#!/bin/bash
# flash.sh - Flash Tag or Beacon to ESP32 or ESP32-S3
# Usage:
#   ./flash.sh <tag|beacon> <esp32|esp32s3>
# Example:
#   ./flash.sh tag esp32

set -e

if [ $# -lt 2 ]; then
    echo "Usage: $0 <tag|beacon> <esp32|esp32s3>"
    exit 1
fi

MODE=$1         # tag or beacon
CHIP=$2         # esp32 or esp32s3

BIN_PATH="_deploy"
case $MODE in
    tag)
        BIN="$BIN_PATH/zephyr_tag.bin"
        ;;
    beacon)
        BIN="$BIN_PATH/zephyr_beacon.bin"
        ;;
    *)
        echo "Invalid mode: $MODE (use tag or beacon)"
        exit 1
        ;;
esac

# Map chip name to esptool target
case $CHIP in
    esp32)
        CHIP_ARG="esp32"
        ;;
    esp32s3)
        CHIP_ARG="esp32s3"
        ;;
    *)
        echo "Invalid chip: $CHIP (use esp32 or esp32s3)"
        exit 1
        ;;
esac

echo "Flashing $MODE binary to $CHIP..."

esptool.py --chip $CHIP_ARG \
    --baud 921600 \
    --before default_reset \
    --after hard_reset write_flash \
    -u --flash_mode dio --flash_freq 40m \
    --flash_size detect 0x1000 "$BIN"

echo "Flash complete!"
