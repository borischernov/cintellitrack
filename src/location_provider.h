/*
 * location_provider.h
 *
 *	LocationProvider  -  base class for all location providers
 *
 *  Created on: 06/10/2011
 *      Author: boris
 */

#ifndef LOCATION_PROVIDER_H_
#define LOCATION_PROVIDER_H_

#include <pthread.h>

#include "location.h"
#include "config_file.h"
#include "threaded_class.h"

class LocationProvider : public threaded_class {
public:
	LocationProvider(ConfigFile::HashType config) {};
	~LocationProvider() { stop_listening(); };
	virtual Location get_location(void) = 0;
	virtual int get_num_satellites(void) { return -1; }

	void listen_for_updates(void (*cbk)(Location), unsigned int interval = 0, unsigned int distance = 0);
	void stop_listening() { this->stop_thread(); };

protected:
	Location current_location;
	void (*callback)(Location);
	unsigned int update_interval;
	unsigned int update_distance;
	virtual void thread_body(void);
};


#endif /* LOCATION_PROVIDER_H_ */
