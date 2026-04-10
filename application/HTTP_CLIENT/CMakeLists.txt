set(SOURCES httpClient.cpp)
idf_component_register(
    SRCS ${SOURCES}

    INCLUDE_DIRS
        "."

    REQUIRES
        esp_http_client
    
)