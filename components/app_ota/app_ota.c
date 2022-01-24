#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_system.h"
#include "aiot_state_api.h"
#include "aiot_sysdep_api.h"
#include "aiot_ota_api.h"
#include "aiot_mqtt_api.h"
#include "ota_private.h"
#include "app_dynreg_mq.h"
#include "cJSON.h"
#include "app_ota.h"
#include "app_version.h"
#include "app_flash.h"
static char* TAG = "APP OTA";

static void* mqtt_handle = NULL;
app_ota_image_t app_ota_image;

extern const char* ali_ca_cert;
char ali_ota_msg[1024] = { 0 };


char* create_ota_progress_cjson(int progress);
int esp_aiot_download_report_progress(int progress);

/**
 * @brief 版本比较 01.00.01.11 添加
 *
 * @param desc_version
 * @param running_version
 * @return int
 */
int version_cmp(char* desc_version, char* running_version)
{
    char desc_v[4];
    char run_v[4];
    uint32_t desc_version_num = 0;
    uint32_t run_version_num = 0;

    desc_v[0] = atoi(strtok(desc_version, "."));

    for (size_t i = 1; i < 3; i++)
        desc_v[i] = atoi(strtok(NULL, "."));

    run_v[0] = atoi(strtok(running_version, "."));
    for (size_t i = 1; i < 3; i++)
        run_v[i] = atoi(strtok(NULL, "."));
    for (size_t i = 0; i < 3; i++)
    {
        desc_version_num += desc_v[i];
        run_version_num += run_v[i];
        if (i<2) {
            desc_version_num *= 10;
            run_version_num *= 10;
        }
    }
    if ((run_version_num > desc_version_num) ||(run_version_num == desc_version_num)) return 1;
    return 0;
}
/**
 * @brief
 *
 * @param evt
 * @return esp_err_t
 */
esp_err_t _http_event_handler(esp_http_client_event_t* evt)
{
    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
    }
    return ESP_OK;
}
/**
 * @brief 启动OTA
 *
 */
void ota_download_thread(void)
{

    if (app_read_ota_url(app_ota_image.image_url)==ESP_OK) {
        printf("ota url= %s", app_ota_image.image_url);
        esp_http_client_config_t ota_config = {
              .url = app_ota_image.image_url,
              .cert_pem = ali_ca_cert,
              .event_handler = _http_event_handler,
        };
        esp_err_t ret = esp_https_ota(&ota_config);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "esp_ota_set_boot_partition succeeded");
            app_erase_ota_url();
            esp_restart();
        }
        else {
            //  pthread_join(g_ota_thread_handle, NULL);
            ESP_LOGE(TAG, "Firmware Upgrades Failed");
        }
    }
}
/**
 * @brief Get the ota msg object
 *      获取ota 信息
 * @param cjson_data
 */
 //
void get_ota_msg(char* cjson_data)
{
    cJSON* root = cJSON_Parse(cjson_data);
    if (root==NULL) {
        cJSON_Delete(root);
        return;
    }
    cJSON* data = cJSON_GetObjectItem(root, "data");
    if (data==NULL) {
        cJSON_Delete(root);
        return;
    }
    cJSON* data_size = cJSON_GetObjectItem(data, "size");
    cJSON* data_version = cJSON_GetObjectItem(data, "version");
    cJSON* data_url = cJSON_GetObjectItem(data, "url");

    app_ota_image.image_size = (uint32_t)data_size->valueint;
    strcpy(app_ota_image.image_version, data_version->valuestring);
    strcpy(app_ota_image.image_url, data_url->valuestring);
}
/**
 * @brief aiot_mqtt_recv_ota_handler
 *        OTA信息下发处理
 * @param handle
 * @param packet
 * @param userdata
 */
void aiot_mqtt_recv_ota_handler(void* handle, const aiot_mqtt_recv_t* packet, void* userdata)
{
    if (packet->type==AIOT_MQTTRECV_PUB) {
        const esp_app_desc_t* app_desc_version;
        app_desc_version = esp_ota_get_app_description();
        get_ota_msg((char*)packet->data.pub.payload);
        ESP_LOGI(TAG, "imge version:%s,imge size:%d", app_ota_image.image_version, app_ota_image.image_size);
        ESP_LOGI(TAG, "url:%s", app_ota_image.image_url);
        //if (!version_cmp(app_ota_image.image_version, app_desc_version->version)) {
        app_svae_ota_url(app_ota_image.image_url);
        ESP_LOGW(TAG, "2 sec restart and update");
        sleep(2);

        esp_restart();
        // }
    }
}
/**
 * @brief 配置OTA
 *
 * @param mqtt_handle
 */
int32_t esp_ota_aiot_pthread(void* mqtt_handle_t)
{
    int32_t res = STATE_SUCCESS;
    char topic[128] = { 0 };
    sprintf(topic, "%s/%s/%s", OTA_FOTA_TOPIC_PREFIX, product_key, device_name);
    mqtt_handle = mqtt_handle_t;
    res = aiot_mqtt_sub(mqtt_handle, topic, aiot_mqtt_recv_ota_handler, 1, NULL);

    return res;
}

/**
* @brief Create a ota progress cjson object
*        创建升级进度的json数据
* @param progress
* @return char*
*/
char* create_ota_progress_cjson(int progress)
{
    char progress_str[2] = { 0 };
    cJSON* Root = cJSON_CreateObject();
    cJSON* params = cJSON_CreateObject();
    cJSON_AddStringToObject(Root, "id", "12345");
    cJSON_AddItemToObject(Root, "params", params);
    sprintf(progress_str, "%d", progress);
    cJSON_AddStringToObject(params, "step", progress_str);
    cJSON_AddStringToObject(params, "desc", "");
    // cJSON_AddStringToObject(params, "module", "");
    char* cjson_data = cJSON_PrintUnformatted(Root);
    cJSON_Delete(Root);
    return cjson_data;
}
/**
 * @brief esp_aiot_download_report_progress
 *  上传升级百分比到 云平台
 * @param progress
 * @return int
 */
int esp_aiot_download_report_progress(int progress)
{
    int32_t res = STATE_SUCCESS;
    char ota_topic[128] = { 0 };

    sprintf(ota_topic, "%s/%s/%s", OTA_PROGRESS_TOPIC_PREFIX, product_key, device_name);
    char* src = create_ota_progress_cjson(progress);
    res = aiot_mqtt_pub(mqtt_handle, ota_topic, (uint8_t*)src, strlen(src), 1);
    if (res<STATE_SUCCESS) {
        ESP_LOGE(TAG, "pub failed:-0x%04X", -res);
    }
    free(src);
    return res;
}