cmake_minimum_required(VERSION 3.22.1)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(app_tag LANGUAGES C)

target_include_directories(app
    PRIVATE 
    app/
    ${CMAKE_SOURCE_DIR}/../../modules/ble
)

target_sources(app
    PRIVATE 
    main.c
    app/app.c
)

target_link_libraries(app
    PRIVATE
    #TODO: figure out why then the modules is build outside of a tree this weird name shows up
    ${CMAKE_BINARY_DIR}/modules/ble_app/lib..__..__..__workspace__modules__ble_app.a
)
