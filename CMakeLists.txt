cmake_minimum_required(VERSION 3.20.0)

list(APPEND ZEPHYR_EXTRA_MODULES  ${CMAKE_CURRENT_SOURCE_DIR}/lcd)

set(DTC_OVERLAY_FILE  "application.overlay lcd.overlay")

find_package(Zephyr)

project(lcdtest)

target_sources(app PRIVATE src/main.c)
