#include "Mqtt.h"

namespace ESP32MQTT {

    MQTTClient::state_e MQTTClient::_state{MQTTClient::state_e::NOT_INITIALISED};
    std::mutex                                  MQTTClient::state_mutx{};
    void MQTTClient::HandleGlobalEvent(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
    {
        auto inst = static_cast<MQTTClient*>(handler_args);
  
        inst->event_handler(base,event_id,event_data);
    }

    esp_err_t MQTTClient::init() {
        std::lock_guard<std::mutex> state_guard(state_mutx);
        _mqtt_client = esp_mqtt_client_init(&_mqtt_config);
        esp_mqtt_client_register_event(_mqtt_client, MQTT_EVENT_ANY, HandleGlobalEvent, this);        
        _state = state_e::INITIALISED;
        return esp_mqtt_client_start(_mqtt_client);
    }

    void MQTTClient::event_handler(esp_event_base_t base, int32_t event_id, void *event_data)
    {
        esp_mqtt_event_handle_t event = static_cast<esp_mqtt_event_handle_t>(event_data);
        int msg_id;
            switch ((esp_mqtt_event_id_t)event_id) {
                case MQTT_EVENT_CONNECTED:
                {
                    std::lock_guard<std::mutex> state_guard(state_mutx);
                    _state = state_e::CONNECTED;
                    ESP_LOGI(_logtag, "MQTT_EVENT_CONNECTED");
                    for (auto & rec : _topicSubscriptionList)
                    {
                        msg_id = esp_mqtt_client_subscribe(_mqtt_client, rec.topic.c_str(), rec.qos);
                        rec.msg_id = msg_id;
                        rec.subscribed = false;
                        ESP_LOGI(_logtag, "sent subscribe successful, topic: %s, msg_id=%d", rec.topic.c_str(), msg_id);
                   
                    }
                    break;
                }


                case MQTT_EVENT_DISCONNECTED:
                {
                    std::lock_guard<std::mutex> state_guard(state_mutx);
                    _state = state_e::DISCONNECTED;
                    ESP_LOGI(_logtag, "MQTT_EVENT_DISCONNECTED");
                    break;
                }


                 case MQTT_EVENT_SUBSCRIBED:
                 {
                    msg_id = ((esp_mqtt_event_handle_t)event_data)->msg_id;
                    ESP_LOGI(_logtag, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", msg_id);
                    for(auto & rec : _topicSubscriptionList)
                    {
                        if(rec.msg_id == msg_id)
                        {
                            rec.subscribed = true;
                            break;
                        }
                    }
                    break;
                 }



                case MQTT_EVENT_UNSUBSCRIBED:
                {
                    msg_id = ((esp_mqtt_event_handle_t)event_data)->msg_id;

                        ESP_LOGI(_logtag, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", msg_id);

                        for (auto rec = _topicSubscriptionList.begin(); rec != _topicSubscriptionList.end(); ++rec)
                        {
                            if (rec->msg_id == msg_id)
                            {
                                _topicSubscriptionList.erase(rec);
                                break;
                            }
                        }

                    break;
                }

                case MQTT_EVENT_PUBLISHED:
                {
                    ESP_LOGI(_logtag, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
                    break;
                }

                case MQTT_EVENT_DATA:
                {
                    ESP_LOGI(_logtag, "MQTT_EVENT_DATA");
                    
                    std::string topic_str(event->topic, event->topic_len);
                    onMessageReceivedCallback(topic_str.c_str(), event->data, event->data_len);

                    break;
                }

                 case MQTT_EVENT_ERROR:
                 {
                    std::lock_guard<std::mutex> state_guard(state_mutx);
                    _state = state_e::ERROR;
                    ESP_LOGI(_logtag, "MQTT_EVENT_ERROR");      
                    break;
                 }

                default:
                    ESP_LOGI(_logtag, "Other event id:%d", event->event_id);
                    break;
            }
        }

        bool MQTTClient::subscribe(const std::string& topic,MessageReceivedCallbackWithTopic callback,int qos)
        {
            int msg_id=esp_mqtt_client_subscribe(_mqtt_client, topic.c_str(), qos);
            if(msg_id != -1)
            {
                 _topicSubscriptionList.push_back({topic,msg_id,qos,false, callback});
            }
            return msg_id!=-1;
        

        }
        bool MQTTClient::publish(const std::string &topic, const std::string &payload, int qos, bool retain)
        {
            {
                std::lock_guard<std::mutex> state_guard(state_mutx);
                if (_state != MQTTClient::state_e::CONNECTED) return false;
            }

            bool status =(-1 != esp_mqtt_client_publish(_mqtt_client, topic.c_str(), payload.c_str(), 0, qos, retain));
            if (status)
            ESP_LOGI(_logtag, "MQTT << [%s] %s", topic.c_str(), payload.c_str());
            else
            ESP_LOGW(_logtag, "Publish failed, is the message too long ? (see setMaxPacketSize())"); // This can occurs if the message is too long according to the maximum defined in PubsubClient.h
            return status;
        }

        void MQTTClient::onMessageReceivedCallback(const char *topic, char *payload, unsigned int length)
        {
            if (strlen(topic) + length + 9 >= DEFAULT_PACKET_SIZE)
            {
                ESP_LOGE(_logtag, "MQTT! Your message may be truncated, please set setMaxPacketSize() to a higher value.");
                return;
            }
                // Create a copy of the payload, don't modify the original buffer
            std::string payloadStr;
            std::string topicStr(topic);
            if (payload && length > 0)
            {
                payloadStr = std::string(payload, length);
            }
            else
            {
                payloadStr = "";
            }
            ESP_LOGI(_logtag, "MQTT >> [%s] %s", topic, payloadStr.c_str());

            for (std::size_t i = 0; i < _topicSubscriptionList.size(); i++)
            {  
                if(_topicSubscriptionList[i].callbackWithTopic != nullptr)
                {
                    _topicSubscriptionList[i].callbackWithTopic(topicStr, payloadStr);
                    break;
                }
                    
                
            }
        }

}