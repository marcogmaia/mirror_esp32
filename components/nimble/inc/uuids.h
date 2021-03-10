#pragma once
/*
e7946a77-561c-4e63-aae7-b5d6a9e15525
1879224c-9358-4be2-8089-5750ca67756c
46ac1f62-7e90-4d56-9adb-31e3663bb755
24b83068-e707-4a19-b595-09cd62fb1b8c
afe05301-efc2-4fb4-8bca-35446dae2f46
9593a690-3529-4a9b-bbf0-673d3cb52692
*/


#include "host/ble_uuid.h"


#ifdef __cplusplus
extern "C" {
#endif

#undef BLE_UUID128_INIT
#define BLE_UUID128_INIT(uuid128...) \
    {                                \
        BLE_UUID_TYPE_128, {         \
            uuid128                  \
        }                            \
    }


/* 5f8fa7ee-a9b8-11e9-a2a3-2a2ae2dbcce4 */
#define GATT_SVC_1_UUID                                                    \
    BLE_UUID128_INIT(0xe4, 0xcc, 0xdb, 0xe2, 0x2a, 0x2a, 0xa3, 0xa2, 0xe9, \
                     0x11, 0xb8, 0xa9, 0xee, 0xa7, 0x8f, 0x5f)

/* 5f8fabc2-a9b8-11e9-a2a3-2a2ae2dbcce4 */
#define GATT_CHR_PRICES_UUID                                               \
    BLE_UUID128_INIT(0xe4, 0xcc, 0xdb, 0xe2, 0x2a, 0x2a, 0xa3, 0xa2, 0xe9, \
                     0x11, 0xb8, 0xa9, 0xc2, 0xab, 0x8f, 0x5f)

/* 5f8fad2a-a9b8-11e9-a2a3-2a2ae2dbcce4 */
#define GATT_CHR_2_UUID                                                    \
    BLE_UUID128_INIT(0xe4, 0xcc, 0xdb, 0xe2, 0x2a, 0x2a, 0xa3, 0xa2, 0xe9, \
                     0x11, 0xb8, 0xa9, 0x2a, 0xad, 0x8f, 0x5f)

/* 5f8faea6-a9b8-11e9-a2a3-2a2ae2dbcce4 */
#define GATT_CHR_DATETIME_UUID                                             \
    BLE_UUID128_INIT(0xe4, 0xcc, 0xdb, 0xe2, 0x2a, 0x2a, 0xa3, 0xa2, 0xe9, \
                     0x11, 0xb8, 0xa9, 0xa6, 0xae, 0x8f, 0x5f)

/* 5f8fb2ca-a9b8-11e9-a2a3-2a2ae2dbcce4 */
#define GATT_CHR_LUMINOSITY_UUID                                           \
    BLE_UUID128_INIT(0xe4, 0xcc, 0xdb, 0xe2, 0x2a, 0x2a, 0xa3, 0xa2, 0xe9, \
                     0x11, 0xb8, 0xa9, 0xca, 0xb2, 0x8f, 0x5f)

/* 5f8fb428-a9b8-11e9-a2a3-2a2ae2dbcce4 */
#define GATT_CHR_LUM_SCHED_UUID                                            \
    BLE_UUID128_INIT(0xe4, 0xcc, 0xdb, 0xe2, 0x2a, 0x2a, 0xa3, 0xa2, 0xe9, \
                     0x11, 0xb8, 0xa9, 0x28, 0xb4, 0x8f, 0x5f)

/* 5f8fb55e-a9b8-11e9-a2a3-2a2ae2dbcce4 */
#define GATT_CHR_CHANGE_PASS_UUID                                          \
    BLE_UUID128_INIT(0xe4, 0xcc, 0xdb, 0xe2, 0x2a, 0x2a, 0xa3, 0xa2, 0xe9, \
                     0x11, 0xb8, 0xa9, 0x5e, 0xb5, 0x8f, 0x5f)


#ifdef __cplusplus
}
#endif
