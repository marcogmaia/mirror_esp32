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

#include "esp_bt.h"
#include "esp_log.h"

/* BLE */
#include "esp_nimble_hci.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "host/ble_hs_pvcy.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "ble_server.h"
#include "services/gap/ble_svc_gap.h"

#include "uuids.h"

static const char *tag = "NimBLE_BLE_PRPH";
static int bleprph_gap_event(struct ble_gap_event *event, void *arg);
static uint8_t own_addr_type;

extern "C" void ble_store_config_init(void);

/**
 * Logs information about a connection to the console.
 */
static void bleprph_print_conn_desc(struct ble_gap_conn_desc *desc) {
    MODLOG_DFLT(INFO, "handle=%d\n\tour_ota_addr_type=%d\n\tour_ota_addr=",
                desc->conn_handle, desc->our_ota_addr.type);
    print_addr(desc->our_ota_addr.val);
    MODLOG_DFLT(INFO,
                "\n\tour_id_addr_type=%d our_id_addr=", desc->our_id_addr.type);
    print_addr(desc->our_id_addr.val);
    MODLOG_DFLT(INFO, "\n\tpeer_ota_addr_type=%d peer_ota_addr=",
                desc->peer_ota_addr.type);
    print_addr(desc->peer_ota_addr.val);
    MODLOG_DFLT(INFO, "\n\tpeer_id_addr_type=%d peer_id_addr=",
                desc->peer_id_addr.type);
    print_addr(desc->peer_id_addr.val);
    MODLOG_DFLT(INFO,
                "\n\tconn_itvl=%d conn_latency=%d supervision_timeout=%d "
                "encrypted=%d authenticated=%d bonded=%d\n",
                desc->conn_itvl, desc->conn_latency, desc->supervision_timeout,
                desc->sec_state.encrypted, desc->sec_state.authenticated,
                desc->sec_state.bonded);
}

/**
 * Enables advertising with the following parameters:
 *     o General discoverable mode.
 *     o Undirected connectable mode.
 */
static void bleprph_advertise(void) {
    static struct ble_gap_adv_params adv_params;
    static struct ble_hs_adv_fields fields;
    static const char *name = ble_svc_gap_device_name();

    /**
     *  Set the advertisement data included in our advertisements:
     *     o Flags (indicates advertisement type and other general info).
     *     o Device name.
     *     o one 128-bit service UUID.
     */

    /* Advertise two flags:
     *     o Discoverability in forthcoming advertisement (general)
     *     o BLE-only (BR/EDR unsupported).
     */
    if(!ble_gap_adv_active()) {
        memset(&fields, 0, sizeof fields);
        fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

        // Indicate that the TX power level field should NOT be included;
        fields.tx_pwr_lvl_is_present = 0;
        fields.tx_pwr_lvl            = BLE_HS_ADV_TX_PWR_LVL_AUTO;

        fields.name             = (uint8_t *)name;
        fields.name_len         = strlen(name);
        fields.name_is_complete = 1;

        /* No memory for 128 bit adv (error msg size) */
        static ble_uuid128_t adv_svc_uuid = GATT_SVC_ADV_UUID;
        fields.uuids128                   = &adv_svc_uuid;
        fields.num_uuids128               = 1;
        fields.uuids128_is_complete       = 1;

        int rc = ble_gap_adv_set_fields(&fields);
        if(rc != 0) {
            MODLOG_DFLT(ERROR, "error setting advertisement data; rc=%d\n", rc);
            return;
        }

        /* Begin advertising. */
        memset(&adv_params, 0, sizeof adv_params);
        adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
        adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;


        rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER, &adv_params,
                               bleprph_gap_event, NULL);
        if(rc != 0) {
            MODLOG_DFLT(ERROR, "error enabling advertisement; rc=%d\n", rc);
        }
    }
    else {
        MODLOG_DFLT(INFO, "Advertise already active.\n");
    }
    return;
}

/**
 * The nimble host executes this callback when a GAP event occurs.  The
 * application associates a GAP event callback with each connection that forms.
 * bleprph uses the same callback for all connections.
 *
 * @param event                 The type of event being signalled.
 * @param ctxt                  Various information pertaining to the event.
 * @param arg                   Application-specified argument; unuesd by
 *                                  bleprph.
 *
 * @return                      0 if the application successfully handled the
 *                                  event; nonzero on failure.  The semantics
 *                                  of the return code is specific to the
 *                                  particular GAP event being signalled.
 */
static int bleprph_gap_event(struct ble_gap_event *event, void *arg) {
    struct ble_gap_conn_desc desc;
    int rc;

    switch(event->type) {
        case BLE_GAP_EVENT_CONNECT: {
            int cnt = 0;
            ble_store_util_count(BLE_STORE_OBJ_TYPE_OUR_SEC, &cnt);
            if(cnt > 2) {
                ble_store_util_delete_oldest_peer();
                MODLOG_DFLT(INFO, "Deleting oldest peer, cnt: %d\n", cnt);
            }

            /* A new connection was established or a connection attempt failed.
             */
            MODLOG_DFLT(INFO, "connection %s; status=%d ",
                        event->connect.status == 0 ? "established" : "failed",
                        event->connect.status);

            if(event->connect.status == 0) {
                rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
                assert(rc == 0);
                bleprph_print_conn_desc(&desc);
                MODLOG_DFLT(INFO, "\n");

                // ble_gap_security_initiate(event->connect.conn_handle);
            }

            bleprph_advertise();

            return 0;
        }

        case BLE_GAP_EVENT_DISCONNECT:
            MODLOG_DFLT(INFO, "disconnect; reason=%d ",
                        event->disconnect.reason);
            bleprph_print_conn_desc(&event->disconnect.conn);
            MODLOG_DFLT(INFO, "\n");

            /* Connection terminated; resume advertising. */
            bleprph_advertise();
            return 0;

        case BLE_GAP_EVENT_CONN_UPDATE:
            /* The central has updated the connection parameters. */
            MODLOG_DFLT(INFO, "connection updated; status=%d ",
                        event->conn_update.status);
            rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
            assert(rc == 0);
            bleprph_print_conn_desc(&desc);
            MODLOG_DFLT(INFO, "\n");
            return 0;

        case BLE_GAP_EVENT_ADV_COMPLETE:
            MODLOG_DFLT(INFO, "advertise complete; reason=%d",
                        event->adv_complete.reason);
            bleprph_advertise();
            return 0;

        case BLE_GAP_EVENT_ENC_CHANGE:
            /* Encryption has been enabled or disabled for this connection. */
            MODLOG_DFLT(INFO, "encryption change event; status=%d ",
                        event->enc_change.status);
            rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
            assert(rc == 0);
            bleprph_print_conn_desc(&desc);
            MODLOG_DFLT(INFO, "\n");
            return 0;

        case BLE_GAP_EVENT_SUBSCRIBE:
            MODLOG_DFLT(
                INFO,
                "subscribe event; conn_handle=%d attr_handle=%d "
                "reason=%d prevn=%d curn=%d previ=%d curi=%d\n",
                event->subscribe.conn_handle, event->subscribe.attr_handle,
                event->subscribe.reason, event->subscribe.prev_notify,
                event->subscribe.cur_notify, event->subscribe.prev_indicate,
                event->subscribe.cur_indicate);
            return 0;

        case BLE_GAP_EVENT_MTU:
            MODLOG_DFLT(INFO,
                        "mtu update event; conn_handle=%d cid=%d mtu=%d\n",
                        event->mtu.conn_handle, event->mtu.channel_id,
                        event->mtu.value);
            return 0;

        case BLE_GAP_EVENT_REPEAT_PAIRING:
            /* We already have a bond with the peer, but it is attempting to
             * establish a new secure link.  This app sacrifices security for
             * convenience: just throw away the old bond and accept the new
             * link.
             */

            /* Delete the old bond. */
            rc = ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc);
            assert(rc == 0);
            ble_store_util_delete_peer(&desc.peer_id_addr);

            /* Return BLE_GAP_REPEAT_PAIRING_RETRY to indicate that the host
             * should continue with the pairing operation.
             */
            return BLE_GAP_REPEAT_PAIRING_RETRY;

        case BLE_GAP_EVENT_PASSKEY_ACTION:
            ESP_LOGI(tag, "PASSKEY_ACTION_EVENT started \n");
            struct ble_sm_io pkey;
            memset(&pkey, 0, sizeof pkey);

            if(event->passkey.params.action == BLE_SM_IOACT_DISP) {
                pkey.action = event->passkey.params.action;

                // i2c::ds3232::read_ram(0, &pkey.passkey, sizeof pkey.passkey);
                // ssd1306::show_password(pkey.passkey);

                ESP_LOGI(tag, "Enter passkey %06d on the peer side",
                         pkey.passkey);
                rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
                ESP_LOGI(tag, "ble_sm_inject_io result: %d\n", rc);
            }
            else if(event->passkey.params.action == BLE_SM_IOACT_OOB) {
                static uint8_t tem_oob[16] = {0};
                pkey.action                = event->passkey.params.action;
                for(int i = 0; i < 16; i++) {
                    pkey.oob[i] = tem_oob[i];
                }
                rc = ble_sm_inject_io(event->passkey.conn_handle, &pkey);
                ESP_LOGI(tag, "ble_sm_inject_io result: %d\n", rc);
            }
            return 0;
    }
    return 0;
}

static void bleprph_on_reset(int reason) {
    MODLOG_DFLT(ERROR, "Resetting state; reason=%d\n", reason);
}

static void bleprph_on_sync(void) {
    int rc;

    // ble_hs_pvcy_rpa_config(1);
    rc = ble_hs_util_ensure_addr(0);
    assert(rc == 0);
    // own_addr_type = BLE_ADDR_RANDOM_ID;

    /* Figure out address to use while advertising (no privacy for now) */
    rc = ble_hs_id_infer_auto(BLE_ADDR_PUBLIC, &own_addr_type);
    if(rc != 0) {
        MODLOG_DFLT(ERROR, "error determining address type; rc=%d\n", rc);
        return;
    }

    /* Printing ADDR */
    uint8_t addr_val[6] = {0};
    rc                  = ble_hs_id_copy_addr(own_addr_type, addr_val, NULL);

    MODLOG_DFLT(INFO, "Device Address: ");
    print_addr(addr_val);
    MODLOG_DFLT(INFO, "\n");

    /* Begin advertising. */
    bleprph_advertise();
}

void bleprph_host_task(void *param) {
    ESP_LOGI(tag, "BLE Host Task Started");
    /* This function will return only when nimble_port_stop() is executed */
    nimble_port_run();

    nimble_port_freertos_deinit();
}


void nimble_ble_init(void) {
    ESP_ERROR_CHECK(esp_nimble_hci_and_controller_init());

    nimble_port_init();

    /* Initialize the NimBLE host configuration. */
    ble_hs_cfg.reset_cb          = bleprph_on_reset;
    ble_hs_cfg.sync_cb           = bleprph_on_sync;
    ble_hs_cfg.gatts_register_cb = gatt_svr_register_cb;
    ble_hs_cfg.store_status_cb   = ble_store_util_status_rr;

    ble_hs_cfg.sm_io_cap = BLE_SM_IO_CAP_NO_IO;

    /* bonding habilitado, enquanto a ESP32 permanecer ligada */
    // ble_hs_cfg.sm_bonding = 1; /* bonding */

    /**
     * os Android mais atuais, frequentemente mudam o endereço MAC que está
     * conectando, essa feature se chama RPA (Resolvable Private Address), uma
     * vez pareado, o controller pode enviar o MAC da máquina, ao invés do MAC
     * aleatório
     */
    // ble_hs_cfg.sm_our_key_dist   = BLE_SM_PAIR_KEY_DIST_ID;
    // ble_hs_cfg.sm_their_key_dist = BLE_SM_PAIR_KEY_DIST_ID;

    // ble_hs_cfg.sm_mitm = 1;
    // ble_hs_cfg.sm_sc   = 1; /* secure connection */


    int rc = gatt_svr_init();
    assert(rc == 0);

#define STRHELPER(x) #x
#define STR(x)       STRHELPER(x)
    static_assert(
        sizeof(CONFIG_BT_NIMBLE_SVC_GAP_DEVICE_NAME) - 1
            <= CONFIG_BT_NIMBLE_GAP_DEVICE_NAME_MAX_LEN,
        "DEVICE_NAME must be <= " STR(
            CONFIG_BT_NIMBLE_GAP_DEVICE_NAME_MAX_LEN) " characters long");

    /* Set the default device name. */
    rc = ble_svc_gap_device_name_set(CONFIG_BT_NIMBLE_SVC_GAP_DEVICE_NAME);
    // #ifdef sizeof(CONFIG_BT_NIMBLE_SVC_GAP_DEVICE_NAME)
    assert(rc == 0);

    // XXX Need to have template for store
    ble_store_config_init();

    nimble_port_freertos_init(bleprph_host_task);

    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_P9);
}
