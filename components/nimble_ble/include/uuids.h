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

// e7 94 6a 77-56 1c-4e 63-aa e7-b5 d6 a9 e1 55 25
// e7946a77-561c-4e63-aae7-b5d6a9e15525
#define GATT_SVC_ADV_UUID                                                      \
    BLE_UUID128_INIT(0x25, 0x55, 0xe1, 0xa9, 0xd6, 0xb5, 0xe7, 0xaa, 0x63, \
                     0x4e, 0x1c, 0x56, 0x77, 0x64, 0x94, 0xe7);

// 18 79 22 4c-93 58-4b e2-80 89-57 50 ca 67 75 6c
// 1879224c-9358-4be2-8089-5750ca67756c
#define GATT_CHAR_BRIGHTNESS_UUID                                          \
    BLE_UUID128_INIT(0x6c, 0x75, 0x67, 0xca, 0x50, 0x57, 0x89, 0x80, 0xe2, \
                     0x4b, 0x58, 0x93, 0x4c, 0x22, 0x79, 0x18);

#ifdef __cplusplus
}
#endif
