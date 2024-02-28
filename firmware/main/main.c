/* Play an MP3 file or changes the volume upon receiving a particular POST command through ethernet
   Configure Static IP and other configurations through menuconfig
*/

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
#include "esp_peripherals.h"
#include "board.h"
#include <stdio.h>
#include <stdlib.h>
#include "esp_system.h"
#include "esp_spi_flash.h"
#include <esp_http_server.h>
#include "esp_event.h"
#include "driver/gpio.h"
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/api.h>
#include <lwip/netdb.h>
#include "driver/gpio.h"
#include "driver/periph_ctrl.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "driver/spi_common.h"
#include "sdmmc_cmd.h"
#include "soc/soc_caps.h"
#include "esp_vfs_fat.h"
#include "esp_spiffs.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "wifi_connect.h"
#include "utils/dht/DHT.h"

static const char *TAG = "DHT_update_OTA";
float humidity, temperature =0.;

#define DHT_PIN     GPIO_NUM_4

wifi_cred_t wifi_cred = {
    .wifi_name = "clairco",
    .wifi_pass = "clair123",
    .k_timeout = 10000
};


void DHT_task(void *pvParameter)
{
    setDHTgpio(4);
    ESP_LOGI(TAG, "Starting DHT Task\n\n");

    while (1)
    {
        ESP_LOGI(TAG, "=== Reading DHT ===\n");
        int ret = readDHT();

        errorHandler(ret);

        ESP_LOGI(TAG, "Hum: %.1f Tmp: %.1f\n", getHumidity(), getTemperature());

        // -- wait at least 2 sec before reading again ------------
        // The interval of whole process must be beyond 2 seconds !!
        vTaskDelay(2000 / portTICK_RATE_MS);
    }
}

// Function to read data from DHT sensor
// Returns 0 on success, -1 on failure
int read_dht_data(float *humidity, float *temperature) {
    // Send start signal
    gpio_set_direction(DHT_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DHT_PIN, 0);
    vTaskDelay(20 / portTICK_PERIOD_MS);
    gpio_set_level(DHT_PIN, 1);
    vTaskDelay(40 / portTICK_PERIOD_MS);
    
    // Switch to input mode to read data
    gpio_set_direction(DHT_PIN, GPIO_MODE_INPUT);
    
    // Wait for sensor response
    int32_t timeout = 80;
    while (gpio_get_level(DHT_PIN) == 0) {
        if (--timeout == 0) return -1; // Timeout
        vTaskDelay(1);
    }
    timeout = 80;
    while (gpio_get_level(DHT_PIN) == 1) {
        if (--timeout == 0) return -1; // Timeout
        vTaskDelay(1);
    }
    
    // Read data
    uint8_t data[5] = {0};
    for (int i = 0; i < 5; ++i) {
        uint8_t byte = 0;
        for (int j = 7; j >= 0; --j) {
            timeout = 80;
            while (gpio_get_level(DHT_PIN) == 0) {
                if (--timeout == 0) return -1; // Timeout
                vTaskDelay(1);
            }
            int64_t start = esp_timer_get_time();
            timeout = 80;
            while (gpio_get_level(DHT_PIN) == 1) {
                if (--timeout == 0) return -1; // Timeout
                vTaskDelay(1);
            }
            int64_t end = esp_timer_get_time();
            byte <<= 1;
            if (end - start > 40 * 1000) {
                byte |= 1;
            }
        }
        data[i] = byte;
    }
    
    // Checksum validation
    if (data[4] != (data[0] + data[1] + data[2] + data[3])) {
        return -1; // Checksum error
    }
    
    // Parse data
    *humidity = ((data[0] << 8) + data[1]) / 10.0f;
    *temperature = ((data[2] << 8) + data[3]) / 10.0f;
    
    return 0; // Success
}

/* Handler gets called upon POST request*/  
esp_err_t post_handler(httpd_req_t *req)
{
 char content[100];
 size_t recv_size=sizeof(content);
 int ret=httpd_req_recv(req, content, recv_size);
 int a=0;
 if (content[0] == 'h' && content[1]==':'){
  int i=2;
  char a; 
  int digit,number=0; 
  //to get the humidity value from data received
  while ( content[i] != '|') {
        a = content[i]; 
	    if(a>='0' && a<='9') //to confirm it's a digit 
	     { 
		digit = a - '0'; 
		number = number*10 + digit; 
	     }
        i++;

    }
    humidity = number;  
    a=i; 
  if( content[a+1] == 't' && content[a+2]==':'){
    
   
    const char resp[] = "Updated tempearture and humidity values";
    ESP_LOGI(TAG,"%s\n",resp); 
    ESP_LOGI(TAG,"%d\n",number); 
    
    char b; 
    digit=0;
    number =0;
    i = 0; 
    //to get the temperature value from data received
    for(i= a+3;i<strlen(content);i++) 
       { 
	    b = content[i]; 
	    if(b>='0' && b<='9') //to confirm it's a digit 
	     { 
		digit = b - '0'; 
		number = number*10 + digit; 
	     }
       }
    temperature = number; 
    ESP_LOGI(TAG,"%.1f\n",temperature); 
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    
 }
      
    }
    
else
 {
   const char resp[] = "Please correct the input";
   //ESP_LOGI(TAG, "Please correct the input");
   ESP_LOGI(TAG,"%s\n",resp);
   httpd_resp_send(req, resp , HTTPD_RESP_USE_STRLEN); 
 }

 return ESP_OK;
  
}


httpd_uri_t uri_post = {
    .uri = "/post",
    .method = HTTP_POST,
    .handler = post_handler,
    .user_ctx = NULL};
    
/* Creates a server and registers the handler*/
httpd_handle_t setup_server(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_register_uri_handler(server, &uri_post);
        
    }

    return server;
}


void app_main(void)
{   
    gpio_set_pull_mode(DHT_PIN, GPIO_PULLUP_ONLY);
   
   /* Inititalizes wifi*/
    nvs_flash_init();
    wifi_init();
    wifi_connect_sta(&wifi_cred);

    /* Inititalizes the server */
    setup_server();
    ESP_LOGI(TAG, " running ... ...\n");

    //xTaskCreate(&DHT_task, "DHT_task", 2048, NULL, 5, NULL); 

    while (1)
    {
        if (read_dht_data(&humidity, &temperature) == 0) {
            printf("Temperature: %.1f°C, Humidity: %.1f%%\n", temperature, humidity);
        } else {
            printf("Read time out\n");
        }
        vTaskDelay(2000 / portTICK_PERIOD_MS); // Delay before next reading
        ESP_LOGI(TAG, "Hum: %.1f Tmp: %.1f\n", humidity, temperature);   //Prints humidity and temperature value either default, read or updated ones
    }    
}

