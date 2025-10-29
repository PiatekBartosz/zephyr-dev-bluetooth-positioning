# ===== Makefile =====
PROJECT_NAME := main
BUILD_DIR := _build

TAG_BUILD_DIR := $(BUILD_DIR)/tag
BEACON_BUILD_DIR := $(BUILD_DIR)/beacon

TAG_SOURCES := app/tag
BEACON_SOURCES := app/beacon

DEPLOY_DIR := _deploy
BLOBS_DIR := /opt/zephyr/modules/hal/espressif/zephyr/blobs/lib/esp32/

# Default board (can override on command line: make BOARD=myboard)
BOARD ?= m5stack_core2/esp32/procpu
# BOARD ?= esp32s3_devkitc/esp32s3/procpu

# Zephyr external modules
ZEPHYR_EXTRA_MODULES := /workspace/modules/ble_app

# Export so it's available to cmake/west environment
export ZEPHYR_EXTRA_MODULES

.PHONY: all build_tag build_beacon build clean deploy fetch_blobs flash rebuild

all: build deploy

# -------------------
# Build Targets
# -------------------

build_tag:
	@mkdir -p $(TAG_BUILD_DIR)
	@echo ">>> Building Tag with BOARD=$(BOARD)"
	@cmake -B $(TAG_BUILD_DIR) -S $(TAG_SOURCES) -GNinja -DBOARD=$(BOARD)
	@ninja -C $(TAG_BUILD_DIR)
	@echo "Tag build done!"

build_beacon:
	@mkdir -p $(BEACON_BUILD_DIR)
	@echo ">>> Building Beacon with BOARD=$(BOARD)"
	@cmake -B $(BEACON_BUILD_DIR) -S $(BEACON_SOURCES) -GNinja -DBOARD=$(BOARD)
	@ninja -C $(BEACON_BUILD_DIR)
	@echo "Beacon build done!"

build: build_tag build_beacon
	@echo "All builds done!"

# -------------------
# Clean
# -------------------

clean:
	@rm -rf $(BUILD_DIR)
	@rm -rf $(DEPLOY_DIR)
	@echo "Clean done!"

# -------------------
# Zephyr blobs
# -------------------

fetch_blobs:
	@west blobs fetch hal espressif

# -------------------
# Deploy
# -------------------

deploy:
	@mkdir
