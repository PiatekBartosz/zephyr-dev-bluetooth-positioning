#!/bin/bash

BAUD=115200

esptool --chip auto \
    erase_flash

esptool --chip ESP32S3 \
    --baud $BAUD \
    --before default_reset \
    --after hard_reset write_flash \
    --flash_size detect 0x0000 _deploy/zephyr.bin
