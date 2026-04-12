
set(SOURCES  Mqtt.cpp)
idf_component_register(
    SRCS ${SOURCES}

    INCLUDE_DIRS
        "."

    REQUIRES
        mqtt
    EMBED_FILES "isrgrootx1.pem"
)