PROJECT_NAME=main
BUILD_DIR=_build

TAG_BUILD_DIR=${BUILD_DIR}/tag
BEACON_BUILD_DIR=${BUILD_DIR}/beacon

TAG_SOURCES=app/tag
BEACON_TAG_SOURCES=app/beacon

DEPLOY_DIR = _deploy
BLOBS_DIR = /opt/zephyr/modules/hal/espressif/zephyr/blobs/lib/esp32/

BOARD = "m5stack_core2/esp32/procpu"

.PHONY: all cmake build clean run rebuild

all: build deploy

build_tag:
	mkdir -p ${TAG_BUILD_DIR}
	cmake -B ${TAG_BUILD_DIR} -S ${TAG_SOURCES} -GNinja -DBOARD=${BOARD} && ninja -C ${TAG_BUILD_DIR}
	echo "Beacon build done!"

build_beacon:
	mkdir -p ${BEACON_BUILD_DIR}
	cmake -B ${BEACON_BUILD_DIR} -S ${BEACON_TAG_SOURCES} -GNinja -DBOARD=${BOARD} && ninja -C ${BEACON_BUILD_DIR}
	echo "Tag build done!"

build: build_tag build_beacon
	echo "Build done!"

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(DEPLOY_DIR)
	
fetch_blobs:
	west blobs fetch hal espressif

deploy:
	mkdir -p $(DEPLOY_DIR)
	cp $(BUILD_DIR)/zephyr/zephyr.bin $(DEPLOY_DIR)
	echo "Deploy done!"

flash:
	./tools/esptool/flash.sh

rebuild: clean all
