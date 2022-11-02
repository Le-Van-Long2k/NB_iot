#ifndef SIMCOM7020_H_
#define SIMCOM7020_H_

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "driver/uart.h"
#include <string.h>

#define NUMERO_PORTA_SERIALE2 UART_NUM_2
#define BUF_SIZE (1024 * 2)
#define RD_BUF_SIZE (1024)
#define U2RXD 16
#define U2TXD 17

typedef struct simcom_t
{
  uart_port_t uart_num;
  int tx_io_num;
  int rx_io_num;
  int baud_rate;
  int timestamp;
  int tp;
  bool AT_buff_avai;
  uint8_t AT_buff[BUF_SIZE];
  void (*mqtt_CB)(char *data);
} simcom;
extern simcom simcom_7020;

typedef enum
{
  AT_OK,
  AT_ERROR,
  AT_TIMEOUT,
} AT_flag;

typedef struct client_t
{
  char uri[51];
  int port;
  char user_name[50];
  char client_id[50];
  char password[50];
  int mqtt_id;
} client;

// cau hinh uart cho simcom
void init_simcom(uart_port_t uart_num, int tx_num, int rx_num, int baud_rate);
// ISR uart
void UART_ISR_ROUTINE(void *pvParameters);
// gui AT command
void send_ATComand_Sim(char *ATcommand, int retry);
// reset simcom
void restart_simcom();
// init simcom
bool isInit(int retry);
// kiem tra ket noi mang
bool isRegistered(int retry);
// Check network
void Check_network();
// check sim
bool isCheck_sim(int retry);
// khoi tao ket noi mqtt
bool mqtt_start(client clientMQTT, int versionMQTT, int keepalive, int clean_session, int retry);
// dinh dang message cua ban tin public la raw (AT+CREVHEX=0) | hex (AT+CREVHEX=1)
bool isconverhex(int retry);
// ngat ket noi mqtt
bool mqtt_stop(client clientMQTT, int retry); 
// dang ki topic de nhan ban tin dieu khien
bool mqtt_subscribe(client clientMQTT, char *topic, int qos, int retry,  void (*mqttSubcribeCB)(char * data));
// gui ban tin vao topic
bool mqtt_message_publish(client clientMQTT, char *data, char *topic,int qos,  int retry);



bool get_signal_strength(char *rssi, char *rsrp, char *rsrq, int retry);
bool getCellId(int *mcc, int *mnc, char *lac, char *cid, int retry);
int filter_comma_t(char *respond_data, int begin, int end, char *output);

#endif /* SIMCOM7020_H_ */
