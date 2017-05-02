/*
 * location_manager.h
 *
 *  Created on: 09/10/2011
 *      Author: boris
 */

#ifndef LOCATION_MANAGER_H_
#define LOCATION_MANAGER_H_

#include <time.h>
#include <vector>
using namespace std;

#include "location.h"
#include "config_file.h"
#include "location_provider.h"
#include "mutex.h"

class LocationManager {
protected:
	static LocationManager *instance;
	const static char *module;
	Location current_location;
	Location locations[3];
	vector<LocationProvider *> providers;
	volatile double distance_tolerance;       // meters
	volatile time_t time_tolerance;			  // seconds
	volatile bool send_updates;
	double maxctd; // max cross-track distance for route minimization
	Mutex mutex;
	double stop_speed;
	double drive_speed;

	void populate_providers(void);
	void destroy_providers(void);

	void start_locating(void);
	void stop_locating(void);

	bool is_better(Location l);
	bool in_route(Location l);
public:
	static LocationManager *get_instance(void) {
		if (instance == NULL) instance = new LocationManager();
		return instance;
	}

	static void destroy_instance(void) {
		if (instance != NULL) delete instance;
		instance = NULL;
	}

	~LocationManager() { destroy_providers(); };
	LocationManager();

	Location get_location(void) {return current_location;};

	void update_location(Location l);
	static void update_location_cb(Location l) { LocationManager::get_instance()->update_location(l); }

	void set_updates_switch(bool s) { send_updates = s; };
	bool get_updates_switch(void) { return send_updates; };

	void set_distance_tolerance(double t) { distance_tolerance = t; };
	double get_distance_tolerance(void) { return distance_tolerance; };

	void set_time_tolerance(time_t t) { time_tolerance = t; };
	time_t get_time_tolerance(void) { return time_tolerance; };

	int get_num_satellites(void);
};


#endif /* LOCATION_MANAGER_H_ */
