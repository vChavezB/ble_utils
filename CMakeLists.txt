# Copyright 2023, Victor Chavez
# SPDX-License-Identifier: Apache-2.0

if(CONFIG_BLE_UTILS)
set(lib_name ble_utils)

zephyr_library_named(${lib_name})
zephyr_library_sources(src/ble_utils.cpp)
target_include_directories(app PUBLIC include)
zephyr_include_directories(include)

target_compile_options(${lib_name} PRIVATE -Wall -Wextra -Wfatal-errors)
endif() # CONFIG_PHYPHOX_BLE
