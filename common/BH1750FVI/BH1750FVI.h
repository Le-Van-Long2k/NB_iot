#ifndef BH1750FVI_H_
#define BH1750FVI_H_

#include <stdio.h>
#include "driver/gpio.h"
#include "driver/i2c.h"

/** Default I2C adrress for BH1750FVI*/
#define BH1750FVI_ADDR_MSB (0x5C)
#define BH1750FVI_ADDR_LSB (0x23)

#define WRITE_BIT I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ   /*!< I2C master read */

#define ACK_CHECK_EN 0x1 /*!< I2C master will check ack from slave*/
#define NACK_VAL 0x1     /*!< I2C nack value */
#define ACK_VAL 0x0      /*!< I2C ack value */
#define NACK_VAL 0x1     /*!< I2C nack value */

#define DATA_LENGTH 2 /*!<Data buffer length for test buffer*/

/** ESP32 master I2C pin */
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_Master_SDA 21
#define I2C_Master_SCL 22

// State
enum BH1750FVI_State
{
    BH1750FVI_POWER_ON = 0x00,
    BH1750FVI_POWER_OFF = 0x01,
    BH1750FVI_RESET = 0x07
};

// Resolution mode
enum BH1750FVI_ResolutionMode
{
    BH1750FVI_CONT_HIGH = 0x10,  //< 1lx, measure time 120ms
    BH1750FVI_CONT_HIGH2 = 0x11, //< 0.5lx, measure time 120ms
    BH1750FVI_CONT_LOW = 0x13,   //< 4lx, measure time 16ms
    BH1750FVI_ONCE_HIGH = 0x20,  //< 1lx, measure time 120ms
    BH1750FVI_ONCE_HIGH2 = 0x21, //< 0.5lx, measure time 120ms
    BH1750FVI_ONCE_LOW = 0x23    //< 4lx, measure time 16ms
};

#define BH1750FVI_REFERENCE_TIME 0x45 //  69 = default

// Mode
enum BH1750FVI_Mode
{
    BH1750FVI_MODE_LOW = 0x00,
    BH1750FVI_MODE_HIGH = 0x01,
    BH1750FVI_MODE_HIGH2 = 0x02
};

// ERROR CODES
enum BH1750FVI_status
{
    BH1750FVI_ERROR_WIRE_REQUEST = -10,
    BH1750FVI_OK = 0
};

typedef struct BH1750FVI
{
    uint8_t address;
    uint8_t sensitivityFactor;
    int status;
    uint8_t mode;
    uint32_t requestTime;
    int lux;
    uint8_t data_h;
    uint8_t data_l;

} BH1750FVI_t;

extern BH1750FVI_t BH1750;

void BH1750_Init();
// void BH1750_ConfigureMode(BH1750FVI_ResolutionMode);
// void BH1750_sleep(bool state);
// void BH1750_reset();
int BH1750_getLux();

// // Set Mode
void BH1750_setContHighRes();
void BH1750_setContHigh2Res();
void BH1750_setContLowRes();

void BH1750_setOnceHighRes();
void BH1750_setOnceHigh2Res();
void BH1750_setOnceLowRes();

bool BH1750_isReady(); //  only after setOnce...Res();

esp_err_t i2c_master_sensor_test(i2c_port_t i2c_num, uint8_t *data_h, uint8_t *data_l);

#endif