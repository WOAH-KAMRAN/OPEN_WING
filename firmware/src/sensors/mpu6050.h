#ifndef MPU6050_H
#define MPU6050_H

#include <Arduino.h>
#include <Wire.h>

struct MPU6050Data {
    float accel_x, accel_y, accel_z;  // g
    float gyro_x, gyro_y, gyro_z;     // rad/s
    float temp;                       // °C
    uint32_t timestamp;
};

class MPU6050 {
private:
    uint8_t device_address;
    float accel_scale, gyro_scale;
    float gyro_bias_x, gyro_bias_y, gyro_bias_z;
    
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegister(uint8_t reg, uint8_t &value);
    bool readRegisters(uint8_t reg, uint8_t *buffer, uint8_t length);
    
public:
    MPU6050(uint8_t addr = 0x68);
    
    bool begin();
    bool configure(uint16_t sample_rate = 400);
    void calibrateGyro(uint16_t samples = 1000);
    bool readRaw(int16_t &ax, int16_t &ay, int16_t &az,
                 int16_t &gx, int16_t &gy, int16_t &gz);
    bool readScaled(MPU6050Data &data);
    void setAccelScale(uint8_t scale);
    void setGyroScale(uint8_t scale);
    float getGyroBiasX() const { return gyro_bias_x; }
    float getGyroBiasY() const { return gyro_bias_y; }
    float getGyroBiasZ() const { return gyro_bias_z; }
};

#endif
