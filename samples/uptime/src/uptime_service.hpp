/*!*****************************************************************
* Copyright 2023, Victor Chavez
* SPDX-License-Identifier: Apache-2.0
* @file uptime_service.hpp
* @author Victor Chavez (chavez-bermudez@fh-aachen.de)
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
    void ccc_changed(CCCValue_e value) override
    {
        int val = static_cast<int>(value);
        printk("Characteristic Notify Uptime CCC changed %d\n",val);
    }
};

class Indicate final: public ble_utils::gatt::CharacteristicIndicate
{
public:
    Indicate();
private:
    void ccc_changed(CCCValue_e value) override
    {
        int val = static_cast<int>(value);
        printk("Characteristic Indicate Uptime CCC changed %d\n",val);
    }
    void indicate_rsp() override
    {
        printk("Characteristic Indicate Uptime Completed\n");
    }

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
