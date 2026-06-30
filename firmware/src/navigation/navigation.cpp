#include "navigation.h"

Navigation::Navigation() : current_waypoint_index(0), num_waypoints(0) {
    for (int i = 0; i < 10; i++) {
        waypoints[i].latitude = 0.0f;
        waypoints[i].longitude = 0.0f;
        waypoints[i].altitude = 0.0f;
        waypoints[i].reached = false;
    }
    home.latitude = 0.0;
    home.longitude = 0.0;
    home.altitude = 0.0f;
    home.set = false;
}

void Navigation::setHome(double lat, double lon, float alt) {
    home.latitude = lat;
    home.longitude = lon;
    home.altitude = alt;
    home.set = true;
}

void Navigation::addWaypoint(float lat, float lon, float alt) {
    if (num_waypoints < 10) {
        waypoints[num_waypoints].latitude = lat;
        waypoints[num_waypoints].longitude = lon;
        waypoints[num_waypoints].altitude = alt;
        waypoints[num_waypoints].reached = false;
        num_waypoints++;
    }
}

void Navigation::clearWaypoints() {
    num_waypoints = 0;
    current_waypoint_index = 0;
}

Waypoint Navigation::getCurrentWaypoint() {
    if (current_waypoint_index < num_waypoints) {
        return waypoints[current_waypoint_index];
    }
    return waypoints[0];
}

void Navigation::setWaypointReached() {
    if (current_waypoint_index < num_waypoints) {
        waypoints[current_waypoint_index].reached = true;
        current_waypoint_index++;
    }
}

bool Navigation::hasMoreWaypoints() {
    return current_waypoint_index < num_waypoints;
}
