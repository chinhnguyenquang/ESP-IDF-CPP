#pragma once 

#include <string>
#include <vector>
#include <mqtt_client.h>
#include <functional>
#include "esp_log.h"  
#include <mutex>





extern const uint8_t isrgrootx1_pem_start[] asm("_binary_isrgrootx1_pem_start");
extern const uint8_t isrgrootx1_pem_end[] asm("_binary_isrgrootx1_pem_end"); //broken beehive

namespace ESP32MQTT {
    class MQTTClient {

    esp_mqtt_client_config_t _mqtt_config; // C so different naming
    esp_mqtt_client_handle_t _mqtt_client;

    static constexpr uint16_t DEFAULT_PACKET_SIZE = 1024;
    static constexpr uint16_t URI_BUFFER_SIZE = 200;

    static constexpr const char *_logtag = "MQTTClient";
    //! Routes c-style callback to c++ handler
    static void HandleGlobalEvent(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
    void event_handler(esp_event_base_t base, int32_t event_id, void *event_data);
    typedef std::function<void(const std::string &topicStr, const std::string &message)> MessageReceivedCallbackWithTopic;;

    struct TopicSubscriptionRecord
    {
        std::string topic;
        int msg_id;
        int qos;
        bool subscribed;

        MessageReceivedCallbackWithTopic callbackWithTopic;
    };
    std::vector<TopicSubscriptionRecord> _topicSubscriptionList;

    public:
        MQTTClient(const char *uri,const char *username, const char *password )
        {
            _mqtt_config.broker.address.uri = uri; //"mqtts://your-endpoint:8883"
            _mqtt_config.credentials.username = username;
            _mqtt_config.credentials.authentication.password = password;
           _mqtt_config.buffer.out_size= DEFAULT_PACKET_SIZE;
           _mqtt_config.buffer.size= DEFAULT_PACKET_SIZE;
           _mqtt_config.session.disable_clean_session = true;
           _mqtt_config.network.disable_auto_reconnect = false;
           _mqtt_config.broker.verification.certificate = (const char *)isrgrootx1_pem_start;
        }
        
            enum class state_e
        {
            NOT_INITIALISED,
            INITIALISED,
            CONNECTED,
            DISCONNECTED,
            ERROR
        }; ///< WiFi states


        static const state_e& get_state(void) { return _state; } 
     

        esp_err_t init();
        bool check_all_subscribed();
        bool publish(const std::string &topic, const std::string &payload, int qos, bool retain);
        bool subscribe(const std::string& topic,MessageReceivedCallbackWithTopic callback=nullptr,int qos = 0);

        
        void onMessageReceivedCallback(const char *topic, char *payload, unsigned int length);

    private:
        static state_e _state;
        static std::mutex state_mutx;   ///<State_e mutex
  
    };
}