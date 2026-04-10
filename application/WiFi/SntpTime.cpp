#include "SntpTime.h"

#include "esp_log.h"

namespace SNTP
{

std::chrono::system_clock::time_point Sntp::last_update{}; ///< Time of the last NTP update

Sntp::time_source_e Sntp::source{Sntp::time_source_e::TIME_SRC_UNKNOWN}; ///< SNTP API time source

Sntp::Tim_Sntp_Callback Sntp::time_sntp_callback{nullptr}; ///< SNTP time update callback

bool Sntp::_running{false}; ///< SNTP running flag

/// @brief Callback for when a new SNTP update happens
///
/// @param[in] tv : ctime timeval struct pointer
void Sntp::callback_on_ntp_update(timeval* tv)
{
    //ESP_LOGD(_log_tag, "Time is %s", ascii_time_now()); // TODO remove extra newline
    ESP_LOGI("time", "Time is %s", ascii_time_now());
    if(time_sntp_callback){
        time_sntp_callback();
        ESP_LOGI("SNTP", "Called SNTP time callback");
    }
    // TODO update "last_update"

}

/// @brief Initialise and start the SNTP service (blocking)
///
/// @attention Will block until WiFi is connected
///
/// @return 
/// 	- ESP_OK if SNTP running, else ESP_FAIL
esp_err_t Sntp::init(void)
{
    if (!_running)
    {
        while (WIFI::Wifi_pro::state_e::CONNECTED != WIFI::Wifi_pro::get_state())
            vTaskDelay(pdMS_TO_TICKS(1000));

        setenv("TZ", "UTC-7", 1); // Vietnam
        tzset();

        sntp_setoperatingmode(SNTP_OPMODE_POLL);

        sntp_setservername(0, "time.google.com");
        sntp_setservername(1, "pool.ntp.com");

        sntp_set_time_sync_notification_cb(&callback_on_ntp_update);
        sntp_set_sync_interval(15*1000); // TODO 15 mins once testing verified working

        sntp_init();

        source = TIME_SRC_NTP;

        _running = true;
    }

    if (_running)
        return ESP_OK;
    return ESP_FAIL;
}

[[nodiscard]] const char* Sntp::ascii_time_now(void)
{
  
    static char formatted_time[32];
    const std::time_t time_now{std::chrono::system_clock::to_time_t(time_point_now())};
    std::tm* tm_info = std::localtime(&time_now);
    
    std::strftime(formatted_time, sizeof(formatted_time), "%H:%M:%S %d/%m/%Y", tm_info);
    
    return formatted_time;
} ///< Return the current time as a cstring

} // namespace SNTP