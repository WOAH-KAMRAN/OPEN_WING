#ifndef FLASH_MANAGER_H
#define FLASH_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>

#define CONFIG_VERSION 1

struct PersistentConfig {
    uint8_t config_version;

    float roll_rate_kp, roll_rate_ki, roll_rate_kd;
    float pitch_rate_kp, pitch_rate_ki, pitch_rate_kd;
    float roll_angle_kp, roll_angle_ki, roll_angle_kd;
    float pitch_angle_kp, pitch_angle_ki, pitch_angle_kd;

    float gyro_bias_x, gyro_bias_y, gyro_bias_z;

    float mag_offset_x, mag_offset_y, mag_offset_z;
    float mag_scale_x, mag_scale_y, mag_scale_z;
    bool mag_calibrated;

    double home_latitude, home_longitude;
    float home_altitude;
    bool home_set;
};

class FlashManager {
private:
    Preferences prefs;
    PersistentConfig config;
    bool loaded;

    void initDefaults();

public:
    FlashManager();
    bool begin();
    bool load();
    bool save();
    void resetToDefaults();
    PersistentConfig& getConfig() { return config; }
};

#endif
