zephyr_library(ble_app)

zephyr_library_sources(
    ble_beacon.c
)

zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR})

zephyr_library_include_directories(
    ${ZEPHYR_BASE}/include
)
