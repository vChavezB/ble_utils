/*!*****************************************************************
 * Copyright 2023, Victor Chavez
 * SPDX-License-Identifier: Apache-2.0
 * @file ble.cpp
 * @author Victor Chavez (chavez-bermudez@fh-aachen.de)
 *
 * @brief
 * BLE connection implementation for uptime service demo
 *
 * @par Dependencies
 * - language: C++17
 * - OS: Zephyr v3.2.x
 ********************************************************************/
#include <zephyr/bluetooth/conn.h>
#include "ble.hpp"

namespace ble
{
    constexpr size_t max_sd_services = 5;
    static size_t current_sd_services = 0;

    static struct bt_data sd_data[max_sd_services] = {};
    static constexpr bt_le_adv_param adv_param = BT_LE_ADV_PARAM_INIT(BT_LE_ADV_OPT_CONNECTABLE |
                                                                          BT_LE_ADV_OPT_USE_NAME,
                                                                      BT_GAP_ADV_FAST_INT_MIN_2,
                                                                      BT_GAP_ADV_FAST_INT_MAX_2,
                                                                      NULL);

    static constexpr bt_data adv_data[] =
        {
            BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR))};
    static void connected(bt_conn *conn, uint8_t conn_err)
    {
        int err;
        bt_conn_info info;
        char addr[BT_ADDR_LE_STR_LEN];

        bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

        if (conn_err)
        {
            printk("Connection failed (err %d)\n", conn_err);
            return;
        }

        err = bt_conn_get_info(conn, &info);
        if (err)
        {
            printk("Failed to get connection info (err %d)\n", err);
        }
        else
        {
            printk("Connected: %s\n", addr);
        }
    }

    static void disconnected(struct bt_conn *conn, uint8_t reason)
    {
        printk("Disconnected (reason 0x%02x)\n", reason);
    }

    BT_CONN_CB_DEFINE(conn_callbacks) =
        {
            .connected = connected,
            .disconnected = disconnected,
    };

    bool register_service_to_scan_response(bt_uuid_128 *uuid)
    {
        if (current_sd_services < max_sd_services)
        {
            bt_data dat = {.type = BT_DATA_UUID128_ALL, .data_len = 16, .data = (reinterpret_cast<const uint8_t *>(uuid) + 1)};
            sd_data[current_sd_services++] = dat;
            return true;
        }
        return false;
    }

    static int start_adv(void)
    {
        int err = bt_le_adv_start(&adv_param,
                                  adv_data,
                                  ARRAY_SIZE(adv_data),
                                  sd_data,
                                  current_sd_services);
        if (err)
        {
            printk("Failed to create advertiser set (err %d)\n", err);
            return err;
        }
        return 0;
    }

    int init()
    {
        int err;
        do
        {
            err = bt_enable(NULL);
            if (err)
            {
                printk("Bluetooth init failed (err %d)\n", err);
                break;
            }

            printk("Bluetooth initialized\n");
            err = start_adv();
            if (err)
            {
                printk("Advertising failed to create (err %d)\n", err);
                break;
            }
        } while (0);
        return err;
    }
} // namespace ble
