/*
 * location_manager.cpp
 *
 *  Created on: 10/10/2011
 *      Author: boris
 */

#include <cmath>
using namespace std;

#include "location_manager.h"
#include "log_file.h"
#include "config_file.h"
#include "nmea_location_provider.h"
#include "test_location_provider.h"
#include "utils.h"
#include "communication_manager.h"

LocationManager *LocationManager::instance = NULL;
const char*LocationManager::module = "LOC_MANAGER";

LocationManager::LocationManager() {
	ConfigFile *config = ConfigFile::get_instance();
	ConfigFile::HashType group;

	if (config->get_group("location_manager", &group, true) == ConfigFile::OK) {
		distance_tolerance = stod(group["distance_tolerance"], 100.0);
		time_tolerance = stoul(group["time_tolerance"], 60);
		maxctd = stod(group["max_ctd"], 10);
		drive_speed = stou(group["drive_speed"], 6);
		stop_speed = stou(group["stop_speed"], 2);
	}

	populate_providers();
	send_updates = true;
	start_locating();
}

void LocationManager::destroy_providers(void) {
	LOG_DEBUG(module, "Destroying providers");

	for (size_t c = 0; c < providers.size(); c++)
		delete providers[c];
	providers.clear();
}

void LocationManager::populate_providers(void) {
	ConfigFile *config = ConfigFile::get_instance();
	ConfigFile::HashType group;

	LOG_DEBUG(module, "Loading providers");

	if (providers.size()) destroy_providers();

	for(bool do_rewind = true; config->get_group("location_provider", &group, do_rewind) == ConfigFile::OK; do_rewind = false) {
		if (!group["type"].compare("nmea")) {
			LOG_DEBUG(module, "Adding NMEA location provider");
			providers.insert(providers.begin(), (LocationProvider *)(new NmeaLocationProvider(group)));
		} else if (!group["type"].compare("test")) {
				LOG_DEBUG(module, "Adding test location provider");
				providers.insert(providers.begin(), (LocationProvider *)(new TestLocationProvider(group)));
		} else {
			LOG_ERROR(module, "Unknown location provider type: %s (at line %d); skipping.",
						group["type"].c_str(), config->get_current_line());
			continue;
		}
	}

	LOG_DEBUG(module, "%u providers loaded", providers.size());
}

void LocationManager::start_locating() {
	LOG_DEBUG(module, "Starting listening for updates");

	for (size_t c = 0; c < providers.size(); c++)
		providers[c]->listen_for_updates(LocationManager::update_location_cb, 5, 0);
}

void LocationManager::stop_locating() {
	LOG_DEBUG(module, "Stopping listening for updates");

	for (size_t c = 0; c < providers.size(); c++)
		providers[c]->stop_listening();
}

void LocationManager::update_location(Location l) {
	mutex.lock();
	LOG_DEBUG(module, "Location update : %lf / %lf", l.latitude, l.longitude);

	if (is_better(l)) {

		// Detect motion mode (stopped / moving)
		if (current_location.mode == Location::Stopped && l.speed > drive_speed) {
			LOG_DEBUG(module, "Mode change: Stopped => Moving");
			l.mode = Location::Moving;
		} else if (current_location.mode == Location::Moving && l.speed < stop_speed) {
			LOG_DEBUG(module, "Mode change: Moving => Stopped");
			l.mode = Location::Stopped;
		} else
			l.mode = current_location.mode;

		current_location = l;

		if (in_route(l)) {
			LOG_DEBUG(module, "New location : %lf / %lf", locations[0].latitude, locations[0].longitude);

			if (send_updates)
				CommunicationManager::get_instance()->send_location_update(locations[0]);
			else
				LOG_DEBUG(module, "Sending updates is off; not sending");

		} else
			LOG_DEBUG(module, "Location rejected by route minimizer");
	}
	mutex.unlock();
}

bool LocationManager::is_better(Location l) {
	if (!l.has_fix) return false;				// No fix isn't better :)

	if (!current_location.has_fix) return true; // Any fix is better than nothing

	if (l.provider == Location::NET and current_location.provider == Location::GPS) // NET is less precise than GPS
	{
		// Displacement tolerance check
		if (l.provider == current_location.provider and current_location.distance_to(l) > distance_tolerance) return true;

		// Time tolerance check
		if (l.time - current_location.time > time_tolerance) return true; // Any location is better than a very old one

		return false;
	}

	return true;
}

int LocationManager::get_num_satellites(void) {
	int ns = 0;

	for (size_t c = 0; c < providers.size(); c++)
		if (providers[c]->get_num_satellites() > ns)
			ns = providers[c]->get_num_satellites();

	return ns;
}

bool LocationManager::in_route(Location l) {
	if (!locations[0].has_fix) {	// First point always belongs to route
		locations[0] = l;
		return true;
	}
	if (!locations[1].has_fix) {    // Need to get the third point to decide
		locations[1] = l;
		return false;
	}
	if (locations[2].has_fix)		// Shift
		locations[1] = locations[2];
	locations[2] = l;

	if (abs(locations[0].crosstrack(locations[2], locations[1])) > maxctd ||         // Max CTD exceeded
		locations[1].time - locations[0].time > time_tolerance ||					 // Time tolerance exceeded
		locations[0].distance_to(locations[1]) > distance_tolerance || 				 // Distance tolerance exceeded
		locations[0].mode != locations[1].mode										 // Mode changed
		) {
		locations[0] = locations[1];
		return true;
	}

	return false;
}
