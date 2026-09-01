#pragma once

// Future hardware adapters attach here. The swarm core must not include them.

struct SensorFrame {
    float values[8];
    uint8_t count;
    uint32_t timestamp_ms;
};

class Sensor {
public:
    virtual ~Sensor() {}
    virtual void sample(SensorFrame& frame) = 0;
};
