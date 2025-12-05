cmake_minimum_required(VERSION 3.13.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(app LANGUAGES C)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

target_include_directories(
    app PRIVATE 
    src/app/
)

target_sources(
    app PRIVATE 
    src/main.c
    src/app/app.c
    src/app/ble/ble_rtls.c
    src/app/ui/ui.c
)
