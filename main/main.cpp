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
#include <ctime>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "storage.hpp"
#include "leds.hpp"
#include "ble_server.h"

constexpr auto *TAG = "MAIN";

extern "C" void app_main(void) {
    storage::init();
    leds::init();
    nimble_ble_init();
}
