#pragma once

#include "driver/gpio.h"

namespace board_configs {

constexpr gpio_num_t GPIO_LED_IN  = GPIO_NUM_2;
constexpr gpio_num_t GPIO_LED_OUT = GPIO_NUM_15;

constexpr uint32_t default_task_priority = 5;

}  // namespace board_configs
