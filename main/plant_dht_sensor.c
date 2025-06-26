#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "dht.h"

#define PIR_SENSOR_GPIO 17
#define DHT_GPIO        21
#define GAS_SENSOR_GPIO 14

void pir_to_serial_task(void *pvParameter) {
    gpio_set_direction(PIR_SENSOR_GPIO, GPIO_MODE_INPUT);
    while (1) {
        int motion = gpio_get_level(PIR_SENSOR_GPIO);
        printf("{\"motion\": %s}\n", motion ? "true" : "false");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void dht_to_serial_task(void *pvParameter) {
    while (1) {
        int16_t temperature = 0;
        int16_t humidity = 0;
        esp_err_t result = dht_read_data(DHT_TYPE_AM2301, DHT_GPIO, &humidity, &temperature);

        if (result == ESP_OK) {
            float temp = temperature / 10.0;
            float hum = humidity / 10.0;
            printf("{\"temperature\": %.1f, \"humidity\": %.1f}\n", temp, hum);
        }
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void gas_to_serial_task(void *pvParameter) {
    gpio_set_direction(GAS_SENSOR_GPIO, GPIO_MODE_INPUT);
    while (1) {
        int gas_level = gpio_get_level(GAS_SENSOR_GPIO);
        printf("{\"gas\": %s}\n", gas_level ? "true" : "false");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void app_main(void) {
    xTaskCreate(pir_to_serial_task, "PIR JSON Task", 2048, NULL, 5, NULL);
    xTaskCreate(dht_to_serial_task, "DHT JSON Task", 2048, NULL, 5, NULL);
    xTaskCreate(gas_to_serial_task, "Gas JSON Task", 2048, NULL, 5, NULL);
}
