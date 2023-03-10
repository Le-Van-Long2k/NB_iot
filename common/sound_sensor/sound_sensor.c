#include <stdio.h>
#include <esp_log.h>
#include "driver/adc.h"
#include "sound_sensor.h"
#include "driver/gpio.h"
#include "esp_adc_cal.h"


void SoundSensor_Init()
{
    // initialize ADC
    adc1_config_width(ADC_WIDTH_12Bit);
    adc1_config_channel_atten(ADC1_CHANNEL_0,ADC_ATTEN_11db);
}

// map voltage to db
int mapC(int x, int in_min, int in_max, int out_min, int out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int SoundSensor_GetData()
{
    // Continuously sample ADC1
    int adc_value = 0;
    int PeakToPeak = 0;
    int maximum_signal = 0;    // minimum value
    int minimum_signal = 4095; // maximum value
    uint32_t start_time = esp_timer_get_time();

    // Multisampling
    while (esp_timer_get_time() - start_time < 50*1000)
    {
        adc_value = adc1_get_raw(ADC1_CHANNEL_0);
        if (adc_value < 4095)
        {
            if (adc_value > maximum_signal)
            {
                maximum_signal = adc_value;
            }
            else if (adc_value < minimum_signal)
            {
                minimum_signal = adc_value;
            }
        }
    }

    PeakToPeak = maximum_signal - minimum_signal;
    int db = mapC(PeakToPeak, 30, 550, 35, 75);

    return db;
}