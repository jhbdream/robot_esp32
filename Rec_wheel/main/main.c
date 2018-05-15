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
#include "driver/uart.h"
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
#include "math.h"
#include "uart1.h"
/**
             * - read the sensor data, if vfr4 connected.
 * - i2c master(ESP32) will write data to i2c slave(ESP32).
 * - i2c master(ESP32) will read data from i2c slave(ESP32).
 */

#define T_HEAD  "/HEAD"
#define T_HANDL "/HANDL"
#define T_HANDR "/HEADR"
#define DATA_LEMGTH 11
#define SPEED       50

 QueueHandle_t Q_Head;
 
 // Create a queue capable of containing 10 uint32_t values.
union adcval{
    uint16_t adcval[4];
    uint8_t  receive[8];
}adc;

union wheel_speed{
    int s;
    uint8_t  s_c[4];
}wheel1,wheel2;
const char *MQTT_MAINTAG = "mqtt MAIN";
const char *MAINTAG = "MAIN";

mqtt_client * Myclient;
static EventGroupHandle_t wifi_event_group;
const static int CONNECTED_BIT = BIT0;
const static int MQTT_CONNECT_BIT = BIT1;

//发送轮子速度到ｓｔｍ３２
void send_speed(int speed1,int speed2)
{

    static uint8_t send_buf[12],i=0;
    
    wheel1.s = speed1;
    wheel2.s = speed2;
     
    send_buf[0]=0X0D;
    send_buf[1]=0X0A;

    for( i = 0; i < 4; i++)
    {
        send_buf[i+2]=wheel1.s_c[i];
        send_buf[i+6]=wheel2.s_c[i];

    }
    send_buf[10]=0X0A;
    send_buf[11]=0X0D;
    uart_write_bytes(UART_NUM_1, (const char *) send_buf, sizeof(send_buf));

}


//发送5位数据  比如字符串PXXXX,RXXXX,AXXXX,返回整形XXXX 发送格式必须固定
//str:需要解析的字符串 flag:字符串其实标注 length：待解析字符串长度 result：返回结果
void String(char * str,char flag,int length,int * result){
    int j=0;
    for( j=0; j<length; j++,str++)
    {
        
        if(*str == flag){
            for(int i=4;i >=0 ;i --)
            {
                if(*(str+i) >= '0' && *(str+i) <= '9')
                {  
                   
                    (*result)+=((int)(*(str+i)-'0')*(int)pow(10,4-i));
                  
                }
                else if(*(str+i) == '-')
                {
                    *result =-*result;

                }
                else if(*(str+i) == ' ' )
                {
                    break;
                }
            }
            break;
        }

    }
    if(j==length)
        *result = 5000;
}
//在这里订阅需要订阅的topic
void connected_cb(void *self, void *params)
{
    Myclient = (mqtt_client *)self;

    /* 订阅三个topic*/
  
    xEventGroupSetBits(wifi_event_group, MQTT_CONNECT_BIT);

}
void disconnected_cb(void *self, void *params)
{

}
void reconnect_cb(void *self, void *params)
{

}
void subscribe_cb(void *self, void *params)
{
   // ESP_LOGI(MQTT_MAINTAG, "[APP] Subscribe ok, test publish msg");
   // mqtt_client *client = (mqtt_client *)self;
   // mqtt_publish(client, "/test", "abcde", 5, 0, 0);
}

void publish_cb(void *self, void *params)
{

}
void data_cb(void *self, void *params)
{
    mqtt_event_data_t *event_data = (mqtt_event_data_t *)params;

    /* 提取话题　　根据话题写入不同队列　*/
    if(event_data->data_offset == 0) {

            char *topic = malloc(event_data->topic_length + 1);
            memcpy(topic, event_data->topic, event_data->topic_length);
            topic[event_data->topic_length] = 0;
            
            //ESP_LOGI(MQTT_MAINTAG, "[APP] Publish topic: %s", topic);
            
            if( !strcmp(topic,T_HEAD) )
            {
                xQueueSend( Q_Head, (char *)event_data->data, ( TickType_t )10 );
            }
        
            else printf("error topic !\n");
            
            free(topic);
    }

}   
  
/*  
    printf("data:%s length:%d\n",data,event_data->data_length);
    String(data,'P',event_data->data_length,&pitch);
    String(data,'R',event_data->data_length,&roll);
    String(data,'A',event_data->data_length,&ad);
*/

/*
     if(pitch!=5000){
        
        printf("recive Pitch:%d\n",pitch);
     }
    if(roll!=5000){
        
        printf("recive Roll:%d\n",roll);
     }
    if(ad!=5000){
        
        printf("recive AD:%d\n",ad);
     }
*/
  /*   ESP_LOGI(MQTT_MAINTAG, "[APP] Publish data[%d/%d bytes]",
             event_data->data_length + event_data->data_offset,
             event_data->data_total_length);
*/

//    mqtt_client *client = (mqtt_client *)self;
//    mqtt_subscribe(client, "/test", 0);
//    mqtt_publish(client, "/test", "howdy!", 6, 0, 0);

    mqtt_settings settings = {
    .host = "60.205.183.228",
    //.host = "192.168.31.165",
#if defined(CONFIG_MQTT_SECURITY_ON)
    .port = 8883, // encrypted
#else
    .port = 1883, // unencrypted
#endif
    .client_id = "wheel",
    .username = "user",
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
    mqtt_start(&settings);

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

void smartconfig(void)
{
    ESP_ERROR_CHECK( esp_smartconfig_set_type(SC_TYPE_ESPTOUCH) );
    ESP_ERROR_CHECK( esp_smartconfig_start(sc_callback) );
    
}
void user_init(void)
{

    uint16_t apCount = 0;
    wifi_config_t wifi_config;
    
    wifi_scan_config_t scan_config;
    ESP_ERROR_CHECK( nvs_flash_init() );
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();


    Q_Head  = xQueueCreate( 10, sizeof( int8_t) * DATA_LEMGTH );

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
            if( apCount !=0 )   //扫描到存储的WiFi
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
       
}

static void Head_task(void* arg)
{
    int8_t pitch=0,roll=0,yaw=0;
    int16_t adc0,adc1,adc2,adc3;

    int SPEEDL=0,SPEEDR=0;
    int8_t * data =malloc(4);


    while(1){
        xQueueReceive( Q_Head, data , ( TickType_t ) portMAX_DELAY );

        pitch = data[0] ;
        roll  = data[1] ;
        yaw   = data[2] ;
               

        if( roll > 20 )
        {
            SPEEDL = 200;
            SPEEDR = 200;

        }
        else if( roll < -20)
        {
            SPEEDL = -200;
            SPEEDR = -200;

        }
        else if( pitch > 20 )
        {
            SPEEDL = 200;
            SPEEDR = 0;
        }
        else if( pitch < -20)
        {
          
            SPEEDL = 0;
            SPEEDR = 200;
        }   
        else
        {
            SPEEDL = 0;
            SPEEDR = 0;
        }
        send_speed(SPEEDL,SPEEDR);
        printf("%4d %4d \n",SPEEDL,SPEEDR);

        /* 数据接收和处理在这里进行　　根据TOPIC 执行不同的响应　三个订阅内容  strcmp (topic, "")*/
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
    
    uart1_Init();

    xTaskCreate(&Head_task,  "Head_task",   2048, NULL, 6, NULL);
  
    ESP_LOGI(MAINTAG, "wait to mqtt_subscribeS ");

    uxBits = xEventGroupWaitBits(wifi_event_group, MQTT_CONNECT_BIT , true, false, portMAX_DELAY); 
 
    
    mqtt_subscribe( Myclient, T_HEAD,    0);    

}



