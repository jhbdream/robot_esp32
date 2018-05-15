/* ADC1 Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_system.h"


#define V_REF   3300
#define ADC1_TEST_CHANNEL (ADC1_CHANNEL_6)      //GPIO 34
#define ADC_ATTEN ADC_ATTEN_11db





void adc_init()
{
//Init ADC and Characteristics
    adc1_config_width(ADC_WIDTH_12Bit);
    adc1_config_channel_atten(ADC1_TEST_CHANNEL, ADC_ATTEN);//ADC1通道设定 增益设定
}

void adc_read()
{

    uint32_t voltage;
    while(1){
        voltage = adc1_get_voltage(ADC1_TEST_CHANNEL);
        printf("%d mV\n",voltage);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }   
}
