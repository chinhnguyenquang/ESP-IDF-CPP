idf_component_register(
    SRCS "Src/main.cpp"
    INCLUDE_DIRS "Inc"
    REQUIRES freertos esp_common Gpio  esp_event nvs_flash WiFi Nvs32
                                                                  
)