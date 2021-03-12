idf_component_register(
    SRCS "main.cpp"
    
    INCLUDE_DIRS ""

    PRIV_REQUIRES esp_adc_cal

    REQUIRES
    nvs_flash nimble_ble leds storage
)
