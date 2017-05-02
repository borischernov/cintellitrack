/*
 * nmea_location_provider.h
 *
 *  Created on: 06/10/2011
 *      Author: boris
 */

#ifndef NMEA_LOCATION_PROVIDER_H_
#define NMEA_LOCATION_PROVIDER_H_

#include "stdio.h"
#include "location_provider.h"
#include "serialport.h"
#include "config_file.h"
#include "location_filter.h"

class NmeaLocationProvider : public LocationProvider {
private:
	SerialPort port;
	static const char *module;
	bool debug;
	bool time_source;
	int prev_num_satellites;
	int num_satellites;
	float hdop;
	FILE *location_log;
	unsigned int min_satellites;
	float max_hdop;
	double max_speed;
	LocationFilter kf;

	virtual void thread_body(void);
	bool location_acceptable(Location l);

public:
	NmeaLocationProvider(ConfigFile::HashType config);
	~NmeaLocationProvider();
	Location get_location(void);
	int get_num_satellites(void) { return num_satellites; };
};

#endif /* NMEA_LOCATION_PROVIDER_H_ */
