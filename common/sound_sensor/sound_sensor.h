#ifndef SOUND_SENSOR_H
#define SOUND_SENSOR_H
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"


void SoundSensor_Init();

int SoundSensor_GetData();


#endif