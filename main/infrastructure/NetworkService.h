#pragma once

struct SensorData {
    float temp1;
    float temp2;
};

class NetworkService {
public:
    bool Connect();
    bool PublishBuffer(SensorData* buffer, int count);
    void Disconnect();
};
