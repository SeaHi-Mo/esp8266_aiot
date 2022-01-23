#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "esp_log.h"
#include "esp_err.h"
#include "esp_sntp.h"
#include "aiot_state_api.h"
#include "aiot_sysdep_api.h"
#include "aiot_mqtt_api.h"
#include "ntp_private.h"
#include "cJSON.h"
#include "lwip/apps/sntp.h"
#include "lwip/apps/sntp_opts.h"
#include "app_ntp.h"
#include "esp_wifi.h"
static char* TAG = "APP_NTP";

#ifndef APP_DYNREG_ENABLE
extern char* product_key;
extern char* device_name;
extern char* device_secret;
#endif

time_t now = 0;
/**
 * @brief   aiot_set_system_time
 *        把NTP时间设置为系统时间
 * @param time_date
 */
static void aiot_set_system_time(char* time_date)
{
    cJSON* ROOT = cJSON_Parse(time_date);

    if (ROOT==NULL) {

        cJSON_Delete(ROOT);
        return;
    }
    cJSON* SST = cJSON_GetObjectItem(ROOT, "serverSendTime");
    if (SST==NULL) {

        cJSON_Delete(ROOT);
        return;
    }
    cJSON* SRT = cJSON_GetObjectItem(ROOT, "serverRecvTime");
    if (SRT==NULL) {
        cJSON_Delete(ROOT);
        return;
    }
    if ((((long)(SST->valuedouble/1000))-now)>120 || (now-((long)(SST->valuedouble/1000)))>120)
        now = (long)(SST->valuedouble/1000);
    struct timeval tv;
    tv.tv_sec = now;
    setenv("TZ", "CST-8", 1);
    tzset();
    sntp_sync_time(&tv);

    cJSON_Delete(ROOT);
}
/**
 * @brief aiot_mqtt_recv_ntp_handler
 *        阿里云ntp 数据下发回调
 * @param handle
 * @param packet
 * @param userdata
 */
static void aiot_mqtt_recv_ntp_handler(void* handle, const aiot_mqtt_recv_t* packet, void* userdata)
{
    if (packet->type==AIOT_MQTTRECV_PUB) {
        ESP_LOGI(TAG, "%.*s", packet->data.pub.payload_len, packet->data.pub.payload);
        aiot_set_system_time((char*)packet->data.pub.payload);
    }
}
/**
 * @brief   app_aiot_get_ntp_time
 *          获取NTP时间来更新系统时间
 * @param arg
 * @return void*
 */
void* app_aiot_get_ntp_time(void* mqtt_handle)
{
    int32_t res = STATE_SUCCESS;
    static char topic[128];
    static uint8_t payload[128];
    static char device_name[13] = { 0 };
    int i = 0;

    memset(topic, 0, 128);
    uint8_t* mac = malloc(6);
    esp_wifi_get_mac(WIFI_IF_STA, mac);
    memset(device_name, 0, 13);
    sprintf(device_name, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    sprintf(topic, NTP_RESPONSE_TOPIC_FMT, product_key, device_name);
    res = aiot_mqtt_sub(mqtt_handle, topic, aiot_mqtt_recv_ntp_handler, 1, NULL);
    if (res < 0) {
        ESP_LOGE(TAG, "aiot_mqtt_sub ntp failse: %d", res);
        return NULL;
    }
    memset(topic, 0, 128);

    sprintf(topic, NTP_REQUEST_TOPIC_FMT, product_key, device_name);
    memset(payload, 0, 128);
    sprintf((char*)payload, NTP_REQUEST_PAYLOAD_FMT, now);

    res = aiot_mqtt_pub(mqtt_handle, topic, payload, strlen((char*)payload), 0);
    if (res < 0) {
        ESP_LOGE(TAG, "aiot_mqtt_sub ntp failse: %d", res);
    }
    while (1) {
        time(&now);
        i++;
        if (i>180) {
            memset(payload, 0, 128);
            sprintf((char*)payload, NTP_REQUEST_PAYLOAD_FMT, now);
            res = aiot_mqtt_pub(mqtt_handle, topic, payload, strlen((char*)payload), 0);
            if (res < 0) {
                ESP_LOGE(TAG, "aiot_mqtt_sub ntp failse: %d", res);
            }
            i = 0;
        }
        sleep(1);
    }
    free(mac);
    return NULL;
}
