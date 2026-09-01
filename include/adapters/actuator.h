#pragma once

struct ActuatorCommand {
    uint8_t opcode;
    float arg0;
    float arg1;
};

class Actuator {
public:
    virtual ~Actuator() {}
    virtual void apply(const ActuatorCommand& command) = 0;
};
