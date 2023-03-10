set(app_src sound_sensor.c)
set(pri_req esp_adc_cal)
idf_component_register(SRCS ${app_src}
                    INCLUDE_DIRS "."
                    REQUIRES ${pri_req})