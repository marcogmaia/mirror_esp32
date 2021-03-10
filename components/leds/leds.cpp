/**
 * @file leds.cpp
 * @author Marco A. G. Maia (marcogmaia@gmail.com)
 * @brief
 * @version 0.1
 * @date 2021-03-10
 *
 * @copyright Copyright (c) 2021
 *
 */

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "leds.hpp"
#include "board_configs.hpp"

namespace leds {

namespace {

constexpr ledc_timer_bit_t pwm_duty_resolution
    = ledc_timer_bit_t::LEDC_TIMER_11_BIT;

QueueHandle_t q_brightness = xQueueCreate(sizeof(message_t), 4);
void task(void* ignore) {
    while(true) {
        message_t message;
        xQueueReceive(q_brightness, &message, portMAX_DELAY);
        set_brightness(message.channel, message.brightness);
    }
}

}  // namespace


void init() {
    static bool initialized = false;
    if(initialized) {
        return;
    }
    initialized = true;

    ledc_channel_config_t chan0_conf = {
        .gpio_num   = board_configs::GPIO_LED_IN,
        .speed_mode = ledc_mode_t::LEDC_HIGH_SPEED_MODE,
        .channel    = ledc_channel_t::LEDC_CHANNEL_0,
        .intr_type  = ledc_intr_type_t::LEDC_INTR_DISABLE,
        .timer_sel  = ledc_timer_t::LEDC_TIMER_0,
        .duty       = 0,
        .hpoint     = 0,
    };

    ledc_channel_config_t chan1_conf = {
        .gpio_num   = board_configs::GPIO_LED_OUT,
        .speed_mode = ledc_mode_t::LEDC_HIGH_SPEED_MODE,
        .channel    = ledc_channel_t::LEDC_CHANNEL_1,
        .intr_type  = ledc_intr_type_t::LEDC_INTR_DISABLE,
        .timer_sel  = ledc_timer_t::LEDC_TIMER_0,
        .duty       = 0,
        .hpoint     = 0,
    };

    ledc_timer_config_t timer_conf = {
        .speed_mode      = ledc_mode_t::LEDC_HIGH_SPEED_MODE,
        .duty_resolution = pwm_duty_resolution,
        .timer_num       = ledc_timer_t::LEDC_TIMER_0,
        .freq_hz         = 20000,
        .clk_cfg         = ledc_clk_cfg_t::LEDC_AUTO_CLK,
    };

    ledc_timer_config(&timer_conf);
    ledc_channel_config(&chan0_conf);
    ledc_channel_config(&chan1_conf);

    xTaskCreatePinnedToCore(task, "ledsTask", configMINIMAL_STACK_SIZE * 3,
                            nullptr, 5, nullptr, APP_CPU_NUM);
}

void set_brightness(channel_t channel, uint8_t brightness) {
    // max_bri == 2048
    constexpr auto max_bri = 1 << pwm_duty_resolution;
    uint32_t duty
        = brightness * static_cast<double>(max_bri) / static_cast<double>(100);
    // channel0 == IN_LEDS
    if(channel == channel_t::channel0) {
        ledc_set_duty(ledc_mode_t::LEDC_HIGH_SPEED_MODE,
                      ledc_channel_t::LEDC_CHANNEL_0, duty);
        ledc_update_duty(ledc_mode_t::LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
    }
    // channel1 == OUT_LEDS
    else {
        ledc_set_duty(ledc_mode_t::LEDC_HIGH_SPEED_MODE,
                      ledc_channel_t::LEDC_CHANNEL_1, duty);
        ledc_update_duty(ledc_mode_t::LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1);
    }
}

}  // namespace leds
