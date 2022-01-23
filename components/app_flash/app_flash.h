#ifndef APP_FLASH_H
#define APP_FLASH_H

#include "esp_err.h"
#include "app_dynreg_mq.h"
esp_err_t app_flash_svae_dynreg_msg(void);
esp_err_t app_flash_read_dynreg(cloud_device_wl_t* cloud_device);
esp_err_t app_flash_set_version(void);
esp_err_t app_flash_get_version(char* version);
#endif