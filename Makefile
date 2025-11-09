PROJECT_NAME = main
BUILD_DIR = _build
DEPLOY_DIR = _deploy

# BOARD = "m5stack_core2/esp32/procpu"
BOARD = "esp32s3_devkitm/esp32s3/procpu"

BUILD_TYPE = Debug

FORMAT_EXTENSIONS = -name "*.c" -o -name "*.h" -o -name "*.cpp" -o -name "*.hpp"
SOURCE_FILES_TO_FORMAT := $(shell find src/ \( $(FORMAT_EXTENSIONS) \))

CLANG_FORMAT = clang-format ./.clang-format

.PHONY: all cmake build clean run rebuild format

all: build deploy

build:
	mkdir -p $(BUILD_DIR)
	cmake -B _build -S . -GNinja -DBOARD=${BOARD} -DBUILD_TYPE=${BUILD_TYPE} && ninja -C _build
	echo "Build done!"

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(DEPLOY_DIR)

deploy:
	mkdir -p $(DEPLOY_DIR)
	cp $(BUILD_DIR)/zephyr/zephyr.bin $(DEPLOY_DIR)/
	echo "Deploy done!"

flash:
	./tools/esptool/flash.sh

rebuild: clean all

format:
	@if [ -z "$(SOURCE_FILES_TO_FORMAT)" ]; then \
		echo "No source files found to format."; \
	else \
		${CLANG_FORMAT} -i $(SOURCE_FILES_TO_FORMAT) \
		echo "Formatting complete."; \
	fi
