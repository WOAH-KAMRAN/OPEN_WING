#include "pid.h"

PID::PID(float kp_param, float ki_param, float kd_param) 
    : kp(kp_param), ki(ki_param), kd(kd_param),
      integral(0.0f), prev_error(0.0f), output_limit(1.0f), first_update(true) {
}

void PID::setGains(float kp_param, float ki_param, float kd_param) {
    kp = kp_param;
    ki = ki_param;
    kd = kd_param;
}

void PID::reset() {
    integral = 0.0f;
    prev_error = 0.0f;
    first_update = true;
}

float PID::update(float error, float dt) {
    if (dt <= 0.0f) {
        return 0.0f;
    }
    
    // Proportional term
    float p_term = kp * error;
    
    // Integral term with anti-windup
    integral += error * dt;
    
    // Anti-windup: clamp integral
    if (ki > 0.0f) {
        float max_integral = output_limit / ki;
        if (integral > max_integral) integral = max_integral;
        if (integral < -max_integral) integral = -max_integral;
    }
    
    float i_term = ki * integral;
    
    // Derivative term
    float d_term = 0.0f;
    if (!first_update) {
        d_term = kd * (error - prev_error) / dt;
    }
    prev_error = error;
    first_update = false;
    
    // Calculate output
    float output = p_term + i_term + d_term;
    
    // Clamp output
    if (output > output_limit) output = output_limit;
    if (output < -output_limit) output = -output_limit;
    
    return output;
}

float PID::update(float setpoint, float measurement, float dt) {
    float error = setpoint - measurement;
    return update(error, dt);
}
