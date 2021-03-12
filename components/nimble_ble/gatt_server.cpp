/**
 * @file gatt_server.cpp
 * @author Marco A. G. Maia (marcogmaia@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-03-12
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <cassert>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <utility>

#include "esp_err.h"
#include "esp_log.h"

#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "host/ble_hs.h"
#include "ble_server.h"
#include "uuids.h"

#include "leds.hpp"

static constexpr auto* TAG = "GATT";

/* UUIDs of Services and Characteristics */
static constexpr ble_uuid128_t uuid_svc_adv         = GATT_SVC_ADV_UUID;
static constexpr ble_uuid128_t uuid_char_brightness = GATT_CHAR_BRIGHTNESS_UUID;


static int gatt_svr_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt* ctxt, void* arg);


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        .type            = BLE_GATT_SVC_TYPE_PRIMARY,  // type
        .uuid            = &uuid_svc_adv.u,       // uuid
        .includes        = nullptr,
        .characteristics =  
        (struct ble_gatt_chr_def[]){
            {
                // .uuid      = &guid_char_brightness.u,   // uuid
                .uuid      = &uuid_char_brightness.u,   // uuid
                .access_cb = gatt_svr_chr_access,  // access_cb
                // nullptr, // nullptr,
                .flags = BLE_GATT_CHR_F_WRITE,  // | BLE_GATT_CHR_F_WRITE_ENC,
                                                // flags
            },
            {
                0, // No more characteristics in this service.
            },
        },
    },
    {
        0,  // No more services.
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
    // TODO pass_invalid ?
    return false;
}

static int gatt_svr_chr_access(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt* ctxt, void* arg) {
    uint8_t buffer[256] = {0};
    auto* p_data        = buffer;
    const auto* uuid    = ctxt->chr->uuid;
    uint16_t len        = 0;
    int rc              = gatt_svr_chr_write(ctxt->om, 0, 256, buffer, &len);

    // Determine which characteristic is being accessed by examining its
    // ! 128-bit UUID.

    // update brightness
    if(ble_uuid_cmp(uuid, &uuid_char_brightness.u) == 0) {
        leds::message_t message;
        constexpr auto msize = sizeof message;
        rc = gatt_svr_chr_write(ctxt->om, msize, msize, &message, nullptr);
        if(rc == 0) {
            leds::push_message(message);
        }
        return rc;
    }

    // Unknown characteristic; the nimble stack should not have called this
    // function.
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
