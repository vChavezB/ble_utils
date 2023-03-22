/*!*****************************************************************
* Copyright 2023, Victor Chavez
* SPDX-License-Identifier: Apache-2.0
* @file uptime_service.cpp
* @author Victor Chavez (chavez-bermudez@fh-aachen.de)
* @date 13.03.2023
*
* @brief
* BLE Service implementation
*
* @par Dependencies
* - language: C++17
* - OS: Zephyr v3.2.x
********************************************************************/

#include "uptime_service.hpp"

namespace uptime
{

namespace uuid
{
    static constexpr bt_uuid_128 svc_base = ble_utils::uuid::uuid128_init(0xABCD0000, 
                                                                        0x1234,
                                                                        0x5678,
                                                                        0x9ABC,
                                                                        0xDEF012345678);

    static constexpr bt_uuid_128 char_basic = ble_utils::uuid::derive_uuid(svc_base,0x0001);
    static constexpr bt_uuid_128 char_notify = ble_utils::uuid::derive_uuid(svc_base,0x0002);
    static constexpr bt_uuid_128 char_indicate = ble_utils::uuid::derive_uuid(svc_base,0x0003);
}

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

Indicate::Indicate():
    ble_utils::gatt::CharacteristicIndicate((const bt_uuid*)&uuid::char_indicate)    
{
}

} // namespace characteristic

Service::Service():
    ble_utils::gatt::Service((const bt_uuid*)&uuid::svc_base)
{
    register_char(&m_basic);
    register_char(&m_notify);
    register_char(&m_indicate);
}

void Service::update(uint32_t uptime)
{
    m_basic.update(uptime);
    m_notify.notify(&uptime,sizeof(uptime));
    m_indicate.indicate(&uptime,sizeof(uptime));
}

} // namespace uptime
