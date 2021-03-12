/**
 * @file storage.cpp
 * @author Marco A. G. Maia (marcogmaia@gmail.com)
 * @brief
 * @version 0.1
 * @date 2021-03-12
 *
 * @copyright Copyright (c) 2021
 *
 */
#include <cstring>

#include "nvs.h"
#include "nvs_flash.h"

#include "storage.hpp"
#include "esp_log.h"


namespace storage {

namespace {

constexpr auto *TAG   = "STORAGE";
constexpr auto *nvkey = "storage";

static void flash_init() {
    auto res = nvs_flash_init();
    if(res == ESP_ERR_NVS_NO_FREE_PAGES
       || res == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        res = nvs_flash_init();
    }
    ESP_ERROR_CHECK(res);

    res = nvs_flash_init_partition(nvkey);
    if(res == ESP_ERR_NVS_NO_FREE_PAGES
       || res == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase_partition(nvkey));
        res = nvs_flash_init_partition(nvkey);
    }
    ESP_ERROR_CHECK(res);
}

}  // namespace

void get_values(uint8_t (&bris)[2]) {
    nvs_handle_t nvhandle;
    esp_err_t ret;
    ESP_ERROR_CHECK(
        ret = nvs_open_from_partition(
            nvkey, nvkey, nvs_open_mode_t::NVS_READWRITE, &nvhandle));
    uint16_t bri16;
    ret = nvs_get_u16(nvhandle, nvkey, &bri16);
    nvs_close(nvhandle);
    memcpy(bris, &bri16, sizeof bris);
}

void set_values(const uint8_t (&bris)[2]) {
    nvs_handle_t nvhandle;
    esp_err_t ret;
    ESP_ERROR_CHECK(
        ret = nvs_open_from_partition(
            nvkey, nvkey, nvs_open_mode_t::NVS_READWRITE, &nvhandle));
    // if(ret != ESP_OK) return;
    uint16_t new_bri16;
    uint16_t old_bri16;
    memcpy(&new_bri16, bris, sizeof new_bri16);
    ret = nvs_get_u16(nvhandle, nvkey, &old_bri16);
    if(new_bri16 != old_bri16) {
        ESP_ERROR_CHECK(ret = nvs_set_u16(nvhandle, nvkey, new_bri16));
        ESP_ERROR_CHECK(ret = nvs_commit(nvhandle));
        ESP_LOGI(TAG, "values set to: %03u, %03u", bris[0], bris[1]);
    }
    nvs_close(nvhandle);
}

void init() {
    static bool initialized = false;
    if(initialized) {
        return;
    }
    initialized = true;

    flash_init();
}

}  // namespace storage
