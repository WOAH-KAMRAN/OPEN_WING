#include <Arduino.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <Wire.h>

#include "config/config.h"
#include "config/pins.h"
#include "sensors/mpu6050.h"
#include "sensors/magnetometer.h"
#include "ahrs/madgwick.h"
#include "control/flight_control.h"
#include "io/ibus.h"
#include "io/pwm.h"
#include "telemetry/telemetry.h"
#include "navigation/navigation.h"
#include "storage/flash_manager.h"
#include "io/webserver.h"

// Global objects
MPU6050 imu;
Magnetometer mag;
MadgwickAHRS ahrs;
FlightControl flight_control;
IBUS rc_input;
PWMOutput pwm_output;
Telemetry telemetry;
Navigation navigation;
FlashManager flash_manager;
WebServerHandler web_server;

// Task handles
TaskHandle_t high_freq_task_handle;
TaskHandle_t med_freq_task_handle;
TaskHandle_t low_freq_task_handle;

// Shared data (protected by mutexes)
struct SharedData {
    AttitudeData attitude;
    IBUSData rc_data;
    ControlOutput control_output;
    FlightMode current_mode;
    bool magnetometer_healthy;
    bool home_set;
} shared_data;

SemaphoreHandle_t attitude_mutex;
SemaphoreHandle_t rc_mutex;
SemaphoreHandle_t control_mutex;

// Mag failure tracking (not shared, task-local)
static uint8_t mag_fail_count = 0;

void blinkLED(int count, int delay_ms) {
    for (int i = 0; i < count; i++) {
        digitalWrite(PIN_LED_STATUS, HIGH);
        delay(delay_ms);
        digitalWrite(PIN_LED_STATUS, LOW);
        delay(delay_ms);
    }
}

void processSerialCommands() {
    if (Serial.available() <= 0) return;

    static char line[64];
    static uint8_t pos = 0;

    while (Serial.available() > 0) {
        char c = Serial.read();
        if (c == '\n' || c == '\r') {
            line[pos] = '\0';
            if (pos == 0) continue;

            char cmd[16], axis[16];
            float kp = 0, ki = 0, kd = 0;
            int parsed = sscanf(line, "%s %s %f %f %f", cmd, axis, &kp, &ki, &kd);

            if (strcmp(cmd, "SET") == 0 && parsed == 5) {
                if (strcmp(axis, "ROLL_RATE") == 0) { flight_control.setRollRateGains(kp, ki, kd); }
                else if (strcmp(axis, "PITCH_RATE") == 0) { flight_control.setPitchRateGains(kp, ki, kd); }
                else if (strcmp(axis, "ROLL_ANGLE") == 0) { flight_control.setRollAngleGains(kp, ki, kd); }
                else if (strcmp(axis, "PITCH_ANGLE") == 0) { flight_control.setPitchAngleGains(kp, ki, kd); }
                else { Serial.printf("Unknown axis: %s\n", axis); pos = 0; continue; }
                Serial.printf("SET %s: %.4f %.4f %.4f\n", axis, kp, ki, kd);
            }
            else if (strcmp(cmd, "SET") == 0 && parsed == 4) {
                flight_control.setRollRateGains(kp, ki, kd);
                flight_control.setPitchRateGains(kp, ki, kd);
                flight_control.setRollAngleGains(kp, ki, kd);
                flight_control.setPitchAngleGains(kp, ki, kd);
                Serial.printf("SET ALL: %.4f %.4f %.4f\n", kp, ki, kd);
            }
            else if (strcmp(cmd, "GET") == 0) {
                Serial.printf("RollRate  Kp=%.4f Ki=%.4f Kd=%.4f\n",
                    flight_control.getRollRatePID().getKp(),
                    flight_control.getRollRatePID().getKi(),
                    flight_control.getRollRatePID().getKd());
                Serial.printf("PitchRate Kp=%.4f Ki=%.4f Kd=%.4f\n",
                    flight_control.getPitchRatePID().getKp(),
                    flight_control.getPitchRatePID().getKi(),
                    flight_control.getPitchRatePID().getKd());
                Serial.printf("RollAngle Kp=%.4f Ki=%.4f Kd=%.4f\n",
                    flight_control.getRollAnglePID().getKp(),
                    flight_control.getRollAnglePID().getKi(),
                    flight_control.getRollAnglePID().getKd());
                Serial.printf("PitchAngle Kp=%.4f Ki=%.4f Kd=%.4f\n",
                    flight_control.getPitchAnglePID().getKp(),
                    flight_control.getPitchAnglePID().getKi(),
                    flight_control.getPitchAnglePID().getKd());
            }
            else if (strcmp(cmd, "SAVE") == 0) {
                flight_control.saveToFlash(flash_manager);
                Serial.println("Config saved to flash");
            }
            else if (strcmp(cmd, "LOAD") == 0) {
                flight_control.loadFromFlash(flash_manager);
                Serial.println("Config loaded from flash");
            }
            else if (strcmp(cmd, "RESET") == 0) {
                flight_control.begin();
                Serial.println("Config reset to factory defaults (not saved)");
            }
            else if (strcmp(cmd, "HELP") == 0) {
                Serial.println("--- Serial CLI Commands ---");
                Serial.println("SET <AXIS> <KP> <KI> <KD>  Set PID gains for axis");
                Serial.println("  Axes: ROLL_RATE, PITCH_RATE, ROLL_ANGLE, PITCH_ANGLE");
                Serial.println("  SKIP <AXIS> name to set ALL 4 axis the same gains");
                Serial.println("GET                        Print current PID gains");
                Serial.println("SAVE                       Save gains to flash");
                Serial.println("LOAD                       Load gains from flash");
                Serial.println("RESET                      Reset to firmware defaults");
                Serial.println("HELP                       This message");
            }
            else {
                Serial.printf("Unknown: %s (type HELP)\n", cmd);
            }

            pos = 0;
        } else if (pos < sizeof(line) - 1) {
            if (c != '\r') line[pos++] = c;
        }
    }
}

void highFrequencyTask(void *parameter) {
    TickType_t last_wake_time = xTaskGetTickCount();
    MPU6050Data imu_data;
    MagnetometerData mag_data;

    Serial.println("High frequency task started (400 Hz)");

    while (1) {
        if (imu.readScaled(imu_data)) {
            bool mag_ok = mag.readScaled(mag_data);

            if (!mag_ok) {
                mag_fail_count++;
                if (mag_fail_count >= MAG_FAIL_THRESHOLD) {
                    if (xSemaphoreTake(control_mutex, pdMS_TO_TICKS(1)) == pdTRUE) {
                        shared_data.magnetometer_healthy = false;
                        xSemaphoreGive(control_mutex);
                    }
                }
            } else {
                mag_fail_count = 0;
            }

            if (shared_data.magnetometer_healthy) {
                ahrs.update(imu_data.gyro_x, imu_data.gyro_y, imu_data.gyro_z,
                           imu_data.accel_x, imu_data.accel_y, imu_data.accel_z,
                           mag_data.mag_x, mag_data.mag_y, mag_data.mag_z,
                           imu_data.timestamp);
            } else {
                ahrs.updateIMU(imu_data.gyro_x, imu_data.gyro_y, imu_data.gyro_z,
                              imu_data.accel_x, imu_data.accel_y, imu_data.accel_z,
                              imu_data.timestamp);
            }

            if (xSemaphoreTake(attitude_mutex, pdMS_TO_TICKS(1)) == pdTRUE) {
                shared_data.attitude = ahrs.getAttitude();
                xSemaphoreGive(attitude_mutex);
            }
        }

        IBUSData rc;
        if (xSemaphoreTake(rc_mutex, pdMS_TO_TICKS(1)) == pdTRUE) {
            rc = shared_data.rc_data;
            xSemaphoreGive(rc_mutex);
        }

        if (xSemaphoreTake(attitude_mutex, pdMS_TO_TICKS(1)) == pdTRUE) {
            flight_control.update(shared_data.attitude, imu_data, rc);
            shared_data.control_output = flight_control.getOutput();
            xSemaphoreGive(attitude_mutex);
        }

        pwm_output.writeServo(LEDC_CHANNEL_0, shared_data.control_output.aileron);
        pwm_output.writeServo(LEDC_CHANNEL_1, shared_data.control_output.elevator);
        pwm_output.writeServo(LEDC_CHANNEL_2, shared_data.control_output.throttle);
        pwm_output.writeServo(LEDC_CHANNEL_3, shared_data.control_output.rudder);

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(HIGH_FREQ_PERIOD_MS));
    }
}

void mediumFrequencyTask(void *parameter) {
    TickType_t last_wake_time = xTaskGetTickCount();
    uint32_t last_mag_retry = 0;

    Serial.println("Medium frequency task started (50 Hz)");

    while (1) {
        rc_input.update();
        if (rc_input.hasNewData()) {
            if (xSemaphoreTake(rc_mutex, pdMS_TO_TICKS(1)) == pdTRUE) {
                shared_data.rc_data = rc_input.getData();
                xSemaphoreGive(rc_mutex);
            }
        }

        uint16_t mode_channel = rc_input.getChannel(5);
        FlightMode new_mode;
        if (mode_channel < 1300) {
            new_mode = MANUAL;
        } else if (mode_channel < 1700) {
            new_mode = FBWA;
        } else {
            new_mode = STABILIZE;
        }

        if (new_mode != shared_data.current_mode) {
            shared_data.current_mode = new_mode;
            flight_control.setMode(new_mode);
            Serial.printf("Mode changed to: %d\n", new_mode);
        }

        if (rc_input.isFailsafe()) {
            Serial.println("RC FAILSAFE DETECTED!");
        }

        if (!shared_data.magnetometer_healthy && (millis() - last_mag_retry > 1000)) {
            last_mag_retry = millis();
            Serial.println("Attempting magnetometer re-init...");
            if (mag.detectDevice() && mag.configure()) {
                if (xSemaphoreTake(control_mutex, pdMS_TO_TICKS(1)) == pdTRUE) {
                    shared_data.magnetometer_healthy = true;
                    xSemaphoreGive(control_mutex);
                }
                Serial.println("Magnetometer re-initialized successfully");
            }
        }

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(MED_FREQ_PERIOD_MS));
    }
}

void lowFrequencyTask(void *parameter) {
    TickType_t last_wake_time = xTaskGetTickCount();
    uint32_t loop_count = 0;

    Serial.println("Low frequency task started (10 Hz)");

    while (1) {
        processSerialCommands();

        if (xSemaphoreTake(attitude_mutex, pdMS_TO_TICKS(1)) == pdTRUE &&
            xSemaphoreTake(rc_mutex, pdMS_TO_TICKS(1)) == pdTRUE &&
            xSemaphoreTake(control_mutex, pdMS_TO_TICKS(1)) == pdTRUE) {

            bool mag_ok = shared_data.magnetometer_healthy;
            bool home_ok = shared_data.home_set;

            telemetry.update(shared_data.attitude, shared_data.rc_data,
                           shared_data.control_output, shared_data.current_mode,
                           mag_ok, home_ok);
            telemetry.sendPacket();

            xSemaphoreGive(control_mutex);
            xSemaphoreGive(rc_mutex);
            xSemaphoreGive(attitude_mutex);
        }

        loop_count++;
        if (loop_count >= 10) {
            loop_count = 0;

            if (xSemaphoreTake(attitude_mutex, pdMS_TO_TICKS(1)) == pdTRUE) {
                Serial.printf("Roll: %.1f, Pitch: %.1f, Yaw: %.1f, Mode: %d, Mag: %s\n",
                             shared_data.attitude.roll * 180.0f / M_PI,
                             shared_data.attitude.pitch * 180.0f / M_PI,
                             shared_data.attitude.yaw * 180.0f / M_PI,
                             shared_data.current_mode,
                             shared_data.magnetometer_healthy ? "OK" : "FALLBACK");
                xSemaphoreGive(attitude_mutex);
            }
        }

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(LOW_FREQ_PERIOD_MS));
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("========================================");
    Serial.println("OpenWing Flight Controller Initializing");
    Serial.println("========================================");

    pinMode(PIN_LED_STATUS, OUTPUT);
    blinkLED(3, 100);

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQ);
    Serial.println("I2C initialized");

    if (!imu.begin()) {
        Serial.println("ERROR: MPU6050 initialization failed!");
        while (1) {
            blinkLED(5, 200);
            delay(1000);
        }
    }
    Serial.println("MPU6050 initialized");

    if (!imu.configure(HIGH_FREQ_RATE_HZ)) {
        Serial.println("ERROR: MPU6050 configuration failed!");
        while (1);
    }
    Serial.printf("MPU6050 configured at %d Hz\n", HIGH_FREQ_RATE_HZ);

    Serial.println("Calibrating gyro - keep sensor still...");
    imu.calibrateGyro(500);
    Serial.println("Gyro calibration complete");

    PersistentConfig &cfg = flash_manager.getConfig();
    cfg.gyro_bias_x = imu.getGyroBiasX();
    cfg.gyro_bias_y = imu.getGyroBiasY();
    cfg.gyro_bias_z = imu.getGyroBiasZ();

    if (!mag.begin()) {
        Serial.println("WARNING: Magnetometer init failed - continuing without mag");
        shared_data.magnetometer_healthy = false;
    } else {
        Serial.println("Magnetometer initialized");
        if (cfg.mag_calibrated) {
            MagCalibration mag_cal;
            mag_cal.offset_x = cfg.mag_offset_x;
            mag_cal.offset_y = cfg.mag_offset_y;
            mag_cal.offset_z = cfg.mag_offset_z;
            mag_cal.scale_x = cfg.mag_scale_x;
            mag_cal.scale_y = cfg.mag_scale_y;
            mag_cal.scale_z = cfg.mag_scale_z;
            mag_cal.calibrated = cfg.mag_calibrated;
            mag.loadCalibration(mag_cal);
        }
        shared_data.magnetometer_healthy = true;
    }

    ahrs.reset();
    Serial.println("AHRS initialized");

    flight_control.begin();
    flight_control.loadFromFlash(flash_manager);
    Serial.println("Flight control initialized (PID from flash)");

    if (!rc_input.begin()) {
        Serial.println("ERROR: IBUS initialization failed!");
        while (1) {
            blinkLED(3, 200);
            delay(1000);
        }
    }
    Serial.println("IBUS RC input initialized");

    pwm_output.begin();

    ServoConfig aileron_config = {SERVO_AILERON_PIN, LEDC_CHANNEL_0, PWM_MIN_US, PWM_MAX_US, -1.0f, 1.0f};
    ServoConfig elevator_config = {SERVO_ELEVATOR_PIN, LEDC_CHANNEL_1, PWM_MIN_US, PWM_MAX_US, -1.0f, 1.0f};
    ServoConfig throttle_config = {SERVO_THROTTLE_PIN, LEDC_CHANNEL_2, PWM_MIN_US, PWM_MAX_US, 0.0f, 1.0f};
    ServoConfig rudder_config = {PIN_SERVO_RUDDER, LEDC_CHANNEL_3, PWM_MIN_US, PWM_MAX_US, -1.0f, 1.0f};

    pwm_output.configureServo(aileron_config);
    pwm_output.configureServo(elevator_config);
    pwm_output.configureServo(throttle_config);
    pwm_output.configureServo(rudder_config);
    Serial.println("Servos configured");

    if (!telemetry.begin()) {
        Serial.println("WARNING: Telemetry init failed - continuing without telemetry");
    } else {
        Serial.println("Telemetry initialized (v2, 64B)");
    }

    Serial.println("Navigation initialized");
    shared_data.home_set = cfg.home_set;

    attitude_mutex = xSemaphoreCreateMutex();
    rc_mutex = xSemaphoreCreateMutex();
    control_mutex = xSemaphoreCreateMutex();

    if (attitude_mutex == NULL || rc_mutex == NULL || control_mutex == NULL) {
        Serial.println("ERROR: Mutex creation failed!");
        while (1);
    }

    shared_data.current_mode = FBWA;
    shared_data.magnetometer_healthy = true;

    web_server.begin(&shared_data.attitude, &flight_control, &shared_data.current_mode, control_mutex);
    web_server.start();
    Serial.println("Web server started");

    Serial.println("Creating FreeRTOS tasks...");

    xTaskCreatePinnedToCore(highFrequencyTask, "HighFreq", 4096, NULL, 5, &high_freq_task_handle, 0);
    xTaskCreatePinnedToCore(mediumFrequencyTask, "MedFreq", 2048, NULL, 3, &med_freq_task_handle, 1);
    xTaskCreatePinnedToCore(lowFrequencyTask, "LowFreq", 2048, NULL, 1, &low_freq_task_handle, 1);

    Serial.println("========================================");
    Serial.println("Flight Controller Ready!");
    Serial.println("High-freq task: 400 Hz (Core 0)");
    Serial.println("Med-freq task: 50 Hz (Core 1)");
    Serial.println("Low-freq task: 10 Hz (Core 1)");
    Serial.println("========================================");

    blinkLED(5, 50);
}

void loop() {
    web_server.handleClient();
    vTaskDelay(pdMS_TO_TICKS(1000));
}
