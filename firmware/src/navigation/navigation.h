#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <Arduino.h>
#include "../config/config.h"

struct Waypoint {
    float latitude;
    float longitude;
    float altitude;
    bool reached;
};

struct HomePosition {
    double latitude;
    double longitude;
    float altitude;
    bool set;
};

class Navigation {
private:
    Waypoint waypoints[10];
    uint8_t current_waypoint_index;
    uint8_t num_waypoints;
    HomePosition home;
    
public:
    Navigation();

    void setHome(double lat, double lon, float alt);
    HomePosition getHome() { return home; }
    bool hasHome() { return home.set; }

    void addWaypoint(float lat, float lon, float alt);
    void clearWaypoints();
    Waypoint getCurrentWaypoint();
    void setWaypointReached();
    bool hasMoreWaypoints();
};

#endif
