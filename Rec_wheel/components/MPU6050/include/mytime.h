#include "driver/timer.h"

#define TIMER_INTR_SEL TIMER_INTR_LEVEL  /*!< Timer level interrupt */
#define TIMER_DIVIDER   80               /*!< Hardware timer clock divider AHB 80MHZ */
#define TIMER_SCALE    (TIMER_BASE_CLK / TIMER_DIVIDER)  /*!< used to calculate counter value */
#define TEST_WITHOUT_RELOAD   0   /*!< example of auto-reload mode */
#define TEST_WITH_RELOAD   1      /*!< example without auto-reload mode */
#define TIMER_GROUP TIMER_GROUP_0
#define TIMER_IDX  TIMER_1

void tg0_timer1_init();
void delay_ms(uint16_t nms);

