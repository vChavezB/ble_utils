/*!*****************************************************************
* Copyright 2023-2024, Victor Chavez
* SPDX-License-Identifier: Apache-2.0
* @file main.cpp
* @author Victor Chavez (vchavezb@protonmail.com)
*
* @brief
* BLE connection implementation for uptime service demo
*
* @par Dependencies
* - language: C++17
* - OS: Zephyr v3.2.x
********************************************************************/
#include <zephyr/bluetooth/conn.h>
#include <zephyr/logging/log.h>
#include "ble.hpp"
LOG_MODULE_REGISTER(ble, CONFIG_LOG_DEFAULT_LEVEL);

namespace ble
{

constexpr size_t MAX_SD_SIZE {5U};
static size_t sd_cnt = 0;

static struct bt_data sd_data[MAX_SD_SIZE] = {};
static constexpr bt_le_adv_param adv_param = BT_LE_ADV_PARAM_INIT(BT_LE_ADV_OPT_CONNECTABLE |
																		BT_LE_ADV_OPT_USE_NAME,
																	BT_GAP_ADV_FAST_INT_MIN_2,
																	BT_GAP_ADV_FAST_INT_MAX_2,
																	NULL);

static constexpr bt_data adv_data[] =
{
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR))
};

static void connected(bt_conn *conn, uint8_t conn_err)
{
	int err;
	bt_conn_info info;
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (conn_err)
    {
		LOG_INF("Connection failed (err %d)", conn_err);
		return;
	}

	err = bt_conn_get_info(conn, &info);
	if (err)
    {
		LOG_ERR("Failed to get connection info (err %d)", err);
	}
    else 
    {
		LOG_INF("Connected: %s\n", addr);
	}
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	LOG_INF("Disconnected (reason 0x%02x)", reason);
}

BT_CONN_CB_DEFINE(conn_callbacks) =
{
	.connected = connected,
	.disconnected = disconnected,
};

static int start_adv(void)
{
 	int err = bt_le_adv_start(&adv_param,
                                adv_data,
                                ARRAY_SIZE(adv_data),
                                sd_data,
                                sd_cnt);

	if (err) {
		LOG_ERR("Failed to create advertiser set (err %d)", err);
		return err;
	}
	return 0;
}

bool register_svc_to_scan_rsp(const bt_uuid *uuid)
{
	uint8_t data_len;
	const uint8_t * p_data;
	switch(uuid->type){
		case BT_UUID_TYPE_16:
			data_len = 2;
			p_data = (const uint8_t *)&((bt_uuid_16 *)uuid)->val;
			break;
		case BT_UUID_TYPE_32:
			data_len = 4;
			p_data = (const uint8_t *)&((bt_uuid_32 *)uuid)->val;
			break;
		case BT_UUID_TYPE_128:
			data_len = 16;
			p_data = (const uint8_t *)&((bt_uuid_128 *)uuid)->val[0];
			break;
		default:
			return false;
	}

	if (sd_cnt < MAX_SD_SIZE) {
		bt_data data = {.
			type = uuid->type,
			.data_len = data_len,
			.data = p_data
		};
		sd_data[sd_cnt++] = data;
		return true;
	}
	return false;
}

int init()
{
  	int err;
	do
	{
		err = bt_enable(NULL);
		if (err)
		{
			LOG_ERR("Bluetooth init failed (err %d)", err);
			break;
		}

		LOG_INF("Bluetooth initialized\n");
		err = start_adv();
		if (err)
		{
			LOG_ERR("Advertising failed to create (err %d)", err);
			break;
		}
	}while(0);
	return err;  
}
} // namespace ble
