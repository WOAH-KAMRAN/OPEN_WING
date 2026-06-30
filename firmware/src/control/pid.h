#ifndef PID_H
#define PID_H

#include <Arduino.h>

class PID {
private:
    float kp, ki, kd;
    float integral;
    float prev_error;
    float output_limit;
    bool first_update;
    
public:
    PID(float kp = 0.0f, float ki = 0.0f, float kd = 0.0f);
    
    void setGains(float kp, float ki, float kd);
    void setOutputLimit(float limit) { output_limit = limit; }
    void reset();
    float update(float error, float dt);
    float update(float setpoint, float measurement, float dt);
    
    float getKp() const { return kp; }
    float getKi() const { return ki; }
    float getKd() const { return kd; }
    float getIntegral() const { return integral; }
    float getPrevError() const { return prev_error; }
};

#endif
