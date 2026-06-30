#ifndef MADGWICK_H
#define MADGWICK_H

#include <Arduino.h>
#include "../config/config.h"

struct AttitudeData {
    float q0, q1, q2, q3;  // Quaternion
    float roll, pitch, yaw;  // Euler angles (rad)
    uint32_t timestamp;
};

class MadgwickAHRS {
private:
    float beta;
    float q0, q1, q2, q3;
    uint32_t last_update_us;
    
    float invSqrt(float x);
    void quaternionToEuler(float q0, float q1, float q2, float q3,
                          float &roll, float &pitch, float &yaw);
    
public:
    MadgwickAHRS(float beta = 0.1f);
    
    void reset();
    void update(float gx, float gy, float gz,
                float ax, float ay, float az,
                float mx, float my, float mz,
                uint32_t timestamp_us);
    void updateIMU(float gx, float gy, float gz,
                   float ax, float ay, float az,
                   uint32_t timestamp_us);
    
    void getQuaternion(float &q0, float &q1, float &q2, float &q3);
    void getEulerAngles(float &roll, float &pitch, float &yaw);
    AttitudeData getAttitude();
    void setBeta(float beta_param) { beta = beta_param; }
};

#endif
