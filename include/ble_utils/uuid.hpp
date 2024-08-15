/*!*****************************************************************
* Copyright 2023-2024 Victor Chavez
* SPDX-License-Identifier: Apache-2.0
* @file uuid.hpp
* @author Victor Chavez (vchavezb@protonmail.com)
*
* @brief
* Utilities to generate Zephyr 128 bit UUIDs
*
* @par Dependencies
* - language: C++17
* - OS: Zephyr v3.2.x
********************************************************************/

#pragma once

#include <zephyr/bluetooth/uuid.h>
#include <stdint.h>

namespace ble_utils::uuid
{
/** @brief Get the raw bytes of a bt_uuid_128 datatype
     @details Useful to initialize uuid in adv. data. <br>
               Example: <br>
               bt_uuid_128 my_uuid;
               static constexpr bt_data adv_data[] = {
                   BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL)),
                   BT_DATA_BYTES(BT_DATA_UUID128_ALL, RAW_UUID_128(my_uuid)),
                };
    @param UUID_128 128-bit UUID  bt_uuid_128 struct
 */
#define RAW_UUID_128(UUID_128)  UUID_128.val[0],UUID_128.val[1],UUID_128.val[2],UUID_128.val[3], \
                                UUID_128.val[4],UUID_128.val[5],UUID_128.val[6],UUID_128.val[7], \
                                UUID_128.val[8],UUID_128.val[9],UUID_128.val[10],UUID_128.val[11], \
                                UUID_128.val[12],UUID_128.val[13],UUID_128.val[14],UUID_128.val[15]

/**
 * @brief Initialize a 128 bit UUID 
 * 
 * @param w32 
 * @param w1 
 * @param w2 
 * @param w3 
 * @param w48 
 * @return constexpr bt_uuid_128 
 */
static constexpr bt_uuid_128 uuid128_init(uint32_t w32,uint16_t w1,uint16_t w2,uint16_t w3,uint64_t w48)
{
    const uint8_t b0 =  static_cast<uint8_t>(w48 & 0xFF);
    const uint8_t b1 =  static_cast<uint8_t>(w48 >>8 & 0xFF);
    const uint8_t b2 =  static_cast<uint8_t>(w48 >>16 & 0xFF);
    const uint8_t b3 =  static_cast<uint8_t>(w48 >>24 & 0xFF);
    const uint8_t b4 =  static_cast<uint8_t>(w48 >>32 & 0xFF);
    const uint8_t b5 =  static_cast<uint8_t>(w48 >>40 & 0xFF);
    const uint8_t b6 =  static_cast<uint8_t>(w3 & 0xFF);
    const uint8_t b7 =  static_cast<uint8_t>(w3 >>8 & 0xFF);
    const uint8_t b8 =  static_cast<uint8_t>(w2 & 0xFF);
    const uint8_t b9 =  static_cast<uint8_t>(w2 >> 8 & 0xFF);
    const uint8_t b10 =  static_cast<uint8_t>(w1 & 0xFF);
    const uint8_t b11 =  static_cast<uint8_t>(w1 >> 8 & 0xFF);
    const uint8_t b12 =  static_cast<uint8_t>(w32 & 0xFF);
    const uint8_t b13 =  static_cast<uint8_t>(w32 >> 8 & 0xFF);
    const uint8_t b14 =  static_cast<uint8_t>(w32 >>16 & 0xFF);
    const uint8_t b15 =  static_cast<uint8_t>(w32 >> 24 & 0xFF);
    bt_uuid_128 uuid {
        .uuid = {.type = BT_UUID_TYPE_128},
        .val = {b0,b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13,b14,b15}
    };
    return uuid;
}


/*!
    * @brief        Create a Derived UUID from a Base UUID
    *
    * @param [in]    base    The 128 bit uuid base
    * @param [in]    uuid_short The 16 bit short uuid 
    * @details      Replaces the 2 bytes previous to the end of the base UUID 
                    Example
                    
                    Base UUID    6E400000-B5A3-F393-E0A9-E50E24DCCA9E
                    Derived UUID 6E40xxxx-B5A3-F393-E0A9-E50E24DCCA9E

                    where xxxx are the replaced bytes

    * @retval bt_uuid_128 UUID derived from base uuid
*/
constexpr bt_uuid_128 derive_uuid(bt_uuid_128 base,uint16_t uuid_short)
{
    const uint8_t uuid_short_msb = BT_UUID_SIZE_128-3;
    const uint8_t uuid_short_lsb = BT_UUID_SIZE_128-4;
    bt_uuid_128 derived_uuid = base;
    derived_uuid.val[uuid_short_msb] = uuid_short >> 8;
    derived_uuid.val[uuid_short_lsb] = uuid_short & 0xFF;
    return derived_uuid;
}
} // namespace ble_utils

