#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_smartconfig.h"
void user_init(void);
void start_task(void);
void smartconfig(void);
static esp_err_t event_handler(void *ctx, system_event_t *event);
static void sc_callback(smartconfig_status_t status, void *pdata);



