#ifndef APP_VERSION_H
#define APP_SERSION_H
#include "esp_ota_ops.h"
#include "esp_https_ota.h"
extern esp_app_desc_t* app_desc_version;
int32_t app_send_new_version(void* mqtt_handle);
#endif
