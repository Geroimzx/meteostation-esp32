#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "driver/gpio.h"

#include "domain/ThermistorModel.h"
#include "infrastructure/AdcService.h"
#include "infrastructure/NetworkService.h"

constexpr gpio_num_t NTC_POWER_PIN = GPIO_NUM_4;
constexpr gpio_num_t BUTTON_PIN = GPIO_NUM_33;
constexpr adc_channel_t NTC1_CHANNEL = ADC_CHANNEL_6; 
constexpr adc_channel_t NTC2_CHANNEL = ADC_CHANNEL_7; 

constexpr uint64_t SLEEP_TIME_US = 5 * 60 * 1000000ULL;
constexpr int MAX_BUFFER_SIZE = 24;

RTC_DATA_ATTR SensorData dataBuffer[MAX_BUFFER_SIZE];
RTC_DATA_ATTR int bufferCount = 0;

extern "C" void app_main() {
    AdcService adcService;
    ThermistorModel ntcModel;
    NetworkService networkService;

    adcService.Init(NTC1_CHANNEL, NTC2_CHANNEL);

    gpio_set_direction(NTC_POWER_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(NTC_POWER_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(5));

    int mv1 = adcService.ReadMillivolts(NTC1_CHANNEL);
    int mv2 = adcService.ReadMillivolts(NTC2_CHANNEL);

    gpio_set_level(NTC_POWER_PIN, 0);

    if (bufferCount < MAX_BUFFER_SIZE) {
        dataBuffer[bufferCount] = {
            ntcModel.CalculateCelsius(mv1),
            ntcModel.CalculateCelsius(mv2)
        };
        bufferCount++;
    }

    if (networkService.Connect()) {
        if (networkService.PublishBuffer(dataBuffer, bufferCount)) {
            bufferCount = 0;
        }
        networkService.Disconnect();
    }

    esp_sleep_enable_timer_wakeup(SLEEP_TIME_US);
    esp_sleep_enable_ext0_wakeup(BUTTON_PIN, 0);
    esp_deep_sleep_start();
}
