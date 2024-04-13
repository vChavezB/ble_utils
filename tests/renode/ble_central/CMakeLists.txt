# Copyright (c) 2024 Victor Chavez
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
# Root dir contains Ble utils
set(ZEPHYR_EXTRA_MODULES ${ROOT_DIR})

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(tester_ble_central)

target_sources(app PRIVATE src/main.cpp
                            src/discovery.cpp)

target_include_directories(app PRIVATE ${ROOT_DIR}/samples/uptime/src
                                        ${ROOT_DIR}/include)