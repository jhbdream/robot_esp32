#include"inv_mpu.h"

#define DEFAULT_MPU_HZ  (200)
#define q30  1073741824.0f
void DMP_Init(void);
uint8_t Read_DMP(float* Pitch,float* Roll,float* Yaw);

