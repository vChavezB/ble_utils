/*!*****************************************************************
* Copyright 2023, Victor Chavez
* SPDX-License-Identifier: Apache-2.0
* @file main.cpp
* @author Victor Chavez (vchavezb@protonmail.com)
*
* @brief
* BLE connection functions for uptime service demo
*
* @par Dependencies
* - language: C++17
* - OS: Zephyr v3.2.x
********************************************************************/
#include <zephyr/bluetooth/uuid.h>

namespace ble
{
/**
 * @brief Initialize BLE connection
 * 
 * @return zephyr bluetooth error value
 */
int init();

/**
 * @brief Registers a service to the scan response message
 * when advertising.
 *
 * @param uuid The 128 bit uuid of the service.
 * @return true on success.
 * @return false when the maximum number of configurable services is reached.
 */
bool register_svc_to_scan_rsp(const bt_uuid *uuid);

} // namespace ble
