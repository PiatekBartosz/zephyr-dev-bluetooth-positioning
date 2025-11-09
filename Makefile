PROJECT_NAME = main
BUILD_DIR = _build
DEPLOY_DIR = _deploy
BLOBS_DIR = /opt/zephyr/modules/hal/espressif/zephyr/blobs/lib/esp32/

BOARD = "m5stack_core2/esp32/procpu"

.PHONY: all cmake build clean run rebuild

# Default target: build the project
all: build deploy

# Build the project
build:
	mkdir -p $(BUILD_DIR)
	cmake -B _build -S . -GNinja -DBOARD=${BOARD} && ninja -C _build
	echo "Build done!"

# Clean the build directory
clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(DEPLOY_DIR)
	
# Fetch esp32 blobs if they are not already provided
fetch_blobs:
	west blobs fetch hal espressif

# Run the compiled executable
deploy:
	mkdir -p $(DEPLOY_DIR)
	cp $(BUILD_DIR)/zephyr/zephyr.bin $(DEPLOY_DIR)
	echo "Deploy done!"

# Flash the board
flash:
	./tools/esptool/flash.sh

# Rebuild the project from scratch
rebuild: clean all