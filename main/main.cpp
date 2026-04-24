#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "driver/gpio.h"

#include "AdcService.h"
#include "ThermistorModel.h"
#include "NetworkService.h"
#include "config.h"

static const char* TAG = "METEO";

constexpr int MAX_BUFFER_SIZE = 24;

RTC_DATA_ATTR SensorData dataBuffer[MAX_BUFFER_SIZE];
RTC_DATA_ATTR int bufferCount = 0;

void blink_led() {
    gpio_reset_pin(STATUS_LED_GPIO);
    gpio_set_direction(STATUS_LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(STATUS_LED_GPIO, 1);
    vTaskDelay(pdMS_TO_TICKS(200));
    gpio_set_level(STATUS_LED_GPIO, 0);
}

extern "C" void app_main() {
    AdcService adcService;
    ThermistorModel ntcModel;
    NetworkService networkService;

    // Ініціалізація АЦП
    adcService.Init(ADC_CHANNEL_6, ADC_CHANNEL_7);

    // Зчитування датчиків (SOLID: логіка ізольована)
    gpio_set_direction(NTC_POWER_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(NTC_POWER_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(15)); // Час на стабілізацію

    float t1 = ntcModel.CalculateCelsius(adcService.ReadMillivolts(ADC_CHANNEL_6));
    float t2 = ntcModel.CalculateCelsius(adcService.ReadMillivolts(ADC_CHANNEL_7));

    gpio_set_level(NTC_POWER_PIN, 0);

    // Логіка буферизації
    if (bufferCount < MAX_BUFFER_SIZE) {
        dataBuffer[bufferCount].temp1 = t1;
        dataBuffer[bufferCount].temp2 = t2;
        bufferCount++;
    }

    // Відправка: наприклад, кожні 3 цикли (15 хв) або якщо буфер заповнений
    // Щоб відправляти щоразу - змініть bufferCount >= 1
    if (bufferCount >= 1) {
        if (networkService.Connect()) {
            if (networkService.PublishBuffer(dataBuffer, bufferCount)) {
                bufferCount = 0;
                blink_led(); // Візуальне підтвердження відправки
            }
            networkService.Disconnect();
        }
    }

    // Налаштування таймера сну
    esp_sleep_enable_timer_wakeup(SLEEP_TIME_MIN * 60 * 1000000ULL);

    ESP_LOGI(TAG, "Going to sleep for %d minutes...", SLEEP_TIME_MIN);
    esp_deep_sleep_start();
}
