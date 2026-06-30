#ifndef OPENWING_WEBSERVER_H
#define OPENWING_WEBSERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "../config/config.h"
#include "../ahrs/madgwick.h"
#include "../control/flight_control.h"

class WebServerHandler {
private:
    WebServer server;
    bool running;
    
    // Pointers to shared data
    const AttitudeData* attitude_data;
    const FlightControl* flight_control;
    const FlightMode* flight_mode;
    
    // Mutex for data access
    SemaphoreHandle_t data_mutex;
    
public:
    WebServerHandler();
    
    void begin(const AttitudeData* attitude, const FlightControl* fc, const FlightMode* mode, SemaphoreHandle_t mutex);
    void start();
    void stop();
    bool isRunning() const { return running; }
    void handleClient();
    
private:
    void setupRoutes();
    void handleRoot();
    void handleData();
    void handleNotFound();
    
    String getHTMLPage();
};

#endif
