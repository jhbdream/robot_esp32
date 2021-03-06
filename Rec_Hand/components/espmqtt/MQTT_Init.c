#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event_loop.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt.h"

const char *MQTT_TAG = "MQTT_SAMPLE";
mqtt_client * Myclient;
static EventGroupHandle_t wifi_event_group;
const static int CONNECTED_BIT = BIT0;
//在这里订阅需要订阅的topic
void connected_cb(void *self, void *params)
{
	Myclient = (mqtt_client *)self;
}
void disconnected_cb(void *self, void *params)
{

}
void reconnect_cb(void *self, void *params)
{

}
void subscribe_cb(void *self, void *params)
{
   // ESP_LOGI(MQTT_TAG, "[APP] Subscribe ok, test publish msg");
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
        ESP_LOGI(MQTT_TAG, "[APP] Publish topic: %s", topic);
				//printf("%s",topic);
        free(topic);
    }

     char *data = malloc(event_data->data_length + 1);
     memcpy(data, event_data->data, event_data->data_length);
     data[event_data->data_length] = 0;
     printf("\n%s\n",data);
		 ESP_LOGI(MQTT_TAG, "[APP] Publish data[%d/%d bytes]",
             event_data->data_length + event_data->data_offset,
             event_data->data_total_length);
   

    // free(data);

}
//    mqtt_client *client = (mqtt_client *)self;
//    mqtt_subscribe(client, "/test", 0);
//    mqtt_publish(client, "/test", "howdy!", 6, 0, 0);
void MQTT_Init()
{
	
	mqtt_settings settings = {
    //.host = "45.76.192.45",
    .host = "192.168.1.108",
#if defined(CONFIG_MQTT_SECURITY_ON)
    .port = 8883, // encrypted
#else
    .port = 1883, // unencrypted
#endif
    .client_id = "mqtt_client_id",
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

	mqtt_start(&settings);
}
