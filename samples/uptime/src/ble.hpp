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
namespace ble
{
/**
 * @brief Initialize BLE connection
 * 
 * @return zephyr bluetooth error value
 */
int init();
} // namespace ble
