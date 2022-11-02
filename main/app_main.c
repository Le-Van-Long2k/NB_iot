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

#define URI_MQTT "mqtt.inoway.vn"
#define MQTT_IP "116.101.122.190"
#define DEVICE_ID "4dadc1cf-7cdd-476f-bb55-c512bfeeeed0"
#define DEVICE_TOKEN "5mFnKK3FjMmyHyxhweMHssKlUJdie4UU"

simcom simcom_7020;
static const char *TAG_SIM = "";
static char topic_pub[100];
static char topic_sub[100];

client client_mqtt = {
    .uri = MQTT_IP,
    .port = 1883,
    .user_name = "levanlong",
    .client_id = DEVICE_ID,
    .password = DEVICE_TOKEN,
    .mqtt_id = 0};

void subcribe_callback(char *data)
{
    //ESP_LOGI(TAG_SIM, "$$$ Receive: %s", (char *)data);
}

static void SendMessageToCloudMQTT_task(void *arg)
{
    while (1)
    {
    PUB:
        if (mqtt_message_publish(client_mqtt, "Hello", topic_pub, 2, 5))
        {
            ESP_LOGI(TAG_SIM, "Public is successfully");
            vTaskDelay(500 / portTICK_RATE_MS);
        }
        else
        {
            ESP_LOGI(TAG_SIM, "Public is failed");
            goto PUB;
        }
        vTaskDelay(5000 / portTICK_RATE_MS);
    }
}



void app_main(void)
{

    // dinh dang topic sub va pub
    sprintf(topic_pub, "messages/%s/attributets", DEVICE_ID);
    sprintf(topic_sub, "commands/%s/device/controls", DEVICE_ID);

    init_simcom(NUMERO_PORTA_SERIALE2, U2RXD, U2TXD, 115200);
    Check_network();

    // send_ATComand_Sim("AT+CPIN?", 5);
    // send_ATComand_Sim("AT+CSQ", 5);
    // send_ATComand_Sim("AT+CGREG?", 5);
    // send_ATComand_Sim("AT+CGACT?", 5);
    // send_ATComand_Sim("AT+COPS?", 5);
    // send_ATComand_Sim("AT+CGCONTRDP", 5);
    // char bufAT[300];
    // sprintf(bufAT, "AT+CDNSGIP=%c%s%c", '"', "mqtt.inoway.vn", '"');
    // send_ATComand_Sim(bufAT, 4);
    // char bufAT2[300];
    // sprintf(bufAT2, "AT+CDNSGIP=%c%s%c", '"', "mqtt://mqtt.innoway.vn", '"');
    // send_ATComand_Sim(bufAT2, 4);
    // char bufIP2[300];
    // sprintf(bufIP2, "AT+CIPPING=%c%s%c", '"', "116.101.122.190", '"');
    // send_ATComand_Sim(bufIP2, 4);

RECONECT:
    if (mqtt_start(client_mqtt, 3, 64800, 1, 3))
    {
        isconverhex(3); // dinh dang message cua pub la string (AT+CREVHEX=1 la gui hex)
        ESP_LOGI(TAG_SIM, "MQTT IS CONNECTED");
        vTaskDelay(2000 / portTICK_RATE_MS);

    SUB:
        if (mqtt_subscribe(client_mqtt, topic_sub, 2, 3, subcribe_callback))
        {
            ESP_LOGI(TAG_SIM, "Sent subcribe is successfully");
            vTaskDelay(500 / portTICK_RATE_MS);
        }
        else
        {
            ESP_LOGI(TAG_SIM, "Sent subcribe is failed");
            goto SUB;
        }
    }
    else
    {
        ESP_LOGI(TAG_SIM, "MQTT IS NOT CONNECTED");
        mqtt_stop(client_mqtt, 2);
        goto RECONECT;
    }

    xTaskCreate(SendMessageToCloudMQTT_task, "SendMessageToCloudMQTT_task", 1024 * 4, NULL, configMAX_PRIORITIES - 1, NULL);
}
