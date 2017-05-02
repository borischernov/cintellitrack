/*
 * test_location_provider.cpp
 *
 *  Created on: 06/10/2011
 *      Author: boris
 */

#include <time.h>
#include <stdio.h>
using namespace std;

#include "test_location_provider.h"
#include "config_file.h"
#include "log_file.h"
#include "utils.h"

const char *TestLocationProvider::module = "TEST_LOC";

TestLocationProvider::TestLocationProvider(ConfigFile::HashType config) : LocationProvider(config) {
	loc.provider = Location::Provider(stou(config["provider"], Location::GPS));
	location_log = fopen(config["location_log"].c_str(), "r");
	if (!location_log)
		LOG_ERROR(module, "Error opening location log %s", config["location_log"].c_str());
}

TestLocationProvider::~TestLocationProvider() {
	if (location_log)
		fclose(location_log);
}

Location TestLocationProvider::get_location(void) {
	char buf[256];

	if (!location_log || !fgets(buf, 255, location_log)) {
		loc.has_fix = false;
		return loc;
	}

	sscanf(buf, "%lu,%lf,%lf,%lf,%lf,%f,%d",
			&loc.time, &loc.latitude, &loc.longitude, &loc.altitude, &loc.speed, &loc.hdop, &loc.num_satellites);
	loc.has_fix = true;
	loc.has_speed = true;

	LOG_DEBUG(module, "Got fix: %lf, %lf, %lf, %f, %d", loc.latitude, loc.longitude, loc.speed, loc.hdop, loc.num_satellites);
	return loc;
}
