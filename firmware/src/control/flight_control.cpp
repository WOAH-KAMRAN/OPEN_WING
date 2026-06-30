#include "flight_control.h"
#include <math.h>

#define ROLL_RATE_KP  0.15f
#define ROLL_RATE_KI  0.0f
#define ROLL_RATE_KD  0.005f

#define PITCH_RATE_KP 0.15f
#define PITCH_RATE_KI 0.0f
#define PITCH_RATE_KD 0.005f

#define ROLL_ANGLE_KP 4.5f
#define ROLL_ANGLE_KI 0.0f
#define ROLL_ANGLE_KD 0.0f

#define PITCH_ANGLE_KP 5.0f
#define PITCH_ANGLE_KI 0.0f
#define PITCH_ANGLE_KD 0.0f

#define MAX_ROLL_ANGLE  0.5236f
#define MAX_PITCH_ANGLE 0.5236f
#define MAX_ROLL_RATE   3.1416f
#define MAX_PITCH_RATE  3.1416f

FlightControl::FlightControl() : current_mode(MANUAL),
                                  max_roll_angle(MAX_ROLL_ANGLE),
                                  max_pitch_angle(MAX_PITCH_ANGLE),
                                  max_roll_rate(MAX_ROLL_RATE),
                                  max_pitch_rate(MAX_PITCH_RATE) {
}

void FlightControl::begin() {
    roll_rate_pid.setGains(ROLL_RATE_KP, ROLL_RATE_KI, ROLL_RATE_KD);
    pitch_rate_pid.setGains(PITCH_RATE_KP, PITCH_RATE_KI, PITCH_RATE_KD);
    
    roll_angle_pid.setGains(ROLL_ANGLE_KP, ROLL_ANGLE_KI, ROLL_ANGLE_KD);
    pitch_angle_pid.setGains(PITCH_ANGLE_KP, PITCH_ANGLE_KI, PITCH_ANGLE_KD);
    
    roll_rate_pid.setOutputLimit(max_roll_rate);
    pitch_rate_pid.setOutputLimit(max_pitch_rate);
    
    roll_angle_pid.setOutputLimit(max_roll_rate);
    pitch_angle_pid.setOutputLimit(max_pitch_rate);
    
    roll_rate_pid.reset();
    pitch_rate_pid.reset();
    roll_angle_pid.reset();
    pitch_angle_pid.reset();
    
    output.aileron = 0.0f;
    output.elevator = 0.0f;
    output.throttle = 0.0f;
    output.timestamp = 0;
}

void FlightControl::loadFromFlash(FlashManager &flash) {
    PersistentConfig &cfg = flash.getConfig();
    setRollRateGains(cfg.roll_rate_kp, cfg.roll_rate_ki, cfg.roll_rate_kd);
    setPitchRateGains(cfg.pitch_rate_kp, cfg.pitch_rate_ki, cfg.pitch_rate_kd);
    setRollAngleGains(cfg.roll_angle_kp, cfg.roll_angle_ki, cfg.roll_angle_kd);
    setPitchAngleGains(cfg.pitch_angle_kp, cfg.pitch_angle_ki, cfg.pitch_angle_kd);
}

void FlightControl::saveToFlash(FlashManager &flash) {
    PersistentConfig &cfg = flash.getConfig();
    cfg.roll_rate_kp = roll_rate_pid.getKp();
    cfg.roll_rate_ki = roll_rate_pid.getKi();
    cfg.roll_rate_kd = roll_rate_pid.getKd();
    cfg.pitch_rate_kp = pitch_rate_pid.getKp();
    cfg.pitch_rate_ki = pitch_rate_pid.getKi();
    cfg.pitch_rate_kd = pitch_rate_pid.getKd();
    cfg.roll_angle_kp = roll_angle_pid.getKp();
    cfg.roll_angle_ki = roll_angle_pid.getKi();
    cfg.roll_angle_kd = roll_angle_pid.getKd();
    cfg.pitch_angle_kp = pitch_angle_pid.getKp();
    cfg.pitch_angle_ki = pitch_angle_pid.getKi();
    cfg.pitch_angle_kd = pitch_angle_pid.getKd();
    flash.save();
}

void FlightControl::setRollRateGains(float kp, float ki, float kd) {
    roll_rate_pid.setGains(kp, ki, kd);
}

void FlightControl::setPitchRateGains(float kp, float ki, float kd) {
    pitch_rate_pid.setGains(kp, ki, kd);
}

void FlightControl::setRollAngleGains(float kp, float ki, float kd) {
    roll_angle_pid.setGains(kp, ki, kd);
}

void FlightControl::setPitchAngleGains(float kp, float ki, float kd) {
    pitch_angle_pid.setGains(kp, ki, kd);
}

void FlightControl::setMode(FlightMode mode) {
    if (mode != current_mode) {
        roll_rate_pid.reset();
        pitch_rate_pid.reset();
        roll_angle_pid.reset();
        pitch_angle_pid.reset();
        
        current_mode = mode;
    }
}

float FlightControl::mapRC(float rc_value, float in_min, float in_max, float out_min, float out_max) {
    return (rc_value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void FlightControl::update(const AttitudeData &attitude_data, const MPU6050Data &imu_data, const IBUSData &rc_data) {
    attitude = attitude_data;
    rc_input = rc_data;
    
    switch (current_mode) {
        case MANUAL:
            updateManual();
            break;
        case FBWA:
        case STABILIZE:
        case RTL:
        case AUTO:
        case LOITER:
            updateFBWA(imu_data);
            break;
    }
    
    applyLimits();
    output.timestamp = micros();
}

void FlightControl::updateManual() {
    output.aileron = mapRC(rc_input.channels[0], 1000.0f, 2000.0f, -1.0f, 1.0f);
    output.elevator = mapRC(rc_input.channels[1], 1000.0f, 2000.0f, -1.0f, 1.0f);
    output.throttle = mapRC(rc_input.channels[2], 1000.0f, 2000.0f, 0.0f, 1.0f);
}

void FlightControl::updateFBWA(const MPU6050Data &imu_data) {
    float dt = 1.0f / HIGH_FREQ_RATE_HZ;
    
    float rc_roll = mapRC(rc_input.channels[0], 1000.0f, 2000.0f, -1.0f, 1.0f);
    float rc_pitch = mapRC(rc_input.channels[1], 1000.0f, 2000.0f, -1.0f, 1.0f);
    float rc_throttle = mapRC(rc_input.channels[2], 1000.0f, 2000.0f, 0.0f, 1.0f);
    
    float desired_roll = rc_roll * max_roll_angle;
    float desired_pitch = rc_pitch * max_pitch_angle;
    
    float desired_roll_rate = roll_angle_pid.update(desired_roll, attitude.roll, dt);
    float desired_pitch_rate = pitch_angle_pid.update(desired_pitch, attitude.pitch, dt);
    
    float measured_roll_rate = imu_data.gyro_x;
    float measured_pitch_rate = imu_data.gyro_y;
    
    output.aileron = roll_rate_pid.update(desired_roll_rate, measured_roll_rate, dt);
    output.elevator = pitch_rate_pid.update(desired_pitch_rate, measured_pitch_rate, dt);
    output.throttle = rc_throttle;
}

void FlightControl::applyLimits() {
    if (output.aileron > 1.0f) output.aileron = 1.0f;
    if (output.aileron < -1.0f) output.aileron = -1.0f;
    
    if (output.elevator > 1.0f) output.elevator = 1.0f;
    if (output.elevator < -1.0f) output.elevator = -1.0f;
    
    if (output.throttle > 1.0f) output.throttle = 1.0f;
    if (output.throttle < 0.0f) output.throttle = 0.0f;
}
