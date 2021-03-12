#pragma once

#include "driver/ledc.h"

namespace leds {


enum channel_t : uint8_t {
    channel0 = ledc_channel_t::LEDC_CHANNEL_0,
    channel1 = ledc_channel_t::LEDC_CHANNEL_1,
};

struct message_t {
    channel_t channel;
    uint8_t brightness;
};


/**
 * @brief initialize PWM for the 2 channels to control the LEDs
 */
void init();

// /**
//  * @brief Set the brightness object
//  *
//  * @param channel
//  * @param brightness [0-100]
//  */
// void set_brightness(channel_t channel, uint8_t brightness);

void push_message(const message_t &message);

}  // namespace leds