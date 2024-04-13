/*!*****************************************************************
* Copyright 2023-2024, Victor Chavez
* SPDX-License-Identifier: Apache-2.0
* @file main.cpp
* @author Victor Chavez (vchavezb@protonmail.com)
*
* @brief
* Main source file that implements an uptime service demo for the BLE utils module
*
* @par Dependencies
* - language: C++17
* - OS: Zephyr v3.2.x
********************************************************************/

#include "ble.hpp"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "uptime_service.hpp"
LOG_MODULE_REGISTER(main, CONFIG_LOG_DEFAULT_LEVEL);

uptime::Service uptime_service;

int main(void)
{
	LOG_INF("Starting Uptime BLE Utils sample");
	uptime_service.init();
	ble::init();
	for (;;)
	{
		const uint32_t uptime_ms = k_uptime_get_32();
		uptime_service.update(uptime_ms/1000U);
		k_sleep(K_MSEC(1000));		
	}
	return 0;
}
