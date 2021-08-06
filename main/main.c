#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/sockets.h"
#include "lwip/err.h"
#include "sdkconfig.h"

//defined in config file to avoid leaking secrets
#define SSID CONFIG_ESP_WIFI_SSID
#define WIFI_PASSWORD CONFIG_ESP_WIFI_PASSWORD 

#define TCP_SERVER_IP CONFIG_TCP_SERVER_IP
#define TCP_SERVER_PORT CONFIG_TCP_SERVER_PORT 

void on_wifi_disconnect(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data){
    ESP_LOGI("wifi", "Wifi disconnected, trying to reconnect...");
    esp_err_t err = esp_wifi_connect();
    if (err == ESP_ERR_WIFI_NOT_STARTED) {
        return;
    }
    ESP_ERROR_CHECK(err);
}

void wifi_connect(){
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = SSID,
            .password= WIFI_PASSWORD,
        },
    };

    ESP_LOGI("wifi","Connecting to %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());

    //call on_wifi_disconnect() if wifi connection gets lost
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &on_wifi_disconnect, NULL, NULL));
}

int send_data(char* msg){
    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr(TCP_SERVER_IP);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(TCP_SERVER_PORT);
   

    //create socket
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (s < 0) {
        ESP_LOGE("TCP", "Unable to create socket: errno %s", strerror(errno));
        return 1;
    }
    ESP_LOGI("TCP", "Socket created, connecting to %s:%d", TCP_SERVER_IP, TCP_SERVER_PORT);

    //connect to server
    if (connect(s, (struct sockaddr*)&dest_addr, sizeof(struct sockaddr_in)) != 0) {
        ESP_LOGE("TCP", "Socket unable to connect: %s", strerror(errno));
        shutdown(s, 0);
        close(s);
        return 2;
    }
    ESP_LOGI("TCP", "Socket successfully connected");

    //send message
    if (send(s, msg, strlen(msg), 0) < 0) {
        ESP_LOGE("TCP", "Error occurred during sending:%s", strerror(errno));
        shutdown(s, 0);
        close(s);
        return 3;
    }

    ESP_LOGI("TCP", "Shutting down socket...");
    shutdown(s, 0);
    close(s);
    return 0;
}

void app_main(void){
    int stat=0;
    int errcount=0;
    char wrd[50];

    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_connect();
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    for (int i=0;;i++){
        sprintf(wrd, "%d prev error count: %d", i, errcount);

        //sends message wrd to server. retries until success 
        while ((stat=send_data(wrd)) != 0){
            errcount++;
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        printf("%d: return state %d\n", i, stat);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}
