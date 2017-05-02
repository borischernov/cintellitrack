/*
 * location_filter.cpp
 *
 *  Created on: 12/11/2011
 *      Author: boris
 */

#include "location_filter.h"
#include "log_file.h"

const char*LocationFilter::module = "LOC_FILTER";

LocationFilter::LocationFilter(double iv1, double iv2, double iq1, double iq2, double p0) {
	v1 = iv1 > 0 ? iv1 : 2;
	v2 = iv2 > 0 ? iv2 : 12.5;
	q1 = iq1 > 0 ? iq1 : 20;
	q2 = iq2 > 0 ? iq2 : 5;
	p = p0 > 0 ? p0 : 1;
}

LocationFilter::LocationFilter(void) {
	v1 = 2;
	v2 = 12.5;
	q1 = 20;
	q2 = 5;
	p = 1;
}

Location LocationFilter::filter(Location l) {
	double k;
	Location r;

	if (!loc.has_fix) {
		loc = l;
		return l;
	}
//	LOG_DEBUG(module, "Current location : %lf, %lf, %lf, %f, %d", loc.latitude, loc.longitude, loc.speed, loc.hdop, loc.num_satellites);
//	LOG_DEBUG(module, "Location fed : %lf, %lf, %lf, %f, %d", l.latitude, l.longitude, l.speed, l.hdop, l.num_satellites);
//	LOG_DEBUG(module, "P = %lf", p);
	// Prediction
	p += (l.speed < v2 && l.speed > v1) ? q1 : q2;
//	LOG_DEBUG(module, "P- = %lf", p);
	// Correction
	k = p / (p + double(loc.hdop) * double(loc.hdop));
//	LOG_DEBUG(module, "K = %lf", k);

	loc.latitude += k * (l.latitude - loc.latitude);
	loc.longitude += k * (l.longitude - loc.longitude);
	loc.speed += k * (l.speed - loc.speed);
	loc.altitude = k * (l.altitude - loc.altitude);

	p = (1 - k) * p;

	r = l;
	r.latitude = loc.latitude;
	r.longitude = loc.longitude;
	r.altitude = loc.altitude;
	r.speed = loc.speed;

	//	LOG_DEBUG(module, "P = %lf", p);
	//	LOG_DEBUG(module, "Out location : %lf, %lf, %lf, %f, %d", r.latitude, r.longitude, r.speed, r.hdop, r.num_satellites);

	return r;
}
