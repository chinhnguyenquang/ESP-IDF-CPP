#pragma once 


#include <stdlib.h>
#include <string.h>
#include "cJSON.h"


/*
POST data mẫu:
{
    "ID": "ABCDEF123456",
    "S": "1111177;23.5;82.9;12:26:13 12/04/2024"
}

GET data mẫu:
App.IoTVision.vn/api/BoardSTR423_DuLieuGuiXuongBoard?CheDo=1&key=ABCDEF123456




*/
struct Data_S_IOTVision
{
    uint8_t K1:1;
    uint8_t K2:1;
    uint8_t K3:1;   
    uint8_t K4:1;
    uint8_t MODE:1;
    uint8_t RSSI;
    float Temp;
    float Humi;
    char Time[20];

};



bool json_create(const char* id, const char* s_data,char *OutputBuffer) 
{
    if (!id || !s_data) return false;
    
    cJSON *root = cJSON_CreateObject();
    if (!root) return false;
    
    cJSON_AddStringToObject(root, "ID", id);
    cJSON_AddStringToObject(root, "S", s_data);
    
    strcpy(OutputBuffer, cJSON_Print(root));
    cJSON_Delete(root);
    
    return true;  
}

bool create_str_to_post(struct Data_S_IOTVision data, char* s_output) 
{
    int len = snprintf(s_output, 64, "%d%d%d%d%d;%d;%.1f;%.1f;%s", 
             data.K1, data.K2, data.K3, data.K4, data.MODE,
             data.RSSI,
             data.Temp,
             data.Humi,
             data.Time);
    return (len > 0 && len < 64);
}