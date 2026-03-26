#pragma once 

#include <cstring>

#include "esp_err.h"
#include "nvs_flash.h"
#include "nvs.h"

namespace Nvs32
{
    class Nvs
    {
        nvs_handle_t        handle{};                   ///< API NVS handle
        const char* const   partition_name{nullptr};    ///< cstring of partition name
    public:
        constexpr Nvs(const char* const partition_name = "nvs") :
        partition_name{partition_name}
    {}

        [[nodiscard]] esp_err_t open(void)
        {
            return nvs_open(partition_name, NVS_READWRITE, &handle);
        }

        void close(void)
        {
            nvs_close(handle);
        }

        template <typename T>

        [[nodiscard]] esp_err_t get_buf(const char* const key, T* output,size_t& len)
        {
            return _get_buf(handle, key, output, len);
        }

        template <typename T>
        [[nodiscard]] esp_err_t set_buffer(const char* const key, const T* input)
        {
             return _set_buf(handle, key, input); 
        }

    private:
        template<typename T>
        [[nodiscard]] esp_err_t _get_buf(nvs_handle_t handle, const char* const key, 
                                            T* output, size_t& len)
        {
            size_t n_bytes{sizeof(T)*len};
            if (nullptr == key || 0 == n_bytes || nullptr == output)
                return ESP_ERR_INVALID_ARG;

            const esp_err_t status = nvs_get_blob(handle, key, output, &n_bytes);

            len = n_bytes / sizeof(T);

            return status;

        }

        template <typename T>
        [[nodiscard]] esp_err_t _set_buf(nvs_handle_t handle, const char* const key, 
                                            const T* input)
        {
            size_t n_bytes{sizeof(T)};
            if (nullptr == key || strlen(key) == 0 || nullptr == input)
                return ESP_ERR_INVALID_ARG;
            
            esp_err_t status = nvs_set_blob(handle, key, input, n_bytes);
            
            if (ESP_OK == status)  status = nvs_commit(handle);
            if  (ESP_OK==status)  status =_verify_buf(handle, key, input, n_bytes);
            return status;
        }


        template<typename T>
        [[nodiscard]] esp_err_t _verify_buf(nvs_handle_t handle, const char* const key, 
                                            const T* input, const size_t n_bytes)
        {
            esp_err_t status{ESP_OK};

            T*      buf_in_nvs{new T[n_bytes]{}};
            size_t  n_items_in_nvs{n_bytes};
            if (buf_in_nvs)
            {
            status = _get_buf(handle, key, buf_in_nvs, n_items_in_nvs);

            if (ESP_OK == status)
            {
                if (n_bytes == n_items_in_nvs)
                {
                    if (0 != memcmp(input, buf_in_nvs, n_bytes))
                        status = ESP_FAIL;
                }
                else
                    status = ESP_ERR_NVS_INVALID_LENGTH;
            }

            delete[] buf_in_nvs;
            }
            else
            {
                status = ESP_ERR_NO_MEM;
            }

            return status;
        }



    };
};