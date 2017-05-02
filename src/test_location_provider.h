/*
 * test_location_provider.h
 *
 *  Created on: 06/10/2011
 *      Author: boris
 */

#ifndef TEST_LOCATION_PROVIDER_H_
#define TEST_LOCATION_PROVIDER_H_

#include <stdio.h>

#include "location_provider.h"
#include "serialport.h"
#include "config_file.h"

class TestLocationProvider : public LocationProvider {
private:
	static const char *module;
	Location loc;
	FILE *location_log;
public:
	TestLocationProvider(ConfigFile::HashType config);
	~TestLocationProvider();
	Location get_location(void);
};

#endif /* TEST_LOCATION_PROVIDER_H_ */
