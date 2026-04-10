#include"httpClient.h"

namespace HTTP_CLIENT
{
    esp_err_t HttpClient::HandleGlobalEvent(esp_http_client_event_t *evt)
    {
        auto inst = static_cast<HttpClient*>(evt->user_data);
        return inst->HandleEvent(evt);

    }

    esp_err_t HttpClient::HandleEvent(esp_http_client_event_t *evt)
    {
        switch(evt->event_id) {
            case HTTP_EVENT_ERROR:
                //ESP_LOGI(log_tag, "HTTP_EVENT_ERROR");
                break;
            case HTTP_EVENT_ON_CONNECTED:
                //ESP_LOGI(log_tag, "HTTP_EVENT_ON_CONNECTED");
                break;
            case HTTP_EVENT_HEADER_SENT:
                //ESP_LOGI(log_tag, "HTTP_EVENT_HEADER_SENT");
                break;
            case HTTP_EVENT_ON_HEADER:
                //ESP_LOGI(log_tag, "HTTP_EVENT_ON_HEADER");
                printf("%.*s", evt->data_len, (char*)evt->data);
                break;
            case HTTP_EVENT_ON_DATA:
                //ESP_LOGI(log_tag, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
                if (!esp_http_client_is_chunked_response(evt->client))
                {
                    //printf("%.*s", evt->data_len, (char*)evt->data);
                }

                break;
            case HTTP_EVENT_ON_FINISH:
                //ESP_LOGI(log_tag, "HTTP_EVENT_ON_FINISH");
                break;
            case HTTP_EVENT_DISCONNECTED:
                //ESP_LOGI(log_tag, "HTTP_EVENT_DISCONNECTED");
                break;
            default:
                break;
        }

        if( getCbMP != nullptr )
        {
            (getCbMP)(evt);
        }

        return ESP_OK;

    } // end HandleEvent()


    esp_err_t HttpClient::Get(std::vector<char> & respBufIR)
    {
        // Create call back to process client response and store in buffer
        configM.method = HTTP_METHOD_GET;
        respBufIR.clear();
        auto saveResponseCb = [&respBufIR](esp_http_client_event_t *evt)
        {
            if( evt->event_id == HTTP_EVENT_ON_DATA )
            {
                // Resize response buffer to hold data and copy in response
                const std::size_t curSizeC = respBufIR.size();
                const std::size_t dataLenC = evt->data_len;
                respBufIR.resize(curSizeC + dataLenC);
                memcpy(&respBufIR[curSizeC], evt->data, dataLenC);
            }
        };

        // Perform the query using the above lambda to store the response
        return Perform(nullptr,saveResponseCb);

    } // end Get()


//******************************************************************************
//! Perform()
//
//! Performs an HTTP query, calling the given function for each HTTP event
//******************************************************************************
    esp_err_t HttpClient::Perform(std::function<void(esp_http_client_handle_t)> beforePerform,std::function<void(esp_http_client_event_t *evt)> cb)
    {
        getCbMP = cb;
        clientM = esp_http_client_init(&configM);
        if(beforePerform)
        {
            beforePerform(clientM);
            
        }
        esp_err_t err = esp_http_client_perform(clientM);



        if (err == ESP_OK)
        {
            ESP_LOGI(log_tag, "Status = %d, content_length = %lld",
                    esp_http_client_get_status_code(clientM),
                    esp_http_client_get_content_length(clientM)
                    );
        }
        esp_http_client_cleanup(clientM);

        getCbMP = nullptr;

        return err;

    }


    esp_err_t HttpClient::Post(const char * postDataIC,std::vector<char> & respBufIR)
    {
        configM.method = HTTP_METHOD_POST;
        respBufIR.clear();
        auto saveResponseCb = [&respBufIR](esp_http_client_event_t *evt)
        {
            if( evt->event_id == HTTP_EVENT_ON_DATA )
            {
                // Resize response buffer to hold data and copy in response
                const std::size_t curSizeC = respBufIR.size();
                const std::size_t dataLenC = evt->data_len;
                respBufIR.resize(curSizeC + dataLenC);
                memcpy(&respBufIR[curSizeC], evt->data, dataLenC);
            }
        };

        // Perform the query using the above lambda to store the response
        return Perform(
        [postDataIC](esp_http_client_handle_t client)
        {
            
            esp_http_client_set_header(client, "Content-Type", "application/json");
            esp_http_client_set_header(client, "Accept", "application/json");
            esp_http_client_set_post_field(client, postDataIC, strlen(postDataIC));
        },
        saveResponseCb);

   
    }
}



