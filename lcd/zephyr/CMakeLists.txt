# SPDX-License-Identifier: Apache-2.0
project(pcf8576_driver)
zephyr_sources_ifdef(CONFIG_PCF8576 pcf8576.c)
zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR})
