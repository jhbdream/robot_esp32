#include "mytime.h"
#include "driver/timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
/*
 * @brief timer group0 hardware timer1 init
 */
 void tg0_timer1_init()
{
    int timer_group = TIMER_GROUP;
    int timer_idx 	= TIMER_IDX;
    timer_config_t config;
    config.alarm_en 	= TIMER_ALARM_DIS;
    config.auto_reload 	= TIMER_AUTORELOAD_EN;
    config.counter_dir 	= TIMER_COUNT_DOWN;
    config.divider 		= TIMER_DIVIDER;
    config.intr_type 	= TIMER_INTR_SEL;
    config.counter_en 	= TIMER_PAUSE;
    /*Configure timer*/
    timer_init(timer_group, timer_idx, &config);
    /*Stop timer counter*/
    timer_pause(timer_group, timer_idx);
    /*Load counter value */
    timer_set_counter_value(timer_group, timer_idx, 0x00000000ULL);
    /*Start timer counter*/
    timer_start(timer_group, timer_idx);
}

void delay_ms(uint16_t nms)
{
	vTaskDelay(nms / portTICK_RATE_MS);

}


