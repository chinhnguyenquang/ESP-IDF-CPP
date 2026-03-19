set(SOURCES Wifi_pro.cpp SntpTime.cpp)
idf_component_register(
    SRCS ${SOURCES}

    INCLUDE_DIRS
        "."

    REQUIRES
        esp_wifi
        esp_event
        esp_netif
        nvs_flash
        lwip
        wifi_provisioning
        espressif__qrcode
        bt
        protocomm
)