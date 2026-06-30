#ifndef MAGNETOMETER_H
#define MAGNETOMETER_H

#include <Arduino.h>
#include <Wire.h>

struct MagnetometerData {
    float mag_x, mag_y, mag_z;  // Gauss
    uint32_t timestamp;
};

struct MagCalibration {
    float offset_x, offset_y, offset_z;
    float scale_x, scale_y, scale_z;
    bool calibrated;
};

class Magnetometer {
private:
    uint8_t device_address;
    uint8_t device_type;  // 0=QMC5883L, 1=HMC5883L
    MagCalibration calibration;
    
    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegister(uint8_t reg, uint8_t &value);
    bool readRegisters(uint8_t reg, uint8_t *buffer, uint8_t length);
    
public:
    Magnetometer(uint8_t addr = 0x0D);  // QMC5883L default
    
    bool begin();
    bool detectDevice();
    bool configure();
    void calibrate(uint16_t samples = 1000);
    bool readRaw(int16_t &mx, int16_t &my, int16_t &mz);
    bool readScaled(MagnetometerData &data);
    void applyCalibration(float &mx, float &my, float &mz);
    MagCalibration getCalibration() { return calibration; }
    void loadCalibration(const MagCalibration &cal);
};

#endif
