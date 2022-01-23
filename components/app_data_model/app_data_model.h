#ifndef APP_DATA_MODEL_H
#define APP_DATA_MODEL_H

void* app_aiot_data_model_init(void* mqtt_handle);
int32_t app_send_property_post(void* dm_handle, uint8_t temp, uint8_t humi);
#endif
