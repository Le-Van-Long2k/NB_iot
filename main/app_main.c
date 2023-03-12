#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "output_iot.h"
#include "input_iot.h"
#include "simcom7020.h"
#include "driver/uart.h"
#include "string.h"
#include "freertos/semphr.h"
#include "dht11.h"
#include "BH1750FVI.h"
#include "sound_sensor.h"

/** so do ket noi
 *
 * +-------------------+
 * |                3v3|- VCC (DHT11, BH1750, sim7020, SoundSensor)
 * |                   |-
 * |                 VP|- OUT (SoundSensor)
 * |                   |
 * |                  4|- DATA (DHT11)
 * |                   |
 * |                   |
 * |      ESP32        |
 * |                 16|- RX (sim7020)
 * |                 17|- TX (sim7020)
 * |                   |
 * |                 21|- SDA (BH1750)
 * |                 22|- SCL (BH1750)
 * |                 23|- ADD (BH1750)
 * +-------------------+
 */

#define URI_MQTT "mqtt.inoway.vn"
#define MQTT_IP "116.101.122.190"
#define DEVICE_ID "4dadc1cf-7cdd-476f-bb55-c512bfeeeed0"
#define DEVICE_TOKEN "5mFnKK3FjMmyHyxhweMHssKlUJdie4UU"

simcom simcom_7020;
BH1750FVI_t BH1750;
dht11_t DHT11;

int db;

static const char *TAG_SIM = "Tag_sim: ";
static char topic_pub[200];
static char topic_sub[200];
static char pubMQTTBuffer[500]; // data publish gia tri nhiet do, do am, cuong do am thanh, cuong do anh sang
                                // VD: pubMQTTBuffer = "25,80,30,5000"

bool Flag_connect_mqtt = false;

// event group
#define BIT_EVENT_DHT11 (1 << 0)
#define BIT_EVENT_BH1750 (1 << 1)
#define BIT_EVENT_KY037 (1 << 2)
EventGroupHandle_t xEventGroupSendPub;

client client_mqtt = {
    .uri = MQTT_IP,
    .port = 1883,
    .user_name = "levanlong",
    .client_id = DEVICE_ID,
    .password = DEVICE_TOKEN,
    .mqtt_id = 0};

// function xu ly data nhan tu broker MQTT
void subcribe_callback(char *data)
{
    // char *_buff;
    // _buff = strstr(data, "{");
    // ESP_LOGW(TAG_SIM, "Received data !!!");
    // ESP_LOGI(TAG_SIM, "Data : %s", _buff);
}

static void sendMessageMQTT_task(void *arg)
{
    while (1)
    {
        // wait 3 bit event
        xEventGroupWaitBits(
            xEventGroupSendPub,                                                      /* The event group being tested. */
            BIT_EVENT_DHT11 | BIT_EVENT_BH1750 | BIT_EVENT_KY037, /* The bits within the event group to wait for. */
            pdTRUE,                                                                   /* BIT_0 & BIT_4 should be cleared before returning. */
            pdTRUE,                                                                   /*  wait for both bits, either bit will do. */
            portMAX_DELAY);                                                           /* Wait a maximum for either bit to be set. */

        sprintf(pubMQTTBuffer, "{\\\"temperature\\\":%d, \\\"humidity\\\":%d, \\\"sound\\\":%d, \\\"light\\\":%d}", DHT11.temperature, DHT11.humidity, db, BH1750.lux);
        ESP_LOGW("SIM7020", "%s",pubMQTTBuffer);

    PUB:
        if (mqtt_message_publish(client_mqtt, pubMQTTBuffer, topic_pub, 0, 3))
        {
            ESP_LOGW(TAG_SIM, "Public is successfully");
            vTaskDelay(pdMS_TO_TICKS(500));
        }
        else
        {
            ESP_LOGW(TAG_SIM, "Public is failed");
            goto PUB;
        }

        xEventGroupClearBits(xEventGroupSendPub, BIT_EVENT_DHT11);
        xEventGroupClearBits(xEventGroupSendPub, BIT_EVENT_BH1750);
        xEventGroupClearBits(xEventGroupSendPub, BIT_EVENT_KY037);

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

static void dht11_task(void *arg)
{
    while (1)
    {
        DHT11.temperature = DHT11_read().temperature;
        DHT11.humidity = DHT11_read().humidity;
        if (DHT11_read().status == DHT11_OK)
        {
            ESP_LOGW("DHT11", "temperature: %d, humidity: %d", DHT11.temperature, DHT11.humidity);
            xEventGroupSetBits(xEventGroupSendPub, BIT_EVENT_DHT11);
        }
        else
        {
            ESP_LOGW("DHT11", "failed");
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

static void bh1750_task(void *arg)
{
    while (1)
    {
        BH1750_setOnceHigh2Res();
        BH1750_getLux();
        if (BH1750.status == BH1750FVI_OK || BH1750_isReady() == 0)
        {
            ESP_LOGW("BH1750", "Light: %d", BH1750.lux);
            xEventGroupSetBits(xEventGroupSendPub, BIT_EVENT_BH1750);
        }
        else
        {
            ESP_LOGW("BH1750", "failed");
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

static void sound_task(void *arg)
{
    while (1)
    {
        db = SoundSensor_GetData();
        ESP_LOGW("KY-037", "Sound: %d", db);
        xEventGroupSetBits(xEventGroupSendPub, BIT_EVENT_KY037);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

static void Setup()
{
    // dinh dang topic sub va pub
    sprintf(topic_pub, "messages/%s/update", DEVICE_ID);
    sprintf(topic_sub, "messages/%s/status", DEVICE_ID);

    // khoi tao uart vs simcom
    init_simcom(NUMERO_PORTA_SERIALE2, U2RXD, U2TXD, 115200);

    // khoi tao dht11
    DHT11_init(GPIO_NUM_4);

    // khoi tao bh1750
    BH1750_Init();

    // khoi tao sound sensor
    SoundSensor_Init();

    // Khoi tao gia tri nhiet do, do am, am thanh, anh sang
    DHT11.temperature = 25;
    DHT11.humidity = 80;
    db = 0;
    BH1750.lux = 0;

    // Create EventGroup
    xEventGroupSendPub = xEventGroupCreate();

    // check connect between simcom vs esp32
    if (!Flag_connect_mqtt)
    {
        printf("---------------> WAITING ... <------------\r\n");
        Check_network();
    RECONECT:
        vTaskDelay(pdMS_TO_TICKS(1000));
        if (mqtt_start(client_mqtt, 3, 64800, 1, 3))
        {
            isconverhex(3); // dinh dang message cua pub la string (AT+CREVHEX=1 la gui hex)
            ESP_LOGW(TAG_SIM, "MQTT IS CONNECTED");
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
        else
        {
            ESP_LOGW(TAG_SIM, "MQTT IS NOT CONNECTED");
            mqtt_stop(client_mqtt, 2);
            Flag_connect_mqtt = false;
            goto RECONECT;
        }
    }

    // subscribe topic
    // SUB:
    //     if (mqtt_subscribe(client_mqtt, topic_sub, 0, 3, subcribe_callback))
    //     {
    //         ESP_LOGW(TAG_SIM, "Sent subcribe is successfully");
    //         vTaskDelay(pdMS_TO_TICKS(500));
    //     }
    //     else
    //     {
    //         ESP_LOGW(TAG_SIM, "Sent subcribe is failed");
    //         goto SUB;
    //     }
}

void app_main(void)
{

    Setup();

    xTaskCreatePinnedToCore(dht11_task, "dht11_task", 1024 * 4, NULL, 8, NULL, 1);
    xTaskCreatePinnedToCore(bh1750_task, "bh1750_task", 1024 * 4, NULL, 8, NULL, 1);
    xTaskCreatePinnedToCore(sound_task, "sound_task", 1024 * 4, NULL, 8, NULL, 1);
    xTaskCreatePinnedToCore(sendMessageMQTT_task, "sendMessageMQTT_task", 5 * 1024, NULL, 15, NULL, 1);
}
