/*
    Copyright (c) 2024 Victor Chavez
    SPDX-License-Identifier: Apache-2.0
*/

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include "discovery.hpp"

LOG_MODULE_REGISTER(main, CONFIG_LOG_DEFAULT_LEVEL);

int main()
{
    int rc = bt_enable(nullptr);
    if (rc != 0) {
        LOG_ERR("bt_enable failed,err  %d",rc);
        return rc;
    }
    rc = discovery::start_scan();
    if (rc != 0) {
        LOG_ERR("Error starting scan,err  %d",rc);
        return rc;
    }
    return 0;
}
