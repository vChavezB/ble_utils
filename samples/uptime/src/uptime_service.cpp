/*!*****************************************************************
* Copyright 2023-2024, Victor Chavez
* SPDX-License-Identifier: Apache-2.0
* @file main.cpp
* @author Victor Chavez (vchavezb@protonmail.com)
*
* @brief
* BLE Service implementation
*
* @par Dependencies
* - language: C++17
* - OS: Zephyr v3.2.x
********************************************************************/
#include <zephyr/logging/log.h>
#include "uptime_service.hpp"

LOG_MODULE_REGISTER(uptime_svc, CONFIG_LOG_DEFAULT_LEVEL);

namespace uptime
{



namespace characteristic
{

Basic::Basic():
    ble_utils::gatt::Characteristic((const bt_uuid*)&uuid::char_basic,
                                    BT_GATT_CHRC_READ,
                                    BT_GATT_PERM_READ)
{
}

ssize_t Basic::read_cb(void *buf, uint16_t len)
{
    uint8_t * pUptime = static_cast<uint8_t*>(buf);
    pUptime[0] = m_uptime & 0xFF;
    pUptime[1] = m_uptime>>8 & 0xFF;
    pUptime[2] = m_uptime>>16 & 0xFF;
    pUptime[3] = m_uptime>>24 & 0xFF;
    return sizeof(m_uptime);
}

void Basic::update(uint32_t uptime)
{
    m_uptime = uptime;
}

Notify::Notify():
    ble_utils::gatt::CharacteristicNotify((const bt_uuid*)&uuid::char_notify)    
{
}

void Notify::ccc_changed(CCCValue_e value)
{
    int val = static_cast<int>(value);
    LOG_INF("Characteristic Notify Uptime CCC changed %d\n",val);
}

Indicate::Indicate():
    ble_utils::gatt::CharacteristicIndicate((const bt_uuid*)&uuid::char_indicate)    
{
}

void Indicate::ccc_changed(CCCValue_e value)
{
    int val = static_cast<int>(value);
    LOG_INF("Characteristic Indicate Uptime CCC changed %d\n",val);
}
void Indicate::indicate_rsp()
{
    LOG_INF("Characteristic Indicate Uptime Completed\n");
}

} // namespace characteristic

Service::Service():
    ble_utils::gatt::Service((const bt_uuid*)&uuid::svc_base)
{
    register_char(&m_basic);
    register_char(&m_indicate);
    register_char(&m_notify);
}

void Service::update(uint32_t uptime)
{
    m_basic.update(uptime);
    m_notify.notify(&uptime,sizeof(uptime));
    m_indicate.indicate(&uptime,sizeof(uptime));
}

} // namespace uptime
