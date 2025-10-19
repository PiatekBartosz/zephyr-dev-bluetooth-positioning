#!/bin/bash
esptool --chip auto \
    --baud 921600 \
    --before default_reset \
    --after hard_reset write_flash \
    -u --flash_mode dio --flash_freq 40m \
    --flash_size detect 0x1000 _deploy/zephyr.bin
