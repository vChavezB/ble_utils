#
# Copyright 2023, Victor Chavez
#
# SPDX-License-Identifier: Apache-2.0
#
cmake_minimum_required(VERSION 3.20.0)

set(ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_SOURCE_DIR}/../..)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(peripheral_hr)


target_sources(app PRIVATE
  src/main.cpp
  src/ble.cpp
  src/uptime_service.cpp
)

target_compile_options(app PUBLIC)

