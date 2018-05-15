#include "driver/i2c.h"

#define I2C_MASTER_SCL_IO  19
#define I2C_MASTER_SDA_IO  18
#define I2C_MASTER_FREQ_HZ 400000
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0
#define I2C_MASTER_NUM I2C_NUM_1
#define WRITE_BIT I2C_MASTER_WRITE
#define READ_BIT  I2C_MASTER_READ
#define ACK_CHECK_EN  0X1
#define ACK_CHECK_DIS 0X0
#define ACK_VAL       0X0
#define NACK_VAL      0X1

int DMP_IIC_Write(uint8_t addr,uint8_t reg,uint8_t len,uint8_t *data );
int DMP_IIC_Read(uint8_t addr,uint8_t reg,uint8_t len,uint8_t *buf);
void i2c_master_init();

