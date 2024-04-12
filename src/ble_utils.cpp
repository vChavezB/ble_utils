/*!*****************************************************************
 * Copyright 2023, Victor Chavez
 * SPDX-License-Identifier: Apache-2.0
 * @file ble_utils.cpp
 * @author Victor Chavez (chavez-bermudez@fh-aachen.de)
 ********************************************************************/

#include <ble_utils/ble_utils.hpp>

namespace ble_utils::gatt
{
    /**
     * @brief Internal UUIDs for BLE Service and Characteristics
     *
     */
    namespace uuid
    {
        static constexpr bt_uuid_16 PRIMARY_SVC = BT_UUID_INIT_16(BT_UUID_GATT_PRIMARY_VAL);
        static constexpr bt_uuid_16 CHRC_VAL = BT_UUID_INIT_16(BT_UUID_GATT_CHRC_VAL);
        static constexpr bt_uuid_16 CHRC_CCC = BT_UUID_INIT_16(BT_UUID_GATT_CCC_VAL);
    } // namespace uuid

    /**
     * @brief Constructor that defines a BLE Characteristic
     * @details The member list initializer gives an insight on how zephyr OS Requires
                to initialize the struct bt_gatt_attr for a characteristic. <br>.
     *          This can be compared to the C MACRO BT_GATT_CHARACTERISTIC
     *
     */
    Characteristic::Characteristic(const bt_uuid *uuid, uint8_t props, uint8_t perm, bool ccc_enable) : m_attr({.uuid = static_cast<const bt_uuid *>(static_cast<const void *>(&uuid::CHRC_VAL)),
                                                                                                                .read = bt_gatt_attr_read_chrc,
                                                                                                                .write = nullptr,
                                                                                                                .user_data = const_cast<bt_gatt_chrc *>(&m_gatt_chrc),
                                                                                                                .handle = 0,
                                                                                                                .perm = BT_GATT_PERM_READ}),
                                                                                                        m_attr_value({.uuid = uuid,
                                                                                                                      .read = _read_cb,
                                                                                                                      .write = _write_cb,
                                                                                                                      .user_data = this,
                                                                                                                      .handle = 0,
                                                                                                                      .perm = perm}),
                                                                                                        m_gatt_chrc({.uuid = uuid,
                                                                                                                     .value_handle = 0,
                                                                                                                     .properties = props}),
                                                                                                        m_ccc_enable(ccc_enable)
    {
    }
    ssize_t Characteristic::_read_cb(struct bt_conn *conn,
                                     const struct bt_gatt_attr *attr,
                                     void *buf, uint16_t len,
                                     uint16_t offset)
    {
        ARG_UNUSED(conn);
        ARG_UNUSED(offset);
        auto instance = static_cast<Characteristic *>(attr->user_data);
        return instance->read_cb(buf, len);
    }
    ssize_t Characteristic::_write_cb(struct bt_conn *conn,
                                      const struct bt_gatt_attr *attr,
                                      const void *buf,
                                      uint16_t len,
                                      uint16_t offset,
                                      uint8_t flags)
    {
        ARG_UNUSED(conn);
        ARG_UNUSED(offset);
        ARG_UNUSED(flags);
        auto instance = static_cast<Characteristic *>(attr->user_data);
        return instance->write_cb(buf, len);
    }

    Service::Service(const bt_uuid *uuid) : m_gatt_service(
                                                {.attrs = attrs,
                                                 .attr_count = SVC_ATTR_SIZE,
                                                 .node = {nullptr}})
    {
        const bt_gatt_attr svc_attr =
            {
                .uuid = static_cast<const bt_uuid *>(static_cast<const void *>(&uuid::PRIMARY_SVC)),
                .read = bt_gatt_attr_read_service,
                .write = nullptr,
                .user_data = static_cast<void *>(const_cast<bt_uuid *>(uuid)),
                .handle = 0,
                .perm = BT_GATT_PERM_READ};
        attrs[0] = svc_attr;
    }

    void Service::register_char(const Characteristic *chrc)
    {
        const uint8_t chrc_attr_size = chrc->m_ccc_enable ? ICharacteristicCCC::attr_size
                                                          : Characteristic::attr_size;
        const auto req_size{m_gatt_service.attr_count + chrc_attr_size};
        __ASSERT(req_size <= MAX_ATTR, "Max. attribute size reached");
        attrs[m_gatt_service.attr_count++] = chrc->m_attr;
        attrs[m_gatt_service.attr_count++] = chrc->m_attr_value;
        if (chrc->m_ccc_enable)
        {
            auto char_ccc = static_cast<const ICharacteristicCCC *>(chrc);
            attrs[m_gatt_service.attr_count++] = char_ccc->m_ccc_attr;
        }
    }
    int Service::init()
    {
        const int res = bt_gatt_service_register(&m_gatt_service);
        return res;
    }

    bt_uuid_128 *Service::get_uuid()
    {
        return reinterpret_cast<bt_uuid_128 *>(attrs[0].user_data);
    }

    /**
     * @brief Constructor that defines a BLE Characteristic CCC
     * @details The member list initializer gives an insight on how zephyr OS Requires
                to initialize the struct bt_gatt_attr for a CCC characteristic. <br>.
     *          This can be compared to the C MACRO BT_GATT_CCC
     *
     */
    ICharacteristicCCC::ICharacteristicCCC(const bt_uuid *uuid, uint8_t props, uint8_t perm) : Characteristic(uuid, props, perm, true),
                                                                                               m_ccc_data({{
                                                                                                               .cfg{},
                                                                                                               .value{0},
                                                                                                               .cfg_changed = _ccc_changed,
                                                                                                               .cfg_write = nullptr,
                                                                                                               .cfg_match = nullptr,
                                                                                                           },
                                                                                                           this}),
                                                                                               m_ccc_attr({.uuid = static_cast<const bt_uuid *>(static_cast<const void *>(&uuid::CHRC_CCC)),
                                                                                                           .read = bt_gatt_attr_read_ccc,
                                                                                                           .write = bt_gatt_attr_write_ccc,
                                                                                                           .user_data = static_cast<void *>(const_cast<gatt_ccc *>(&m_ccc_data)),
                                                                                                           .handle = 0,
                                                                                                           .perm = BT_GATT_PERM_READ | BT_GATT_PERM_WRITE})
    {
    }
    ICharacteristicCCC::~ICharacteristicCCC() {}

    void ICharacteristicCCC::_ccc_changed(const bt_gatt_attr *attr, uint16_t value)
    {
        auto ccc_data = static_cast<const gatt_ccc *>(attr->user_data);
        auto instance = static_cast<ICharacteristicCCC *>(ccc_data->ctx);
        if (value > BT_GATT_CCC_INDICATE)
        {
            instance->ccc_changed(CCCValue_e::NA);
        }
        else
        {
            const CCCValue_e ccc_value = static_cast<CCCValue_e>(value);
            instance->ccc_changed(ccc_value);
        }
    }

    CharacteristicNotify::CharacteristicNotify(const bt_uuid *uuid, uint8_t props, uint8_t perm) : ICharacteristicCCC(uuid, props | BT_GATT_CHRC_NOTIFY, perm) {}

    CharacteristicNotify::CharacteristicNotify(const bt_uuid *uuid) : CharacteristicNotify(uuid, BT_GATT_CHRC_NOTIFY, 0) {}

    int CharacteristicNotify::notify(const void *data, const uint16_t len)
    {
        const int gatt_res = bt_gatt_notify_uuid(nullptr,
                                                 Characteristic::m_attr_value.uuid,
                                                 nullptr,
                                                 data,
                                                 len);
        return gatt_res;
    }

    CharacteristicIndicate::CharacteristicIndicate(const bt_uuid *uuid) : CharacteristicIndicate(uuid, BT_GATT_CHRC_INDICATE, 0) {}

    CharacteristicIndicate::CharacteristicIndicate(const bt_uuid *uuid, uint8_t props, uint8_t perm) : ICharacteristicCCC(uuid, props | BT_GATT_CHRC_INDICATE, perm),
                                                                                                       indicate_params({.uuid = Characteristic::m_attr_value.uuid,
                                                                                                                        .attr = &m_attr_value,
                                                                                                                        .func = nullptr,
                                                                                                                        .destroy = _indicate_rsp,
                                                                                                                        .data = nullptr,
                                                                                                                        .len = 0,
                                                                                                                        ._ref = 0})
    {
    }

    void CharacteristicIndicate::_indicate_rsp(struct bt_gatt_indicate_params *params)
    {
        auto instance = static_cast<CharacteristicIndicate *>(params->attr->user_data);
        instance->indicate_rsp();
    }

    int CharacteristicIndicate::indicate(const void *data, const uint16_t len)
    {
        indicate_params.data = data;
        indicate_params.len = len;
        const int gatt_res = bt_gatt_indicate(nullptr,
                                              &indicate_params);
        return gatt_res;
    }

} // namespace ble_utils::gatt
