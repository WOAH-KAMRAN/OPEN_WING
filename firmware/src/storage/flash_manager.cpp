#include "flash_manager.h"

static const char* NAMESPACE = "openwing";
static const char* CONFIG_KEY = "config";

FlashManager::FlashManager() : loaded(false) {
    initDefaults();
}

void FlashManager::initDefaults() {
    config.config_version = CONFIG_VERSION;
    config.roll_rate_kp = 0.15f; config.roll_rate_ki = 0.0f; config.roll_rate_kd = 0.005f;
    config.pitch_rate_kp = 0.15f; config.pitch_rate_ki = 0.0f; config.pitch_rate_kd = 0.005f;
    config.roll_angle_kp = 4.5f; config.roll_angle_ki = 0.0f; config.roll_angle_kd = 0.0f;
    config.pitch_angle_kp = 5.0f; config.pitch_angle_ki = 0.0f; config.pitch_angle_kd = 0.0f;
    config.gyro_bias_x = 0; config.gyro_bias_y = 0; config.gyro_bias_z = 0;
    config.mag_offset_x = 0; config.mag_offset_y = 0; config.mag_offset_z = 0;
    config.mag_scale_x = 1; config.mag_scale_y = 1; config.mag_scale_z = 1;
    config.mag_calibrated = false;
    config.home_latitude = 0; config.home_longitude = 0; config.home_altitude = 0;
    config.home_set = false;
}

bool FlashManager::begin() {
    if (!prefs.begin(NAMESPACE, false)) {
        return false;
    }
    return load();
}

bool FlashManager::load() {
    size_t read_size = prefs.getBytes(CONFIG_KEY, &config, sizeof(config));
    if (read_size != sizeof(config) || config.config_version != CONFIG_VERSION) {
        initDefaults();
        return false;
    }
    loaded = true;
    return true;
}

bool FlashManager::save() {
    config.config_version = CONFIG_VERSION;
    return prefs.putBytes(CONFIG_KEY, &config, sizeof(config)) == sizeof(config);
}

void FlashManager::resetToDefaults() {
    initDefaults();
    save();
}
