#include "madgwick.h"
#include <math.h>

MadgwickAHRS::MadgwickAHRS(float beta_param) : beta(beta_param),
                                                 q0(1.0f), q1(0.0f), q2(0.0f), q3(0.0f),
                                                 last_update_us(0) {
}

void MadgwickAHRS::reset() {
    q0 = 1.0f;
    q1 = 0.0f;
    q2 = 0.0f;
    q3 = 0.0f;
    last_update_us = micros();
}

float MadgwickAHRS::invSqrt(float x) {
    float halfx = 0.5f * x;
    float y = x;
    long i = *(long*)&y;
    i = 0x5f3759df - (i >> 1);
    y = *(float*)&i;
    y = y * (1.5f - (halfx * y * y));
    return y;
}

void MadgwickAHRS::update(float gx, float gy, float gz,
                         float ax, float ay, float az,
                         float mx, float my, float mz,
                         uint32_t timestamp_us) {
    float dt;
    
    if (last_update_us == 0) {
        dt = 0.0f;
    } else {
        dt = (float)(timestamp_us - last_update_us) * 1e-6f;
    }
    last_update_us = timestamp_us;
    
    if (dt <= 0.0f || dt > 0.1f) {
        return;  // Invalid time step
    }
    
    // Normalize accelerometer measurement
    float norm = invSqrt(ax * ax + ay * ay + az * az);
    ax *= norm;
    ay *= norm;
    az *= norm;
    
    // Normalize magnetometer measurement
    norm = invSqrt(mx * mx + my * my + mz * mz);
    mx *= norm;
    my *= norm;
    mz *= norm;
    
    // Reference direction of Earth's magnetic field
    float hx = mx * (q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3) +
               my * (2.0f * q1 * q2 - 2.0f * q0 * q3) +
               mz * (2.0f * q1 * q3 + 2.0f * q0 * q2);
    float hy = mx * (2.0f * q1 * q2 + 2.0f * q0 * q3) +
               my * (q0 * q0 - q1 * q1 + q2 * q2 - q3 * q3) +
               mz * (2.0f * q2 * q3 - 2.0f * q0 * q1);
    float bx = sqrtf((hx * hx) + (hy * hy));
    float bz = mx * (2.0f * q1 * q3 - 2.0f * q0 * q2) +
               my * (2.0f * q2 * q3 + 2.0f * q0 * q1) +
               mz * (q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3);
    
    // Gradient descent algorithm corrective step
    float s0 = 2.0f * q1 * (2.0f * q2 * q3 - 2.0f * q0 * q1 - ax) +
               2.0f * q2 * (2.0f * q0 * q2 + 2.0f * q1 * q3 - ay) +
               2.0f * q3 * (1.0f - 2.0f * q1 * q1 - 2.0f * q2 * q2 - az) -
               2.0f * bx * q3 * (bx * (1.0f - 2.0f * q2 * q2 - 2.0f * q3 * q3) +
               hy * (2.0f * q1 * q3 - 2.0f * q0 * q2) +
               bz * (2.0f * q0 * q1 + 2.0f * q2 * q3)) +
               2.0f * bz * q2 * (bx * (2.0f * q1 * q3 - 2.0f * q0 * q2) +
               hy * (2.0f * q0 * q1 + 2.0f * q2 * q3) +
               bz * (1.0f - 2.0f * q1 * q1 - 2.0f * q2 * q2));
    
    float s1 = 2.0f * q0 * (2.0f * q2 * q3 - 2.0f * q0 * q1 - ax) +
               2.0f * q3 * (2.0f * q0 * q2 + 2.0f * q1 * q3 - ay) +
               2.0f * q2 * (1.0f - 2.0f * q1 * q1 - 2.0f * q2 * q2 - az) +
               2.0f * bx * q2 * (bx * (1.0f - 2.0f * q2 * q2 - 2.0f * q3 * q3) +
               hy * (2.0f * q1 * q3 - 2.0f * q0 * q2) +
               bz * (2.0f * q0 * q1 + 2.0f * q2 * q3)) -
               2.0f * bz * q1 * (bx * (2.0f * q1 * q3 - 2.0f * q0 * q2) +
               hy * (2.0f * q0 * q1 + 2.0f * q2 * q3) +
               bz * (1.0f - 2.0f * q1 * q1 - 2.0f * q2 * q2));
    
    float s2 = 2.0f * q0 * (2.0f * q0 * q2 + 2.0f * q1 * q3 - ay) +
               2.0f * q1 * (2.0f * q2 * q3 - 2.0f * q0 * q1 - ax) +
               2.0f * q3 * (1.0f - 2.0f * q1 * q1 - 2.0f * q2 * q2 - az) +
               2.0f * bx * q1 * (bx * (1.0f - 2.0f * q2 * q2 - 2.0f * q3 * q3) +
               hy * (2.0f * q1 * q3 - 2.0f * q0 * q2) +
               bz * (2.0f * q0 * q1 + 2.0f * q2 * q3)) +
               2.0f * bz * q0 * (bx * (2.0f * q1 * q3 - 2.0f * q0 * q2) +
               hy * (2.0f * q0 * q1 + 2.0f * q2 * q3) +
               bz * (1.0f - 2.0f * q1 * q1 - 2.0f * q2 * q2));
    
    float s3 = 2.0f * q0 * (1.0f - 2.0f * q1 * q1 - 2.0f * q2 * q2 - az) +
               2.0f * q1 * (2.0f * q0 * q2 + 2.0f * q1 * q3 - ay) +
               2.0f * q2 * (2.0f * q2 * q3 - 2.0f * q0 * q1 - ax) +
               2.0f * bx * q0 * (bx * (1.0f - 2.0f * q2 * q2 - 2.0f * q3 * q3) +
               hy * (2.0f * q1 * q3 - 2.0f * q0 * q2) +
               bz * (2.0f * q0 * q1 + 2.0f * q2 * q3)) -
               2.0f * bz * q3 * (bx * (2.0f * q1 * q3 - 2.0f * q0 * q2) +
               hy * (2.0f * q0 * q1 + 2.0f * q2 * q3) +
               bz * (1.0f - 2.0f * q1 * q1 - 2.0f * q2 * q2));
    
    // Normalize gradient step
    norm = invSqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3);
    s0 *= norm;
    s1 *= norm;
    s2 *= norm;
    s3 *= norm;
    
    // Compute rate of change of quaternion
    float qDot0 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz) - beta * s0;
    float qDot1 = 0.5f * (q0 * gx + q2 * gz - q3 * gy) - beta * s1;
    float qDot2 = 0.5f * (q0 * gy - q1 * gz + q3 * gx) - beta * s2;
    float qDot3 = 0.5f * (q0 * gz + q1 * gy - q2 * gx) - beta * s3;
    
    // Integrate to yield quaternion
    q0 += qDot0 * dt;
    q1 += qDot1 * dt;
    q2 += qDot2 * dt;
    q3 += qDot3 * dt;
    
    // Normalize quaternion
    norm = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    q0 *= norm;
    q1 *= norm;
    q2 *= norm;
    q3 *= norm;
}

void MadgwickAHRS::updateIMU(float gx, float gy, float gz,
                            float ax, float ay, float az,
                            uint32_t timestamp_us) {
    float dt;
    
    if (last_update_us == 0) {
        dt = 0.0f;
    } else {
        dt = (float)(timestamp_us - last_update_us) * 1e-6f;
    }
    last_update_us = timestamp_us;
    
    if (dt <= 0.0f || dt > 0.1f) {
        return;  // Invalid time step
    }
    
    // Normalize accelerometer measurement
    float norm = invSqrt(ax * ax + ay * ay + az * az);
    ax *= norm;
    ay *= norm;
    az *= norm;
    
    // Gradient descent algorithm corrective step
    float s0 = 2.0f * q1 * (2.0f * q2 * q3 - 2.0f * q0 * q1 - ax) +
               2.0f * q2 * (2.0f * q0 * q2 + 2.0f * q1 * q3 - ay) +
               2.0f * q3 * (1.0f - 2.0f * q1 * q1 - 2.0f * q2 * q2 - az);
    
    float s1 = 2.0f * q0 * (2.0f * q2 * q3 - 2.0f * q0 * q1 - ax) +
               2.0f * q3 * (2.0f * q0 * q2 + 2.0f * q1 * q3 - ay) +
               2.0f * q2 * (1.0f - 2.0f * q1 * q1 - 2.0f * q2 * q2 - az);
    
    float s2 = 2.0f * q0 * (2.0f * q0 * q2 + 2.0f * q1 * q3 - ay) +
               2.0f * q1 * (2.0f * q2 * q3 - 2.0f * q0 * q1 - ax) +
               2.0f * q3 * (1.0f - 2.0f * q1 * q1 - 2.0f * q2 * q2 - az);
    
    float s3 = 2.0f * q0 * (1.0f - 2.0f * q1 * q1 - 2.0f * q2 * q2 - az) +
               2.0f * q1 * (2.0f * q0 * q2 + 2.0f * q1 * q3 - ay) +
               2.0f * q2 * (2.0f * q2 * q3 - 2.0f * q0 * q1 - ax);
    
    // Normalize gradient step
    norm = invSqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3);
    s0 *= norm;
    s1 *= norm;
    s2 *= norm;
    s3 *= norm;
    
    // Compute rate of change of quaternion
    float qDot0 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz) - beta * s0;
    float qDot1 = 0.5f * (q0 * gx + q2 * gz - q3 * gy) - beta * s1;
    float qDot2 = 0.5f * (q0 * gy - q1 * gz + q3 * gx) - beta * s2;
    float qDot3 = 0.5f * (q0 * gz + q1 * gy - q2 * gx) - beta * s3;
    
    // Integrate to yield quaternion
    q0 += qDot0 * dt;
    q1 += qDot1 * dt;
    q2 += qDot2 * dt;
    q3 += qDot3 * dt;
    
    // Normalize quaternion
    norm = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    q0 *= norm;
    q1 *= norm;
    q2 *= norm;
    q3 *= norm;
}

void MadgwickAHRS::getQuaternion(float &q0_out, float &q1_out, float &q2_out, float &q3_out) {
    q0_out = q0;
    q1_out = q1;
    q2_out = q2;
    q3_out = q3;
}

void MadgwickAHRS::quaternionToEuler(float q0, float q1, float q2, float q3,
                                     float &roll, float &pitch, float &yaw) {
    // Roll (x-axis rotation)
    roll = atan2f(2.0f * (q0 * q1 + q2 * q3), 1.0f - 2.0f * (q1 * q1 + q2 * q2));
    
    // Pitch (y-axis rotation)
    pitch = asinf(2.0f * (q0 * q2 - q3 * q1));
    
    // Yaw (z-axis rotation)
    yaw = atan2f(2.0f * (q0 * q3 + q1 * q2), 1.0f - 2.0f * (q2 * q2 + q3 * q3));
}

void MadgwickAHRS::getEulerAngles(float &roll, float &pitch, float &yaw) {
    quaternionToEuler(q0, q1, q2, q3, roll, pitch, yaw);
}

AttitudeData MadgwickAHRS::getAttitude() {
    AttitudeData data;
    data.q0 = q0;
    data.q1 = q1;
    data.q2 = q2;
    data.q3 = q3;
    quaternionToEuler(q0, q1, q2, q3, data.roll, data.pitch, data.yaw);
    data.timestamp = last_update_us;
    return data;
}
