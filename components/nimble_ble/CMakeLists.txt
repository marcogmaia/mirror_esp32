idf_component_register(
    SRCS
    "ble_server.cpp"
    "gatt_server.cpp"
    "misc.cpp"
    
    INCLUDE_DIRS 
        "."
        "include"

    PRIV_REQUIRES

    REQUIRES bt nvs_flash log leds
)
