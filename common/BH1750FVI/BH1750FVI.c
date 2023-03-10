#include <stdio.h>
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/timer.h"
#include "BH1750FVI.h"

void BH1750_Init()
{
    BH1750.address = BH1750FVI_ADDR_LSB; // chan Add = 0;  neu Add = 1 thi adrr = BH1750FVI_ADDR_MSB
    BH1750.sensitivityFactor = BH1750FVI_REFERENCE_TIME;
    BH1750.status = BH1750FVI_OK;
    BH1750.mode = BH1750FVI_MODE_HIGH;
    BH1750.requestTime = 0;
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_Master_SDA,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_Master_SCL,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };
    i2c_param_config(I2C_MASTER_NUM, &conf);
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

/**
 * @brief Test code to write esp-i2c-slave
 *        Master device write data to slave(both esp32),
 *        the data will be stored in slave buffer.
 *        We can read them out from slave buffer.
 *
 * ____________________________________________________________________
 * | start | slave_addr + wr_bit + ack | command 1 bytes + ack | stop |
 * --------|-----7b---------1b------1b-|--------8b---------1b--|------|
 *
 */
static esp_err_t command(uint8_t *data_wr)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (BH1750.address << 1) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, data_wr, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    BH1750.status = ret;
    return BH1750.status;
}

/**
 * @brief test code to read esp-i2c-slave
 *        We need to fill the buffer of esp slave device, then master can read them out.
 *
 * ______________________________________________________________________________________________
 * | start | slave_addr + rd_bit +ack | High Bytes [15:8] + ack | Low Bytes [7:0] + nack | stop |
 * --------|----7b---------1b---------|---------8b--------------|---------8b-------------|------|
 *
 */
static esp_err_t BH1750_readData()
{
    uint8_t data_h = 0;
    uint8_t data_l = 0;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (BH1750.address << 1) | READ_BIT, ACK_CHECK_EN);
    i2c_master_read_byte(cmd, &data_h, ACK_VAL);
    i2c_master_read_byte(cmd, &data_l, NACK_VAL);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    BH1750.data_h = data_h;
    BH1750.data_l = data_l;
    BH1750.status = ret;
    return BH1750.status;
}

// ////////////////////////////////////////////
// //
// //  operational mode
// //
void BH1750_setContHighRes()
{
    BH1750.mode = BH1750FVI_MODE_HIGH;
    command(BH1750FVI_CONT_HIGH);
    BH1750.requestTime = esp_timer_get_time();
};

void BH1750_setContHigh2Res()
{
    BH1750.mode = BH1750FVI_MODE_HIGH2;
    command(BH1750FVI_CONT_HIGH2);
    BH1750.requestTime = esp_timer_get_time();
};

void BH1750_setContLowRes()
{
    BH1750.mode = BH1750FVI_MODE_LOW;
    command(BH1750FVI_CONT_LOW);
    BH1750.requestTime = esp_timer_get_time();
};

void BH1750_setOnceHighRes()
{
    BH1750.mode = BH1750FVI_MODE_HIGH;
    command(BH1750FVI_ONCE_HIGH);
    BH1750.requestTime = esp_timer_get_time();
};

void BH1750_setOnceHigh2Res()
{
    BH1750.mode = BH1750FVI_MODE_HIGH2;
    command(BH1750FVI_ONCE_HIGH2);
    BH1750.requestTime = esp_timer_get_time();
};

void BH1750_setOnceLowRes()
{
    BH1750.mode = BH1750FVI_MODE_LOW;
    command(BH1750FVI_ONCE_LOW);
    BH1750.requestTime = esp_timer_get_time();
};

bool BH1750_isReady()
{
    // Max times from datasheet P5
    uint8_t timeout[3] = {16, 120, 120};
    if (BH1750.mode < 3)
    {
        float f = timeout[BH1750.mode] * BH1750.sensitivityFactor / BH1750FVI_REFERENCE_TIME; // Illuminance per 1 count ( lx / count )
        return (esp_timer_get_time() - BH1750.requestTime) > f;
    }
    return false;
}

int BH1750_getLux()
{
    BH1750_readData();
    uint16_t data;
    data = BH1750.data_h;
    data <<= 8;
    data |= BH1750.data_l;
    BH1750.lux = (int)(data / 1.2);
    return BH1750.lux;
}

/**
 * @brief test code to operate on BH1750 sensor
 *
 * 1. set operation mode(e.g One time L-resolution mode)
 * _________________________________________________________________
 * | start | slave_addr + wr_bit + ack | write 1 byte + ack  | stop |
 * --------|---------------------------|---------------------|------|
 * 2. wait more than 24 ms
 * 3. read data
 * ______________________________________________________________________________________
 * | start | slave_addr + rd_bit + ack | read 1 byte + ack  | read 1 byte + nack | stop |
 * --------|---------------------------|--------------------|--------------------|------|
 */
