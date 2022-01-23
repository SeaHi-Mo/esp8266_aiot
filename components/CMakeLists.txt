
idf_component_register(SRCS "app_data_model/app_data_model.c" 
                            "app_dynreg_mqtt/app_dynreg_mq.c"
                            "app_flash/app_flash.c"
                            "app_ntp/app_ntp.c"
                            "app_ota/app_ota.c"
                            "app_version/app_version.c"
                            
                    INCLUDE_DIRS app_data_model app_dynreg_mqtt app_flash app_ntp app_ota app_version
                    REQUIRES nvs_flash app_update esp_https_ota lwip mbedtls
                    )