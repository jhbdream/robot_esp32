#include <stdio.h>
#include "driver/i2c.h"
#include "driver/timer.h"
#include "esp_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "soc/timer_group_struct.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event_loop.h"
#include "driver/periph_ctrl.h"
#include "Mpu6050.h"
#include "mytime.h"
#include "mqtt.h"
#include "esp_log.h"
#include "esp_smartconfig.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "nvs_flash.h"
#include "tcpip_adapter.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "bsp_adc.h"
#include "driver/adc.h"
#include "esp_system.h"
#include "esp_adc_cal.h"

#define T_HEAD  "/HEAD"
#define T_HANDL "/HANDL"
#define T_HANDR "/HEADR"
#define DATA_LEMGTH 11
union adcval{
    uint16_t read[4];
    uint8_t  send[8];
}adc;

const char *MQTT_MAINTAG = "mqtt MAIN";
const char *MAINTAG = "MAIN";
esp_adc_cal_characteristics_t characteristics;

    
mqtt_client * Myclient;
static EventGroupHandle_t wifi_event_group;
const static int CONNECTED_BIT = BIT0;
const static int MQTT_CONNECTED_BIT = BIT1;
//在这里订阅需要订阅的topic
void connected_cb(void *self, void *params)
{
    Myclient = (mqtt_client *)self;
    xEventGroupSetBits(wifi_event_group, MQTT_CONNECTED_BIT);
} 
void disconnected_cb(void *self, void *params)
{
    xEventGroupClearBits(wifi_event_group, MQTT_CONNECTED_BIT);
}
void reconnect_cb(void *self, void *params)
{

}
void subscribe_cb(void *self, void *params)
{
   // ESP_LOGI(MQTT_MAINTAG, "[APP] Subscribe ok, test publish msg");
   // mqtt_client *client = (mqtt_client *)self;
   //mqtt_publish(client, "/test", "abcde", 5, 0, 0);
}

void publish_cb(void *self, void *params)
{

}
void data_cb(void *self, void *params)
{
    mqtt_client *client = (mqtt_client *)self;
    mqtt_event_data_t *event_data = (mqtt_event_data_t *)params;

    if(event_data->data_offset == 0) {

        char *topic = malloc(event_data->topic_length + 1);
        memcpy(topic, event_data->topic, event_data->topic_length);
        topic[event_data->topic_length] = 0;
        ESP_LOGI(MQTT_MAINTAG, "[APP] Publish topic: %s", topic);
                //printf("%s",topic);
        free(topic);
    }

     char *data = malloc(event_data->data_length + 1);
     memcpy(data, event_data->data, event_data->data_length);
     data[event_data->data_length] = 0;
     printf("\n%s\n",data);
         ESP_LOGI(MQTT_MAINTAG, "[APP] Publish data[%d/%d bytes]",
             event_data->data_length + event_data->data_offset,
             event_data->data_total_length);
   

    // free(data);

}
//    mqtt_client *client = (mqtt_client *)self;
//    mqtt_subscribe(client, "/test", 0);
//    mqtt_publish(client, "/test", "howdy!", 6, 0, 0);

mqtt_settings mqtt_cfg = { 

    .host = "60.205.183.228",
   // .host = "192.168.31.165",
    /*
#if defined(CONFIG_MQTT_SECURITY_ON)
    .port = 8883, // encrypted
#else
    .port = 1883, // unencrypted
#endif*/
    .port = 1883,
    .client_id = "SEND_Head",
    .username = "user1",
    .password = "pass",
    .clean_session = 0,
    .keepalive = 120,
    .lwt_topic = "/lwt",
    .lwt_msg = "offline",
    .lwt_qos = 0,
    .lwt_retain = 0,
    .connected_cb = connected_cb,
    .disconnected_cb = disconnected_cb,
    .subscribe_cb = subscribe_cb,
    .publish_cb = publish_cb,
    .data_cb = data_cb

    };
void MQTT_Init()
{
    
    ESP_LOGI(MQTT_MAINTAG, "connect port %d\n", mqtt_cfg.port);

    mqtt_start(&mqtt_cfg);
}


static void disp_buf(uint8_t* buf, int len)
{
    int i;
    for (i = 0; i < len; i++) {
        printf("%02x ", buf[i]);
        if (( i + 1 ) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

void MY_adc_init()
{

    //Init ADC and Characteristics
    adc1_config_width(ADC_WIDTH_BIT_12);
 
    adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11);     /*!< ADC1 channel 5 is GPIO32 */
    adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_DB_11);     /*!< ADC1 channel 4 is GPIO33 */    
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);     /*!< ADC1 channel 6 is GPIO34 */
    adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_11);     /*!< ADC1 channel 7 is GPIO35 */

    esp_adc_cal_get_characteristics(1100, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, &characteristics);


}
/* adc读取　GPIO32-GPIO35 ADC1 */
void MY_adc_read()
{



    adc.read[0] = adc1_to_voltage(ADC1_CHANNEL_4, &characteristics);
    adc.read[1] = adc1_to_voltage(ADC1_CHANNEL_5, &characteristics);
    adc.read[2] = adc1_to_voltage(ADC1_CHANNEL_6, &characteristics);
    adc.read[3] = adc1_to_voltage(ADC1_CHANNEL_7, &characteristics);

/*
    printf("---------------------------------------------------------\n");
    printf("GPIO32   GPIO33   GPIO34   GPIO35\n");

    for(int i=0; i < 4; i++) 
    printf("%4dmV   ",voltage[i]);
    printf("\n---------------------------------------------------------\n\n");
*/
}
static void mpu6050_read_task(void* arg)
{	   
	uint8_t idf=1;
    EventBits_t uxBits;
	int time=0;
    int8_t data[20];
	float   Pitch,Roll,Yaw;
    int8_t  C_Pitch,C_Roll,C_Yaw;
    i2c_master_init();
	DMP_IIC_Read(0x68,0x75,1,&idf);
	printf("WHO IS %x\n",idf);
	DMP_Init();
    while (1) {
        
        Read_DMP(&Pitch,&Roll,&Yaw);
        
        C_Pitch =(int8_t)Pitch;
        C_Roll  =(int8_t)Roll;
        C_Yaw   =(int8_t)Yaw;
       
        data[0] = C_Pitch;
        data[1] = C_Roll;
        data[2] = C_Yaw;
        
        printf("%4d %4d %4d \n",C_Pitch,C_Roll,C_Yaw);

        uxBits = xEventGroupWaitBits(wifi_event_group, MQTT_CONNECTED_BIT , false, false, 0); 
        if( uxBits & MQTT_CONNECTED_BIT )
          { 
                mqtt_publish(Myclient, T_HEAD, (char *)data,3, 0,  0);  
          }
        vTaskDelay( 100 / portTICK_RATE_MS);

    }
   
}



static void sc_callback(smartconfig_status_t status, void *pdata)
{
    switch (status) {
        case SC_STATUS_WAIT:
            ESP_LOGI(MAINTAG, "SC_STATUS_WAIT");
            break;
        case SC_STATUS_FIND_CHANNEL:
            ESP_LOGI(MAINTAG, "SC_STATUS_FINDING_CHANNEL");
            break;
        case SC_STATUS_GETTING_SSID_PSWD:
            ESP_LOGI(MAINTAG, "SC_STATUS_GETTING_SSID_PSWD");
            break;
        case SC_STATUS_LINK:
            ESP_LOGI(MAINTAG, "SC_STATUS_LINK");
            wifi_config_t *wifi_config = pdata;
            ESP_LOGI(MAINTAG, "SSID:%s", wifi_config->sta.ssid);
            ESP_LOGI(MAINTAG, "PASSWORD:%s", wifi_config->sta.password);
            ESP_ERROR_CHECK( esp_wifi_disconnect() );
            ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, wifi_config) );
            ESP_ERROR_CHECK( esp_wifi_connect() );
            break;
        case SC_STATUS_LINK_OVER:
            ESP_LOGI(MAINTAG, "SC_STATUS_LINK_OVER");
            if (pdata != NULL) {
                uint8_t phone_ip[4] = { 0 };
                memcpy(phone_ip, (uint8_t* )pdata, 4);
                ESP_LOGI(MAINTAG, "Phone ip: %d.%d.%d.%d\n", phone_ip[0], phone_ip[1], phone_ip[2], phone_ip[3]);
            }
            esp_smartconfig_stop();
           

            break;
        default:
            break;
    }
}

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
       // mqtt_stop();
        break;
    default:
        break;
    }
    return ESP_OK;
}

void smartconfig(void)
{
    ESP_ERROR_CHECK( esp_smartconfig_set_type(SC_TYPE_ESPTOUCH) );
    ESP_ERROR_CHECK( esp_smartconfig_start(sc_callback) );
    
}
void user_init(void)
{

    uint8_t apCount = 0;
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

    if( strlen((char *)wifi_config.sta.ssid) !=0 )
    {
        memset(&scan_config , 0 ,sizeof(scan_config));
        scan_config.ssid = wifi_config.ap.ssid;
        esp_err_t ret= esp_wifi_scan_start(&scan_config, 1);
        if( ret == ESP_OK )
        {
            
            esp_wifi_scan_get_ap_num(&apCount);

            printf("apcount : is %d\n",apCount);
            if( apCount !=0 )   //扫描到存储的WiFi
            {
                ESP_LOGI(MAINTAG, "esp_wifi_connect\n");

                esp_wifi_connect();
            }
            else //未找到wifi  启动smartconfig
            {
                smartconfig();
            }
        
        }
        else if(ret == ESP_ERR_WIFI_TIMEOUT)
        {
            printf("ap scan time out\n");
            
        }

    }
    else //
    {
        printf("flash no story! start smartconfig\n");
        smartconfig();

    }
       
}
/*
void adc_task(void)
{
    int vol;
    while(1){

        vol=adc_read();
        printf("%d mv",vol);
        vTaskDelay( 1000 / portTICK_RATE_MS);
    }
}*/
void wait_start(void){



   while (1) {
            
           
         //   adc_init();
        //    xTaskCreate(adc_read, "adc_read", 1024 * 2, (void* ) 0, 10, NULL);

            xTaskCreate(mpu6050_read_task, "mpu6050_read_task0", 1024 * 2, (void* ) 0, 10, NULL);
            vTaskDelete(NULL);
    }
}
void app_main()
{

    EventBits_t uxBits;
    user_init();


    ESP_LOGI(MAINTAG, "wait to connect wifi ");

    uxBits = xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT , true, false, portMAX_DELAY); 

    ESP_LOGI(MAINTAG, "WiFi Connected to ap");

    MQTT_Init();

    xEventGroupWaitBits(wifi_event_group, MQTT_CONNECTED_BIT , false, false, portMAX_DELAY); 

    ESP_LOGI(MAINTAG, "MQTT Connected to SERVER");
    wait_start();
    vTaskSuspend(NULL); 
}



