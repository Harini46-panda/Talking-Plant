#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "dht.h"

#define PIR_SENSOR_GPIO     17
#define DHT_GPIO            21
#define GAS_SENSOR_GPIO     14
#define WIFI_SSID           "USERNAME"
#define WIFI_PASS           "PASSWORD"

static const char *TAG = "SMART_PLANT";

// ================= Wi-Fi ===================
void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Disconnected. Reconnecting...");
        esp_wifi_connect();
    }
}

void wifi_init_sta(void) {
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();

    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_wifi_start();

    ESP_LOGI(TAG, "Wi-Fi Init STA finished.");
}

// ================= Tasks ===================
void pir_sensor_task(void *pvParameter) {
    gpio_set_direction(PIR_SENSOR_GPIO, GPIO_MODE_INPUT);
    while (1) {
        int level = gpio_get_level(PIR_SENSOR_GPIO);
        if (level == 1) {
            ESP_LOGI(TAG, "Motion Detected! Hello there!");
        }
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void dht_sensor_task(void *pvParameter) {
    while (1) {
        int16_t temperature = 0, humidity = 0;
        esp_err_t result = dht_read_data(DHT_TYPE_AM2301, DHT_GPIO, &humidity, &temperature);

        if (result == ESP_OK) {
            float tempC = temperature / 10.0;
            ESP_LOGI(TAG, "Temp: %.1f C, Humidity: %.1f%%", tempC, humidity / 10.0);

            if (tempC > 30) {
                ESP_LOGI(TAG, "It's too hot today.");
            } else if (tempC < 20) {
                ESP_LOGI(TAG, "It's quite cold today.");
            } else {
                ESP_LOGI(TAG, "Such a pleasant day!");
            }
        } else {
            ESP_LOGE(TAG, "Failed to read from DHT sensor");
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void gas_sensor_task(void *pvParameter) {
    gpio_set_direction(GAS_SENSOR_GPIO, GPIO_MODE_INPUT);
    while (1) {
        int gas_level = gpio_get_level(GAS_SENSOR_GPIO);
        if (gas_level == 1) {
            ESP_LOGW(TAG, "Gas Detected! Stay safe!");
        } else {
            ESP_LOGI(TAG, "Air is clean.");
        }
        vTaskDelay(pdMS_TO_TICKS(4000));
    }
}

// ================= app_main ===================
void app_main(void) {
    ESP_LOGI(TAG, "Smart Talking Plant Starting...");
    wifi_init_sta();

    xTaskCreate(pir_sensor_task, "PIR Task", 2048, NULL, 5, NULL);
    xTaskCreate(dht_sensor_task, "DHT Task", 2048, NULL, 5, NULL);
    xTaskCreate(gas_sensor_task, "Gas Task", 2048, NULL, 5, NULL);
}
