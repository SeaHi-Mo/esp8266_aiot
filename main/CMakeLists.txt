idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES nvs_flash mbedtls esp_https_ota lwip json app_update
                    )