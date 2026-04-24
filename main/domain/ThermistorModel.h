#pragma once
#include <cmath>

class ThermistorModel {
private:
    const float rSeries = 10000.0f;
    const float rNominal = 10000.0f;
    const float tNominalK = 298.15f;
    const float beta = 3950.0f;
    const float vSupplyMv = 3300.0f;

public:
    float CalculateCelsius(int voltageMv) const {
        if (voltageMv <= 0 || voltageMv >= vSupplyMv) return -273.15f;
        
        float rNtc = rSeries * ((vSupplyMv / voltageMv) - 1.0f);
        float steinhart = std::log(rNtc / rNominal) / beta + (1.0f / tNominalK);
        return (1.0f / steinhart) - 273.15f;
    }
};
