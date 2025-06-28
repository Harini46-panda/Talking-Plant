#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "dht.h"

#define PIR_SENSOR_GPIO     17
#define DHT_GPIO            21
#define GAS_SENSOR_GPIO     14
#define DFPLAYER_UART       UART_NUM_1
#define DFPLAYER_TXD        GPIO_NUM_17

static const char *TAG = "SMART_PLANT";

// ================= DFPlayer ===================
void dfplayer_init() {
    const uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    uart_driver_install(DFPLAYER_UART, 256, 0, 0, NULL, 0);
    uart_param_config(DFPLAYER_UART, &uart_config);
    uart_set_pin(DFPLAYER_UART, DFPLAYER_TXD, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

void dfplayer_play_track(uint8_t track) {
    uint8_t cmd[10] = {0x7E, 0xFF, 0x06, 0x03, 0x00, 0x00, track, 0x00, 0x00, 0xEF};
    uint16_t checksum = 0 - (0xFF + 0x06 + 0x03 + 0x00 + 0x00 + track);
    cmd[7] = (checksum >> 8) & 0xFF;
    cmd[8] = checksum & 0xFF;
    uart_write_bytes(DFPLAYER_UART, (const char *)cmd, 10);
    ESP_LOGI(TAG, "Played track %d", track);
}

// ================= Tasks ===================
void pir_sensor_task(void *pvParameter) {
    gpio_set_direction(PIR_SENSOR_GPIO, GPIO_MODE_INPUT);
    while (1) {
        int level = gpio_get_level(PIR_SENSOR_GPIO);
        if (level == 1) {
            ESP_LOGI(TAG, "Motion Detected! Hello there!");
            dfplayer_play_track(4);  // "Someone is near me. Hello!"
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
                dfplayer_play_track(2);  // "It's too hot today."
            } else if (tempC < 20) {
                dfplayer_play_track(5);  // "It's quite cold today."
            } else {
                dfplayer_play_track(1);  // "Such a pleasant day!"
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
            dfplayer_play_track(3);  // "Gas detected. Please stay safe."
        } else {
            ESP_LOGI(TAG, "Air is clean.");
        }
        vTaskDelay(pdMS_TO_TICKS(4000));
    }
}
void app_main(void) {
    ESP_LOGI(TAG, "Smart Talking Plant Starting...");
    dfplayer_init();
    xTaskCreate(pir_sensor_task, "PIR Task", 2048, NULL, 5, NULL);
    xTaskCreate(dht_sensor_task, "DHT Task", 2048, NULL, 5, NULL);
    xTaskCreate(gas_sensor_task, "Gas Task", 2048, NULL, 5, NULL);
}
