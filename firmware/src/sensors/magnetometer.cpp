#include "magnetometer.h"
#include "../config/config.h"
#include <math.h>

// QMC5883L registers
#define QMC5883L_ADDR         0x0D
#define QMC5883L_REG_CONTROL  0x09
#define QMC5883L_REG_DATA     0x00
#define QMC5883L_REG_STATUS   0x06
#define QMC5883L_REG_ID       0x0D

// HMC5883L registers
#define HMC5883L_ADDR         0x1E
#define HMC5883L_REG_CONFIG_A 0x00
#define HMC5883L_REG_CONFIG_B 0x01
#define HMC5883L_REG_MODE     0x02
#define HMC5883L_REG_DATA     0x03
#define HMC5883L_REG_ID       0x0A

// Scale factors
#define QMC5883L_SCALE        30.0f  // LSB/Gauss
#define HMC5883L_SCALE        1090.0f // LSB/Gauss at 8.1 gain

Magnetometer::Magnetometer(uint8_t addr) : device_address(addr),
                                           device_type(0),
                                           calibration({0, 0, 0, 1, 1, 1, false}) {
}

bool Magnetometer::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(device_address);
    Wire.write(reg);
    Wire.write(value);
    return (Wire.endTransmission() == 0);
}

bool Magnetometer::readRegister(uint8_t reg, uint8_t &value) {
    Wire.beginTransmission(device_address);
    Wire.write(reg);
    if (Wire.endTransmission() != 0) return false;
    
    Wire.requestFrom(device_address, (uint8_t)1);
    if (Wire.available() < 1) return false;
    
    value = Wire.read();
    return true;
}

bool Magnetometer::readRegisters(uint8_t reg, uint8_t *buffer, uint8_t length) {
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

bool Magnetometer::detectDevice() {
    // Try QMC5883L first
    device_address = QMC5883L_ADDR;
    uint8_t id;
    if (readRegister(QMC5883L_REG_ID, id)) {
        device_type = 0;  // QMC5883L
        return true;
    }
    
    // Try HMC5883L
    device_address = HMC5883L_ADDR;
    uint8_t id_buffer[3];
    if (readRegisters(HMC5883L_REG_ID, id_buffer, 3)) {
        // HMC5883L ID should be 0x48 0x34 0x33
        if (id_buffer[0] == 0x48 && id_buffer[1] == 0x34 && id_buffer[2] == 0x33) {
            device_type = 1;  // HMC5883L
            return true;
        }
    }
    
    return false;
}

bool Magnetometer::begin() {
    Wire.begin();
    
    if (!detectDevice()) {
        return false;
    }
    
    return configure();
}

bool Magnetometer::configure() {
    if (device_type == 0) {
        // QMC5883L configuration
        // Control register: 0x01 = continuous mode, 50Hz, 2G range, 512 oversampling
        if (!writeRegister(QMC5883L_REG_CONTROL, 0x1D)) {
            return false;
        }
    } else {
        // HMC5883L configuration
        // Config A: 8 samples averaged, 75Hz output rate, normal measurement
        if (!writeRegister(HMC5883L_REG_CONFIG_A, 0x78)) {
            return false;
        }
        // Config B: Gain = 1090 LSB/G (8.1 gain)
        if (!writeRegister(HMC5883L_REG_CONFIG_B, 0x20)) {
            return false;
        }
        // Mode: Continuous measurement
        if (!writeRegister(HMC5883L_REG_MODE, 0x00)) {
            return false;
        }
    }
    
    delay(10);
    return true;
}

void Magnetometer::calibrate(uint16_t samples) {
    int16_t min_x = 32767, max_x = -32768;
    int16_t min_y = 32767, max_y = -32768;
    int16_t min_z = 32767, max_z = -32768;
    
    Serial.println("Calibrating magnetometer... Move sensor in figure-8 pattern!");
    
    for (uint16_t i = 0; i < samples; i++) {
        int16_t mx, my, mz;
        if (readRaw(mx, my, mz)) {
            if (mx < min_x) min_x = mx;
            if (mx > max_x) max_x = mx;
            if (my < min_y) min_y = my;
            if (my > max_y) max_y = my;
            if (mz < min_z) min_z = mz;
            if (mz > max_z) max_z = mz;
        }
        delay(10);
    }
    
    // Calculate offsets (hard-iron calibration)
    calibration.offset_x = (float)(max_x + min_x) / 2.0f;
    calibration.offset_y = (float)(max_y + min_y) / 2.0f;
    calibration.offset_z = (float)(max_z + min_z) / 2.0f;
    
    // Calculate scale factors (soft-iron calibration)
    float avg_delta_x = (float)(max_x - min_x) / 2.0f;
    float avg_delta_y = (float)(max_y - min_y) / 2.0f;
    float avg_delta_z = (float)(max_z - min_z) / 2.0f;
    float avg_delta = (avg_delta_x + avg_delta_y + avg_delta_z) / 3.0f;
    
    calibration.scale_x = avg_delta / avg_delta_x;
    calibration.scale_y = avg_delta / avg_delta_y;
    calibration.scale_z = avg_delta / avg_delta_z;
    
    calibration.calibrated = true;
    
    Serial.printf("Mag calibration: offset(%.1f,%.1f,%.1f) scale(%.3f,%.3f,%.3f)\n",
                 calibration.offset_x, calibration.offset_y, calibration.offset_z,
                 calibration.scale_x, calibration.scale_y, calibration.scale_z);
}

bool Magnetometer::readRaw(int16_t &mx, int16_t &my, int16_t &mz) {
    uint8_t buffer[6];
    
    if (device_type == 0) {
        // QMC5883L
        if (!readRegisters(QMC5883L_REG_DATA, buffer, 6)) {
            return false;
        }
        mx = (int16_t)((buffer[0] << 8) | buffer[1]);
        my = (int16_t)((buffer[2] << 8) | buffer[3]);
        mz = (int16_t)((buffer[4] << 8) | buffer[5]);
    } else {
        // HMC5883L
        if (!readRegisters(HMC5883L_REG_DATA, buffer, 6)) {
            return false;
        }
        mx = (int16_t)((buffer[0] << 8) | buffer[1]);
        mz = (int16_t)((buffer[2] << 8) | buffer[3]);
        my = (int16_t)((buffer[4] << 8) | buffer[5]);
    }
    
    return true;
}

bool Magnetometer::readScaled(MagnetometerData &data) {
    int16_t mx, my, mz;
    
    if (!readRaw(mx, my, mz)) {
        return false;
    }
    
    float scale = (device_type == 0) ? QMC5883L_SCALE : HMC5883L_SCALE;
    
    data.mag_x = mx / scale;
    data.mag_y = my / scale;
    data.mag_z = mz / scale;
    
    if (calibration.calibrated) {
        applyCalibration(data.mag_x, data.mag_y, data.mag_z);
    }
    
    data.timestamp = micros();
    
    return true;
}

void Magnetometer::loadCalibration(const MagCalibration &cal) {
    calibration = cal;
}

void Magnetometer::applyCalibration(float &mx, float &my, float &mz) {
    // Apply hard-iron offset
    mx -= calibration.offset_x / (device_type == 0 ? QMC5883L_SCALE : HMC5883L_SCALE);
    my -= calibration.offset_y / (device_type == 0 ? QMC5883L_SCALE : HMC5883L_SCALE);
    mz -= calibration.offset_z / (device_type == 0 ? QMC5883L_SCALE : HMC5883L_SCALE);
    
    // Apply soft-iron scale
    mx *= calibration.scale_x;
    my *= calibration.scale_y;
    mz *= calibration.scale_z;
}
