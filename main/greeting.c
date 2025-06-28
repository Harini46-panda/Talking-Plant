#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "dht.h"

// Define sensor GPIO pins
#define PIR_SENSOR_GPIO     17
#define DHT_GPIO            21
#define GAS_SENSOR_GPIO     14

static const char *TAG = "SMART_PLANT";

// ================= PIR Task ===================
void pir_sensor_task(void *pvParameter) {
    gpio_set_direction(PIR_SENSOR_GPIO, GPIO_MODE_INPUT);
    while (1) {
        int level = gpio_get_level(PIR_SENSOR_GPIO);
        if (level == 1) {
            ESP_LOGI(TAG, "Motion Detected! Somebody here? I’m so glad to see you.");
        } else {
            ESP_LOGI(TAG, " No Motion. I guess I'm alone again.");
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

// ================= DHT Task ===================
void dht_sensor_task(void *pvParameter) {
    while (1) {
        int16_t temperature = 0;
        int16_t humidity = 0;
        esp_err_t result = dht_read_data(DHT_TYPE_AM2301, DHT_GPIO, &humidity, &temperature);

        if (result == ESP_OK) {
            float temp = temperature / 10.0;
            float hum = humidity / 10.0;

            ESP_LOGI(TAG, "Temp: %.1f°C, 💧 Humidity: %.1f%%", temp, hum);

            if (temp < 20.0) {
                ESP_LOGI(TAG, "It's quite chilly today. Stay warm!");
            } else if (temp >= 20.0 && temp <= 30.0) {
                ESP_LOGI(TAG, " Such a pleasant weather. I'm feeling great!");
            } else {
                ESP_LOGI(TAG, " It's really hot! I hope you're staying hydrated.");
            }

        } else {
            ESP_LOGE(TAG, " Failed to read DHT22 sensor.");
        }
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

// ================= Gas Task ===================
void gas_sensor_task(void *pvParameter) {
    gpio_set_direction(GAS_SENSOR_GPIO, GPIO_MODE_INPUT);

    while (1) {
        int gas_level = gpio_get_level(GAS_SENSOR_GPIO);
        if (gas_level == 1) {
            ESP_LOGW(TAG, " Gas Detected! Be careful!");
        } else {
            ESP_LOGI(TAG, " Air is fresh and clean.");
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

// ================= App Main ===================
void app_main(void) {
    ESP_LOGI(TAG, " Hello! I am your Smart Talking Plant.");
    ESP_LOGI(TAG, " Good morning! It's a new day to grow!");
    ESP_LOGI(TAG, " Remember to check if I need water today!");
    ESP_LOGI(TAG, " Ready to sense and speak! Let's grow together!");

    xTaskCreate(pir_sensor_task, "PIR Task", 2048, NULL, 5, NULL);
    xTaskCreate(dht_sensor_task, "DHT22 Task", 2048, NULL, 5, NULL);
    xTaskCreate(gas_sensor_task, "Gas Task", 2048, NULL, 5, NULL);
}
