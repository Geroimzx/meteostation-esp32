#include "AdcService.h"
#include "esp_log.h"

static const char* TAG = "ADC_SERVICE";

AdcService::AdcService() : adc_handle(nullptr), cali_handle(nullptr), calibrated(false) {}

void AdcService::Init(adc_channel_t chan1, adc_channel_t chan2) {
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
        .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, chan1, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, chan2, &config));

    // Використовуємо line_fitting для класичного ESP32
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .default_vref = 1100, // Стандартна опорна напруга для ESP32
    };
    
    esp_err_t ret = adc_cali_create_scheme_line_fitting(&cali_config, &cali_handle);
    if (ret == ESP_OK) {
        calibrated = true;
        ESP_LOGI(TAG, "ADC calibration successful");
    } else {
        ESP_LOGW(TAG, "ADC calibration failed, using raw values");
    }
}

int AdcService::ReadMillivolts(adc_channel_t channel) {
    int raw_value = 0;
    int voltage_mv = 0;
    
    ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, channel, &raw_value));
    
    if (calibrated) {
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(cali_handle, raw_value, &voltage_mv));
    } else {
        voltage_mv = raw_value; 
    }
    
    return voltage_mv;
}

AdcService::~AdcService() {
    if (calibrated) {
        // Також змінюємо функцію видалення схеми на line_fitting
        adc_cali_delete_scheme_line_fitting(cali_handle);
    }
    if (adc_handle != nullptr) {
        adc_oneshot_del_unit(adc_handle);
    }
}
