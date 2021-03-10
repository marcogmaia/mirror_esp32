/* ADC1 Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <cstring>
#include <stdio.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/adc.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_adc_cal.h"
#include "esp_log.h"
#include "ble_server.h"
#include "nvs_flash.h"

static constexpr gpio_num_t GPIO_LED_OUT_1 = GPIO_NUM_2;
static constexpr gpio_num_t GPIO_LED_OUT_2 = GPIO_NUM_15;

static constexpr uint32_t DEFAULT_VREF
    = 3300;  // Use adc2_vref_to_gpio() to obtain a better estimate
static constexpr uint32_t NO_OF_SAMPLES = 100;  // Multisampling

static esp_adc_cal_characteristics_t adc_chars;
static constexpr adc1_channel_t channel1 = ADC1_CHANNEL_6;  // GPIO34 if ADC1
static constexpr adc1_channel_t channel2 = ADC1_CHANNEL_7;  // GPIO35 if ADC1

static constexpr adc_unit_t unit   = ADC_UNIT_1;
static constexpr adc_atten_t atten = adc_atten_t::ADC_ATTEN_DB_11;
static constexpr adc_bits_width_t adc1_bit_width
    = adc_bits_width_t::ADC_WIDTH_BIT_11;
static constexpr ledc_timer_bit_t pwm_duty_resolution
    = ledc_timer_bit_t::LEDC_TIMER_11_BIT;

static void check_efuse(void) {
    // Check TP is burned into eFuse
    if(esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        printf("eFuse Two Point: Supported\n");
    }
    else {
        printf("eFuse Two Point: NOT supported\n");
    }

    // Check Vref is burned into eFuse
    if(esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        printf("eFuse Vref: Supported\n");
    }
    else {
        printf("eFuse Vref: NOT supported\n");
    }
}

static void print_char_val_type(esp_adc_cal_value_t val_type) {
    if(val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("Characterized using Two Point Value\n");
    }
    else if(val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("Characterized using eFuse Vref\n");
    }
    else {
        printf("Characterized using Default Vref\n");
    }
}


static inline void ledc_config() {
    ledc_channel_config_t chan0_conf = {
        .gpio_num   = GPIO_LED_OUT_1,
        .speed_mode = ledc_mode_t::LEDC_HIGH_SPEED_MODE,
        .channel    = ledc_channel_t::LEDC_CHANNEL_0,
        .intr_type  = ledc_intr_type_t::LEDC_INTR_DISABLE,
        .timer_sel  = ledc_timer_t::LEDC_TIMER_0,
        .duty       = 0,
        .hpoint     = 0,
    };

    ledc_channel_config_t chan1_conf = {
        .gpio_num   = GPIO_LED_OUT_2,
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
}

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
    // Check if Two Point or Vref are burned into eFuse
    // check_efuse();

    // // Configure ADC
    // adc1_config_width(adc1_bit_width);
    // adc1_config_channel_atten(channel1, atten);
    // adc1_config_channel_atten(channel2, atten);

    // Characterize ADC
    // memset(&adc_chars, 0, sizeof adc_chars);
    // esp_adc_cal_value_t val_type = esp_adc_cal_characterize(
    //     unit, atten, adc1_bit_width, DEFAULT_VREF, &adc_chars);
    // print_char_val_type(val_type);

    // set LEDC PWM
    ledc_config();

    // Continuously sample ADC1
    // float adc_reading0 = 0;
    // float adc_reading1 = 0;

    while(true) {
        // Multisampling
        // static constexpr float k = 0.9995f;
        // for(int i = 0; i < 100; ++i) {
        //     adc_reading0
        //         = k * adc_reading0 + (1.0f - k) * adc1_get_raw(channel1);
        //     adc_reading1
        //         = k * adc_reading1 + (1.0f - k) * adc1_get_raw(channel2);
        // }
        // int pwm_duty0 = (adc_reading0 >= 2030) ? 2048 : adc_reading0;
        // int pwm_duty1 = (adc_reading1 >= 2030) ? 2048 : adc_reading1;

        // static constexpr char TAG[] = "ADC_DEBUG";
        // ESP_LOGD(TAG, "%lf %lf %d %d", adc_reading0, adc_reading1, pwm_duty0,
        //        pwm_duty1);
        // // printf("%d %d\n", pwm_duty0, pwm_duty1);

        // // update PWM duty cycle.
        // {
        //     ledc_set_duty(ledc_mode_t::LEDC_HIGH_SPEED_MODE,
        //                   ledc_channel_t::LEDC_CHANNEL_0, pwm_duty0);
        //     ledc_set_duty(ledc_mode_t::LEDC_HIGH_SPEED_MODE,
        //                   ledc_channel_t::LEDC_CHANNEL_1, pwm_duty1);
        //     ledc_update_duty(ledc_mode_t::LEDC_HIGH_SPEED_MODE,
        //     LEDC_CHANNEL_0);
        //     ledc_update_duty(ledc_mode_t::LEDC_HIGH_SPEED_MODE,
        //     LEDC_CHANNEL_1);
        // }

        ledc_set_duty(ledc_mode_t::LEDC_HIGH_SPEED_MODE,
                      ledc_channel_t::LEDC_CHANNEL_0, 1792);
        ledc_set_duty(ledc_mode_t::LEDC_HIGH_SPEED_MODE,
                      ledc_channel_t::LEDC_CHANNEL_1, 128);
        ledc_update_duty(ledc_mode_t::LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
        ledc_update_duty(ledc_mode_t::LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_1);

        vTaskDelay((TickType_t(1)));
    }
}
