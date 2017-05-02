/*
 * serialport.h
 *
 *  Created on: 15/04/2012
 *      Author: boris
 */
#ifndef PLATFORM_H_
#define PLATFORM_H_

using namespace std;

#include <time.h>

class Platform {

public:
	Platform(void) {};
	~Platform() {};

	virtual void startup(void) {};
	virtual void start_gps(void) {};
	virtual void stop_gps(void) {};
	virtual void start_gsm(void) {};
	virtual void stop_gsm(void) {};
	virtual void on_gps_data(bool has_fix) {};
	virtual void set_time(time_t time) {};

	static Platform *get_instance(void) {
		if (instance == NULL) instance = init_platform();
		return instance;
	}

	static void destroy_instance(void) {
		if (instance != NULL) delete instance;
		instance = NULL;
	}

protected:
	static Platform *instance;
	static const char *module;

	static Platform *init_platform();

};

#endif
