/*
 * location.cpp
 *
 *  Created on: 07/10/2011
 *      Author: boris
 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string>
using namespace std;

#include "location.h"

/*******************************************************************
 * Set time from HHMMSS.ss and DDMMYY time and date strings
 * (as in NMEA 0183 format)
 */

void Location::set_time(string time, string date) {
	struct tm timeinfo;

	timeinfo.tm_gmtoff = 0;
	timeinfo.tm_year = atoi(date.substr(4, 2).c_str()) + 100;
	timeinfo.tm_mon = atoi(date.substr(2, 2).c_str()) - 1;
	timeinfo.tm_mday = atoi(date.substr(0, 2).c_str());
	timeinfo.tm_hour = atoi(time.substr(0, 2).c_str());
	timeinfo.tm_min = atoi(time.substr(2, 2).c_str());
	timeinfo.tm_sec = atoi(time.substr(4, 2).c_str());

	this->time = mktime(&timeinfo);
}

/*******************************************************************
 * Convert DDMM.mm degrees/minutes and hemisphere char to degrees
 * (as in NMEA 0183 format)
 */

double Location::dmh2d(double dm, char h) {
	int d;
	double m, r;
	d = ((int)dm) / 100;
	m = dm - d * 100;
	r = d + m / 60.0;
	return (h == 'W' || h == 'S') ? -r : r;
}

/*******************************************************************
 * Calculate distance to other location in meters
 */
double Location::distance_to(Location other) {
    return earthradius * acos(
	        sin(latitude * degtorad) * sin(other.latitude * degtorad) +
	        cos(latitude * degtorad) * cos(other.latitude * degtorad) * cos((longitude - other.longitude) * degtorad)
	        );
}

/*******************************************************************
 * Represent location as CSV-string
 */
string Location::to_string(void) {
	char buf[255];
	sprintf(buf, "%lu,%.6lf,%.6lf,%.2lf,%.2lf,%.1f,%d",
			this->time, latitude, longitude, altitude, speed, hdop, num_satellites);
	return string(buf);
}

/*******************************************************************
 * Calculate bearing to other location in radians
 */
double Location::bearing_to(Location other) {
	double y = sin((other.longitude - longitude) * degtorad) * cos(other.latitude * degtorad);
	double x = cos(latitude * degtorad) * sin(other.latitude * degtorad) -
			   sin(latitude * degtorad) * cos(other.latitude * degtorad) * cos((other.longitude - longitude) * degtorad);
	return atan2(y, x);
}

/*******************************************************************
 * Calculate cross-track distance between track (this - to) and pt
 */
double Location::crosstrack(Location to, Location pt) {
	return asin(sin(distance_to(pt) / earthradius) * sin(bearing_to(pt) - bearing_to(to))) * earthradius;
}
