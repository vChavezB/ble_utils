/*!*****************************************************************
* Copyright 2023-2024, Victor Chavez
* SPDX-License-Identifier: Apache-2.0
* @file main.cpp
* @author Victor Chavez (vchavezb@protonmail.com)
*
* @brief
* Uptime BLE Service that uses the BLE Utils module
*
* @par Dependencies
* - language: C++17
* - OS: Zephyr v3.2.x
********************************************************************/

#pragma once
#include <ble_utils/ble_utils.hpp>
#include <ble_utils/uuid.hpp>


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

class Basic final: public ble_utils::gatt::Characteristic
{
public:
    Basic();
    void update(uint32_t uptime);
private:
    ssize_t read_cb(void *buf, uint16_t len) override;
    uint32_t m_uptime{0};
};

class Notify final: public ble_utils::gatt::CharacteristicNotify
{
public:
    Notify();
private:
    void ccc_changed(CCCValue_e value) override;
};

class Indicate final: public ble_utils::gatt::CharacteristicIndicate
{
public:
    Indicate();
private:
    void ccc_changed(CCCValue_e value) override;
    void indicate_rsp();
};

}

class Service: public ble_utils::gatt::Service
{
    public:
        Service();
        void update(uint32_t uptime);
    private:
        characteristic::Basic m_basic;
        characteristic::Indicate m_indicate;
        characteristic::Notify m_notify;   
};

} // namespace uptime
