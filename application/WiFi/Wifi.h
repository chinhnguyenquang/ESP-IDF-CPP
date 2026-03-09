#pragma once



#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
// #include <algorithm>
#include <mutex>

#include <cstring>

namespace WIFI{
class Wifi{


public:
    Wifi(void);
    enum class state_e
    {
        NOT_INITIALISED,
        INITIALISED,
        READY_TO_CONNECT,
        CONNECTING,
        WAITING_FOR_IP,
        CONNECTED,
        DISCONNECTED,
        ERROR
    };

    esp_err_t init(void);  //initialize 
    esp_err_t begin(void); //start wifi

    state_e get_state(void);
    constexpr static const char* get_mac(void) { return mac_addr_cstr; }


    
private:
    void state_machine(void);
    esp_err_t _get_mac(void);
    static char mac_addr_cstr[13];  ///< Buffer to hold MAC as cstring
    static std::mutex init_mutx;    ///< Initialisation mutex
};



};