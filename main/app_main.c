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
#include "cJSON.h"

#define URI_MQTT "mqtt.inoway.vn"
#define MQTT_IP "116.101.122.190"
#define DEVICE_ID "4dadc1cf-7cdd-476f-bb55-c512bfeeeed0"
#define DEVICE_TOKEN "5mFnKK3FjMmyHyxhweMHssKlUJdie4UU"

simcom simcom_7020;
static const char *TAG_SIM = "Tag_sim: ";
static char topic_pub[100];
static char topic_sub[100];
static char pubMQTTBuffer[256];
bool Flag_connect_mqtt = false;

client client_mqtt = {
    .uri = MQTT_IP,
    .port = 1883,
    .user_name = "levanlong",
    .client_id = DEVICE_ID,
    .password = DEVICE_TOKEN,
    .mqtt_id = 0};

void subcribe_callback(char *data)
{
    char *_buff;
    _buff = strstr(data, "{");
    ESP_LOGW(TAG_SIM, "Received data !!!");
    ESP_LOGI(TAG_SIM, "Data : %s", _buff);
}

static void sendMessageMQTT_task(void *arg)
{
    while (1)
    {

    PUB:
        if (mqtt_message_publish(client_mqtt, "{10.9, 30.8, 40.3}", topic_pub, 0, 5))
        {
            ESP_LOGI(TAG_SIM, "Public is successfully");
            vTaskDelay(500 / portTICK_RATE_MS);
        }
        else
        {
            ESP_LOGI(TAG_SIM, "Public is failed");
            goto PUB;
        }
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

static void getMessageMQTT_task(void *arg)
{
    while (1)
    {
    SUB:
        if (mqtt_subscribe(client_mqtt, topic_sub, 0, 3, subcribe_callback))
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

    vTaskDelay(1000 / portTICK_RATE_MS);
}

static void createMessageJSON()
{
    cJSON *Json1;
    Json1 = cJSON_CreateObject();

    cJSON_AddStringToObject(Json1, "ID1", "id_1234");
    cJSON_AddNumberToObject(Json1, "Data1", 100.01);
    cJSON_AddNumberToObject(Json1, "Data2", 200.22);

    strcpy(pubMQTTBuffer, cJSON_PrintUnformatted(Json1));
    ESP_LOGI(TAG_SIM, "JSON: %s", pubMQTTBuffer);
}

static void readMessageJSON(char *data)
{
    cJSON *root;
    cJSON *ID1;
    cJSON *Data1;
    cJSON *Data2;

    root = cJSON_Parse(data);
    if (root == NULL)
    {
        ESP_LOGW(TAG_SIM, "no JSON");
    }

    char id1[] = "";
    char data1[] = "";
    char data2[] = "";

    ID1 = cJSON_GetObjectItem(root, "ID1");
    strcpy(id1, cJSON_Print(ID1));
    ESP_LOGI(TAG_SIM, "ID1: %s", id1);

    Data1 = cJSON_GetObjectItem(root, "Data1");
    strcpy(data1, cJSON_Print(Data1));
    ESP_LOGI(TAG_SIM, "Data1 = %0.2f", atof(data1));

    Data2 = cJSON_GetObjectItem(root, "Data2");
    strcpy(data2, cJSON_Print(Data2));
    ESP_LOGI(TAG_SIM, "Data2 = %0.2f", atof(data2));

    cJSON_Delete(root);
}

void app_main(void)
{

    // dinh dang topic sub va pub
    sprintf(topic_pub, "messages/%s/attribute", DEVICE_ID);
    sprintf(topic_sub, "messages/%s/attribute", DEVICE_ID);

    // khoi tao uart vs simcom
    init_simcom(NUMERO_PORTA_SERIALE2, U2RXD, U2TXD, 115200);

    // check connect between simcom vs esp32
    if (!Flag_connect_mqtt)
    {
        printf("---------------> WAITING ... <------------\r\n");
        Check_network();
    RECONECT:
        vTaskDelay(1000 / portTICK_RATE_MS);
        if (mqtt_start(client_mqtt, 3, 64800, 1, 3))
        {
            isconverhex(3); // dinh dang message cua pub la string (AT+CREVHEX=1 la gui hex)
            ESP_LOGI(TAG_SIM, "MQTT IS CONNECTED");
            vTaskDelay(2000 / portTICK_RATE_MS);
        }
        else
        {
            ESP_LOGI(TAG_SIM, "MQTT IS NOT CONNECTED");
            mqtt_stop(client_mqtt, 2);
            Flag_connect_mqtt = false;
            goto RECONECT;
        }
    }

    createMessageJSON();
    xTaskCreate(sendMessageMQTT_task, "sendMessageMQTT_task", 1024 * 4, NULL, 10, NULL);
    xTaskCreate(getMessageMQTT_task, "getMessageMQTT_task", 1024 * 4, NULL, 10, NULL);
}
