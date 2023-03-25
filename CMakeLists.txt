cmake_minimum_required(VERSION 3.20.0)
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE ZDebug)
endif()
# Point to the build type-specific config file
set(CONF_FILE "prj.${CMAKE_BUILD_TYPE}.conf")

list(APPEND ZEPHYR_EXTRA_MODULES  ${CMAKE_CURRENT_SOURCE_DIR}/lcd)

set(DTC_OVERLAY_FILE  "application.overlay lcd.overlay")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(lcdtest)

target_sources(app PRIVATE src/main.c)
