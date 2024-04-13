
#include <stddef.h>
#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/types.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/logging/log.h>
#include <string>
#include "discovery.hpp"
#include "uptime_service.hpp"

LOG_MODULE_REGISTER(central, CONFIG_LOG_DEFAULT_LEVEL);


namespace discovery {

static bt_conn *default_conn;
static bool device_found;
static uint8_t device_char_cnt;


static bt_gatt_discover_params discover_params;
static bt_gatt_subscribe_params subscribe_params;

static constexpr bt_le_conn_param conn_default_param =
{
	.interval_min = 0x18,
	.interval_max= 0x28,
	.latency = 0,
	.timeout= 400
};

static constexpr bt_conn_le_create_param conn_create_param =
{
	.options=BT_CONN_LE_OPT_NONE,
	.interval=BT_GAP_SCAN_FAST_INTERVAL, /* scan fast interval 60 ms */
	.window=BT_GAP_SCAN_FAST_INTERVAL, /* scan fast interval 60 ms */
	.interval_coded=0,
	.window_coded=0,
	.timeout=0
};

static constexpr uint8_t TOTAL_CHARACTERISTICS = 3;
static const bt_uuid * uptime_characteristics [TOTAL_CHARACTERISTICS] =
{
	&uptime::uuid::char_basic.uuid,
	&uptime::uuid::char_indicate.uuid,
	&uptime::uuid::char_notify.uuid
};

static uint8_t uptime_notify_cb(bt_conn *conn,
			   				bt_gatt_subscribe_params *params,
			   				const void *data, uint16_t length)
{
	if (conn == NULL) {
		return BT_GATT_ITER_CONTINUE;
	}

	if (!data) {
		LOG_INF("Unsubscribed");
		params->value_handle = 0U;
		return BT_GATT_ITER_CONTINUE;
	}

	if(length != sizeof(uint32_t)) {
		LOG_ERR("Uptime data len %d does not match expected len %d", length,sizeof(uint32_t));
		return BT_GATT_ITER_STOP;
	}
	const uint32_t uptime = sys_get_le32((uint8_t*)(data));
	LOG_INF("Notification Uptime value %d", uptime);
	return BT_GATT_ITER_CONTINUE;
}

static int discover_characteristics(bt_conn *conn,
								const bt_gatt_attr *attr,
								uint8_t idx)
{
	discover_params.uuid = uptime_characteristics[idx];
	discover_params.start_handle = attr->handle + 1;
	discover_params.type = BT_GATT_DISCOVER_CHARACTERISTIC;
	/* Discover the characteristics */
	int err = bt_gatt_discover(conn, &discover_params);
	if (err) {
		LOG_DBG("Char. %d discover failed (err %d)\n",idx,err);
		return err;
	}
	return 0;
}


static uint8_t service_discover_cb(bt_conn *conn,
			     const bt_gatt_attr *attr,
			     bt_gatt_discover_params *params)
{
	if (!attr) {
		(void)memset(params, 0, sizeof(*params));
		if (!device_found) {
			LOG_ERR("BLE peripheral not found");
		} else {
			LOG_INF("BLE peripheral found");
			device_found = false;
		}
		device_char_cnt = 0;
		return BT_GATT_ITER_STOP;
	}
	const bool svc_found = bt_uuid_cmp(params->uuid, &uptime::uuid::svc_base.uuid) == 0;
	const bool char_found = bt_uuid_cmp(params->uuid,
										uptime_characteristics[device_char_cnt])== 0;
	if (svc_found) {
		LOG_INF("Service found");
		device_char_cnt = 0;
		discover_characteristics(conn, attr, device_char_cnt);
	} else if(char_found) {
		LOG_INF("Chrc %d/%d found",device_char_cnt+1,TOTAL_CHARACTERISTICS);
		if (bt_uuid_cmp(params->uuid,
						&uptime::uuid::char_notify.uuid) == 0) {
			// Subscribe to uptime notification
			subscribe_params.notify = uptime_notify_cb;
			subscribe_params.value = BT_GATT_CCC_NOTIFY;
			subscribe_params.ccc_handle = attr->handle+2;
			subscribe_params.value_handle = bt_gatt_attr_value_handle(attr);
			const int err = bt_gatt_subscribe(conn, &subscribe_params);
			if (err != 0 && err != -EALREADY) {
				LOG_ERR("Subscribe failed (err %d)", err);
			} else {
				LOG_INF("Subscribed");
			}
			return BT_GATT_ITER_STOP;
		}
		if (device_char_cnt == TOTAL_CHARACTERISTICS-1) {
			device_found = true;
		} else if (device_char_cnt < TOTAL_CHARACTERISTICS) {
			device_char_cnt++;
			discover_characteristics(conn, attr, device_char_cnt);
		}
	}
	return BT_GATT_ITER_STOP;
}



static bool adv_data_cb(bt_data *data, void *user_data)
{
	auto addr = static_cast<bt_addr_le_t*>(user_data);
	LOG_INF("Adv data type %u len %u", data->type, data->data_len);
	switch (data->type) {
	case BT_DATA_UUID128_SOME:
	case BT_DATA_UUID128_ALL:
		if (data->data_len % BT_UUID_SIZE_128 != 0U) {
			LOG_INF("AD malformed");
			return true;
		}
		for (int i = 0; i < data->data_len; i += BT_UUID_SIZE_128) {
			int err;

			bt_uuid_128 adv_uuid ={	.uuid = { BT_UUID_TYPE_128 }};
			memcpy(adv_uuid.val, data->data+i, BT_UUID_SIZE_128);
			const int uuid_match = bt_uuid_cmp(&adv_uuid.uuid, &uptime::uuid::svc_base.uuid);
			if (uuid_match != 0) {
				continue;
			}
			LOG_INF("Matched Uptime adv. UUID");
			err = bt_le_scan_stop();
			if (err) {
				LOG_INF("Stop LE scan failed (err %d)", err);
				return false;
			}

			LOG_INF("Connecting..");
			err = bt_conn_le_create(addr, &conn_create_param,
						&conn_default_param, &default_conn);
			if (err) {
				LOG_ERR("Create conn failed (err %d)", err);
				start_scan();
			}
			return false;
		}
	}
	return true;
}

static void device_found_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
			 net_buf_simple *ad)
{
	char dev[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(addr, dev, sizeof(dev));
	//LOG_INF("[DEVICE]: %s, AD evt type %u, AD data len %u, RSSI %i",
	//       dev, type, ad->len, rssi);

	if (type == BT_GAP_ADV_TYPE_ADV_IND ||
	    type == BT_GAP_ADV_TYPE_ADV_DIRECT_IND ||
		type == BT_GAP_ADV_TYPE_SCAN_RSP) {
		bt_data_parse(ad, adv_data_cb, (void *)addr);
	}
}

int start_scan()
{
	int err;

	/* Use active scanning and disable duplicate filtering to handle any
	 * devices that might update their advertising data at runtime. */
	bt_le_scan_param scan_param = {
		.type       = BT_LE_SCAN_TYPE_ACTIVE,
		.options    = BT_LE_SCAN_OPT_NONE,
		.interval   = BT_GAP_SCAN_FAST_INTERVAL,
		.window     = BT_GAP_SCAN_FAST_WINDOW,
	};

	err = bt_le_scan_start(&scan_param, device_found_cb);
	if (err) {
		LOG_ERR("Scanning failed to start (err %d)", err);
		return err;
	}

	LOG_INF("Scanning successfully started");
	return err;
}

static void find_main_service()
{
	discover_params.uuid = &uptime::uuid::svc_base.uuid;
	discover_params.func = service_discover_cb;
	discover_params.start_handle = BT_ATT_FIRST_ATTRIBUTE_HANDLE;
	discover_params.end_handle = BT_ATT_LAST_ATTRIBUTE_HANDLE;
	discover_params.type = BT_GATT_DISCOVER_PRIMARY;

	int err = bt_gatt_discover(default_conn, &discover_params);
	if (err) {
		LOG_INF("Service discover failed (err %d)", err);
	}
}


static void connected(bt_conn *conn, uint8_t conn_err)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (conn_err) {
		LOG_INF("Failed to connect to %s (%u)", addr, conn_err);

		bt_conn_unref(default_conn);
		default_conn = NULL;

		start_scan();
		return;
	}

	LOG_INF("Connected: %s", addr);

	if (conn == default_conn) {
		find_main_service();
	}
}

static void disconnected(bt_conn *conn, uint8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	LOG_INF("Disconnected: %s (reason 0x%02x)", addr, reason);

	if (default_conn != conn) {
		return;
	}

	bt_conn_unref(default_conn);
	default_conn = NULL;

	start_scan();
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
};

}