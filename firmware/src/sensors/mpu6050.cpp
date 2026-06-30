#include "mpu6050.h"
#include "../config/config.h"
#include <math.h>

// MPU6050 register addresses
#define MPU6050_ADDR_AD0_LOW     0x68
#define MPU6050_ADDR_AD0_HIGH    0x69
#define MPU6050_REG_PWR_MGMT_1   0x6B
#define MPU6050_REG_CONFIG       0x1A
#define MPU6050_REG_GYRO_CONFIG  0x1B
#define MPU6050_REG_ACCEL_CONFIG 0x1C
#define MPU6050_REG_ACCEL_XOUT_H 0x3B
#define MPU6050_REG_TEMP_OUT_H   0x41
#define MPU6050_REG_GYRO_XOUT_H  0x43
#define MPU6050_REG_SMPLRT_DIV   0x19

// Scale factors
#define ACCEL_SCALE_2G   16384.0f
#define ACCEL_SCALE_4G   8192.0f
#define ACCEL_SCALE_8G   4096.0f
#define ACCEL_SCALE_16G  2048.0f

#define GYRO_SCALE_250DPS  131.0f
#define GYRO_SCALE_500DPS  65.5f
#define GYRO_SCALE_1000DPS 32.8f
#define GYRO_SCALE_2000DPS 16.4f

MPU6050::MPU6050(uint8_t addr) : device_address(addr), 
                                   accel_scale(ACCEL_SCALE_2G),
                                   gyro_scale(GYRO_SCALE_250DPS),
                                   gyro_bias_x(0), gyro_bias_y(0), gyro_bias_z(0) {
}

bool MPU6050::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(device_address);
    Wire.write(reg);
    Wire.write(value);
    return (Wire.endTransmission() == 0);
}

bool MPU6050::readRegister(uint8_t reg, uint8_t &value) {
    Wire.beginTransmission(device_address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) return false;
    
    Wire.requestFrom(device_address, (uint8_t)1);
    if (Wire.available() < 1) return false;
    
    value = Wire.read();
    return true;
}

bool MPU6050::readRegisters(uint8_t reg, uint8_t *buffer, uint8_t length) {
    Wire.beginTransmission(device_address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) return false;
    
    Wire.requestFrom(device_address, length);
    for (uint8_t i = 0; i < length; i++) {
        if (Wire.available()) {
            buffer[i] = Wire.read();
        } else {
            return false;
        }
    }
    return true;
}

bool MPU6050::begin() {
    Wire.begin();
    
    // Wake up MPU6050
    if (!writeRegister(MPU6050_REG_PWR_MGMT_1, 0x00)) {
        return false;
    }
    
    delay(100);
    
    // Verify device ID
    uint8_t who_am_i;
    if (!readRegister(0x75, who_am_i)) return false;
    if (who_am_i != 0x68) return false;
    
    return true;
}

bool MPU6050::configure(uint16_t sample_rate) {
    // Set sample rate divider
    // Sample rate = gyro_rate / (1 + divider)
    // Default gyro rate is 8kHz
    uint8_t divider = 8000 / sample_rate - 1;
    if (divider > 255) divider = 255;
    if (!writeRegister(MPU6050_REG_SMPLRT_DIV, divider)) return false;
    
    // Configure digital low-pass filter (DLPF)
    // 0 = 260Hz, 1 = 184Hz, 2 = 94Hz, 3 = 44Hz, 4 = 21Hz, 5 = 10Hz, 6 = 5Hz
    if (!writeRegister(MPU6050_REG_CONFIG, 0x03)) return false;
    
    // Set gyro scale to ±250 deg/s
    if (!writeRegister(MPU6050_REG_GYRO_CONFIG, 0x00)) return false;
    
    // Set accel scale to ±2g
    if (!writeRegister(MPU6050_REG_ACCEL_CONFIG, 0x00)) return false;
    
    delay(10);
    return true;
}

void MPU6050::setAccelScale(uint8_t scale) {
    // scale: 0=±2g, 1=±4g, 2=±8g, 3=±16g
    if (!writeRegister(MPU6050_REG_ACCEL_CONFIG, scale << 3)) return;
    
    switch (scale) {
        case 0: accel_scale = ACCEL_SCALE_2G; break;
        case 1: accel_scale = ACCEL_SCALE_4G; break;
        case 2: accel_scale = ACCEL_SCALE_8G; break;
        case 3: accel_scale = ACCEL_SCALE_16G; break;
    }
}

void MPU6050::setGyroScale(uint8_t scale) {
    // scale: 0=±250dps, 1=±500dps, 2=±1000dps, 3=±2000dps
    if (!writeRegister(MPU6050_REG_GYRO_CONFIG, scale << 3)) return;
    
    switch (scale) {
        case 0: gyro_scale = GYRO_SCALE_250DPS; break;
        case 1: gyro_scale = GYRO_SCALE_500DPS; break;
        case 2: gyro_scale = GYRO_SCALE_1000DPS; break;
        case 3: gyro_scale = GYRO_SCALE_2000DPS; break;
    }
}

void MPU6050::calibrateGyro(uint16_t samples) {
    int32_t sum_x = 0, sum_y = 0, sum_z = 0;
    
    Serial.println("Calibrating gyro... Keep sensor still!");
    
    for (uint16_t i = 0; i < samples; i++) {
        int16_t ax, ay, az, gx, gy, gz;
        if (readRaw(ax, ay, az, gx, gy, gz)) {
            sum_x += gx;
            sum_y += gy;
            sum_z += gz;
        }
        delay(1);
    }
    
    gyro_bias_x = (float)sum_x / samples;
    gyro_bias_y = (float)sum_y / samples;
    gyro_bias_z = (float)sum_z / samples;
    
    Serial.printf("Gyro bias: %.2f, %.2f, %.2f\n", gyro_bias_x, gyro_bias_y, gyro_bias_z);
}

bool MPU6050::readRaw(int16_t &ax, int16_t &ay, int16_t &az,
                      int16_t &gx, int16_t &gy, int16_t &gz) {
    uint8_t buffer[14];
    
    if (!readRegisters(MPU6050_REG_ACCEL_XOUT_H, buffer, 14)) {
        return false;
    }
    
    ax = (int16_t)((buffer[0] << 8) | buffer[1]);
    ay = (int16_t)((buffer[2] << 8) | buffer[3]);
    az = (int16_t)((buffer[4] << 8) | buffer[5]);
    
    int16_t temp_raw = (int16_t)((buffer[6] << 8) | buffer[7]);
    
    gx = (int16_t)((buffer[8] << 8) | buffer[9]);
    gy = (int16_t)((buffer[10] << 8) | buffer[11]);
    gz = (int16_t)((buffer[12] << 8) | buffer[13]);
    
    return true;
}

bool MPU6050::readScaled(MPU6050Data &data) {
    int16_t ax, ay, az, gx, gy, gz;
    
    if (!readRaw(ax, ay, az, gx, gy, gz)) {
        return false;
    }
    
    // Convert to physical units
    data.accel_x = ax / accel_scale;
    data.accel_y = ay / accel_scale;
    data.accel_z = az / accel_scale;
    
    // Subtract gyro bias and convert to rad/s
    data.gyro_x = ((float)gx - gyro_bias_x) / gyro_scale * (M_PI / 180.0f);
    data.gyro_y = ((float)gy - gyro_bias_y) / gyro_scale * (M_PI / 180.0f);
    data.gyro_z = ((float)gz - gyro_bias_z) / gyro_scale * (M_PI / 180.0f);
    
    // Temperature (not used in AHRS but available)
    // data.temp = temp_raw / 340.0f + 36.53f;
    
    data.timestamp = micros();
    
    return true;
}
