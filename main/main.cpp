/**
 * @file main.cpp
 * @author Marco A. G. Maia (marcogmaia@gmail.com)
 * @brief
 * @version 0.1
 * @date 2021-03-10
 *
 * @copyright Copyright (c) 2021
 *
 *
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ble_server.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "leds.hpp"

// init flash because BLE needs it
static void flash_init() {
    auto res = nvs_flash_init();
    if(res == ESP_ERR_NVS_NO_FREE_PAGES
       || res == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        res = nvs_flash_init();
    }
    ESP_ERROR_CHECK(res);
}

extern "C" void app_main(void) {
    flash_init();
    nimble_ble_init();
    leds::init();
}
