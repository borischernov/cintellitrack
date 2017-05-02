/*
 * ezdog_platform.h
 *
 *  Created on: 15/04/2012
 *      Author: boris
 */

#ifndef EZDOG_PLATFORM_H_
#define EZDOG_PLATFORM_H_

#include <time.h>
#include "platform.h"

class EzdogPlatform : public Platform {
public:
	EzdogPlatform(void) {};
	~EzdogPlatform() {};

	void startup(void);
	void start_gps(void);
	void stop_gps(void);
	void start_gsm(void);
	void stop_gsm(void);
	void on_gps_data(bool has_fix);
	void set_time(time_t tme);

private:
	static const char *module;
	bool gpio_out(int num, int val);
	bool gpio_set_in(int num, int val);
	int gpio_in(int num);

	// GPIO Pins
	const static int GPS_ONOFF 	= 51;		// 0 - on
	const static int GPS_PWRMON = 92;		// 1 - running
	const static int GSM_ONOFF 	= 74;		// 1 - on
	const static int LED_ORANGE	= 12;		// 0 - on

};

#endif /* EZDOG_PLATFORM_H_ */
