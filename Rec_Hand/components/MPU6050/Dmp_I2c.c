#include <stdio.h>
#include "Dmp_I2c.h"
#include "driver/i2c.h"


//DMP ADDR:DEV_ADDR   REG:REGEIST ADDR
int DMP_IIC_Write(uint8_t addr,uint8_t reg,uint8_t len,uint8_t *data )
{
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (addr << 1) | WRITE_BIT, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
	i2c_master_write(cmd, data, len, ACK_CHECK_EN);
	i2c_master_stop(cmd);
	esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	//return ret;
	return 0x00;
}

int DMP_IIC_Read(uint8_t addr,uint8_t reg,uint8_t len,uint8_t *buf)
{
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, ( addr << 1) | WRITE_BIT, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_MASTER_NUM,cmd,1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, ( addr << 1 ) | READ_BIT, ACK_CHECK_EN);
	
	while(len)
	{
		i2c_master_read_byte(cmd,buf,(len==1));
		buf++;
		len--;
	}
	i2c_master_stop(cmd);
	esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	return 0x00;
}

void i2c_master_init()
{
	int i2c_master_port = I2C_MASTER_NUM;
	i2c_config_t conf;
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = I2C_MASTER_SDA_IO;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_io_num = I2C_MASTER_SCL_IO;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
	i2c_param_config(i2c_master_port, &conf);
	i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE,I2C_MASTER_TX_BUF_DISABLE, 0);
	
}
