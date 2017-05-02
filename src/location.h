/*
 * location.h
 *
 *  Created on: 06/10/2011
 *      Author: boris
 */

#ifndef LOCATION_H_
#define LOCATION_H_

#include <time.h>
#include <string>
using namespace std;

class Location {
public:
	enum Provider {GPS = 0, NET = 1};
	enum Mode { Stopped = 0, Moving = 1};

	static const double ktstoms = 0.514444;  			// kts to m/s conversion factor
	static const double degtorad = 0.017453292; 		// degrees to radians conversion factor
	static const double earthradius = 6371000.0;		// Earth radius in metres

	Provider provider;
	bool has_fix;
	bool has_speed;
	double latitude;
	double longitude;
	double altitude;
	double speed;				// m/s
	float track;				// degrees true
	time_t time;
	float hdop;					// GPS Horizontal Dilution of Precision
	int num_satellites;
	Mode mode;

	Location(void) { this->has_fix = false; mode = Location::Stopped;};
	Location(Provider prv) { this->provider = prv; this->has_fix = false; mode = Location::Stopped;};
	void set_time(string time, string date);
	void set_latitude(double dm, char h) { this->latitude = this->dmh2d(dm, h); };
	void set_longitude(double dm, char h) { this->longitude = this->dmh2d(dm, h); };
	void set_speed_kts(double kts) { this->speed = kts * ktstoms; }

	double distance_to(Location other);
	double bearing_to(Location other);
	double crosstrack(Location to, Location pt);

	string to_string(void);
private:
	double dmh2d(double dm, char h);
};


#endif /* LOCATION_H_ */
