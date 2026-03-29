
set(SOURCES OTA.c)
idf_component_register(
    SRCS ${SOURCES}

    INCLUDE_DIRS
        "."

    REQUIRES
        nvs_flash app_update esp_http_client esp_https_ota esp_wifi
    
)