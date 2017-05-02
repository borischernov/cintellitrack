/*
 * location_provider.cpp
 *
 *	LocationProvider  -  base class for all location providers
 *
 *  Created on: 08/10/2011
 *      Author: boris
 */

#include <unistd.h>
#include <stdio.h>

#include "location_provider.h"

void LocationProvider::listen_for_updates(void (*cbk)(Location loc), unsigned int interval, unsigned int distance) {
	callback = cbk;
	update_interval = interval;
	update_distance = distance;
	start_thread();
}

void LocationProvider::thread_body(void) {
	Location new_location;
	while (!m_stoprequested) {
		new_location = get_location();
		if (new_location.has_fix) {
			if (!update_distance || !current_location.has_fix || current_location.distance_to(new_location) > (double)update_distance) {
				current_location = new_location;
				callback(current_location);
			}
		}
		if (update_interval) sleep(update_interval);
	}
}
