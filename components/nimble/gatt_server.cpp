/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <cassert>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <utility>

// #include "rtc.hpp"
// #include "display.hpp"
// #include "panel_controller.hpp"
// #include "ble_message_types.hpp"

#include "esp_err.h"
#include "esp_log.h"

#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "host/ble_hs.h"
#include "ble_server.h"
#include "uuids.h"


static constexpr auto* TAG = "GATT";

/* UUIDs of Services and Characteristics */
/* 5f8fa7ee-a9b8-11e9-a2a3-2a2ae2dbcce4 */
static const ble_uuid128_t gatt_svr_svc_uuid = GATT_SVC_1_UUID;
/* 5f8fabc2-a9b8-11e9-a2a3-2a2ae2dbcce4 */
static const ble_uuid128_t gatt_svr_chr_prices_uuid = GATT_CHR_PRICES_UUID;
/* 5f8faea6-a9b8-11e9-a2a3-2a2ae2dbcce4 */
static const ble_uuid128_t gatt_svr_chr_datetime_uuid = GATT_CHR_DATETIME_UUID;
/* 5f8fb428-a9b8-11e9-a2a3-2a2ae2dbcce4 */
static const ble_uuid128_t gatt_svr_chr_sched_uuid = GATT_CHR_LUM_SCHED_UUID;
/* 5f8fb55e-a9b8-11e9-a2a3-2a2ae2dbcce4 */
static const ble_uuid128_t gatt_svr_chr_change_pass_uuid
    = GATT_CHR_CHANGE_PASS_UUID;

// namespace guid {
//    constexpr ble_uuid128_t 
// }

static int gatt_svr_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt* ctxt, void* arg);


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        /*** Service: Security test. */
        .type            = BLE_GATT_SVC_TYPE_PRIMARY,  // type
        .uuid            = &gatt_svr_svc_uuid.u,       // uuid
        .includes        = nullptr,
        .characteristics =  
        (struct ble_gatt_chr_def[]){
            {
                // .uuid      = &guid_char_brightness.u,   // uuid
                .access_cb = gatt_svr_chr_access,  // access_cb
                // nullptr, // nullptr,
                .flags = BLE_GATT_CHR_F_WRITE,  // | BLE_GATT_CHR_F_WRITE_ENC,
                                                // flags
            },
            {
                0, /* No more characteristics in this service. */
            },
        },
    },
    {
        0, /* No more services. */
    },
};

#pragma GCC diagnostic pop

static int gatt_svr_chr_write(struct os_mbuf* om, uint16_t min_len,
                              uint16_t max_len, void* dst, uint16_t* len) {
    uint16_t om_len = 0;
    int rc          = -1;

    om_len = OS_MBUF_PKTLEN(om);
    if(om_len < min_len || om_len > max_len) {
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    }

    rc = ble_hs_mbuf_to_flat(om, dst, max_len, len);
    if(rc != 0) {
        return BLE_ATT_ERR_UNLIKELY;
    }

    return 0;
}

bool pass_invalid(uint32_t received_pass) {
    // TODO
    return true;
}

static int gatt_svr_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt* ctxt, void* arg) {
    int rc              = 0;
    uint8_t buffer[256] = {0};
    auto* p_data        = buffer;
    const auto* uuid    = ctxt->chr->uuid;
    uint16_t len        = 0;
    rc                  = gatt_svr_chr_write(ctxt->om, 0, 256, buffer, &len);


    // check if password is valid
    uint32_t received_password = 0;
    memcpy(&received_password, p_data, 4);
    len -= sizeof received_password;
    p_data += sizeof received_password;

    // if invalid pass
    if(pass_invalid(received_password)) {
        ESP_LOGI(TAG, "Invalid pass.");
        return rc;
    }


    time_t datetime = 0;
    memcpy(&datetime, p_data, sizeof datetime);
    len -= sizeof datetime;
    p_data += sizeof datetime;

    if(datetime != 0) {
        struct tm tm = *localtime(&datetime);
        ESP_LOGI(TAG, "\n\tdatetime:\n\t%02d:%02d:%02d - %02d/%02d/%02d",
                 tm.tm_hour, tm.tm_min, tm.tm_sec, tm.tm_mday, tm.tm_mon + 1,
                 tm.tm_year);
        tm.tm_year += 1900;
    }


    /* Determine which characteristic is being accessed by examining its
     * 128-bit UUID.
     */

    /// change pass
    if(ble_uuid_cmp(uuid, &gatt_svr_chr_change_pass_uuid.u) == 0) {
        uint32_t newpass = 0;
        // rc = gatt_svr_chr_write(ctxt->om, sizeof pass, sizeof pass, &pass,
        //                         nullptr);
        memcpy(&newpass, p_data, sizeof newpass);


        // print changed pass info to user and to log
        static char str_pass[16];
        snprintf(str_pass, 15, "%06d", newpass);
        ESP_LOGI(TAG, "Password changed to %s", str_pass);

        return rc;
    }

    /* Unknown characteristic; the nimble stack should not have called this
     * function.
     */
    // assert(0);
    return BLE_ATT_ERR_UNLIKELY;
}

void gatt_svr_register_cb(struct ble_gatt_register_ctxt* ctxt, void* arg) {
    char buf[BLE_UUID_STR_LEN];

    switch(ctxt->op) {
        case BLE_GATT_REGISTER_OP_SVC:
            ESP_LOGD(__FILE__, "registered service %s with handle=%d\n",
                     ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
                     ctxt->svc.handle);
            break;

        case BLE_GATT_REGISTER_OP_CHR:
            ESP_LOGD(__FILE__,
                     "registering characteristic %s with "
                     "def_handle=%d val_handle=%d\n",
                     ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
                     ctxt->chr.def_handle, ctxt->chr.val_handle);
            break;

        case BLE_GATT_REGISTER_OP_DSC:
            ESP_LOGD(__FILE__, "registering descriptor %s with handle=%d\n",
                     ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
                     ctxt->dsc.handle);
            break;

        default:
            assert(0);
            break;
    }
}

int gatt_svr_init(void) {
    int rc = -1;

    ble_svc_gap_init();
    ble_svc_gatt_init();

    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    if(rc != 0) {
        return rc;
    }

    rc = ble_gatts_add_svcs(gatt_svr_svcs);
    if(rc != 0) {
        return rc;
    }

    return 0;
}
