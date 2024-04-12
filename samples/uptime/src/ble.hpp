/*!*****************************************************************
 * Copyright 2023, Victor Chavez
 * SPDX-License-Identifier: Apache-2.0
 * @file ble.hpp
 * @author Victor Chavez (chavez-bermudez@fh-aachen.de)
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
    bool register_service_to_scan_response(bt_uuid_128 *uuid);
} // namespace ble
