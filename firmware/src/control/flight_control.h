#ifndef FLIGHT_CONTROL_H
#define FLIGHT_CONTROL_H

#include <Arduino.h>
#include "../config/config.h"
#include "pid.h"
#include "../ahrs/madgwick.h"
#include "../sensors/mpu6050.h"
#include "../io/ibus.h"
#include "../storage/flash_manager.h"

struct ControlOutput {
    float aileron;
    float elevator;
    float throttle;
    float rudder;
    uint32_t timestamp;
};

class FlightControl {
private:
    PID roll_rate_pid;
    PID pitch_rate_pid;
    PID roll_angle_pid;
    PID pitch_angle_pid;
    
    FlightMode current_mode;
    AttitudeData attitude;
    IBUSData rc_input;
    ControlOutput output;
    
    float max_roll_angle;
    float max_pitch_angle;
    float max_roll_rate;
    float max_pitch_rate;
    
public:
    FlightControl();
    
    void begin();
    void setMode(FlightMode mode);
    void update(const AttitudeData &attitude_data, const MPU6050Data &imu_data, const IBUSData &rc_data);
    ControlOutput getOutput() { return output; }
    FlightMode getMode() { return current_mode; }
    
    void loadFromFlash(FlashManager &flash);
    void saveToFlash(FlashManager &flash);
    
    void setRollRateGains(float kp, float ki, float kd);
    void setPitchRateGains(float kp, float ki, float kd);
    void setRollAngleGains(float kp, float ki, float kd);
    void setPitchAngleGains(float kp, float ki, float kd);
    
    const PID& getRollRatePID() const { return roll_rate_pid; }
    const PID& getPitchRatePID() const { return pitch_rate_pid; }
    const PID& getRollAnglePID() const { return roll_angle_pid; }
    const PID& getPitchAnglePID() const { return pitch_angle_pid; }
    
private:
    void updateManual();
    void updateFBWA(const MPU6050Data &imu_data);
    void applyLimits();
    float mapRC(float rc_value, float in_min, float in_max, float out_min, float out_max);
};

#endif
