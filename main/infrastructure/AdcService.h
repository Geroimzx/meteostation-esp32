#pragma once
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

class AdcService {
private:
    adc_oneshot_unit_handle_t adc_handle;
    adc_cali_handle_t cali_handle;
    bool calibrated;

public:
    AdcService();
    ~AdcService();
    void Init(adc_channel_t chan1, adc_channel_t chan2);
    int ReadMillivolts(adc_channel_t channel);
};
