#include "Mpu6050.h"
#include "stdio.h"
#include <math.h>
#include "inv_mpu_dmp_motion_driver.h"
#include "inv_mpu.h"
#include <esp_types.h>
#include "esp_err.h"


static signed char gyro_orientation[9] = {-1, 0, 0,
                                           0,-1, 0,
                                           0, 0, 1};
                                           

static  unsigned short inv_row_2_scale(const signed char *row)
{
    unsigned short b;

    if (row[0] > 0)
        b = 0;
    else if (row[0] < 0)
        b = 4;
    else if (row[1] > 0)
        b = 1;
    else if (row[1] < 0)
        b = 5;
    else if (row[2] > 0)
        b = 2;
    else if (row[2] < 0)
        b = 6;
    else
        b = 7;      // error
    return b;
}


static  unsigned short inv_orientation_matrix_to_scalar(
    const signed char *mtx)
{
    unsigned short scalar;
    scalar = inv_row_2_scale(mtx);
    scalar |= inv_row_2_scale(mtx + 3) << 3;
    scalar |= inv_row_2_scale(mtx + 6) << 6;


    return scalar;
}

static void run_self_test(void)
{
    int result;
    long gyro[3], accel[3];

    result = mpu_run_self_test(gyro, accel);
    if (result == 0x7) {
        /* Test passed. We can trust the gyro data here, so let's push it down
         * to the DMP.
         */
        float sens;
        unsigned short accel_sens;
        mpu_get_gyro_sens(&sens);
        gyro[0] = (long)(gyro[0] * sens);
        gyro[1] = (long)(gyro[1] * sens);
        gyro[2] = (long)(gyro[2] * sens);
        dmp_set_gyro_bias(gyro);
        mpu_get_accel_sens(&accel_sens);
        accel[0] *= accel_sens;
        accel[1] *= accel_sens;
        accel[2] *= accel_sens;
        dmp_set_accel_bias(accel);
		printf("setting bias succesfully ......\r\n");
    }
}


/**************************************************************************
函数功能：MPU6050内置DMP的初始化
入口参数：无
返回  值：无
作    者：平衡小车之家
**************************************************************************/
void DMP_Init(void)
{ 
// u8 temp[1]={0};
// i2cRead(0x68,0x75,1,temp);
// printf("mpu_set_sensor complete ......\r\n");
//if(temp[0]!=0x68)NVIC_SystemReset();
	if(!mpu_init())
  {
		printf("mpu init is ok!\n");
	  if(!mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL))
	  	 printf("mpu_set_sensor complete ......\r\n");
	  if(!mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL))
	  	 printf("mpu_configure_fifo complete ......\r\n");
	  if(!mpu_set_sample_rate(DEFAULT_MPU_HZ))
	  	 printf("mpu_set_sample_rate complete ......\r\n");
	  if(!dmp_load_motion_driver_firmware())
	  	printf("dmp_load_motion_driver_firmware complete ......\r\n");
	  if(!dmp_set_orientation(inv_orientation_matrix_to_scalar(gyro_orientation)))
	  	 printf("dmp_set_orientation complete ......\r\n");
	  if(!dmp_enable_feature(DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_TAP |
	        DMP_FEATURE_ANDROID_ORIENT | DMP_FEATURE_SEND_RAW_ACCEL | DMP_FEATURE_SEND_CAL_GYRO |
	        DMP_FEATURE_GYRO_CAL))
	  	 printf("dmp_enable_feature complete ......\r\n");
	  if(!dmp_set_fifo_rate(DEFAULT_MPU_HZ))
	  	 printf("dmp_set_fifo_rate complete ......\r\n");
	  run_self_test();
	  if(!mpu_set_dmp_state(1))
	  	 printf("mpu_set_dmp_state complete ......\r\n");
  }
}
/**************************************************************************
函数功能：读取MPU6050内置DMP的姿态信息
入口参数：无
返回  值：无
作    者：平衡小车之家
**************************************************************************/
uint8_t Read_DMP(float* Pitch,float* Roll,float* Yaw)
{	
		short gyro[3], accel[3], sensors;
		float q0=1.0f,q1=0.0f,q2=0.0f,q3=0.0f;
	    unsigned long sensor_timestamp;
		unsigned char more;
		long quat[4];
				if(dmp_read_fifo(gyro, accel, quat, &sensor_timestamp, &sensors, &more)) return 1;		
				if (sensors & INV_WXYZ_QUAT)
				{    
					 q0=quat[0] / q30;
					 q1=quat[1] / q30;
					 q2=quat[2] / q30;
					 q3=quat[3] / q30;
					 *Pitch = (float)asin(-2 * q1 * q3 + 2 * q0* q2)* 57.3; 	
					 *Roll = (float)atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2* q2 + 1)* 57.3; // roll
					 *Yaw = (float)atan2(2*(q1*q2 + q0*q3),q0*q0+q1*q1-q2*q2-q3*q3) * 57.3;
					 return 0;
				}	
				else 
					return 2;
				
}
/*
void Mpu6050_Init()
{	
	uint8_t idf=1;

	i2c_master_init();
	DMP_Init();
	DMP_IIC_Read(0x68,0x75,1,&idf);
	printf("WHO IS %x\n",idf);

}

static void mpu6050_read_task(void* arg)
{	   
	
	float Pitch,Roll,Yaw;
	
    while (1) {
		
    Read_DMP(&Pitch,&Roll,&Yaw);
	printf("Pitch is: %f,Roll is %f,Yaw is %f\n",Pitch,Roll,Yaw);  
    vTaskDelay( 10 / portTICK_RATE_MS);

    }
}*/
//------------------End of File----------------------------
