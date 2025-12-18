#include <stdio.h>
#include "nvs_flash.h"

#include "si7021.h"
#include "wifi_mqtt.h"

static const char *TAG = "mqtt humidity sensor";

void app_main(void)
{
    ESP_LOGI(TAG, "starting...");
    
    // initializes NVS for storing Wifi settings; if there's an issue, erases & reinitializes
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    ESP_LOGI(TAG, "starting humidity sensor");
    ESP_ERROR_CHECK(si7021_init());
    
    ESP_LOGI(TAG, "starting wifi station");
    ESP_ERROR_CHECK(wifi_init_station());
    
    ESP_LOGI(TAG, "starting MQTT client");
    ESP_ERROR_CHECK(mqtt_app_start());;
    
    while (1) {
        float humidity;
        esp_err_t r = si7021_read_humidity(&humidity);
        if (r == ESP_OK) {
            ESP_LOGI(TAG, "Relative Humidity = %f", humidity);
            char humidity_str[16];
            snprintf(humidity_str, sizeof(humidity_str), "%.2f", humidity);
            if (mqtt_is_connected()) {
                int msg_id = mqtt_publish("/topic/humidity", humidity_str, 0, 0);
                if (msg_id > -1) {
                    ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
                } else {
                    ESP_LOGI(TAG, "sent publish unsuccessful; client unhealthy");
                }
            }
        } else {
            ESP_LOGE(TAG, "si7021 read failed: %d", r);
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}