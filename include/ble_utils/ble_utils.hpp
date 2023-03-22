/*!*****************************************************************
* Copyright 2023, Victor Chavez
* SPDX-License-Identifier: Apache-2.0
* @file ble_utils.hpp
* @author Victor Chavez (chavez-bermudez@fh-aachen.de)
*
* @brief
* BLE C++ Utilities for development of BLE GATT applications with the Zephyr OS 
*
* @par Dependencies
* - language: C++17
* - OS: Zephyr v3.2.x
********************************************************************/

#pragma once

#include <zephyr/bluetooth/gatt.h>

namespace ble_utils::gatt
{

class ICharacteristicCCC;
class CharacteristicNotify;
class Service;
class CharacteristicIndicate;

/**
 * @brief A BLE Base Characteristic that can have read and write
 *        functionality.
 * 
 * @details This class provides a base implementation for BLE characteristics with C++-
 *           It abstracts the zephyr structs required to 
 *           describe a BLE characteristic with the macro BT_GATT_CHARACTERISTIC and
 *          provides an easier way to generate BLE characteristics for C++ applications.
 */
class Characteristic
{
public:
    /*! @brief Total attributes (i.e. bt_gatt_attr) 
    *   required to represent a BLE characteristic. 
     * @details This value was obtained from the zephyr macro BT_GATT_CHARACTERISTIC.
     */
    static constexpr uint8_t attr_size{2U};

    /**
     * @brief Construct a new Characteristic object
     * 
     * @param uuid Pointer to UUID that is assigned to the Characteristic
     * @param props Properties that are assigned to the characteristic. 
     *               (e.g. BT_GATT_CHRC_READ,BT_GATT_CHRC_WRITE)
     * @param perm Permissions of the characteristic (see zephyr enum bt_gatt_perm)
     */
    Characteristic(const bt_uuid * uuid,uint8_t props,uint8_t perm):
        Characteristic(uuid,props,perm,false){}

    /**
     * @brief Callback function that requests to read data
     *        from the characteristic
     * 
     * @param buf Buffer to place the read result in
     * @param len  Length of data to read
     * @return Number of bytes read, or in case of an error
     *          BT_GATT_ERR() with a specific BT_ATT_ERR_* error code.
     */
    virtual ssize_t read_cb(void *buf, uint16_t len)
    {
        ARG_UNUSED(buf);
        ARG_UNUSED(len);
        return 0;
    };

    /**
    * @brief Callback function that requests to write data 
    *        to the characteristic.
    * 
    * @param buf  Buffer with the data to write
    * @param len Number of bytes in the buffer
    *  @return Number of bytes written, or in case of an error
    *          BT_GATT_ERR() with a specific BT_ATT_ERR_* error code.
    */
    virtual ssize_t write_cb(const void *buf,uint16_t len)
    {
        ARG_UNUSED(buf);
        ARG_UNUSED(len);
        return 0;
    }

private:
    friend ICharacteristicCCC;
    friend Service;
    friend CharacteristicNotify;
    friend CharacteristicIndicate;

    /**
     * @brief Internal constructor for a Characteristic
     * @details This constructor has an extra parameter that allows to know
     *          if the characteristic has CCC (i.e. notify or indicate)
     * 
     * @param uuid Pointer to UUID that is assigned to the Characteristic
     * @param props Properties that are assigned to the characteristic. 
     *               (e.g. BT_GATT_CHRC_READ,BT_GATT_CHRC_WRITE)
     * @param perm Permissions of the characteristic (see zephyr enum bt_gatt_perm)
     * @param ccc_enable Indicates wether the characteristic uses CCC descriptor or not.
     */
    Characteristic(const bt_uuid * uuid,uint8_t props,uint8_t perm,bool ccc_enable);

    static ssize_t _read_cb(struct bt_conn *conn,
					    const struct bt_gatt_attr *attr,
					    void *buf, uint16_t len,
					    uint16_t offset);

    static ssize_t _write_cb(struct bt_conn *conn,
                                const struct bt_gatt_attr *attr,
                                const void *buf,
                                uint16_t len,
                                uint16_t offset,
                                uint8_t flags);
    const bt_gatt_attr m_attr;
    const bt_gatt_attr m_attr_value;
    const bt_gatt_chrc m_gatt_chrc;
    const bool m_ccc_enable;
};

/**
 * @brief Client Characteristic Configuration (CCC) interface.
 * 
 * @details This class extends the @ref Characteristic class to add support for the
 * Client Characteristic Configuration (CCC), which is used to enable/disable notifications
 * or indications for a characteristic.
 */
class ICharacteristicCCC : public Characteristic
{
public:
    /*! @brief Total attributes (i.e. bt_gatt_attr) 
    *   required to represent a CCC.
        @details Requires attribute size from zephyr characteristic (see macro BT_GATT_CHARACTERISTIC)
                 and one extra attribute to represent CCC (see macro BT_GATT_CCC)
    */
    static constexpr uint8_t attr_size{Characteristic::attr_size+1U};


    ICharacteristicCCC(const bt_uuid * uuid,uint8_t props,uint8_t perm);

    /**
     * @brief CCC descriptor values
     * 
     */
    enum class CCCValue_e
    {
        Disabled=0,                     /*<! CCC disabled */
        Notify=BT_GATT_CCC_NOTIFY,      /*<! CCC Notification enabled */
        Indicate=BT_GATT_CCC_INDICATE,  /*<! CCC Indication enabled */
        NA                              /*<! CCC Unknown value */
    };

    /**
     * @brief CCC descriptor changed callback
     * 
     * @param value The CCC Value that was changed
     */
    virtual void ccc_changed(CCCValue_e value)
    {
        ARG_UNUSED(value);
    };
    
    virtual ~ICharacteristicCCC() = 0;
private:
    static void _ccc_changed(const bt_gatt_attr *attr, uint16_t value);
    /**
     * @brief Custom struct to add context for ccc changed callback
     */
    struct gatt_ccc : public _bt_gatt_ccc
    {
        void * ctx;
    };
    gatt_ccc m_ccc_data;
    const bt_gatt_attr m_ccc_attr; 
    friend Service;
};

/**
 * @brief BLE Characteristic notify implementation
 * @details Characteristic that implements BLE Gatt notification
 */
class CharacteristicNotify : public ICharacteristicCCC
{
public:
    /**
     * @brief BLE Notify Characteristic constructor
     * 
     * @param uuid UUID assigned to the characteristic
     * @param props Properties that are assigned to the characteristic.
     *               (e.g. BT_GATT_CHRC_READ,BT_GATT_CHRC_WRITE)
     * @param perm Permissions of the characteristic (see zephyr enum bt_gatt_perm)
     * @note  Property BT_GATT_CHRC_NOTIFY is initialized by default.
     */
    CharacteristicNotify(const bt_uuid * uuid,uint8_t props,uint8_t perm);
    /**
     * @brief Overload constructor with only UUID 
     * @details Defaults properties
     * @param uuid UUID assigned to the characteristic
     * @note No extra properties and permissions are initialized.
     */
    CharacteristicNotify(const bt_uuid * uuid);
    /**
     * @brief Send BLE Characteristic notification 
     * 
     * @param data Pointer to data buffer
     * @param len Length of the notification data
     * @return The zephyr gatt result from the internal bt api
     */
    int notify(const void * data,const uint16_t len);
private:
    friend Service;
};

class CharacteristicIndicate : public ICharacteristicCCC
{
public:
    /**
     * @brief BLE Indicate Characteristic constructor
     * 
     * @param uuid UUID assigned to the characteristic
     * @param props Properties that are assigned to the characteristic. 
     *               (e.g. BT_GATT_CHRC_READ,BT_GATT_CHRC_WRITE)
     * @param perm Permissions of the characteristic (see zephyr enum bt_gatt_perm)
     * @note  Property BT_GATT_CHRC_INDICATE is initialized by default.
     */
    CharacteristicIndicate(const bt_uuid * uuid,uint8_t props,uint8_t perm);
    /**
     * @brief Overload constructor with only UUID 
     * @details Defaults properties
     * @param uuid UUID assigned to the characteristic
     * @note No extra properties and permissions are initialized.
     */
    CharacteristicIndicate(const bt_uuid * uuid);
    /**
     * @brief Send BLE Characteristic Indication 
     * 
     * @param data Pointer to data buffer
     * @param len Length of the indication data
     * @return int The zephyr gatt result from the internal bt api
     */
    int indicate(const void * data,const uint16_t len);

    /**
     * @brief Callback for indication reception
     * 
     */
    virtual void indicate_rsp(){};

private:
    /**
     * @brief Internal callback for indication reception
     * 
     * @param params Indication params object.
     */
    static void _indicate_rsp(struct bt_gatt_indicate_params *params);
    /*! Internal Indication parameters for @ref indicate*/
    bt_gatt_indicate_params indicate_params;
    friend Service;
};

class Service
{
public:
    /**
     * @brief Construct a BLE Service
     * 
     * @param service_uuid UUID assigned to the service
     */
	Service(const bt_uuid *service_uuid);
    
    /**
     * @brief Register a characteristic to the service
     * @details should be called before @ref init
     * 
     * @param chrc Pointer to characteristic object
     */
	void register_char(const Characteristic * chrc);

    /**
     * @brief Initialize the BLE Service
     * @details should be called only after registering all the characteristics for the service
     *          with @ref register_char
     * @return Zephyr return value from bt_gatt_service_register 
     */
    int init();

private:
    static constexpr uint8_t MAX_ATTR = CONFIG_BLE_UTILS_MAX_ATTR;

    /*! @brief Total attributes (i.e. bt_gatt_attr) 
    *   required to represent a BLE Service. 
     * @details This value is obtained from the zephyr macro BT_GATT_SERVICE_DEFINE.
     */
    static constexpr uint8_t SVC_ATTR_SIZE = 1;

    bt_gatt_attr attrs[MAX_ATTR];

    /**
     * @brief Zephyr struct with BLE Gatt service data
     * @details This struct is changed at run-time when 
     *          more characteristics are added to the service with
     *          @ref register_char
     * 
     */
    bt_gatt_service m_gatt_service;
};

}
