#include "user_wifi.h"
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "tcpip_adapter.h"
#include "esp_smartconfig.h"
static const char *TAG = "smartconfig";
static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
static const int CONNECTED_BIT = BIT0;
static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
			
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
		printf("system get ip!\n");
		xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        esp_wifi_connect();

        break;
    default:
        break;
    }
    return ESP_OK;
}


static void sc_callback(smartconfig_status_t status, void *pdata)
{
    switch (status) {
        case SC_STATUS_WAIT:
            ESP_LOGI(TAG, "SC_STATUS_WAIT");
            break;
        case SC_STATUS_FIND_CHANNEL:
            ESP_LOGI(TAG, "SC_STATUS_FINDING_CHANNEL");
            break;
        case SC_STATUS_GETTING_SSID_PSWD:
            ESP_LOGI(TAG, "SC_STATUS_GETTING_SSID_PSWD");
            break;
        case SC_STATUS_LINK:
            ESP_LOGI(TAG, "SC_STATUS_LINK");
            wifi_config_t *wifi_config = pdata;
            ESP_LOGI(TAG, "SSID:%s", wifi_config->sta.ssid);
            ESP_LOGI(TAG, "PASSWORD:%s", wifi_config->sta.password);
            ESP_ERROR_CHECK( esp_wifi_disconnect() );
            ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, wifi_config) );
            ESP_ERROR_CHECK( esp_wifi_connect() );
            break;
        case SC_STATUS_LINK_OVER:
            ESP_LOGI(TAG, "SC_STATUS_LINK_OVER");
            if (pdata != NULL) {
                uint8_t phone_ip[4] = { 0 };
                memcpy(phone_ip, (uint8_t* )pdata, 4);
                ESP_LOGI(TAG, "Phone ip: %d.%d.%d.%d\n", phone_ip[0], phone_ip[1], phone_ip[2], phone_ip[3]);
            }
			esp_smartconfig_stop();
            break;
        default:
            break;
    }
}

void user_init(void)
{
	EventBits_t uxBits;

	uint16_t apCount = 0;
	wifi_config_t wifi_config;
	
	wifi_scan_config_t scan_config;
	ESP_ERROR_CHECK( nvs_flash_init() );
	tcpip_adapter_init();
	wifi_event_group = xEventGroupCreate();

	ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

	ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
	ESP_ERROR_CHECK( esp_wifi_set_storage( WIFI_STORAGE_FLASH));
	ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
	ESP_ERROR_CHECK( esp_wifi_start() );
	
	memset(&wifi_config,0,sizeof(wifi_config));
	esp_wifi_get_config(ESP_IF_WIFI_STA,&wifi_config);//
	printf("ssid: %s!\npassword:%s\n",wifi_config.sta.ssid,wifi_config.sta.password);

	if( strlen((char *)wifi_config.sta.ssid) !=0 )//
	{
		memset(&scan_config , 0 ,sizeof(scan_config));
		scan_config.ssid = wifi_config.ap.ssid;
		esp_err_t ret= esp_wifi_scan_start(&scan_config, 1);
		if( ret == ESP_OK )//
		{
			
			esp_wifi_scan_get_ap_num(&apCount);
			printf("apcount : is %d\n",apCount);
			if( apCount !=0 )	//扫描到存储的WiFi
			{
				
				esp_wifi_connect();

			}

			else //未找到wifi  启动smartconfig
			{
				smartconfig();
			}
		
		}

		else if(ret == ESP_ERR_WIFI_TIMEOUT)//
		{
			printf("ap scan time out\n");
			
		}

	}

	else //
	{
		printf("flash no story! start smartconfig\n");
		smartconfig();

	}

	while (1) {
        
        uxBits = xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT , true, false, portMAX_DELAY); 
        ESP_LOGI(TAG, "wait to connect wifi ");
        if(uxBits & CONNECTED_BIT) {
            ESP_LOGI(TAG, "WiFi Connected to ap");
        	break;
        }
       
}
	

}
void smartconfig(void)
{
    ESP_ERROR_CHECK( esp_smartconfig_set_type(SC_TYPE_AIRKISS) );
    ESP_ERROR_CHECK( esp_smartconfig_start(sc_callback) );
	
}



