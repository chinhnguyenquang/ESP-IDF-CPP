#pragma once 
#include <esp_http_client.h>
#include <cstring>
#include "esp_log.h"
#include <functional>
#include <vector>
#include <stdlib.h>
namespace HTTP_CLIENT
{
    class HttpClient
    {
    
    private:
    constexpr static const char *log_tag{"HttpClient"};
    
    //! Routes c-style callback to c++ handler
    static esp_err_t HandleGlobalEvent(esp_http_client_event_t *evt);

    //! Handles http events
    esp_err_t HandleEvent(esp_http_client_event_t *evt);

    //! The configuration for the http client
    esp_http_client_config_t configM;

    //! Pointer to the http client
    esp_http_client_handle_t clientM;

    //! Pointer to callback for http event
    std::function<void(esp_http_client_event_t *evt)> getCbMP;
        //! Performs HTTP request based on current configuration and executes the given
    //! call back for every client event.
    esp_err_t Perform(std::function<void(esp_http_client_handle_t)> beforePerform,std::function<void(esp_http_client_event_t *evt)> cb);

    public:
        HttpClient(const char * hostIC, const char * pathIC) :
            configM{},           // ← Không có dấu { } ở đây
            clientM(nullptr),
            getCbMP(nullptr)
        {
            configM.host = hostIC,
            configM.path = pathIC,
            configM.event_handler = HttpClient::HandleGlobalEvent;
            configM.user_data = this;
            configM.timeout_ms=5000;
        }
        ~HttpClient(){ }

    public:
        //! Performs HTTP request based on current configuration and stores response data
        //! into the given vector
        esp_err_t Get(std::vector<char> & respBufIR);

        esp_err_t Post(const char * postDataIC,std::vector<char> & respBufIR);
            //! Sets the http path
        void SetPath(const char * pathIC)
        {
            configM.path = pathIC;
        }

        //! Sets the http query which is appended to path
        //! <host><path>?<query>
        void SetQuery(const char * queryIC)
        {
            configM.query = queryIC;
        }




    };
}