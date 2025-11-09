cmake_minimum_required(VERSION 3.22.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(app_beacon LANGUAGES C)

target_include_directories(app
    PRIVATE 
    app/
)

target_sources(app
    PRIVATE 
    main.c
    app/app.c
    app/ble/ble_beacon.c
)

set_property(TARGET
    app
    PROPERTY
    ZEPHYR_CONFIG
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../config/beacon.conf
)
