#ifndef DHT11_H_
#define DHT11_H_

#include "driver/gpio.h"

enum dht11_status
{
    DHT11_CRC_ERROR = -2,
    DHT11_TIMEOUT_ERROR,
    DHT11_OK
};

typedef struct dht11 
{
    int status;
    int temperature;
    int humidity;
} dht11_t;


void DHT11_init(gpio_num_t);

dht11_t DHT11_read();

#endif